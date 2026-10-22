#include "MutationRatesDialog.h"

#include <algorithm>

#include <imgui.h>

#include <Base/GlobalSettings.h>

#include "AlienGui.h"
#include "EntityAttributeHelp.h"
#include "GenomeEditorPresentation.h"
#include "MutationRatesPresentation.h"
#include "OverlayController.h"
#include "StyleService.h"

namespace
{
    auto constexpr MinSectionWidth = 620.0f;
    auto constexpr MinTreeNodeWidth = 300.0f;

    auto constexpr MaxTreeNodeColumns = 2;

    auto const DialogSize = RealVector2D(800.0f, 400.0f);

    void processConnectionMutationRate(std::string const& name, std::string const& id, ConnectionMutationDesc& mutation, float rightColumnWidth)
    {
        if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(MutationRatesPresentation::label(name)).rank(AlienGui::TreeNodeRank::Default))) {
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Node probability").label(MutationRatesPresentation::label("Node probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationConnectionProbability)),
                &mutation._nodeProbability);
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Value change sigma").label(MutationRatesPresentation::label("Value change sigma"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.3f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationConnectionValueChangeSigma)),
                &mutation._valueChangeSigma);
        }
        AlienGui::EndTreeNode();
    }

    void processNeuronMutationRate(std::string const& name, std::string const& id, NeuronMutationDesc& mutation, float rightColumnWidth)
    {
        if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(MutationRatesPresentation::label(name)).rank(AlienGui::TreeNodeRank::Default))) {
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Node probability").label(MutationRatesPresentation::label("Node probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationNeuronProbability)),
                &mutation._nodeProbability);
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Weight change sigma").label(MutationRatesPresentation::label("Weight change sigma"))
                    .id(id)
                    .min(0.0f)
                    .max(2.0f)
                    .logarithmic(true)
                    .format("%.2f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationNeuronWeightChangeSigma)),
                &mutation._weightChangeSigma);
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Bias change sigma").label(MutationRatesPresentation::label("Bias change sigma"))
                    .id(id)
                    .min(0.0f)
                    .max(2.0f)
                    .logarithmic(true)
                    .format("%.3f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationNeuronBiasChangeSigma)),
                &mutation._biasChangeSigma);
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("ActFn change probability").label(MutationRatesPresentation::label("ActFn change probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationNeuronActfnChangeProbability)),
                &mutation._actfnChangeProbability);
        }
        AlienGui::EndTreeNode();
    }

    void processCellTypePropertiesMutationRate(std::string const& name, std::string const& id, CellTypePropertiesMutationDesc& mutation, float rightColumnWidth)
    {
        if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(MutationRatesPresentation::label(name)).rank(AlienGui::TreeNodeRank::Default))) {
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Node probability").label(MutationRatesPresentation::label("Node probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationCellTypePropertiesProbability)),
                &mutation._nodeProbability);
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Value change sigma").label(MutationRatesPresentation::label("Value change sigma"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.3f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationCellTypePropertiesValueChangeSigma)),
                &mutation._valueChangeSigma);
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Enum change probability").label(MutationRatesPresentation::label("Enum change probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationCellTypePropertiesEnumChangeProbability)),
                &mutation._enumChangeProbability);
        }
        AlienGui::EndTreeNode();
    }

    void processGeometryMutationRate(std::string const& name, std::string const& id, GeometryMutationDesc& mutation, float rightColumnWidth)
    {
        if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(MutationRatesPresentation::label(name)).rank(AlienGui::TreeNodeRank::Default))) {
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Gene probability").label(MutationRatesPresentation::label("Gene probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationGeometryProbability)),
                &mutation._geneProbability);
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Value change sigma").label(MutationRatesPresentation::label("Value change sigma"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.3f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationGeometryValueChangeSigma)),
                &mutation._valueChangeSigma);
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Enum change probability").label(MutationRatesPresentation::label("Enum change probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationGeometryEnumChangeProbability)),
                &mutation._enumChangeProbability);
        }
        AlienGui::EndTreeNode();
    }

    void processCellTypeModeMutationRate(std::string const& name, std::string const& id, CellTypeModeMutationDesc& mutation, float rightColumnWidth)
    {
        if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(MutationRatesPresentation::label(name)).rank(AlienGui::TreeNodeRank::Default))) {
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Node probability").label(MutationRatesPresentation::label("Node probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationCellTypeModeProbability)),
                &mutation._nodeProbability);
        }
        AlienGui::EndTreeNode();
    }

    void processCellTypeMutationRate(std::string const& name, std::string const& id, CellTypeMutationDesc& mutation, float rightColumnWidth)
    {
        if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(MutationRatesPresentation::label(name)).rank(AlienGui::TreeNodeRank::Default))) {
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Node probability").label(MutationRatesPresentation::label("Node probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationCellTypeProbability)),
                &mutation._nodeProbability);
        }
        AlienGui::EndTreeNode();
    }

    void processCustomizationMutationRate(std::string const& name, std::string const& id, CustomizationMutationDesc& mutation, float rightColumnWidth)
    {
        if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(MutationRatesPresentation::label(name)).rank(AlienGui::TreeNodeRank::Default))) {
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Genome probability").label(MutationRatesPresentation::label("Genome probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationCustomizationProbability)),
                &mutation._genomeProbability);
        }
        AlienGui::EndTreeNode();
    }

    void processVoidMutationRate(std::string const& name, std::string const& id, VoidMutationDesc& mutation, float rightColumnWidth)
    {
        if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(MutationRatesPresentation::label(name)).rank(AlienGui::TreeNodeRank::Default))) {
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Node probability").label(MutationRatesPresentation::label("Node probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationVoidProbability)),
                &mutation._nodeProbability);
        }
        AlienGui::EndTreeNode();
    }

    template <typename MutationDesc>
    void processGeneProbabilityMutationRate(
        std::string const& name,
        std::string const& id,
        MutationDesc& mutation,
        float rightColumnWidth,
        EntityAttribute attribute)
    {
        if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(MutationRatesPresentation::label(name)).rank(AlienGui::TreeNodeRank::Default))) {
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Gene probability").label(MutationRatesPresentation::label("Gene probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(attribute)),
                &mutation._geneProbability);
        }
        AlienGui::EndTreeNode();
    }

    template <typename MutationDesc>
    void processNodeProbabilityMutationRate(
        std::string const& name,
        std::string const& id,
        MutationDesc& mutation,
        float rightColumnWidth,
        EntityAttribute attribute)
    {
        if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(MutationRatesPresentation::label(name)).rank(AlienGui::TreeNodeRank::Default))) {
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Node probability").label(MutationRatesPresentation::label("Node probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(attribute)),
                &mutation._nodeProbability);
        }
        AlienGui::EndTreeNode();
    }

    void processConstructorMutationRate(std::string const& name, std::string const& id, ConstructorMutationDesc& mutation, float rightColumnWidth)
    {
        if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(MutationRatesPresentation::label(name)).rank(AlienGui::TreeNodeRank::Default))) {
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Node probability").label(MutationRatesPresentation::label("Node probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationConstructorProbability)),
                &mutation._nodeProbability);
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Value change sigma").label(MutationRatesPresentation::label("Value change sigma"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.3f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationConstructorValueChangeSigma)),
                &mutation._valueChangeSigma);
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Enum change probability").label(MutationRatesPresentation::label("Enum change probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationConstructorEnumChangeProbability)),
                &mutation._enumChangeProbability);
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("Constructor toggle probability").label(MutationRatesPresentation::label("Constructor toggle probability"))
                    .id(id)
                    .min(0.0f)
                    .max(1.0f)
                    .logarithmic(true)
                    .format("%.5f")
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MutationConstructorToggleProbability)),
                &mutation._constructorToggleProbability);
        }
        AlienGui::EndTreeNode();
    }

    template <typename Func>
    void processConcreteMutationRates(Func&& processMutationRates)
    {
        AlienGui::DynamicTableLayout table(std::max(MinTreeNodeWidth, MutationRatesPresentation::parameterLabelWidth() + 180.0f), MaxTreeNodeColumns);
        if (table.begin()) {
            processMutationRates(table);
            table.end();
        }
    }
}

