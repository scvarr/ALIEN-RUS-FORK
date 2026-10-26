#include "DocumentationWindow.h"

#include <algorithm>
#include <iterator>
#include <ranges>

#include <imgui.h>
#include <imgui_internal.h>

#include <Fonts/IconsFontAwesome5.h>

#include <Base/DocumentationReader.h>
#include <Base/GlobalSettings.h>
#include <Base/MarkdownParser.h>
#include <Base/Resources.h>
#include <Base/WebLinkHelper.h>

#include "AlienGui.h"
#include "LocalizationGuiText.h"
#include "StyleService.h"
#include "Utf8Search.h"
#include "WindowController.h"

namespace
{
    constexpr float NavigationWidth = 220.0f;
    constexpr float MinNavigationWidth = 80.0f;
    constexpr float MinContentWidth = 200.0f;
    constexpr float BottomSpace = 50.0f;
    constexpr float PartSpacing = 10.0f;
    constexpr float SectionIndent = 12.0f;
    constexpr size_t SnippetLeadLength = 30;
    constexpr size_t SnippetLength = 100;

    bool selectable(std::string const& name, std::string const& label, bool selected = false)
    {
        auto pos = ImGui::GetCursorScreenPos();
        ImGui::PushOverrideID(ImGui::GetID(name.c_str()));
        auto pressed = ImGui::Selectable("", selected);
        ImGui::PopID();
        ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_Text), label.c_str());
        return pressed;
    }
}

DocumentationWindow::DocumentationWindow()
    : AlienWindow("Documentation", "windows.documentation", true, true, {420.0f, 120.0f}, {1040.0f, 720.0f})
{
    setTitleKey("documentation.title");
}

void DocumentationWindow::initIntern()
{
    _showAfterStartup = _on;
    _navigationWidth =
        GlobalSettings::get().getValue("windows.documentation.navigation width", scale(NavigationWidth)) * WindowController::get().getContentScaleCorrection();
    _language = LocalizationService::get().getLanguage();
    loadChapters();

    auto chapterFilename = GlobalSettings::get().getValue("windows.documentation.chapter", std::string());
    auto findResult = std::ranges::find(_chapters, chapterFilename, &Chapter::filename);
    if (findResult != _chapters.end()) {
        _currentChapterIndex = std::distance(_chapters.begin(), findResult);
    }
}

void DocumentationWindow::shutdownIntern()
{
    _on = _showAfterStartup;
    GlobalSettings::get().setValue("windows.documentation.navigation width", _navigationWidth);
    if (!_chapters.empty()) {
        GlobalSettings::get().setValue("windows.documentation.chapter", _chapters.at(_currentChapterIndex).filename);
    }
    _renderer.releaseTextures();
}

void DocumentationWindow::processIntern()
{
    if (ImGui::BeginChild("##documentation", {0, ImGui::GetContentRegionAvail().y - scale(BottomSpace)}, 0, ImGuiWindowFlags_NoScrollbar)) {
        // The stored width only changes by dragging, so that a temporarily narrow window does not shrink it permanently
        auto maxNavigationWidth = std::max(scale(MinNavigationWidth), ImGui::GetContentRegionAvail().x - scale(MinContentWidth));
        auto navigationWidth = std::clamp(_navigationWidth, scale(MinNavigationWidth), maxNavigationWidth);
        processNavigation(navigationWidth);

        ImGui::SameLine();
        auto draggedNavigationWidth = navigationWidth;
        AlienGui::MovableVerticalSeparator(AlienGui::MovableVerticalSeparatorParameters(), draggedNavigationWidth);
        if (draggedNavigationWidth != navigationWidth) {
            _navigationWidth = draggedNavigationWidth;
        }

        ImGui::SameLine();
        processContent();
    }
    ImGui::EndChild();

    AlienGui::Separator();
    AlienGui::ToggleButton(AlienGui::ToggleButtonParameters().name("Show after startup").label(tr("documentation.showafterstartup")), _showAfterStartup);
}

