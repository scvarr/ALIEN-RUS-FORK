#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>

#include <boost/json.hpp>
#include <imgui_internal.h>
#include <Fonts/IconsFontAwesome5.h>

#include <Base/DocumentationReader.h>
#include <Base/LocalizationService.h>
#include <Base/MarkdownParser.h>
#include <McpTools/Impl/McpDocumentationTools.h>
#include <Rendering/Impl/RenderingFacadeImpl.h>
#include <Gui/Impl/DocumentationWindow.h>
#include <Gui/Impl/Utf8Search.h>
#include <Base/Tests/DocumentationTestTree.h>
#include <Base/Tests/SettingsTestScope.h>

#include "DocumentationBaseline.h"

class DocumentationLocalizationTests : public ::testing::Test
{
protected:
    class Renderer : public _RenderingFacadeImpl
    {
    public:
        TextureData loadTextureFromMemory(std::string const& encoded) override
        {
            EXPECT_FALSE(encoded.empty());
            ++loads;
            return {static_cast<ImTextureID>(loads), 1, 1};
        }
        void deleteTexture(TextureData const& texture) override { ++deletes; }
        int loads = 0;
        int deletes = 0;
    };

    void SetUp() override
    {
        originalRendering = _RenderingFacade::get();
        rendering = std::make_shared<Renderer>();
        _RenderingFacadeImpl::set(rendering);
        originalScale = WindowController::get().getContentScaleFactor();
        originalLanguage = LocalizationService::get().getLanguage();
        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().DisplaySize = {4000, 3000};
        StyleService::get().setup();
        LocalizationService::get().load("resources/locales");
        select("en");
        window._docsPath = tree.root;
        window.initIntern();
    }

    void TearDown() override
    {
        window._renderer.reset();
        _RenderingFacadeImpl::set(originalRendering);
        WindowController::get()._contentScaleFactor = originalScale;
        select(originalLanguage);
        ImGui::DestroyContext();
    }

    void select(std::string const& language)
    {
        LocalizationService::get().requestLanguage(language);
        LocalizationService::get().applyPendingLanguage();
    }

