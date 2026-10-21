#include "TextFontService.h"

#include <Base/Resources.h>

#include <stdexcept>

ImFont* TextFontService::addFont(ImFontAtlas& atlas, TextFontRole role, float size)
{
    // Atlas construction is deferred; these ranges must outlive setup().
    static ImWchar const ranges[] = {0x0020, 0x00ff, 0x0400, 0x045f, 0x2010, 0x2027, 0x2116, 0x2116, 0};
    auto filename = role == TextFontRole::Bold ? "NotoSans-Bold.ttf" : role == TextFontRole::Monospace ? "NotoSansMono-Regular.ttf" : "NotoSans-Regular.ttf";
    auto path = Const::ResourcePath / "fonts" / filename;
    auto font = atlas.AddFontFromFileTTF(path.string().c_str(), size, nullptr, ranges);
    if (!font) {
        throw std::runtime_error("Cannot load GUI font: " + path.string());
    }
    return font;
}