void DocumentationWindow::processBackground()
{
    reloadLanguage();
    if (!isShown()) {
        _renderer.releaseTextures();
    }
}

namespace
{
    void appendText(std::string& target, std::string const& text)
    {
        if (text.empty()) {
            return;
        }
        if (!target.empty()) {
            target += ' ';
        }
        auto start = target.size();
        target += text;
        std::ranges::replace(target.begin() + start, target.end(), '\n', ' ');
    }

    std::string getPlainText(MarkdownBlock const& block)
    {
        if (auto heading = std::get_if<MarkdownHeading>(&block)) {
            return MarkdownParser::toPlainText(heading->spans);
        }
        if (auto paragraph = std::get_if<MarkdownParagraph>(&block)) {
            return MarkdownParser::toPlainText(paragraph->spans);
        }
        if (auto listItem = std::get_if<MarkdownListItem>(&block)) {
            return MarkdownParser::toPlainText(listItem->spans);
        }
        if (auto image = std::get_if<MarkdownImage>(&block)) {
            return image->caption;
        }
        if (auto codeBlock = std::get_if<MarkdownCodeBlock>(&block)) {
            return codeBlock->text;
        }
        std::string result;
        if (auto note = std::get_if<MarkdownNote>(&block)) {
            for (auto const& noteBlock : note->blocks) {
                appendText(result, std::visit([](auto const& value) { return getPlainText(MarkdownBlock(value)); }, noteBlock));
            }
        }
        if (auto table = std::get_if<MarkdownTable>(&block)) {
            for (auto const& cell : table->header) {
                appendText(result, MarkdownParser::toPlainText(cell));
            }
            for (auto const& cell : table->rows | std::views::join) {
                appendText(result, MarkdownParser::toPlainText(cell));
            }
        }
        return result;
    }
}

void DocumentationWindow::loadChapters()
{
    _renderer.reset();
    _searchResults.clear();
    _linkError.reset();
    _chapters.clear();
    auto language = _language == "ru" ? DocumentationLanguage::Russian : DocumentationLanguage::English;
    auto canonicalChapters = DocumentationReader::getGuiChapters(DocumentationLanguage::English, _docsPath);
    for (auto const& contentsChapter : DocumentationReader::getGuiChapters(language, _docsPath)) {
        Chapter chapter{.filename = contentsChapter.filename, .title = contentsChapter.title, .part = contentsChapter.part};
        chapter.id = std::ranges::find(canonicalChapters, chapter.filename, &DocumentationChapter::filename)->title;
        auto content = DocumentationReader::loadChapter(contentsChapter.filename, language, _docsPath);
        chapter.document = MarkdownParser::parse(content.text);
        chapter.basePath = content.basePath;
        chapter.englishFallback = content.englishFallback;
        auto english = MarkdownParser::parse(DocumentationReader::loadChapter(contentsChapter.filename, DocumentationLanguage::English, _docsPath).text);
        auto headings = [](MarkdownDocument const& document) {
            std::vector<MarkdownHeading const*> result;
            for (auto const& block : document.blocks) {
                if (auto heading = std::get_if<MarkdownHeading>(&block)) {
                    result.emplace_back(heading);
                }
            }
            return result;
        };
        auto sourceHeadings = headings(english);
        auto targetHeadings = headings(chapter.document);
        // English fragments remain stable aliases only for an unchanged heading structure.
        if (sourceHeadings.size() == targetHeadings.size()
            && std::ranges::equal(sourceHeadings, targetHeadings, [](auto source, auto target) { return source->level == target->level; })) {
            for (size_t i = 0; i < sourceHeadings.size(); ++i) {
                chapter.anchorAliases.emplace(sourceHeadings.at(i)->anchor, targetHeadings.at(i)->anchor);
            }
        }
        chapter.sections.emplace_back(Section{.title = chapter.title, .id = chapter.id});
        for (auto const& block : chapter.document.blocks) {
            auto heading = std::get_if<MarkdownHeading>(&block);
            if (heading && heading->level == 1) {
                continue;
            }
            if (heading && heading->level == 2) {
                auto title = MarkdownParser::toPlainText(heading->spans);
                auto id = title;
                for (auto source : sourceHeadings) {
                    auto alias = chapter.anchorAliases.find(source->anchor);
                    if (alias != chapter.anchorAliases.end() && alias->second == heading->anchor) {
                        id = MarkdownParser::toPlainText(source->spans);
                        break;
                    }
                }
                chapter.sections.emplace_back(Section{.title = title, .id = id, .anchor = heading->anchor});
                continue;
            }
            appendText(chapter.sections.back().text, getPlainText(block));
        }
        _chapters.emplace_back(std::move(chapter));
    }
    _currentChapterIndex = 0;
    updateSearchResults();
}