    void frame(std::function<void()> const& draw)
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos({30, 40}, ImGuiCond_Once);
        ImGui::SetNextWindowSize({1040 * factor, 720 * factor}, ImGuiCond_Once);
        ImGui::Begin("Documentation test host");
        draw();
        ImGui::End();
        ImGui::Render();
        EXPECT_EQ(0, GImGui->ErrorCountCurrentFrame);
    }

    void search(std::string const& query)
    {
        window._searchText = query;
        window.updateSearchResults();
    }

    void checkSwitchAndSettings()
    {
        tree.translate();
        GlobalSettings::get().setValue("windows.documentation.chapter", std::string("beta.md"));
        GlobalSettings::get().setValue("windows.documentation.navigation width", 275.0f);
        window.initIntern();
        auto navigationWidth = window._navigationWidth;
        window._showAfterStartup = false;
        window.setOn(true);
        window.openLink("alpha.md#energy-1");
        frame([&] { window.processIntern(); });
        // Simulate the renderer's last visible heading after scrolling.
        window._renderer._currentAnchor = "energy-1";
        search("ЁЖ");
        EXPECT_TRUE(window._searchResults.empty());
        for (auto const& language : {"ru", "en", "ru", "en"}) {
            select(language);
            window.processBackground();
            EXPECT_EQ("alpha.md", window._chapters.at(window._currentChapterIndex).filename);
            EXPECT_EQ("ЁЖ", window._searchText);
            EXPECT_EQ(language == std::string("ru") ? "Энергия-1" : "energy-1", window._renderer._pendingAnchor);
            EXPECT_EQ(language == std::string("en"), window._searchResults.empty());
            EXPECT_TRUE(window.isOn());
            EXPECT_FALSE(window._showAfterStartup);
            EXPECT_FLOAT_EQ(navigationWidth, window._navigationWidth);
            EXPECT_EQ(nullptr, window._renderer._renderedDocument);
            EXPECT_TRUE(window._renderer._imageInfoByPath.empty());
            frame([&] { window.processIntern(); });
            window._renderer._currentAnchor = language == std::string("ru") ? "Энергия-1" : "energy-1";
        }
        window.shutdownIntern();
        EXPECT_EQ("alpha.md", GlobalSettings::get().getValue("windows.documentation.chapter", std::string()));
        EXPECT_FLOAT_EQ(navigationWidth, GlobalSettings::get().getValue("windows.documentation.navigation width", 0.0f));
        window.initIntern();
        EXPECT_EQ("alpha.md", window._chapters.at(window._currentChapterIndex).filename);
        // Filename restoration must survive an order change rather than reusing the old index.
        tree.write("contents.md", "# Start\n\n- [Beta](beta.md)\n- [Alpha](alpha.md)\n");
        window.initIntern();
        EXPECT_EQ(1, window._currentChapterIndex);
        EXPECT_EQ("alpha.md", window._chapters.at(window._currentChapterIndex).filename);
    }

    void checkLinks()
    {
        tree.translate();
        select("ru");
        window.processBackground();
        for (auto const& link : {"alpha.md#energy-1", "alpha.md#Энергия-1", "#energy-1", "#Энергия-1"}) {
            window.openLink(link);
            EXPECT_EQ(0, window._currentChapterIndex);
            EXPECT_EQ("Энергия-1", window._renderer._pendingAnchor);
            EXPECT_FALSE(window._linkError);
        }
        window.openLink("beta.md#links");
        EXPECT_EQ("Ссылки", window._renderer._pendingAnchor);
        window.openLink("format-reference.md#syntax");
        EXPECT_EQ("format-reference.md", window._chapters.at(window._currentChapterIndex).filename);
        EXPECT_EQ("Синтаксис", window._renderer._pendingAnchor);
        for (auto const& link : {"unknown.md", "../alpha.md", "alpha.md#unknown"}) {
            window.openLink(link);
            EXPECT_TRUE(window._linkError);
        }
        tree.write("ru/alpha.md", "# Альфа\n\n## Изменённая структура\n\nТекст.\n");
        window.loadChapters();
        window.openLink("alpha.md#energy-1");
        EXPECT_TRUE(window._linkError);
        EXPECT_FALSE(window._renderer._pendingAnchor);
        auto document = MarkdownParser::parse("[External](https://example.com/path#fragment) [HTTP](http://example.com)\n");
        auto const& spans = std::get<MarkdownParagraph>(document.blocks.front()).spans;
        EXPECT_EQ("https://example.com/path#fragment", spans.front().link);
        EXPECT_EQ("http://example.com", spans.back().link);
    }

    void checkSearch()
    {
        tree.translate();
        select("ru");
        window.processBackground();
        for (auto const& query : {"ёж", "ЁЖ", "ЭНЕРГИЯ alien", "ёЖ ALIEN"}) {
            search(query);
            ASSERT_FALSE(window._searchResults.empty()) << query;
            EXPECT_TRUE(Utf8Search::decode(window._searchResults.front().snippet));
        }
        search("еж");
        EXPECT_TRUE(window._searchResults.empty());
        search("энергия");
        ASSERT_FALSE(window._searchResults.empty());
        auto const& first = window._searchResults.front();
        EXPECT_TRUE(Utf8Search::find(window._chapters.at(first.chapterIndex).sections.at(first.sectionIndex).title, "энергия"));
        search("энергия отсутствует");
        EXPECT_TRUE(window._searchResults.empty());
        std::string longText;
        for (int i = 0; i < 200; ++i) {
            longText += "я";
        }
        longText += "ЁЖ";
        for (int i = 0; i < 200; ++i) {
            longText += "ю";
        }
        tree.write("ru/alpha.md", "# Альфа\n\n## Раздел\n\n" + longText + "\n");
        window.loadChapters();
        search("ёж");
        ASSERT_EQ(1, window._searchResults.size());
        auto snippet = window._searchResults.front().snippet;
        EXPECT_TRUE(snippet.starts_with("..."));
        EXPECT_TRUE(snippet.ends_with("..."));
        EXPECT_TRUE(snippet.contains("ЁЖ"));
        EXPECT_TRUE(Utf8Search::decode(snippet));
        search(std::string("\x96"));
        EXPECT_TRUE(window._searchResults.empty());
        tree.write("ru/alpha.md", "# Альфа\n\n## Ёж\n\n" + std::string("bad\xff"));
        window.loadChapters();
        search("ёж");
        ASSERT_EQ(1, window._searchResults.size());
        EXPECT_TRUE(window._searchResults.front().snippet.empty());
        search("bad");
        EXPECT_TRUE(window._searchResults.empty());
    }

    void writeImage(std::filesystem::path const& path)
    {
        // A 1x1 RGBA PNG; the facade stub receives the actual file bytes.
        unsigned char const png[] = {137, 80, 78, 71, 13, 10,  26,  10,  0,   0,   0, 13, 73, 72, 68, 82, 0,  0,   0,   1,   0,  0, 0,
                                     1,   8,  6,  0,  0,  0,   31,  21,  196, 137, 0, 0,  0,  11, 73, 68, 65, 84,  120, 156, 99, 0, 2,
                                     0,   0,  5,  0,  1,  233, 250, 220, 216, 0,   0, 0,  0,  73, 69, 78, 68, 174, 66,  96,  130};
        std::ofstream stream(path, std::ios::binary);
        stream.write(reinterpret_cast<char const*>(png), sizeof(png));
    }

    void checkImages()
    {
        tree.translate();
        writeImage(tree.root / "shared.png");
        writeImage(tree.root / "ru/local.png");
        auto unicodeDirectory = tree.root / std::filesystem::path(L"Изображения");
        std::filesystem::create_directory(unicodeDirectory);
        writeImage(unicodeDirectory / "test.png");
        EXPECT_TRUE(window._renderer.getImageInfo(unicodeDirectory / "test.png"));
        tree.write("alpha.md", "# Alpha\n\n![English](shared.png)\n");
        tree.write("ru/alpha.md", "# Альфа\n\n![Общее](../shared.png)\n\n![Своё](local.png)\n\n![Нет](missing.png)\n");
        window.loadChapters();
        auto draw = [&] { window.processIntern(); };
        frame(draw);
        frame(draw);
        EXPECT_EQ(1, rendering->loads);
        frame(draw);
        EXPECT_EQ(1, rendering->loads);
        select("ru");
        window.processBackground();
        EXPECT_EQ(1, rendering->deletes);
        frame(draw);
        frame(draw);
        EXPECT_EQ(3, rendering->loads);
        EXPECT_EQ(tree.root / "ru", window._renderer._basePath);
        EXPECT_TRUE(window._renderer._imageInfoByPath.contains((tree.root / "shared.png").lexically_normal()));
        EXPECT_TRUE(window._renderer._imageInfoByPath.contains((tree.root / "ru/local.png").lexically_normal()));
        EXPECT_FALSE(window._renderer._imageInfoByPath.at(tree.root / "ru/missing.png"));
        window.setOn(false);
        window.processBackground();
        EXPECT_EQ(3, rendering->deletes);
        window.setOn(true);
        frame(draw);
        frame(draw);
        EXPECT_EQ(5, rendering->loads);
        writeImage(tree.root / "ru/missing.png");
        select("en");
        window.processBackground();
        select("ru");
        window.processBackground();
        for (int i = 0; i < 4; ++i) {
            frame(draw);
        }
        EXPECT_TRUE(window._renderer._imageInfoByPath.at(tree.root / "ru/missing.png"));
        window._renderer.reset();
        EXPECT_EQ(rendering->loads, rendering->deletes);
        EXPECT_TRUE(window._renderer._imageInfoByPath.empty());
        EXPECT_EQ(nullptr, window._renderer._renderedDocument);
    }

    void checkScales()
    {
        tree.translate();
        for (float scale : {1.0f, 1.5f, 2.0f}) {
            factor = scale;
            WindowController::get()._contentScaleFactor = factor;
            ImGui::GetIO().Fonts->Clear();
            ImGui::GetIO().FontDefault = nullptr;
            ImGui::GetStyle() = ImGuiStyle();
            StyleService::get().setup();
            window._navigationWidth = 275 * factor;
            ImGuiID navigationId = 0;
            ImGuiWindow* host = nullptr;
            ImVec2 pos;
            ImVec2 size;
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                window.processBackground();
                for (int i = 0; i < 3; ++i) {
                    frame([&] {
                        if (!host) {
                            host = ImGui::GetCurrentWindow();
                            pos = host->Pos;
                            size = host->Size;
                            navigationId = ImGui::GetID("##documentation");
                        }
                        EXPECT_EQ(host, ImGui::GetCurrentWindow());
                        EXPECT_EQ(navigationId, ImGui::GetID("##documentation"));
                        EXPECT_FLOAT_EQ(pos.x, host->Pos.x);
                        EXPECT_FLOAT_EQ(size.x, host->Size.x);
                        window.processIntern();
                        EXPECT_LE(GImGui->LastItemData.Rect.Max.x, host->WorkRect.Max.x + 1);
                    });
                }
                search("not found");
                frame([&] { window.processIntern(); });
                search("");
                window.openLink("missing.md");
                frame([&] { window.processIntern(); });
                window.openLink("format-reference.md");
                frame([&] { window.processIntern(); });
            }
        }
    }

    void checkMcpBaseline()
    {
        auto& tools = McpDocumentationTools::get();
        auto tool = tools.getTools().front();
        EXPECT_EQ("read_documentation", tool.name);
        EXPECT_EQ(DocumentationBaseline::DESCRIPTION, tool.description);
        EXPECT_EQ(1, tool.inputSchema.at("properties").as_object().size());
        EXPECT_TRUE(tool.inputSchema.at("properties").as_object().contains("chapter"));
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            window.processBackground();
            EXPECT_EQ(DocumentationBaseline::CONTENTS, tool.handler({}).text);
            for (auto const& [filename, expected] : DocumentationBaseline::CHAPTER_HASHES) {
                auto result = tool.handler({{"chapter", filename}});
                uint64_t hash = 14695981039346656037ull;
                for (unsigned char c : result.text) {
                    hash = (hash ^ c) * 1099511628211ull;
                }
                EXPECT_EQ(expected, hash) << filename << ": " << language;
                EXPECT_FALSE(result.isError);
                EXPECT_TRUE(result.images.empty());
                EXPECT_EQ(result.text, tool.handler({{"chapter", filename + "#section"}}).text);
            }
            EXPECT_THROW(tool.handler({{"chapter", "format-reference.md"}}), std::invalid_argument);
        }
    }

    void checkNativeWindowState()
    {
        std::string body;
        for (int i = 0; i < 80; ++i) {
            body += "Paragraph / Абзац " + std::to_string(i) + ".\n\n";
        }
        tree.translate();
        tree.write("alpha.md", "# Alpha\n\n## Energy\n\n" + body + "## Energy\n\n" + body);
        tree.write("ru/alpha.md", "# Альфа\n\n## Энергия\n\n" + body + "## Энергия\n\n" + body);
        window.loadChapters();
        window.setOn(true);
        auto draw = [&] {
            ImGui::NewFrame();
            static_cast<MainLoopEntity&>(window).process();
            ImGui::Render();
            EXPECT_EQ(0, GImGui->ErrorCountCurrentFrame);
        };
        draw();
        auto root = ImGui::FindWindowByName("Documentation");
        ASSERT_NE(nullptr, root);
        for (float scale : {1.0f, 1.5f, 2.0f}) {
            WindowController::get()._contentScaleFactor = scale;
            ImGui::GetIO().Fonts->Clear();
            ImGui::GetIO().FontDefault = nullptr;
            ImGui::GetStyle() = ImGuiStyle();
            StyleService::get().setup();
            ImGui::SetWindowPos(root, {100, 120});
            ImGui::SetWindowSize(root, {1040 * scale, 720 * scale});
            window._navigationWidth = 275 * scale;
            bool firstLanguage = true;
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                draw();
                draw();
                draw();
                if (!firstLanguage) {
                    EXPECT_EQ(language == std::string("ru") ? "Энергия-1" : "energy-1", window._renderer.getCurrentAnchor());
                }
                firstLanguage = false;
                window.openLink("alpha.md#energy-1");
                draw();
                draw();
                EXPECT_EQ(language == std::string("ru") ? "Энергия-1" : "energy-1", window._renderer.getCurrentAnchor());
                ImGuiWindow* content = nullptr;
                for (auto candidate : GImGui->Windows) {
                    if (candidate->RootWindow == root && std::string(candidate->Name).contains("##content")) {
                        content = candidate;
                    }
                }
                ASSERT_NE(nullptr, content);
                EXPECT_GT(content->Scroll.y, 0);
                auto scroll = content->Scroll.y;
                draw();
                EXPECT_FLOAT_EQ(scroll, content->Scroll.y);
                EXPECT_EQ(root, ImGui::FindWindowByName("Documentation"));
                EXPECT_FLOAT_EQ(100, root->Pos.x);
                EXPECT_FLOAT_EQ(120, root->Pos.y);
                EXPECT_FLOAT_EQ(1040 * scale, root->Size.x);
                EXPECT_FLOAT_EQ(720 * scale, root->Size.y);
                EXPECT_FLOAT_EQ(275 * scale, window._navigationWidth);
            }
        }
    }

    void checkShippedTree()
    {
        window._docsPath = Const::DocsPath;
        window.loadChapters();
        EXPECT_EQ(DocumentationReader::getChapters().size() + 1, window._chapters.size());
        auto visitSpans = [&](MarkdownSpans const& spans) {
            for (auto const& span : spans) {
                if (!span.link || span.link->starts_with("https://") || span.link->starts_with("http://")) {
                    continue;
                }
                window.openLink(*span.link);
                EXPECT_FALSE(window._linkError) << *span.link;
            }
        };
        for (size_t index = 0; index < window._chapters.size(); ++index) {
            auto const& chapter = window._chapters.at(index);
            EXPECT_FALSE(chapter.document.blocks.empty()) << chapter.filename;
            for (auto const& block : chapter.document.blocks) {
                window.openChapter(index);
                std::visit(
                    [&](auto const& value) {
                        if constexpr (requires { value.spans; }) {
                            visitSpans(value.spans);
                        } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, MarkdownImage>) {
                            EXPECT_TRUE(window._renderer.getImageInfo(chapter.basePath / value.source)) << value.source;
                        } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, MarkdownTable>) {
                            for (auto const& cell : value.header) {
                                visitSpans(cell);
                            }
                            for (auto const& row : value.rows) {
                                for (auto const& cell : row) {
                                    visitSpans(cell);
                                }
                            }
                        } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, MarkdownNote>) {
                            for (auto const& noteBlock : value.blocks) {
                                std::visit(
                                    [&](auto const& noteValue) {
                                        if constexpr (requires { noteValue.spans; }) {
                                            visitSpans(noteValue.spans);
                                        }
                                    },
                                    noteBlock);
                            }
                        }
                    },
                    block);
            }
        }
        window.openLink("format-reference.md");
        EXPECT_FALSE(window._linkError);
        EXPECT_EQ("format-reference.md", window._chapters.at(window._currentChapterIndex).filename);
    }

    void checkShippedRussianPackage()
    {
        window._docsPath = Const::DocsPath;
        auto const translatedFiles = {
            "contents.md", "getting-started.md", "user-interface.md", "keyboard-and-mouse.md", "genomes.md", "neural-networks.md", "first-creature.md"};
        for (auto const& filename : translatedFiles) {
            SCOPED_TRACE(filename);
            auto english = DocumentationReader::loadChapter(filename, DocumentationLanguage::English, window._docsPath);
            auto russian = DocumentationReader::loadChapter(filename, DocumentationLanguage::Russian, window._docsPath);
            ASSERT_FALSE(russian.englishFallback);
            EXPECT_EQ(window._docsPath / "ru", russian.basePath);
            EXPECT_TRUE(Utf8Search::decode(russian.text));
            EXPECT_NE(english.text, russian.text);
            auto source = MarkdownParser::parse(english.text);
            auto target = MarkdownParser::parse(russian.text);
            ASSERT_EQ(source.blocks.size(), target.blocks.size());
            auto links = [](MarkdownSpans const& spans) {
                std::vector<std::string> result;
                for (auto const& span : spans) {
                    if (span.link) {
                        result.emplace_back(*span.link);
                    }
                }
                return result;
            };
            for (size_t i = 0; i < source.blocks.size(); ++i) {
                auto const& sourceBlock = source.blocks.at(i);
                auto const& targetBlock = target.blocks.at(i);
                ASSERT_EQ(sourceBlock.index(), targetBlock.index()) << i;
                std::visit(
                    [&](auto const& value) {
                        using T = std::decay_t<decltype(value)>;
                        auto const& translated = std::get<T>(targetBlock);
                        if constexpr (requires { value.spans; }) {
                            EXPECT_EQ(links(value.spans), links(translated.spans));
                        }
                        if constexpr (std::is_same_v<T, MarkdownHeading>) {
                            EXPECT_EQ(value.level, translated.level);
                            EXPECT_NE(value.anchor, translated.anchor);
                        } else if constexpr (std::is_same_v<T, MarkdownListItem>) {
                            EXPECT_EQ(value.depth, translated.depth);
                            EXPECT_EQ(value.number, translated.number);
                        } else if constexpr (std::is_same_v<T, MarkdownCodeBlock>) {
                            EXPECT_EQ(value.text, translated.text);
                        } else if constexpr (std::is_same_v<T, MarkdownImage>) {
                            EXPECT_EQ("../" + value.source, translated.source);
                            EXPECT_NE(value.caption, translated.caption);
                            EXPECT_TRUE(window._renderer.getImageInfo(russian.basePath / translated.source));
                        } else if constexpr (std::is_same_v<T, MarkdownTable>) {
                            ASSERT_EQ(value.header.size(), translated.header.size());
                            EXPECT_EQ(value.alignments, translated.alignments);
                            ASSERT_EQ(value.rows.size(), translated.rows.size());
                            for (size_t column = 0; column < value.header.size(); ++column) {
                                EXPECT_EQ(links(value.header.at(column)), links(translated.header.at(column)));
                            }
                            for (size_t row = 0; row < value.rows.size(); ++row) {
                                ASSERT_EQ(value.rows.at(row).size(), translated.rows.at(row).size());
                                for (size_t column = 0; column < value.rows.at(row).size(); ++column) {
                                    EXPECT_EQ(links(value.rows.at(row).at(column)), links(translated.rows.at(row).at(column)));
                                }
                            }
                        } else if constexpr (std::is_same_v<T, MarkdownNote>) {
                            ASSERT_EQ(value.blocks.size(), translated.blocks.size());
                            for (size_t noteIndex = 0; noteIndex < value.blocks.size(); ++noteIndex) {
                                auto const& note = value.blocks.at(noteIndex);
                                auto const& translatedNote = translated.blocks.at(noteIndex);
                                ASSERT_EQ(note.index(), translatedNote.index());
                                std::visit(
                                    [&](auto const& noteValue) {
                                        using N = std::decay_t<decltype(noteValue)>;
                                        if constexpr (requires { noteValue.spans; }) {
                                            EXPECT_EQ(links(noteValue.spans), links(std::get<N>(translatedNote).spans));
                                        }
                                    },
                                    note);
                            }
                        }
                    },
                    sourceBlock);
            }
        }
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            window.processBackground();
            checkShippedTree();
            for (auto const& chapter : window._chapters) {
                auto translated = std::ranges::find(translatedFiles, chapter.filename) != translatedFiles.end();
                EXPECT_EQ(language == std::string("ru") && !translated, chapter.englishFallback) << chapter.filename;
                if (language == std::string("ru")) {
                    EXPECT_TRUE(chapter.part == "Начало работы" || chapter.part == "Изучение возможностей" || chapter.part == "Справочник");
                }
                if (language == std::string("ru") && translated) {
                    EXPECT_NE(chapter.id, chapter.title);
                    EXPECT_TRUE(
                        Utf8Search::find(chapter.part, "начало") || Utf8Search::find(chapter.part, "изучение") || Utf8Search::find(chapter.part, "справочник"));
                }
                auto filename = chapter.filename;
                auto aliases = chapter.anchorAliases;
                for (auto const& [canonical, native] : aliases) {
                    window.openLink(filename + "#" + canonical);
                    EXPECT_FALSE(window._linkError);
                    EXPECT_EQ(native, window._renderer._pendingAnchor);
                    window.openLink(filename + "#" + native);
                    EXPECT_FALSE(window._linkError);
                }
            }
        }
        select("ru");
        window.processBackground();
        for (auto const& pair : {std::pair{"нейросети", "НЕЙРОСЕТИ"}, {"жёсткость", "ЖЁСТКОСТЬ"}, {"соединённых", "СОЕДИНЁННЫХ"}}) {
            search(pair.first);
            ASSERT_FALSE(window._searchResults.empty()) << pair.first;
            auto count = window._searchResults.size();
            search(pair.second);
            EXPECT_EQ(count, window._searchResults.size()) << pair.second;
            for (auto const& result : window._searchResults) {
                EXPECT_TRUE(Utf8Search::decode(result.snippet));
            }
        }
        search("");
    }

    void checkShippedLongChaptersAtScales()
    {
        window._docsPath = Const::DocsPath;
        window.loadChapters();
        window.setOn(true);
        auto draw = [&] {
            ImGui::NewFrame();
            static_cast<MainLoopEntity&>(window).process();
            ImGui::Render();
            EXPECT_EQ(0, GImGui->ErrorCountCurrentFrame);
        };
        draw();
        auto host = ImGui::FindWindowByName("Documentation");
        ASSERT_NE(nullptr, host);
        for (float scale : {1.0f, 1.5f, 2.0f}) {
            WindowController::get()._contentScaleFactor = scale;
            ImGui::GetIO().Fonts->Clear();
            ImGui::GetIO().FontDefault = nullptr;
            ImGui::GetStyle() = ImGuiStyle();
            StyleService::get().setup();
            ImGui::SetWindowSize(host, {1040 * scale, 720 * scale});
            window._navigationWidth = 275 * scale;
            for (auto const& filename : {"user-interface.md", "genomes.md", "neural-networks.md", "first-creature.md"}) {
                select("en");
                draw();
                window.openLink(filename);
                for (auto const& language : {"en", "ru", "en"}) {
                    select(language);
                    draw();
                    EXPECT_EQ(filename, window._chapters.at(window._currentChapterIndex).filename);
                    auto aliases = window._chapters.at(window._currentChapterIndex).anchorAliases;
                    for (auto const& [canonical, native] : aliases) {
                        window.openLink(std::string(filename) + "#" + canonical);
                        EXPECT_FALSE(window._linkError);
                        EXPECT_EQ(native, window._renderer._pendingAnchor);
                        draw();
                        draw();
                        EXPECT_FALSE(window._renderer._pendingAnchor);
                        ImGuiWindow* content = nullptr;
                        for (auto candidate : GImGui->Windows) {
                            if (candidate->RootWindow == host && std::string(candidate->Name).contains("##content")) {
                                content = candidate;
                            }
                        }
                        ASSERT_NE(nullptr, content);
                        auto scroll = content->Scroll.y;
                        EXPECT_GE(scroll, 0);
                        EXPECT_LE(scroll, content->ScrollMax.y);
                        draw();
                        EXPECT_FLOAT_EQ(scroll, content->Scroll.y);
                    }
                    EXPECT_EQ(host, ImGui::FindWindowByName("Documentation"));
                    EXPECT_FLOAT_EQ(1040 * scale, host->Size.x);
                    EXPECT_FLOAT_EQ(720 * scale, host->Size.y);
                    for (auto const& [path, info] : window._renderer._imageInfoByPath) {
                        EXPECT_TRUE(info) << path;
                    }
                }
            }
        }
    }

    void pressChapterButton(std::string const& canonical)
    {
        ImVec2 target;
        auto draw = [&] { window.processChapterButtons(); };
        frame([&] {
            auto id = ImGui::GetID(canonical.c_str());
            ImGui::FocusWindow(ImGui::GetCurrentWindow());
            ImGui::SetNavID(id, ImGuiNavLayer_Main, 0, ImRect());
            draw();
            auto host = ImGui::GetCurrentWindow();
            target = ImGui::WindowRectRelToAbs(host, host->NavRectRel[ImGuiNavLayer_Main]).GetCenter();
            ImGui::ClearActiveID();
        });
        ImGui::GetIO().AddMousePosEvent(target.x, target.y);
        frame(draw);
        ImGui::GetIO().AddMouseButtonEvent(0, true);
        frame(draw);
        ImGui::GetIO().AddMouseButtonEvent(0, false);
        frame(draw);
    }

    void checkLegacyIdsAndButtons()
    {
        tree.translate();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            window.processBackground();
            EXPECT_EQ("Alpha", window._chapters.at(0).id);
            EXPECT_EQ("Energy", window._chapters.at(0).sections.at(1).id);
            window.openLink("beta.md");
            pressChapterButton(ICON_FA_CHEVRON_LEFT "  Alpha");
            EXPECT_EQ("alpha.md", window._chapters.at(window._currentChapterIndex).filename);
            window.openLink("beta.md");
            pressChapterButton("Missing  " ICON_FA_CHEVRON_RIGHT);
            EXPECT_EQ("missing.md", window._chapters.at(window._currentChapterIndex).filename);
        }
    }

    SettingsTestScope settings;
    DocumentationTestTree tree;
    DocumentationWindow window;
    RenderingFacade originalRendering;
    std::shared_ptr<Renderer> rendering;
    std::string originalLanguage;
    float originalScale = 1;
    float factor = 1;
};

