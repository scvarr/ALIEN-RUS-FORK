#pragma once

#include <Base/LocalizationService.h>

namespace SimulationParametersGuiText
{
    inline std::string named(std::string const& key, std::string const& name)
    {
        auto text = tr(key);
        auto position = text.find("{name}");
        if (position != std::string::npos) {
            text.replace(position, 6, name);
        }
        return text;
    }

    inline std::string section(std::string const& name, bool base, bool filtered)
    {
        return named(filtered ? "parameters.section.filtered" : "parameters.section", base ? tr("parameters.base") : name);
    }
}