namespace
{
    std::vector<std::string> splitIntoTerms(std::string const& text)
    {
        std::vector<std::string> result;
        for (auto const& term : text | std::views::split(' ')) {
            if (!term.empty()) {
                result.emplace_back(term.begin(), term.end());
            }
        }
        return result;
    }

    std::string createSnippet(std::string const& text, std::vector<std::string> const& terms)
    {
        auto characters = Utf8Search::decode(text);
        if (!characters || characters->empty()) {
            return {};
        }
        auto matchPos = text.size();
        for (auto const& term : terms) {
            if (auto match = Utf8Search::find(text, term)) {
                matchPos = std::min(matchPos, match->begin);
            }
        }
        if (matchPos == text.size()) {
            matchPos = 0;
        }
        auto match = std::ranges::lower_bound(*characters, matchPos, {}, &Utf8Search::Character::begin);
        auto index = static_cast<size_t>(std::distance(characters->begin(), match));
        auto startIndex = index > SnippetLeadLength ? index - SnippetLeadLength : 0;
        auto endIndex = std::min(characters->size(), startIndex + SnippetLength);
        auto start = characters->at(startIndex).begin;
        auto end = characters->at(endIndex - 1).end;
        return (start > 0 ? "..." : "") + text.substr(start, end - start) + (end < text.size() ? "..." : "");
    }
}

void DocumentationWindow::updateSearchResults()
{
    _searchResults.clear();

    auto terms = splitIntoTerms(_searchText);
    if (terms.empty()) {
        return;
    }

    // Sections whose title contains all terms are listed first
    std::vector<SearchResult> textMatches;
    for (auto const& [chapterIndex, chapter] : std::views::enumerate(_chapters)) {
        for (auto const& [sectionIndex, section] : std::views::enumerate(chapter.sections)) {
            if (!std::ranges::all_of(terms, [&](auto const& term) {
                    return Utf8Search::find(section.title, term).has_value() || Utf8Search::find(section.text, term).has_value();
                })) {
                continue;
            }
            SearchResult result{
                .chapterIndex = static_cast<size_t>(chapterIndex),
                .sectionIndex = static_cast<size_t>(sectionIndex),
                .snippet = createSnippet(section.text, terms)};
            if (std::ranges::all_of(terms, [&](auto const& term) { return Utf8Search::find(section.title, term).has_value(); })) {
                _searchResults.emplace_back(std::move(result));
            } else {
                textMatches.emplace_back(std::move(result));
            }
        }
    }
    _searchResults.insert(_searchResults.end(), textMatches.begin(), textMatches.end());
}

