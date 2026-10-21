#include "SimulationParametersPresentation.h"

#include <Base/LocalizationService.h>
#include <Data/SimulationParameters.h>

#include "Utf8Search.h"

namespace
{
    template <typename Spec, typename Member>
    bool references(ParameterSpec const& parameter, Member member)
    {
        auto spec = std::get_if<Spec>(&parameter._reference);
        if (!spec) {
            return false;
        }
        auto reference = std::get_if<std::shared_ptr<Member>>(&spec->_member);
        return reference && *reference && **reference == member;
    }

    std::string key(ParameterSpec const& spec)
    {
        if (references<Char64Spec>(spec, &SimulationParameters::projectName)) {
            return "parameters.general.project";
        }
        if (references<Char64Spec>(spec, &SimulationParameters::layerName)) {
            return "parameters.general.layer";
        }
        if (references<Char64Spec>(spec, &SimulationParameters::sourceName)) {
            return "parameters.general.source";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerOpacity)) {
            return "parameters.general.opacity";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::sourceRelativeStrength)) {
            return "parameters.general.share";
        }
        if (references<Float2Spec>(spec, &SimulationParameters::layerPosition)) {
            return "parameters.location.layerposition";
        }
        if (references<Float2Spec>(spec, &SimulationParameters::layerVelocity)) {
            return "parameters.location.layervelocity";
        }
        if (references<Float2Spec>(spec, &SimulationParameters::sourcePosition)) {
            return "parameters.location.sourceposition";
        }
        if (references<Float2Spec>(spec, &SimulationParameters::sourceVelocity)) {
            return "parameters.location.sourcevelocity";
        }
        return {};
    }

    std::string translated(std::string const& key, std::string const& fallback)
    {
        if (key.empty()) {
            return fallback;
        }
        auto text = tr(key);
        return text == key ? fallback : text;
    }

    bool matches(std::string const& canonical, std::string const& display, std::string const& query)
    {
        return Utf8Search::find(canonical, query).has_value() || Utf8Search::find(display, query).has_value();
    }

    bool filterParameter(ParameterSpec& spec, std::string const& query)
    {
        auto visible = matches(spec._name, SimulationParametersPresentation::label(spec), query);
        if (auto alternative = std::get_if<AlternativeSpec>(&spec._reference)) {
            for (auto& [name, parameters] : alternative->_alternatives) {
                for (auto& parameter : parameters) {
                    visible = filterParameter(parameter, query) || visible;
                }
            }
        }
        spec._visible = spec._visible && visible;
        return spec._visible;
    }
}

std::string SimulationParametersPresentation::label(ParameterSpec const& spec)
{
    auto prefix = key(spec);
    return translated(prefix.empty() ? prefix : prefix + ".name", spec._name);
}

std::optional<std::string> SimulationParametersPresentation::description(ParameterSpec const& spec)
{
    auto prefix = key(spec);
    if (prefix.empty()) {
        return spec._description;
    }
    return translated(prefix + ".description", spec._description.value_or(""));
}

std::string SimulationParametersPresentation::label(ParameterGroupSpec const& spec)
{
    if (spec._name == "General") {
        return translated("parameters.general.name", spec._name);
    }
    if (spec._name == "Location") {
        return translated("parameters.location.name", spec._name);
    }
    return spec._name;
}

ParametersSpec SimulationParametersPresentation::filter(ParametersSpec const& spec, ParametersFilter const& filter)
{
    auto result = spec;
    if (filter.containedText && !filter.containedText->empty()) {
        for (auto& group : result._groups) {
            if (!matches(group._name, label(group), *filter.containedText)) {
                for (auto& parameter : group._parameters) {
                    filterParameter(parameter, *filter.containedText);
                }
            }
        }
    }
    return result;
}
