#pragma once

#include <array>
#include <string>
#include <vector>

#include <Data/CellTypeConstants.h>

#include "LocalizationGuiText.h"

namespace NeuralNetEditorPresentation
{
    inline std::vector<std::string> activationFunctions()
    {
        std::array keys = {
            "neural.activation.tanh",
            "neural.activation.step",
            "neural.activation.identity",
            "neural.activation.abs",
            "neural.activation.gaussian",
            "neural.activation.mod"};
        static_assert(keys.size() == ActivationFunction_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    inline std::vector<std::string> shortActivationFunctions()
    {
        auto labels = std::array<std::string, 6>{
            "tanh", tr("neural.activation.shortstep"), tr("neural.activation.shortidentity"), "abs", tr("neural.activation.shortgaussian"), "mod"};
        static_assert(labels.size() == ActivationFunction_Count);
        return {labels.begin(), labels.end()};
    }

    inline std::string indexed(std::string const& key, int index)
    {
        return LocalizationGuiText::format(key, {{"index", std::to_string(index)}});
    }
}
