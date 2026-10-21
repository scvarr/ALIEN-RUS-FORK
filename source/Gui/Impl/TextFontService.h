#pragma once

#include <imgui.h>

enum class TextFontRole
{
    Regular,
    Bold,
    Monospace
};

namespace TextFontService
{
    ImFont* addFont(ImFontAtlas& atlas, TextFontRole role, float size);
}
