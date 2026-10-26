#include <gtest/gtest.h>

#include <Base/DocumentationReader.h>
#include "DocumentationTestTree.h"

class DocumentationReaderTests : public ::testing::Test
{};

TEST_F(DocumentationReaderTests, parseChapters_emptyText)
{
    EXPECT_TRUE(DocumentationReader::parseChapters("").empty());
}

TEST_F(DocumentationReaderTests, parseChapters_parts)
{
    auto chapters = DocumentationReader::parseChapters("Introduction with a [link](format-reference.md).\n\n"
                                                       "# Start here\n\n"
                                                       "- [Getting started](getting-started.md)\n\n"
                                                       "# Reference\n\n"
                                                       "- [Cell types](cell-types.md)\n"
                                                       "- Item without link\n"
                                                       "- [Energy](energy.md)\n");

    std::vector<DocumentationChapter> expectedChapters = {
        DocumentationChapter{.filename = "getting-started.md", .title = "Getting started", .part = "Start here"},
        DocumentationChapter{.filename = "cell-types.md", .title = "Cell types", .part = "Reference"},
        DocumentationChapter{.filename = "energy.md", .title = "Energy", .part = "Reference"},
    };
    EXPECT_EQ(expectedChapters, chapters);
}

TEST_F(DocumentationReaderTests, explicitEnglishAndCompleteRussianTrees)
{
    DocumentationTestTree tree;
    tree.translate();
    tree.write("missing.md", "# Complete English chapter\n\nText.\n");
    tree.write("ru/missing.md", "# Полная русская глава\n\nТекст.\n");
    auto english = DocumentationReader::getGuiChapters(DocumentationLanguage::English, tree.root);
    auto russian = DocumentationReader::getGuiChapters(DocumentationLanguage::Russian, tree.root);
    ASSERT_EQ(4, english.size());
    ASSERT_EQ(english.size(), russian.size());
    EXPECT_EQ("Alpha", english.at(0).title);
    EXPECT_EQ("Альфа", russian.at(0).title);
    EXPECT_EQ("Начало", russian.at(0).part);
    for (size_t i = 0; i < english.size(); ++i) {
        EXPECT_EQ(english.at(i).filename, russian.at(i).filename);
        auto content = DocumentationReader::loadChapter(english.at(i).filename, DocumentationLanguage::English, tree.root);
        EXPECT_FALSE(content.text.empty());
        EXPECT_EQ(tree.root, content.basePath);
        content = DocumentationReader::loadChapter(russian.at(i).filename, DocumentationLanguage::Russian, tree.root);
        EXPECT_FALSE(content.text.empty());
        EXPECT_EQ(tree.root / "ru", content.basePath);
        EXPECT_FALSE(content.englishFallback);
    }
    auto content = DocumentationReader::loadChapter("alpha.md", DocumentationLanguage::Russian, tree.root);
    EXPECT_EQ(tree.root / "ru", content.basePath);
    EXPECT_FALSE(content.englishFallback);
    EXPECT_TRUE(content.text.contains("Ёж"));
    content = DocumentationReader::loadChapter("alpha.md", DocumentationLanguage::English, tree.root);
    EXPECT_EQ(tree.root, content.basePath);
    EXPECT_TRUE(content.text.contains("English energy"));
}

TEST_F(DocumentationReaderTests, missingAndPartialRussianContentsPreserveCanonicalOrder)
{
    DocumentationTestTree tree;
    tree.write("ru/alpha.md", "# Альфа\n\nПеревод.\n");
    for (auto const& contents : {"", "# Раздел\n\n- [Чужая](other.md)\n- [Бета](beta.md)\n- [Первая](alpha.md)\n"}) {
        tree.write("ru/contents.md", contents);
        auto chapters = DocumentationReader::getGuiChapters(DocumentationLanguage::Russian, tree.root);
        ASSERT_EQ(4, chapters.size());
        EXPECT_EQ("alpha.md", chapters.at(0).filename);
        EXPECT_EQ("beta.md", chapters.at(1).filename);
        EXPECT_EQ("Beta", chapters.at(1).title);
        EXPECT_EQ("format-reference.md", chapters.back().filename);
        auto content = DocumentationReader::loadChapter("beta.md", DocumentationLanguage::Russian, tree.root);
        EXPECT_TRUE(content.englishFallback);
        EXPECT_EQ(tree.root, content.basePath);
    }
    std::filesystem::remove(tree.root / "ru/contents.md");
    EXPECT_EQ("Альфа", DocumentationReader::getGuiChapters(DocumentationLanguage::Russian, tree.root).at(0).title);
}

TEST_F(DocumentationReaderTests, unreadableEmptyAndUnsafeDocuments)
{
    DocumentationTestTree tree;
    for (auto const& text : {"", " \r\n\t", "<!-- untranslated -->"}) {
        tree.write("ru/alpha.md", text);
        auto content = DocumentationReader::loadChapter("alpha.md", DocumentationLanguage::Russian, tree.root);
        EXPECT_TRUE(content.englishFallback);
        EXPECT_TRUE(content.text.contains("English energy"));
        EXPECT_EQ(tree.root, content.basePath);
    }
    std::filesystem::remove(tree.root / "ru/alpha.md");
    std::filesystem::create_directory(tree.root / "ru/alpha.md");
    EXPECT_TRUE(DocumentationReader::loadChapter("alpha.md", DocumentationLanguage::Russian, tree.root).englishFallback);
    EXPECT_TRUE(DocumentationReader::loadChapter("missing.md", DocumentationLanguage::Russian, tree.root).text.empty());
    EXPECT_TRUE(DocumentationReader::getGuiChapters(DocumentationLanguage::Russian, tree.root / "absent").empty());
    for (auto const& name : {"../alpha.md", "ru/alpha.md", "https://example.com/a.md", "alpha.md#energy", "C:\\alpha.md"}) {
        EXPECT_TRUE(DocumentationReader::loadChapter(name, DocumentationLanguage::Russian, tree.root).text.empty());
    }
}
