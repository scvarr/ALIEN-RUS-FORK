#pragma once

#include <Data/ParametersFilter.h>
#include <Data/SimulationParametersSpecification.h>

namespace SimulationParametersPresentation
{
    std::string label(ParameterSpec const& spec);
    std::optional<std::string> description(ParameterSpec const& spec);
    std::string label(ParameterGroupSpec const& spec);
    std::vector<std::string> alternativeLabels(ParameterSpec const& spec);

    // The GUI copy retains every original index, reference and canonical name.
    // Filtering changes only visibility; the engine specification stays untouched.
    ParametersSpec filter(ParametersSpec const& spec, ParametersFilter const& filter);
}
