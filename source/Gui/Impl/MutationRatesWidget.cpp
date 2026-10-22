#include "MutationRatesWidget.h"

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include <imgui.h>

#include <Base/StringHelper.h>

#include <Data/GenomeDesc.h>

#include "AlienGui.h"
#include "EntityAttributeHelp.h"
#include "GenomeEditorPresentation.h"
#include "MutationRatesPresentation.h"
#include "StyleService.h"

namespace
{
    struct ActiveMutationType
    {
        std::string name;
        std::string probabilities;
        EntityAttribute attribute;
    };

    void addActiveMutationType(
        std::vector<ActiveMutationType>& result,
        std::string const& name,
        EntityAttribute attribute,
        std::initializer_list<float> nodeProbabilities)
    {
        std::string probabilities;
        for (auto const& nodeProbability : nodeProbabilities) {
            if (nodeProbability > 0.0f) {
                if (!probabilities.empty()) {
                    probabilities += ", ";
                }
                probabilities += StringHelper::format(nodeProbability, 5);
            }
        }
        if (!probabilities.empty()) {
            result.emplace_back(name, probabilities, attribute);
        }
    }

    std::vector<ActiveMutationType> getActiveMutationTypes(MutationRatesDesc const& mutationRates)
    {
        std::vector<ActiveMutationType> activeMutations;
        addActiveMutationType(
            activeMutations,
            "Connection mutations",
            EntityAttribute::MutationConnectionProbability,
            {mutationRates._connectionMutations[0]._nodeProbability, mutationRates._connectionMutations[1]._nodeProbability});
        addActiveMutationType(
            activeMutations,
            "Neuron mutations",
            EntityAttribute::MutationNeuronProbability,
            {mutationRates._neuronMutations[0]._nodeProbability, mutationRates._neuronMutations[1]._nodeProbability});
        addActiveMutationType(
            activeMutations,
            "Cell type property mut.",
            EntityAttribute::MutationCellTypePropertiesProbability,
            {mutationRates._cellTypePropertiesMutations[0]._nodeProbability, mutationRates._cellTypePropertiesMutations[1]._nodeProbability});
        addActiveMutationType(
            activeMutations,
            "Geometry mutations",
            EntityAttribute::MutationGeometryProbability,
            {mutationRates._geometryMutations[0]._geneProbability, mutationRates._geometryMutations[1]._geneProbability});
        addActiveMutationType(
            activeMutations, "Cell type mode mut.", EntityAttribute::MutationCellTypeModeProbability, {mutationRates._cellTypeModeMutation._nodeProbability});
        addActiveMutationType(
            activeMutations, "Cell type mutations", EntityAttribute::MutationCellTypeProbability, {mutationRates._cellTypeMutation._nodeProbability});
        addActiveMutationType(
            activeMutations,
            "Customization mutations",
            EntityAttribute::MutationCustomizationProbability,
            {mutationRates._customizationMutation._genomeProbability});
        addActiveMutationType(activeMutations, "Void mutations", EntityAttribute::MutationVoidProbability, {mutationRates._voidMutation._nodeProbability});
        addActiveMutationType(
            activeMutations, "Extend gene mutations", EntityAttribute::MutationExtendGeneProbability, {mutationRates._extendGeneMutation._geneProbability});
        addActiveMutationType(
            activeMutations, "Add node mutations", EntityAttribute::MutationAddNodeProbability, {mutationRates._addNodeMutation._nodeProbability});
        addActiveMutationType(
            activeMutations, "Trim gene mutations", EntityAttribute::MutationTrimGeneProbability, {mutationRates._trimGeneMutation._geneProbability});
        addActiveMutationType(
            activeMutations, "Delete node mutations", EntityAttribute::MutationDeleteNodeProbability, {mutationRates._deleteNodeMutation._nodeProbability});
        addActiveMutationType(
            activeMutations, "Add gene mutations", EntityAttribute::MutationAddGeneProbability, {mutationRates._addGeneMutation._geneProbability});
        addActiveMutationType(
            activeMutations,
            "Duplicate gene mutations",
            EntityAttribute::MutationDuplicateGeneProbability,
            {mutationRates._duplicateGeneMutation._geneProbability});
        addActiveMutationType(
            activeMutations, "Delete gene mutations", EntityAttribute::MutationDeleteGeneProbability, {mutationRates._deleteGeneMutation._geneProbability});
        addActiveMutationType(
            activeMutations, "Swap gene mutations", EntityAttribute::MutationSwapGeneProbability, {mutationRates._swapGeneMutation._geneProbability});
        addActiveMutationType(
            activeMutations,
            "Copy node section mutations",
            EntityAttribute::MutationCopyNodeSectionProbability,
            {mutationRates._copyNodeSectionMutation._geneProbability});
        addActiveMutationType(
            activeMutations,
            "Move node section mutations",
            EntityAttribute::MutationMoveNodeSectionProbability,
            {mutationRates._moveNodeSectionMutation._geneProbability});
        addActiveMutationType(
            activeMutations,
            "Constructor",
            EntityAttribute::MutationConstructorProbability,
            {mutationRates._constructorMutations[0]._nodeProbability, mutationRates._constructorMutations[1]._nodeProbability});
        return activeMutations;
    }
}

