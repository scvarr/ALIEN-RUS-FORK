#pragma once

#include <map>
#include <string>
#include <vector>

#include "EntityAttributeHelp.h"
#include "LocalizationGuiText.h"

namespace GenomeEditorPresentation
{
    using LocalizationGuiText::format;
    std::string count(std::string const& key, int value);
    std::vector<std::string> shapes();
    std::vector<std::string> cellTypes();
    std::string attributeDescription(EntityAttribute attribute);
}