std::optional<MutationRatesDesc> MutationRatesDialog::_clipboard;

MutationRatesDialog::MutationRatesDialog()
    : _modalWindow("Mutation rates", DialogSize, true, "dialogs.mutation rates")
{
    _modalWindow.setTitleKey("mutation.title");
}

void MutationRatesDialog::loadSettings(MutationRatesDesc& mutationRates, std::string const& settingsPrefix)
{
    auto& settings = GlobalSettings::get();

    for (auto i = 0; i < 2; ++i) {
        auto const indexSuffix = i == 0 ? "1" : "2";

        mutationRates._connectionMutations[i]._nodeProbability =
            settings.getValue(settingsPrefix + "connection mutation " + indexSuffix + ".node probability", mutationRates._connectionMutations[i]._nodeProbability);
        mutationRates._connectionMutations[i]._valueChangeSigma =
            settings.getValue(settingsPrefix + "connection mutation " + indexSuffix + ".sigma", mutationRates._connectionMutations[i]._valueChangeSigma);

        mutationRates._neuronMutations[i]._nodeProbability =
            settings.getValue(settingsPrefix + "neuron mutation " + indexSuffix + ".node probability", mutationRates._neuronMutations[i]._nodeProbability);
        mutationRates._neuronMutations[i]._weightChangeSigma =
            settings.getValue(settingsPrefix + "neuron mutation " + indexSuffix + ".weight sigma", mutationRates._neuronMutations[i]._weightChangeSigma);
        mutationRates._neuronMutations[i]._biasChangeSigma =
            settings.getValue(settingsPrefix + "neuron mutation " + indexSuffix + ".bias sigma", mutationRates._neuronMutations[i]._biasChangeSigma);
        mutationRates._neuronMutations[i]._actfnChangeProbability = settings.getValue(
            settingsPrefix + "neuron mutation " + indexSuffix + ".activation function probability",
            mutationRates._neuronMutations[i]._actfnChangeProbability);
    }

    for (auto i = 0; i < 2; ++i) {
        auto const indexSuffix = i == 0 ? "" : " 2";

        mutationRates._cellTypePropertiesMutations[i]._nodeProbability = settings.getValue(
            settingsPrefix + "cell type property mutation" + indexSuffix + ".node probability", mutationRates._cellTypePropertiesMutations[i]._nodeProbability);
        mutationRates._cellTypePropertiesMutations[i]._valueChangeSigma =
            settings.getValue(settingsPrefix + "cell type property mutation" + indexSuffix + ".sigma", mutationRates._cellTypePropertiesMutations[i]._valueChangeSigma);
        mutationRates._cellTypePropertiesMutations[i]._enumChangeProbability = settings.getValue(
            settingsPrefix + "cell type property mutation" + indexSuffix + ".discrete change probability", mutationRates._cellTypePropertiesMutations[i]._enumChangeProbability);
    }

    for (auto i = 0; i < 2; ++i) {
        auto const indexSuffix = i == 0 ? "1" : "2";

        mutationRates._geometryMutations[i]._geneProbability = settings.getValue(
            settingsPrefix + "geometry mutation " + indexSuffix + ".gene probability", mutationRates._geometryMutations[i]._geneProbability);
        mutationRates._geometryMutations[i]._valueChangeSigma =
            settings.getValue(settingsPrefix + "geometry mutation " + indexSuffix + ".sigma", mutationRates._geometryMutations[i]._valueChangeSigma);
        mutationRates._geometryMutations[i]._enumChangeProbability = settings.getValue(
            settingsPrefix + "geometry mutation " + indexSuffix + ".discrete change probability", mutationRates._geometryMutations[i]._enumChangeProbability);
    }

    mutationRates._cellTypeModeMutation._nodeProbability =
        settings.getValue(settingsPrefix + "cell type mode mutation.node probability", mutationRates._cellTypeModeMutation._nodeProbability);
    mutationRates._cellTypeMutation._nodeProbability =
        settings.getValue(settingsPrefix + "cell type mutation.node probability", mutationRates._cellTypeMutation._nodeProbability);
    mutationRates._customizationMutation._genomeProbability =
        settings.getValue(settingsPrefix + "customization mutation.genome probability", mutationRates._customizationMutation._genomeProbability);
    mutationRates._voidMutation._nodeProbability =
        settings.getValue(settingsPrefix + "void mutation.node probability", mutationRates._voidMutation._nodeProbability);
    mutationRates._extendGeneMutation._geneProbability =
        settings.getValue(settingsPrefix + "extend gene mutation.gene probability", mutationRates._extendGeneMutation._geneProbability);
    mutationRates._addNodeMutation._nodeProbability =
        settings.getValue(settingsPrefix + "add node mutation.node probability", mutationRates._addNodeMutation._nodeProbability);
    mutationRates._trimGeneMutation._geneProbability =
        settings.getValue(settingsPrefix + "trim gene mutation.gene probability", mutationRates._trimGeneMutation._geneProbability);
    mutationRates._deleteNodeMutation._nodeProbability =
        settings.getValue(settingsPrefix + "delete node mutation.node probability", mutationRates._deleteNodeMutation._nodeProbability);
    mutationRates._addGeneMutation._geneProbability =
        settings.getValue(settingsPrefix + "add gene mutation.gene probability", mutationRates._addGeneMutation._geneProbability);
    mutationRates._duplicateGeneMutation._geneProbability =
        settings.getValue(settingsPrefix + "duplicate gene mutation.gene probability", mutationRates._duplicateGeneMutation._geneProbability);
    mutationRates._deleteGeneMutation._geneProbability =
        settings.getValue(settingsPrefix + "delete gene mutation.gene probability", mutationRates._deleteGeneMutation._geneProbability);
    mutationRates._swapGeneMutation._geneProbability =
        settings.getValue(settingsPrefix + "swap gene mutation.gene probability", mutationRates._swapGeneMutation._geneProbability);
    mutationRates._copyNodeSectionMutation._geneProbability =
        settings.getValue(settingsPrefix + "copy node section mutation.gene probability", mutationRates._copyNodeSectionMutation._geneProbability);
    mutationRates._moveNodeSectionMutation._geneProbability =
        settings.getValue(settingsPrefix + "move node section mutation.gene probability", mutationRates._moveNodeSectionMutation._geneProbability);

    for (auto i = 0; i < 2; ++i) {
        auto const indexSuffix = i == 0 ? "1" : "2";

        mutationRates._constructorMutations[i]._nodeProbability = settings.getValue(
            settingsPrefix + "constructor mutation " + indexSuffix + ".node probability", mutationRates._constructorMutations[i]._nodeProbability);
        mutationRates._constructorMutations[i]._valueChangeSigma =
            settings.getValue(settingsPrefix + "constructor mutation " + indexSuffix + ".sigma", mutationRates._constructorMutations[i]._valueChangeSigma);
        mutationRates._constructorMutations[i]._enumChangeProbability = settings.getValue(
            settingsPrefix + "constructor mutation " + indexSuffix + ".discrete change probability", mutationRates._constructorMutations[i]._enumChangeProbability);
        mutationRates._constructorMutations[i]._constructorToggleProbability = settings.getValue(
            settingsPrefix + "constructor mutation " + indexSuffix + ".exist constructor probability",
            mutationRates._constructorMutations[i]._constructorToggleProbability);
    }
}

