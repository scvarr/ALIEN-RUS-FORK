#include "DocumentationReader.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>

#include "MarkdownParser.h"
#include "Resources.h"

namespace
{
    auto constexpr ContentsFilename = "contents.md";

    std::string readFile(std::filesystem::path const& path)
    {
        std::ifstream stream(path);
        return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
    }
}

std::vector<DocumentationChapter> DocumentationReader::getChapters()
{
    return parseChapters(readFile(Const::DocsPath / ContentsFilename));
}

std::vector<DocumentationChapter> DocumentationReader::parseChapters(std::string const& contentsText)
{
    // The table of contents lists the chapters as links, grouped under part headings
    std::vector<DocumentationChapter> result;
    std::string part;
    for (auto const& block : MarkdownParser::parse(contentsText).blocks) {
        if (auto heading = std::get_if<MarkdownHeading>(&block)) {
            part = MarkdownParser::toPlainText(heading->spans);
            continue;
        }
        auto listItem = std::get_if<MarkdownListItem>(&block);
        if (!listItem) {
            continue;
        }
        auto linkSpan = std::ranges::find_if(listItem->spans, [](auto const& span) { return span.link.has_value(); });
        if (linkSpan == listItem->spans.end()) {
            continue;
        }
        result.emplace_back(DocumentationChapter{.filename = *linkSpan->link, .title = linkSpan->text, .part = part});
    }
    return result;
}

std::string DocumentationReader::readChapter(DocumentationChapter const& chapter)
{
    return readFile(Const::DocsPath / chapter.filename);
}

namespace
{
    bool isChapterFilename(std::string const& filename)
    {
        auto path = std::filesystem::path(filename);
        return !filename.empty() && path == path.filename() && path.extension() == ".md" && filename.find_first_of("\\/:#?") == std::string::npos;
    }

    std::string readDocument(std::filesystem::path const& path)
    {
        std::error_code error;
        if (!std::filesystem::is_regular_file(path, error)) {
            return {};
        }
        std::ifstream stream(path);
        if (!stream) {
            return {};
        }
        std::string text{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        auto visible = text.find_first_not_of(" \t\r\n");
        while (visible != std::string::npos && text.compare(visible, 4, "<!--") == 0) {
            auto end = text.find("-->", visible + 4);
            visible = end == std::string::npos ? std::string::npos : text.find_first_not_of(" \t\r\n", end + 3);
        }
        if (stream.bad() || visible == std::string::npos || MarkdownParser::parse(text).blocks.empty()) {
            return {};
        }
        return text;
    }
}

DocumentationContent DocumentationReader::loadChapter(std::string const& filename, DocumentationLanguage language, std::filesystem::path const& englishRoot)
{
    if (!isChapterFilename(filename)) {
        return {};
    }
    if (language == DocumentationLanguage::Russian) {
        auto root = englishRoot / "ru";
        auto text = readDocument(root / filename);
        if (!text.empty()) {
            return {.text = std::move(text), .basePath = root};
        }
    }
    return {.text = readDocument(englishRoot / filename), .basePath = englishRoot, .englishFallback = language == DocumentationLanguage::Russian};
}

std::vector<DocumentationChapter> DocumentationReader::getGuiChapters(DocumentationLanguage language, std::filesystem::path const& englishRoot)
{
    auto chapters = parseChapters(readDocument(englishRoot / ContentsFilename));
    std::erase_if(chapters, [](auto const& chapter) { return !isChapterFilename(chapter.filename); });
    if ((!chapters.empty() || !readDocument(englishRoot / "format-reference.md").empty())
        && std::ranges::find(chapters, "format-reference.md", &DocumentationChapter::filename) == chapters.end()) {
        chapters.emplace_back(DocumentationChapter{.filename = "format-reference.md", .title = "Markdown format reference", .part = "Reference"});
    }
    if (language == DocumentationLanguage::Russian) {
        auto translated = parseChapters(readDocument(englishRoot / "ru" / ContentsFilename));
        std::map<std::string, std::string> translatedParts;
        for (auto const& chapter : chapters) {
            auto entry = std::ranges::find(translated, chapter.filename, &DocumentationChapter::filename);
            if (entry != translated.end() && !entry->part.empty()) {
                translatedParts.try_emplace(chapter.part, entry->part);
            }
        }
        for (auto& chapter : chapters) {
            if (auto part = translatedParts.find(chapter.part); part != translatedParts.end()) {
                chapter.part = part->second;
            }
            auto entry = std::ranges::find(translated, chapter.filename, &DocumentationChapter::filename);
            auto content = loadChapter(chapter.filename, language, englishRoot);
            if (content.englishFallback || content.text.empty()) {
                continue;
            }
            if (entry != translated.end() && !entry->title.empty()) {
                chapter.title = entry->title;
            } else {
                for (auto const& block : MarkdownParser::parse(content.text).blocks) {
                    if (auto heading = std::get_if<MarkdownHeading>(&block); heading && heading->level == 1) {
                        chapter.title = MarkdownParser::toPlainText(heading->spans);
                        break;
                    }
                }
            }
        }
    }
    return chapters;
}
