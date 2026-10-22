#pragma once

#include <algorithm>
#include <array>
#include <initializer_list>
#include <string>
#include <utility>

#include <imgui.h>

#include <Base/LocalizationService.h>

#include "StyleService.h"

namespace MutationRatesPresentation
{
    inline std::string label(std::string const& canonical)
    {
        static constexpr auto entries = std::to_array<std::pair<char const*, char const*>>(
            {{"Mutation rates", "mutation.title"},
             {"Node probability", "mutation.nodeprobability"},
             {"Gene probability", "mutation.geneprobability"},
             {"Genome probability", "mutation.genomeprobability"},
             {"Value change sigma", "mutation.valuesigma"},
             {"Weight change sigma", "mutation.weightsigma"},
             {"Bias change sigma", "mutation.biassigma"},
             {"ActFn change probability", "mutation.activationprobability"},
             {"Enum change probability", "mutation.enumprobability"},
             {"Constructor toggle probability", "mutation.constructorprobability"},
             {"Mutation rate", "mutation.rate"},
             {"Mutation rate 1", "mutation.rateone"},
             {"Mutation rate 2", "mutation.ratetwo"},
             {"Connection weight mutations", "mutation.connectionweights"},
             {"Connection mutations", "mutation.connections"},
             {"Neuron mutations", "mutation.neurons"},
             {"Cell type property mutations", "mutation.properties"},
             {"Cell type property mut.", "mutation.propertiessummary"},
             {"Geometry mutations", "mutation.geometry"},
             {"Cell type mode mutations", "mutation.modes"},
             {"Cell type mode mut.", "mutation.modessummary"},
             {"Cell type mutations", "mutation.types"},
             {"Customization mutations", "mutation.customization"},
             {"Void mutations", "mutation.void"},
             {"Extend gene mutations", "mutation.extendgene"},
             {"Add node mutations", "mutation.addnode"},
             {"Trim gene mutations", "mutation.trimgene"},
             {"Delete node mutations", "mutation.deletenode"},
             {"Add gene mutations", "mutation.addgene"},
             {"Duplicate gene mutations", "mutation.duplicategene"},
             {"Delete gene mutations", "mutation.deletegene"},
             {"Swap gene mutations", "mutation.swapgene"},
             {"Copy node section mutations", "mutation.copysection"},
             {"Move node section mutations", "mutation.movesection"},
             {"Constructor mutations", "mutation.constructors"},
             {"Constructor", "mutation.constructorsummary"},
             {"Click to edit", "mutation.clicktoedit"}});
        for (auto const& [name, key] : entries) {
            if (canonical == name) {
                return tr(key);
            }
        }
        return canonical;
    }

    inline float labelWidth(float minimum, std::initializer_list<char const*> names)
    {
        for (auto name : names) {
            minimum = std::max(minimum, scaleInverse(ImGui::CalcTextSize(label(name).c_str()).x) + 35.0f);
        }
        return minimum;
    }

    inline float parameterLabelWidth()
    {
        return labelWidth(
            195.0f,
            {"Node probability",
             "Gene probability",
             "Genome probability",
             "Value change sigma",
             "Weight change sigma",
             "Bias change sigma",
             "ActFn change probability",
             "Enum change probability",
             "Constructor toggle probability"});
    }
}