void MutationRatesDialog::saveSettings(MutationRatesDesc const& mutationRates, std::string const& settingsPrefix)
{
    auto& settings = GlobalSettings::get();

    for (auto i = 0; i < 2; ++i) {
        auto const indexSuffix = i == 0 ? "1" : "2";

        settings.setValue(settingsPrefix + "connection mutation " + indexSuffix + ".node probability", mutationRates._connectionMutations[i]._nodeProbability);
        settings.setValue(settingsPrefix + "connection mutation " + indexSuffix + ".sigma", mutationRates._connectionMutations[i]._valueChangeSigma);

        settings.setValue(settingsPrefix + "neuron mutation " + indexSuffix + ".node probability", mutationRates._neuronMutations[i]._nodeProbability);
        settings.setValue(settingsPrefix + "neuron mutation " + indexSuffix + ".weight sigma", mutationRates._neuronMutations[i]._weightChangeSigma);
        settings.setValue(settingsPrefix + "neuron mutation " + indexSuffix + ".bias sigma", mutationRates._neuronMutations[i]._biasChangeSigma);
        settings.setValue(
            settingsPrefix + "neuron mutation " + indexSuffix + ".activation function probability",
            mutationRates._neuronMutations[i]._actfnChangeProbability);
    }

    for (auto i = 0; i < 2; ++i) {
        auto const indexSuffix = i == 0 ? "1" : "2";

        settings.setValue(
            settingsPrefix + "cell type property mutation " + indexSuffix + ".node probability", mutationRates._cellTypePropertiesMutations[i]._nodeProbability);
        settings.setValue(settingsPrefix + "cell type property mutation " + indexSuffix + ".sigma", mutationRates._cellTypePropertiesMutations[i]._valueChangeSigma);
        settings.setValue(
            settingsPrefix + "cell type property mutation " + indexSuffix + ".discrete change probability", mutationRates._cellTypePropertiesMutations[i]._enumChangeProbability);
    }

    for (auto i = 0; i < 2; ++i) {
        auto const indexSuffix = i == 0 ? "1" : "2";

        settings.setValue(settingsPrefix + "geometry mutation " + indexSuffix + ".gene probability", mutationRates._geometryMutations[i]._geneProbability);
        settings.setValue(settingsPrefix + "geometry mutation " + indexSuffix + ".sigma", mutationRates._geometryMutations[i]._valueChangeSigma);
        settings.setValue(
            settingsPrefix + "geometry mutation " + indexSuffix + ".discrete change probability", mutationRates._geometryMutations[i]._enumChangeProbability);
    }

    settings.setValue(settingsPrefix + "cell type mode mutation.node probability", mutationRates._cellTypeModeMutation._nodeProbability);
    settings.setValue(settingsPrefix + "cell type mutation.node probability", mutationRates._cellTypeMutation._nodeProbability);
    settings.setValue(settingsPrefix + "customization mutation.genome probability", mutationRates._customizationMutation._genomeProbability);
    settings.setValue(settingsPrefix + "void mutation.node probability", mutationRates._voidMutation._nodeProbability);
    settings.setValue(settingsPrefix + "extend gene mutation.gene probability", mutationRates._extendGeneMutation._geneProbability);
    settings.setValue(settingsPrefix + "add node mutation.node probability", mutationRates._addNodeMutation._nodeProbability);
    settings.setValue(settingsPrefix + "trim gene mutation.gene probability", mutationRates._trimGeneMutation._geneProbability);
    settings.setValue(settingsPrefix + "delete node mutation.node probability", mutationRates._deleteNodeMutation._nodeProbability);
    settings.setValue(settingsPrefix + "add gene mutation.gene probability", mutationRates._addGeneMutation._geneProbability);
    settings.setValue(settingsPrefix + "duplicate gene mutation.gene probability", mutationRates._duplicateGeneMutation._geneProbability);
    settings.setValue(settingsPrefix + "delete gene mutation.gene probability", mutationRates._deleteGeneMutation._geneProbability);
    settings.setValue(settingsPrefix + "swap gene mutation.gene probability", mutationRates._swapGeneMutation._geneProbability);
    settings.setValue(settingsPrefix + "copy node section mutation.gene probability", mutationRates._copyNodeSectionMutation._geneProbability);
    settings.setValue(settingsPrefix + "move node section mutation.gene probability", mutationRates._moveNodeSectionMutation._geneProbability);

    for (auto i = 0; i < 2; ++i) {
        auto const indexSuffix = i == 0 ? "1" : "2";

        settings.setValue(settingsPrefix + "constructor mutation " + indexSuffix + ".node probability", mutationRates._constructorMutations[i]._nodeProbability);
        settings.setValue(settingsPrefix + "constructor mutation " + indexSuffix + ".sigma", mutationRates._constructorMutations[i]._valueChangeSigma);
        settings.setValue(settingsPrefix + "constructor mutation " + indexSuffix + ".discrete change probability", mutationRates._constructorMutations[i]._enumChangeProbability);
        settings.setValue(
            settingsPrefix + "constructor mutation " + indexSuffix + ".exist constructor probability",
            mutationRates._constructorMutations[i]._constructorToggleProbability);
    }
}