void DocumentationWindow::processNavigation(float width)
{
    if (ImGui::BeginChild("##navigation", {width, 0})) {
        if (AlienGui::InputFilter(AlienGui::InputFilterParameters().id("Search").hint(tr("documentation.search")), _searchText)) {
            updateSearchResults();
        }
        if (ImGui::BeginChild("##navigationList", {0, 0})) {
            if (_searchText.empty()) {
                processTableOfContents();
            } else {
                processSearchResults();
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
}

void DocumentationWindow::processTableOfContents()
{
    std::optional<std::string> currentPart;
    for (auto const& [chapterIndex, chapter] : std::views::enumerate(_chapters)) {
        ImGui::PushID(static_cast<int>(chapterIndex));

        if (chapter.part != currentPart) {
            if (currentPart.has_value()) {
                ImGui::Dummy({0.0f, scale(PartSpacing)});
            }
            currentPart = chapter.part;
            ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDimColor.Value);
            ImGui::PushFont(StyleService::get().getSmallBoldFont());
            ImGui::TextUnformatted(chapter.part.c_str());
            ImGui::PopFont();
            ImGui::PopStyleColor();
        }

        auto isCurrentChapter = static_cast<size_t>(chapterIndex) == _currentChapterIndex;
        ImGui::PushFont(isCurrentChapter ? StyleService::get().getSmallBoldFont() : StyleService::get().getDefaultFont());
        if (selectable(chapter.id, chapter.title, isCurrentChapter)) {
            openChapter(chapterIndex);
        }
        ImGui::PopFont();

        if (isCurrentChapter) {
            ImGui::Indent(scale(SectionIndent));
            ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDimColor.Value);
            for (auto const& [sectionIndex, section] : chapter.sections | std::views::enumerate | std::views::drop(1)) {
                ImGui::PushID(static_cast<int>(sectionIndex));
                if (selectable(section.id, section.title)) {
                    openChapter(chapterIndex, section.anchor);
                }
                ImGui::PopID();
            }
            ImGui::PopStyleColor();
            ImGui::Unindent(scale(SectionIndent));
        }
        ImGui::PopID();
    }
}

void DocumentationWindow::processSearchResults()
{
    if (_searchResults.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDimColor.Value);
        ImGui::TextUnformatted(tr("documentation.nomatches").c_str());
        ImGui::PopStyleColor();
        return;
    }
    for (auto const& [index, result] : std::views::enumerate(_searchResults)) {
        ImGui::PushID(static_cast<int>(index));
        auto const& chapter = _chapters.at(result.chapterIndex);
        auto const& section = chapter.sections.at(result.sectionIndex);

        ImGui::PushFont(StyleService::get().getSmallBoldFont());
        if (selectable(section.id, section.title)) {
            openChapter(result.chapterIndex, section.anchor.empty() ? std::nullopt : std::make_optional(section.anchor));
        }
        ImGui::PopFont();

        ImGui::PushTextWrapPos(0.0f);
        if (result.sectionIndex > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, Const::HeadlineColor.Value);
            ImGui::TextUnformatted(chapter.title.c_str());
            ImGui::PopStyleColor();
        }
        ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDimColor.Value);
        ImGui::TextUnformatted(result.snippet.c_str());
        ImGui::PopStyleColor();
        ImGui::PopTextWrapPos();

        ImGui::Spacing();
        ImGui::PopID();
    }
}

void DocumentationWindow::processContent()
{
    if (ImGui::BeginChild("##content", {0, 0}, ImGuiChildFlags_AlwaysUseWindowPadding)) {
        if (_scrollToTop) {
            ImGui::SetScrollY(0.0f);
            _scrollToTop = false;
        }
        if (_chapters.empty()) {
            AlienGui::Text(LocalizationGuiText::format("documentation.missing", {{"path", _docsPath.string()}}));
        } else {
            auto const& chapter = _chapters.at(_currentChapterIndex);
            if (chapter.englishFallback && !chapter.document.blocks.empty()) {
                AlienGui::Text(tr("documentation.englishfallback"));
            }
            if (_linkError) {
                AlienGui::Text(LocalizationGuiText::format("documentation.invalidlink", {{"link", *_linkError}}));
            }
            if (chapter.document.blocks.empty()) {
                AlienGui::Text(LocalizationGuiText::format("documentation.chaptermissing", {{"chapter", chapter.filename}}));
            }
            auto clickedLink = _renderer.render(chapter.document, chapter.basePath);
            processChapterButtons();
            if (clickedLink.has_value()) {
                openLink(*clickedLink);
            }
        }
    }
    ImGui::EndChild();
}

