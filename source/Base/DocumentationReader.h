#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct DocumentationChapter
{
    bool operator==(DocumentationChapter const&) const = default;

    std::string filename;
    std::string title;
    std::string part;
};

enum class DocumentationLanguage
{
    English,
    Russian
};

struct DocumentationContent
{
    std::string text;
    std::filesystem::path basePath;
    bool englishFallback = false;
};

class DocumentationReader
{
public:
    static std::vector<DocumentationChapter> getChapters();
    static std::vector<DocumentationChapter> parseChapters(std::string const& contentsText);
    static std::string readChapter(DocumentationChapter const& chapter);

    // GUI catalog: English order with optional translated metadata and a GUI-only format reference.
    static std::vector<DocumentationChapter> getGuiChapters(DocumentationLanguage language, std::filesystem::path const& englishRoot);
    static DocumentationContent loadChapter(std::string const& filename, DocumentationLanguage language, std::filesystem::path const& englishRoot);
};
