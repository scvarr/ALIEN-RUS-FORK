#pragma once

#include <Base/MarkdownDocument.h>
#include <Base/Resources.h>
#include <Base/Singleton.h>

#include <Engine/Interface/Definitions.h>

#include "AlienWindow.h"
#include "Definitions.h"
#include "MarkdownRenderer.h"

class DocumentationWindow : public AlienWindow
{
    friend class DocumentationLocalizationTests;
    MAKE_SINGLETON_NO_DEFAULT_CONSTRUCTION(DocumentationWindow);

private:
    DocumentationWindow();

    void initIntern() override;
    void shutdownIntern() override;
    void processIntern() override;
    void processBackground() override;

    void loadChapters();
    void reloadLanguage();
    void updateSearchResults();

    void processNavigation(float width);
    void processTableOfContents();
    void processSearchResults();
    void processContent();
    void processChapterButtons();

    void openLink(std::string const& link);
    void openChapter(size_t chapterIndex, std::optional<std::string> const& anchor = std::nullopt);

    struct Section
    {
        std::string title;
        std::string id;
        std::string anchor;
        std::string text;
    };
    struct Chapter
    {
        std::string filename;
        std::string title;
        std::string part;
        std::string id;
        std::vector<Section> sections;
        MarkdownDocument document;
        std::filesystem::path basePath;
        bool englishFallback = false;
        std::map<std::string, std::string> anchorAliases;
    };
    std::vector<Chapter> _chapters;
    std::filesystem::path _docsPath = Const::DocsPath;
    std::string _language;
    std::optional<std::string> _linkError;
    size_t _currentChapterIndex = 0;
    bool _scrollToTop = false;

    struct SearchResult
    {
        size_t chapterIndex = 0;
        size_t sectionIndex = 0;
        std::string snippet;
    };
    std::string _searchText;
    std::vector<SearchResult> _searchResults;

    MarkdownRenderer _renderer;
    float _navigationWidth = 0;
    bool _showAfterStartup = true;
};
