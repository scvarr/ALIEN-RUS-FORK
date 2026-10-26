#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

class DocumentationTestTree
{
public:
    DocumentationTestTree()
        : root(std::filesystem::temp_directory_path() / ("alien-documentation-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())))
    {
        write("contents.md", "# Start\n\n- [Alpha](alpha.md)\n- [Beta](beta.md)\n- [Missing](missing.md)\n");
        write("alpha.md", "# Alpha\n\n## Energy\n\nEnglish energy.\n\n## Energy\n\nSecond section.\n");
        write("beta.md", "# Beta\n\n## Links\n\n[Other](alpha.md#energy-1) [Here](#links) [Web](https://example.com)\n");
        write("format-reference.md", "# Markdown format reference\n\n## Syntax\n\nSupported syntax.\n");
    }

    ~DocumentationTestTree() { std::filesystem::remove_all(root); }

    void write(std::string const& filename, std::string const& text)
    {
        auto path = root / filename;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream stream(path, std::ios::binary);
        stream << text;
    }

    void translate()
    {
        write("ru/contents.md", "# Начало\n\n- [Альфа](alpha.md)\n- [Бета](beta.md)\n- [Формат Markdown](format-reference.md)\n");
        write("ru/alpha.md", "# Альфа\n\n## Энергия\n\nЁж и энергия ALIEN.\n\n## Энергия\n\nПовторный раздел.\n");
        write("ru/beta.md", "# Бета\n\n## Ссылки\n\n[Другая](alpha.md#Энергия-1) [Здесь](#Ссылки) [Сайт](https://example.com)\n");
        write("ru/format-reference.md", "# Формат Markdown\n\n## Синтаксис\n\nОписание синтаксиса.\n");
    }

    std::filesystem::path root;
};