void MutationRatesDialog::process()
{
    _modalWindow.process([this] { processContent(); });
}

void MutationRatesDialog::processContent()
{
    // Use a child window with scrolling for the content, reserving space for buttons
    auto buttonAreaHeight = scale(85.0f);
    auto rightColumnWidth = MutationRatesPresentation::parameterLabelWidth();
    auto contentWidth = std::max(ImGui::GetContentRegionAvail().x, scale(rightColumnWidth + 220.0f));
    ImGui::SetNextWindowContentSize({contentWidth, 0});
    if (ImGui::BeginChild("MutationRateContent", ImVec2(0, -buttonAreaHeight), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        AlienGui::DynamicTableLayout sectionTable(std::max(MinSectionWidth, rightColumnWidth + 220.0f));
        if (sectionTable.begin()) {
            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Connection weight mutations").label(MutationRatesPresentation::label("Connection weight mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processConnectionMutationRate("Mutation rate 1", "CMR1", _mutation._connectionMutations[0], rightColumnWidth);
                    table.next();
                    processConnectionMutationRate("Mutation rate 2", "CMR2", _mutation._connectionMutations[1], rightColumnWidth);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Neuron mutations").label(MutationRatesPresentation::label("Neuron mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processNeuronMutationRate("Mutation rate 1", "NMR1", _mutation._neuronMutations[0], rightColumnWidth);
                    table.next();
                    processNeuronMutationRate("Mutation rate 2", "NMR2", _mutation._neuronMutations[1], rightColumnWidth);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Cell type property mutations").label(MutationRatesPresentation::label("Cell type property mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processCellTypePropertiesMutationRate("Mutation rate 1", "CTPM1", _mutation._cellTypePropertiesMutations[0], rightColumnWidth);
                    table.next();
                    processCellTypePropertiesMutationRate("Mutation rate 2", "CTPM2", _mutation._cellTypePropertiesMutations[1], rightColumnWidth);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Geometry mutations").label(MutationRatesPresentation::label("Geometry mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processGeometryMutationRate("Mutation rate 1", "GEOM1", _mutation._geometryMutations[0], rightColumnWidth);
                    table.next();
                    processGeometryMutationRate("Mutation rate 2", "GEOM2", _mutation._geometryMutations[1], rightColumnWidth);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Cell type mode mutations").label(MutationRatesPresentation::label("Cell type mode mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processCellTypeModeMutationRate("Mutation rate", "CTMM", _mutation._cellTypeModeMutation, rightColumnWidth);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Cell type mutations").label(MutationRatesPresentation::label("Cell type mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processCellTypeMutationRate("Mutation rate", "CTM", _mutation._cellTypeMutation, rightColumnWidth);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Customization mutations").label(MutationRatesPresentation::label("Customization mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processCustomizationMutationRate("Mutation rate", "CUM", _mutation._customizationMutation, rightColumnWidth);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Void mutations").label(MutationRatesPresentation::label("Void mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processVoidMutationRate("Mutation rate", "VM", _mutation._voidMutation, rightColumnWidth);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Extend gene mutations").label(MutationRatesPresentation::label("Extend gene mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processGeneProbabilityMutationRate(
                        "Mutation rate", "EXGM", _mutation._extendGeneMutation, rightColumnWidth, EntityAttribute::MutationExtendGeneProbability);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Add node mutations").label(MutationRatesPresentation::label("Add node mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processNodeProbabilityMutationRate(
                        "Mutation rate", "ADNM", _mutation._addNodeMutation, rightColumnWidth, EntityAttribute::MutationAddNodeProbability);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Trim gene mutations").label(MutationRatesPresentation::label("Trim gene mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processGeneProbabilityMutationRate(
                        "Mutation rate", "TRGM", _mutation._trimGeneMutation, rightColumnWidth, EntityAttribute::MutationTrimGeneProbability);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Delete node mutations").label(MutationRatesPresentation::label("Delete node mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processNodeProbabilityMutationRate(
                        "Mutation rate", "DLNM", _mutation._deleteNodeMutation, rightColumnWidth, EntityAttribute::MutationDeleteNodeProbability);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Add gene mutations").label(MutationRatesPresentation::label("Add gene mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processGeneProbabilityMutationRate(
                        "Mutation rate", "ADGM", _mutation._addGeneMutation, rightColumnWidth, EntityAttribute::MutationAddGeneProbability);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Duplicate gene mutations").label(MutationRatesPresentation::label("Duplicate gene mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processGeneProbabilityMutationRate(
                        "Mutation rate", "DPGM", _mutation._duplicateGeneMutation, rightColumnWidth, EntityAttribute::MutationDuplicateGeneProbability);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Delete gene mutations").label(MutationRatesPresentation::label("Delete gene mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processGeneProbabilityMutationRate(
                        "Mutation rate", "DLGM", _mutation._deleteGeneMutation, rightColumnWidth, EntityAttribute::MutationDeleteGeneProbability);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Swap gene mutations").label(MutationRatesPresentation::label("Swap gene mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processGeneProbabilityMutationRate(
                        "Mutation rate", "SWGM", _mutation._swapGeneMutation, rightColumnWidth, EntityAttribute::MutationSwapGeneProbability);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Copy node section mutations").label(MutationRatesPresentation::label("Copy node section mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processGeneProbabilityMutationRate(
                        "Mutation rate", "CNSM", _mutation._copyNodeSectionMutation, rightColumnWidth, EntityAttribute::MutationCopyNodeSectionProbability);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Move node section mutations").label(MutationRatesPresentation::label("Move node section mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processGeneProbabilityMutationRate(
                        "Mutation rate", "MNSM", _mutation._moveNodeSectionMutation, rightColumnWidth, EntityAttribute::MutationMoveNodeSectionProbability);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            if (AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Constructor mutations").label(MutationRatesPresentation::label("Constructor mutations")).rank(AlienGui::TreeNodeRank::High))) {
                processConcreteMutationRates([&](AlienGui::DynamicTableLayout& table) {
                    processConstructorMutationRate("Mutation rate 1", "COM1", _mutation._constructorMutations[0], rightColumnWidth);
                    table.next();
                    processConstructorMutationRate("Mutation rate 2", "COM2", _mutation._constructorMutations[1], rightColumnWidth);
                    table.next();
                });
            }
            AlienGui::EndTreeNode();
            sectionTable.next();

            sectionTable.end();
        }
    }
    ImGui::EndChild();

    AlienGui::Separator();

    processButtons();
}

void MutationRatesDialog::processButtons()
{
    if (AlienGui::DisplayButton("Adopt", tr("mutation.adopt"))) {
        onAdopt();
        _modalWindow.close();
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienGui::DisplayButton("Cancel", tr("mutation.cancel"))) {
        _modalWindow.close();
    }

    ImGui::SameLine();
    if (AlienGui::DisplayButton("Reset", tr("mutation.reset"))) {
        _mutation = MutationRatesDesc();
        printOverlayMessage(tr("mutation.resetmessage"));
    }
    auto actionRowEnd = ImGui::GetItemRectMax().x;
    AlienGui::Tooltip(AlienGui::TooltipParameters().text(tr("mutation.resethelp")));

    // The clipboard buttons belong to the content and are therefore set off from the dialog buttons
    auto rowRight = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
    if (rowRight - actionRowEnd >= calcClipboardButtonsWidth() + ImGui::GetStyle().ItemSpacing.x) {
        ImGui::SameLine();
    }
    auto offset = ImGui::GetContentRegionAvail().x - calcClipboardButtonsWidth();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.0f, offset));

    if (AlienGui::DisplayButton("Copy", tr("mutation.copy"))) {
        _clipboard = _mutation;
        printOverlayMessage(tr("mutation.copied"));
    }
    AlienGui::Tooltip(AlienGui::TooltipParameters().text(tr("mutation.copyhelp")));

    ImGui::SameLine();
    ImGui::BeginDisabled(!_clipboard.has_value());
    if (AlienGui::DisplayButton("Paste", tr("mutation.paste"))) {
        _mutation = _clipboard.value();
        printOverlayMessage(tr("mutation.pasted"));
    }
    ImGui::EndDisabled();
    AlienGui::Tooltip(AlienGui::TooltipParameters().text(tr("mutation.pastehelp")));
}

float MutationRatesDialog::calcClipboardButtonsWidth()
{
    auto& style = ImGui::GetStyle();
    auto textWidth = ImGui::CalcTextSize(tr("mutation.copy").c_str()).x + ImGui::CalcTextSize(tr("mutation.paste").c_str()).x;
    return textWidth + 4 * style.FramePadding.x + style.ItemSpacing.x;
}

void MutationRatesDialog::open(MutationRatesDesc const& mutationRates, std::function<void(MutationRatesDesc const&)> const& onAdoptCallback)
{
    _mutation = mutationRates;
    _onAdoptCallback = onAdoptCallback;
    _modalWindow.open();
}

void MutationRatesDialog::onAdopt()
{
    if (_onAdoptCallback) {
        _onAdoptCallback(_mutation);
    }
}