TEST_F(DocumentationLocalizationTests, languageSwitchRestoresFilenameSectionSearchAndSettings)
{
    checkSwitchAndSettings();
}
TEST_F(DocumentationLocalizationTests, nativeAndEnglishAliasLinksAndDuplicateHeadings)
{
    checkLinks();
}
TEST_F(DocumentationLocalizationTests, unicodeAndSearchRankingWithValidSnippets)
{
    checkSearch();
}
TEST_F(DocumentationLocalizationTests, imageRootsTextureReuseReleaseAndCacheInvalidation)
{
    checkImages();
}
TEST_F(DocumentationLocalizationTests, realImGuiComponentsAcrossLanguagesAndScales)
{
    checkScales();
}
TEST_F(DocumentationLocalizationTests, mcpEnglishResponsesMatchBaseCommitInEveryGuiLanguage)
{
    checkMcpBaseline();
}
TEST_F(DocumentationLocalizationTests, nativeWindowGeometryIdentityAndSectionScrollSurviveLanguageChanges)
{
    checkNativeWindowState();
}
TEST_F(DocumentationLocalizationTests, shippedEnglishTreeLinksImagesAndFormatReferenceAreReachable)
{
    checkShippedTree();
}
TEST_F(DocumentationLocalizationTests, chapterButtonsKeepEnglishIdsAndNavigateAcrossLanguages)
{
    checkLegacyIdsAndButtons();
}
TEST_F(DocumentationLocalizationTests, shippedRussianPackageStructureFallbackLinksImagesAndSearch)
{
    checkShippedRussianPackage();
}
TEST_F(DocumentationLocalizationTests, shippedLongChaptersRenderAndNavigateAtEveryScale)
{
    checkShippedLongChaptersAtScales();
}