void DocumentationWindow::processChapterButtons()
{
    auto chapterIndex = _currentChapterIndex;
    auto hasPreviousChapter = chapterIndex > 0;
    auto hasNextChapter = chapterIndex + 1 < _chapters.size();
    if (!hasPreviousChapter && !hasNextChapter) {
        return;
    }

    AlienGui::Separator();
    if (hasPreviousChapter) {
        auto const& previous = _chapters.at(chapterIndex - 1);
        if (AlienGui::DisplayButton(ICON_FA_CHEVRON_LEFT "  " + previous.id, ICON_FA_CHEVRON_LEFT "  " + previous.title)) {
            openChapter(chapterIndex - 1);
        }
    }
    if (hasNextChapter) {
        auto text = _chapters.at(chapterIndex + 1).title + "  " ICON_FA_CHEVRON_RIGHT;
        auto buttonWidth = ImGui::CalcTextSize(text.c_str()).x + ImGui::GetStyle().FramePadding.x * 2;
        auto rightEdge = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
        if (hasPreviousChapter && ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x + buttonWidth <= rightEdge) {
            ImGui::SameLine();
        }
        ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonWidth));
        if (AlienGui::DisplayButton(_chapters.at(chapterIndex + 1).id + "  " ICON_FA_CHEVRON_RIGHT, text)) {
            openChapter(chapterIndex + 1);
        }
    }
    ImGui::Dummy({0.0f, ImGui::GetStyle().ItemSpacing.y});
}

void DocumentationWindow::openLink(std::string const& link)
{
    if (link.starts_with("http://") || link.starts_with("https://")) {
        WebLinkHelper::openInBrowser(link);
        return;
    }
    auto separatorPos = link.find('#');
    auto filename = link.substr(0, separatorPos);
    auto anchor = separatorPos != std::string::npos ? std::make_optional(link.substr(separatorPos + 1)) : std::nullopt;

    if (filename.empty() && !_chapters.empty()) {
        openChapter(_currentChapterIndex, anchor);
        return;
    }
    auto findResult = std::ranges::find(_chapters, filename, &Chapter::filename);
    if (findResult != _chapters.end()) {
        openChapter(std::distance(_chapters.begin(), findResult), anchor);
    } else {
        _linkError = link;
    }
}

void DocumentationWindow::openChapter(size_t chapterIndex, std::optional<std::string> const& anchor)
{
    if (chapterIndex >= _chapters.size()) {
        return;
    }
    _linkError.reset();
    auto const& chapter = _chapters.at(chapterIndex);
    auto resolvedAnchor = anchor;
    if (resolvedAnchor && !resolvedAnchor->empty()) {
        auto found = std::ranges::any_of(chapter.document.blocks, [&](auto const& block) {
            auto heading = std::get_if<MarkdownHeading>(&block);
            return heading && heading->anchor == *resolvedAnchor;
        });
        if (!found) {
            auto alias = chapter.anchorAliases.find(*resolvedAnchor);
            if (alias != chapter.anchorAliases.end()) {
                resolvedAnchor = alias->second;
            } else {
                _linkError = chapter.filename + "#" + *resolvedAnchor;
                resolvedAnchor.reset();
            }
        }
    }
    if (chapterIndex != _currentChapterIndex || !resolvedAnchor || resolvedAnchor->empty()) {
        _scrollToTop = true;
    }
    _currentChapterIndex = chapterIndex;
    _renderer.scrollToAnchor(resolvedAnchor.value_or(""));
}

void DocumentationWindow::reloadLanguage()
{
    auto language = LocalizationService::get().getLanguage();
    if (_language == language) {
        return;
    }
    std::string filename;
    auto anchor = _renderer.getCurrentAnchor();
    if (!_chapters.empty()) {
        auto const& chapter = _chapters.at(_currentChapterIndex);
        filename = chapter.filename;
        if (anchor) {
            for (auto const& [source, target] : chapter.anchorAliases) {
                if (target == *anchor) {
                    anchor = source;
                    break;
                }
            }
        }
    }
    _language = language;
    loadChapters();
    auto chapter = std::ranges::find(_chapters, filename, &Chapter::filename);
    if (chapter != _chapters.end()) {
        openChapter(std::distance(_chapters.begin(), chapter), anchor);
        _linkError.reset();
    }
}
