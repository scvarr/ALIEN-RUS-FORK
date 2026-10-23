#pragma once

#include <map>
#include <string>

#include <Base/LocalizationService.h>

namespace McpPresentation
{
    inline std::string groupCaption(std::string const& canonical)
    {
        static std::map<std::string, std::string> const keys = {
            {"Temporal control", "mcp.group.temporal"},
            {"Simulations and files", "mcp.group.files"},
            {"World editing", "mcp.group.editing"},
            {"Inspection and view", "mcp.group.inspection"},
            {"Simulation parameters", "mcp.group.parameters"},
            {"Documentation", "mcp.group.documentation"}};
        auto found = keys.find(canonical);
        return found == keys.end() ? canonical : tr(found->second);
    }
}
