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
    std::vector<std::string> provideEnergyGenomes();
    std::vector<std::string> sensorModes();
    std::vector<std::string> generatorModes();
    std::vector<std::string> attackerModes();
    std::vector<std::string> muscleModes();
    std::vector<std::string> defenderModes();
    std::vector<std::string> reconnectorModes();
    std::vector<std::string> memoryModes();
    std::vector<std::string> communicatorModes();
    std::string attributeDescription(EntityAttribute attribute);
}