void MutationRatesWidget::process(MutationRatesDesc& mutationRates, float rightColumnWidth, bool disabled)
{
    ImGui::BeginDisabled(disabled);
    ImGui::BeginGroup();
    rightColumnWidth = MutationRatesPresentation::labelWidth(rightColumnWidth, {"Click to edit"});
    for (auto const& entry : getActiveMutationTypes(mutationRates)) {
        rightColumnWidth = std::max(rightColumnWidth, scaleInverse(ImGui::CalcTextSize(MutationRatesPresentation::label(entry.name).c_str()).x) + 35.0f);
    }

    if (AlienGui::Button(AlienGui::ButtonParameters()
                             .buttonText("Edit")
                             .buttonLabel(tr("mutation.edit"))
                             .name("Click to edit")
                             .label(tr("mutation.clicktoedit"))
                             .textWidth(rightColumnWidth)
                             .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GenomeMutationRatesEdit)))) {
        _dialog.open(mutationRates, [&mutationRates](MutationRatesDesc const& adoptedRates) { mutationRates = adoptedRates; });
    }

    for (auto const& [name, probabilities, attribute] : getActiveMutationTypes(mutationRates)) {
        auto value = probabilities;
        AlienGui::InputText(
            AlienGui::InputTextParameters()
                .name(name)
                .label(MutationRatesPresentation::label(name))
                .readOnly(true)
                .textWidth(rightColumnWidth)
                .tooltip(GenomeEditorPresentation::attributeDescription(attribute)),
            value);
    }
    ImGui::EndGroup();
    ImGui::EndDisabled();

    // The dialog is opened from here and therefore also processed here, but outside of the disabled scope, since
    // BeginDisabled() also applies to popups that are begun inside of it
    _dialog.process();
}

void MutationRatesWidget::processAsSingleRow(MutationRatesDesc& mutationRates, float rightColumnWidth)
{
    rightColumnWidth = MutationRatesPresentation::labelWidth(rightColumnWidth, {"Mutation rates"});
    auto activeMutationTypes = getActiveMutationTypes(mutationRates);
    auto tooltip = GenomeEditorPresentation::attributeDescription(EntityAttribute::GenomeMutationRatesSummary);
    if (!activeMutationTypes.empty()) {
        tooltip += "\n\n" + tr("mutation.active");
        for (auto const& [name, probabilities, attribute] : activeMutationTypes) {
            tooltip += "\n" + MutationRatesPresentation::label(name) + ": " + probabilities;
        }
    }

    if (AlienGui::Button(AlienGui::ButtonParameters()
                             .buttonText("Edit (" + std::to_string(activeMutationTypes.size()) + " active)")
                             .buttonLabel(GenomeEditorPresentation::format("mutation.editsummary", {{"count", std::to_string(activeMutationTypes.size())}}))
                             .name("Mutation rates")
                             .label(tr("mutation.title"))
                             .textWidth(rightColumnWidth)
                             .tooltip(tooltip))) {
        _dialog.open(mutationRates, [&mutationRates](MutationRatesDesc const& adoptedRates) { mutationRates = adoptedRates; });
    }
    _dialog.process();
}
