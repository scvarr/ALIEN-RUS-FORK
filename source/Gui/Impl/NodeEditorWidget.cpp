#include "NodeEditorWidget.h"

#include <ranges>

#include <Base/LocalizationService.h>

#include <boost/range/adaptors.hpp>

#include <Engine/Interface/SimulationFacade.h>

#include "AlienGui.h"
#include "EntityAttributeHelp.h"
#include "GenericMessageDialog.h"
#include "GenomeEditorPresentation.h"
#include "GenomeTabEditData.h"
#include "GenomeTabLayoutData.h"
#include "LoginDialog.h"
#include "NeuralNetEditorWidget.h"
#include "SignalsBufferDialog.h"

namespace
{
    auto constexpr MinTextWidth = 160.0f;
    auto constexpr MaxWidgetWidth = 200.0f;
    auto constexpr MinColumnWidth = 300.0f;
}


NodeEditorWidget _NodeEditorWidget::create(GenomeTabEditData const& editData, GenomeTabLayoutData const& layoutData)
{
    return NodeEditorWidget(new _NodeEditorWidget(editData, layoutData));
}

void _NodeEditorWidget::process()
{
    if (ImGui::BeginChild("NodeEditor", ImVec2(0, 0))) {
        auto nodeIndex = _editData->getSelectedNodeIndex();
        if (nodeIndex.has_value()) {
            ImGui::PushID(_editData->selectedGeneIndex.value());
            ImGui::PushID(nodeIndex.value());
            processNodeAttributes();

            if (!isVoidNodeSelected()) {
                AlienGui::MovableHorizontalSeparator(AlienGui::MovableHorizontalSeparatorParameters().additive(false), _layoutData->neuralNetEditorHeight);

                processNeuralNetEditor();
            }
            ImGui::PopID();
            ImGui::PopID();
        } else {
            processNoSelection();
        }
    }
    ImGui::EndChild();
}

_NodeEditorWidget::_NodeEditorWidget(GenomeTabEditData const& editData, GenomeTabLayoutData const& layoutData)
    : _editData(editData)
    , _layoutData(layoutData)
{
    _neuralNetWidget = _NeuralNetEditorWidget::create();
}

namespace
{
    CellTypeGenomeDesc createCellTypeGenomeDesc(CellType cellType)
    {
        switch (cellType) {
        case CellType_Base:
            return BaseGenomeDesc();
        case CellType_Depot:
            return DepotGenomeDesc();
        case CellType_Sensor:
            return SensorGenomeDesc();
        case CellType_Generator:
            return GeneratorGenomeDesc();
        case CellType_Attacker:
            return AttackerGenomeDesc();
        case CellType_Injector:
            return InjectorGenomeDesc();
        case CellType_Muscle:
            return MuscleGenomeDesc();
        case CellType_Defender:
            return DefenderGenomeDesc();
        case CellType_Reconnector:
            return ReconnectorGenomeDesc();
        case CellType_Detonator:
            return DetonatorGenomeDesc();
        case CellType_Digestor:
            return DigestorGenomeDesc();
        case CellType_Memory:
            return MemoryGenomeDesc();
        case CellType_Communicator:
            return CommunicatorGenomeDesc();
        case CellType_Void:
            return VoidGenomeDesc();
        default:
            CHECK(false);
        }
    }

    MuscleModeGenomeDesc createMuscleModeGenomeDesc(MuscleMode mode)
    {
        switch (mode) {
        case MuscleMode_AutoBending:
            return AutoBendingGenomeDesc();
        case MuscleMode_ManualBending:
            return ManualBendingGenomeDesc();
        case MuscleMode_AngleBending:
            return AngleBendingGenomeDesc();
        case MuscleMode_AutoCrawling:
            return AutoCrawlingGenomeDesc();
        case MuscleMode_ManualCrawling:
            return ManualCrawlingGenomeDesc();
        case MuscleMode_DirectMovement:
            return DirectMovementGenomeDesc();
        default:
            CHECK(false);
        }
    }

    SensorModeGenomeDesc createSensorModeGenomeDesc(SensorMode mode)
    {
        switch (mode) {
        case SensorMode_DetectEnergy:
            return DetectEnergyGenomeDesc();
        case SensorMode_DetectSolid:
            return DetectSolidGenomeDesc();
        case SensorMode_DetectFreeCell:
            return DetectFreeCellGenomeDesc();
        case SensorMode_DetectCreature:
            return DetectCreatureGenomeDesc();
        default:
            CHECK(false);
        }
    }

    AttackerModeGenomeDesc createAttackerModeGenomeDesc(AttackerMode mode)
    {
        switch (mode) {
        case AttackerMode_FreeCell:
            return AttackFreeCellGenomeDesc();
        case AttackerMode_Creature:
            return AttackCreatureGenomeDesc();
        default:
            CHECK(false);
        }
    }

    GeneratorModeGenomeDesc createGeneratorModeGenomeDesc(GeneratorMode mode)
    {
        switch (mode) {
        case GeneratorMode_SquareSignal:
            return SquareSignalGenomeDesc();
        case GeneratorMode_SawtoothSignal:
            return SawtoothSignalGenomeDesc();
        default:
            CHECK(false);
        }
    }

    ReconnectorModeGenomeDesc createReconnectorModeGenomeDesc(ReconnectorMode mode)
    {
        switch (mode) {
        case ReconnectorMode_Solid:
            return ReconnectSolidGenomeDesc();
        case ReconnectorMode_FreeCell:
            return ReconnectFreeCellGenomeDesc();
        case ReconnectorMode_Creature:
            return ReconnectCreatureGenomeDesc();
        default:
            CHECK(false);
        }
    }

    MemoryModeGenomeDesc createMemoryModeGenomeDesc(MemoryMode mode)
    {
        switch (mode) {
        case MemoryMode_SignalDelay:
            return SignalDelayGenomeDesc();
        case MemoryMode_SignalRecorder:
            return SignalRecorderGenomeDesc();
        case MemoryMode_SignalStorage:
            return SignalStorageGenomeDesc();
        case MemoryMode_SignalIntegrator:
            return SignalIntegratorGenomeDesc();
        default:
            CHECK(false);
        }
    }

    CommunicatorModeGenomeDesc createCommunicatorModeGenomeDesc(CommunicatorMode mode)
    {
        switch (mode) {
        case CommunicatorMode_Sender:
            return SenderGenomeDesc();
        case CommunicatorMode_Receiver:
            return ReceiverGenomeDesc();
        default:
            CHECK(false);
        }
    }
}

bool _NodeEditorWidget::isVoidNodeSelected() const
{
    return _editData->getSelectedGeneRef().getCellType(_editData->getSelectedNodeIndex().value()) == CellType_Void;
}

void _NodeEditorWidget::processNodeAttributes()
{
    AlienGui::Group(AlienGui::GroupParameters()
                        .text("Selected: Node " + std::to_string(_editData->getSelectedNodeIndex().value()))
                        .label(GenomeEditorPresentation::format("node.selected", {{"index", std::to_string(_editData->getSelectedNodeIndex().value())}}))
                        .highlighted(true));

    float labelWidth = MinTextWidth;
    for (auto key :
         {"node.label.angle",
          "node.label.customization",
          "node.label.type",
          "node.label.construction",
          "node.label.autotriggerinterval",
          "node.label.constructionangle",
          "node.label.provideenergy",
          "node.label.separation",
          "node.label.numberofbranches",
          "node.label.concatenations",
          "node.label.maxenergyforstorage",
          "node.label.autotrigger",
          "node.label.tagforattackers",
          "node.label.mode",
          "node.label.mindensity",
          "node.label.restricttocolors",
          "node.label.mincreaturecells",
          "node.label.maxcreaturecells",
          "node.label.restricttolineage",
          "node.label.minrange",
          "node.label.maxrange",
          "node.label.additive",
          "node.label.minvalue",
          "node.label.maxvalue",
          "node.label.timeoffset",
          "node.label.period",
          "node.label.geneindex",
          "node.label.maxangledeviation",
          "node.label.forwardbackwardratio",
          "node.label.attractionrepulsionratio",
          "node.label.maxdistancedeviation",
          "node.label.countdown",
          "node.label.energyconductivity",
          "node.label.energyconversion",
          "node.label.delay",
          "node.label.readonly",
          "node.label.signalbuffer",
          "node.label.newsignalweight",
          "node.label.channelmaskbit03",
          "node.label.channelmaskbit47",
          "node.label.range",
          "node.label.oneway"}) {
        labelWidth = std::max(labelWidth, scaleInverse(ImGui::CalcTextSize(tr(key).c_str()).x) + 45.0f);
    }
    float widgetWidth = MaxWidgetWidth;
    for (auto const& values :
         {GenomeEditorPresentation::cellTypes(),
          GenomeEditorPresentation::provideEnergyGenomes(),
          GenomeEditorPresentation::sensorModes(),
          GenomeEditorPresentation::generatorModes(),
          GenomeEditorPresentation::attackerModes(),
          GenomeEditorPresentation::muscleModes(),
          GenomeEditorPresentation::defenderModes(),
          GenomeEditorPresentation::reconnectorModes(),
          GenomeEditorPresentation::memoryModes(),
          GenomeEditorPresentation::communicatorModes()}) {
        for (auto const& value : values) {
            widgetWidth = std::max(widgetWidth, scaleInverse(ImGui::CalcTextSize(value.c_str()).x) + 45.0f);
        }
    }
    auto columnWidth = std::max(MinColumnWidth, labelWidth + widgetWidth);
    if (ImGui::GetContentRegionAvail().x < scale(columnWidth)) {
        ImGui::SetNextWindowContentSize({scale(columnWidth), 0});
    }
    auto height = isVoidNodeSelected() ? 0.0f : -_layoutData->neuralNetEditorHeight;
    if (ImGui::BeginChild("NodeData", ImVec2(0, height), 0, ImGuiWindowFlags_HorizontalScrollbar)) {
        auto& gene = _editData->getSelectedGeneRef();
        auto& node = _editData->getSelectedNodeRef();

        // With homogeneous cell type the cell type and its properties of the first node are shown and edited for every node
        auto& cellTypeNode = gene._homogeneousCellType ? gene._nodes.front() : node;

        auto const simulationParameters = _SimulationFacade::get()->getSimulationParameters();
        auto const& customizationColors = simulationParameters.customizationColors.value;

        AlienGui::DynamicTableLayout table(columnWidth);
        if (table.begin()) {
            auto rightColumnWidth = std::max(labelWidth, scaleInverse(ImGui::GetContentRegionAvail().x - scale(widgetWidth)));

            // Angle
            AlienGui::Group(AlienGui::GroupParameters().text("Base properties").label(tr("node.group.base")));

            auto nodeIndex = _editData->getSelectedNodeIndex();
            auto isInnerNode = nodeIndex.value() != 0 && nodeIndex != gene._nodes.size() - 1;
            //if (!isInnerNode) {
            AlienGui::InputFloat(
                AlienGui::InputFloatParameters()
                    .name("Angle")
                    .label(tr("node.label.angle"))
                    .textWidth(rightColumnWidth)
                    .readOnly(isInnerNode)
                    .infoLabel(isInnerNode ? std::make_optional(std::string(tr("node.deduced"))) : std::nullopt)
                    .format("%.1f")
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::NodeAngle)),
                node._referenceAngle);
            //}

            AlienGui::ComboColor(
                AlienGui::ComboColorParameters()
                    .customizationColors(customizationColors)
                    .name("Customization")
                    .label(tr("node.label.customization"))
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::NodeCustomization)),
                node._color);

            // Type
            auto nodeType = gene.getCellType(nodeIndex.value());
            if (AlienGui::Combo(
                    AlienGui::ComboParameters()
                        .name("Type")
                        .label(tr("node.label.type"))
                        .values(Const::CellTypeStrings)
                        .displayValues(GenomeEditorPresentation::cellTypes())
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::CellType)),
                    nodeType)) {
                if (nodeType == CellType_Void && (gene._homogeneousCellType || nodeIndex.value() == 0)) {
                    GenericMessageDialog::get().localizedInformation("Error", "common.error", "genome.error.firstvoid");
                } else if (nodeType == CellType_Void && nodeIndex.value() == gene._nodes.size() - 1) {
                    GenericMessageDialog::get().localizedInformation("Error", "common.error", "genome.error.lastvoid");
                } else {
                    cellTypeNode._cellType = createCellTypeGenomeDesc(nodeType);
                    if (nodeType == CellType_Void) {
                        node._constructor = std::nullopt;
                    }
                }
            }

            // Void cells cannot have a constructor
            if (nodeType != CellType_Void) {

                // Construction gene combo
                std::vector<std::string> genes;
                std::vector<std::string> geneIds;
                geneIds.emplace_back("None");
                genes.emplace_back(tr("node.none"));
                for (auto const& [index, gene] : _editData->genome._genes | boost::adaptors::indexed(0)) {
                    auto text = std::to_string(index) + ": " + gene._name;
                    if (index == 0) {
                        text += tr("node.root");
                    }
                    geneIds.emplace_back(std::to_string(index) + ": " + gene._name + (index == 0 ? " (root)" : ""));
                    genes.emplace_back(text);
                }
                int constructionGeneIndex = node._constructor.has_value() ? node._constructor.value()._geneIndex + 1 : 0;
                if (AlienGui::Combo(
                        AlienGui::ComboParameters()
                            .name("Construction")
                            .label(tr("node.label.construction"))
                            .values(geneIds)
                            .displayValues(genes)
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::NodeConstructionGene)),
                        constructionGeneIndex)) {
                    if (constructionGeneIndex == 0) {
                        node._constructor = std::nullopt;
                    } else {
                        if (!node._constructor.has_value()) {
                            node._constructor = ConstructorGenomeDesc();
                        }
                        node._constructor.value()._geneIndex = constructionGeneIndex - 1;
                    }
                }

                AlienGui::Group(AlienGui::GroupParameters().text("Construction properties").label(tr("node.group.construction")));

                if (node._constructor.has_value()) {
                    ImGui::PushID("Constructor");
                    AlienGui::BeginIndent();
                    auto& constructor = node._constructor.value();

                    // Auto activation interval
                    AlienGui::InputOptionalInt(
                        AlienGui::InputIntParameters()
                            .name("Auto trigger interval")
                            .label(tr("node.label.autotriggerinterval"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ConstructorAutoTriggerInterval)),
                        constructor._autoTriggerInterval);

                    // Construction angle
                    AlienGui::InputFloat(
                        AlienGui::InputFloatParameters()
                            .name("Construction angle")
                            .label(tr("node.label.constructionangle"))
                            .textWidth(rightColumnWidth)
                            .format("%.1f")
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ConstructorConstructionAngle)),
                        constructor._constructionAngle);

                    // Provide energy
                    int provideEnergy = constructor._provideEnergy;
                    if (AlienGui::Combo(
                            AlienGui::ComboParameters()
                                .name("Provide energy")
                                .label(tr("node.label.provideenergy"))
                                .textWidth(rightColumnWidth)
                                .values(Const::ProvideEnergyGenomeStrings)
                                .displayValues(GenomeEditorPresentation::provideEnergyGenomes())
                                .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ConstructorProvideEnergy)),
                            provideEnergy)) {
                        constructor._provideEnergy = static_cast<ProvideEnergyGenome>(provideEnergy);
                    }

                    // Separation
                    AlienGui::Checkbox(
                        AlienGui::CheckboxParameters()
                            .name("Separation")
                            .label(tr("node.label.separation"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ConstructorSeparation)),
                        &constructor._separation);

                    // Number of branches
                    AlienGui::BeginIndent();
                    if (!constructor._separation) {
                        auto numBranches = constructor._numBranches - 1;
                        AlienGui::Switcher(
                            AlienGui::SwitcherParameters()
                                .name("Number of branches")
                                .label(tr("node.label.numberofbranches"))
                                .values({"1", "2", "3", "4", "5", "6"})
                                .textWidth(rightColumnWidth)
                                .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ConstructorNumBranches)),
                            &numBranches);
                        constructor._numBranches = numBranches + 1;
                    }
                    AlienGui::EndIndent();

                    // Concatenations
                    AlienGui::InputInt(
                        AlienGui::InputIntParameters()
                            .name("Concatenations")
                            .label(tr("node.label.concatenations"))
                            .infinity(true)
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ConstructorNumConcatenations)),
                        constructor._numConcatenations);

                    AlienGui::EndIndent();
                    ImGui::PopID();
                }
            }

            table.next();

            AlienGui::Group(AlienGui::GroupParameters().text("Type-specific properties").label(tr("node.group.typespecific")));

            if (nodeType == CellType_Base) {
            } else if (nodeType == CellType_Depot) {
                auto& depot = std::get<DepotGenomeDesc>(cellTypeNode._cellType);
                AlienGui::InputFloat(
                    AlienGui::InputFloatParameters()
                        .name("Max energy for storage")
                        .label(tr("node.label.maxenergyforstorage"))
                        .format("%.1f")
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::DepotStorageLimit)),
                    depot._storageLimit);
            } else if (nodeType == CellType_Sensor) {

                ImGui::PushID("Sensor");

                // Auto activation interval
                auto& sensor = std::get<SensorGenomeDesc>(cellTypeNode._cellType);
                AlienGui::Checkbox(
                    AlienGui::CheckboxParameters()
                        .name("Auto trigger")
                        .label(tr("node.label.autotrigger"))
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorAutoTrigger)),
                    &sensor._autoTrigger);
                AlienGui::Checkbox(
                    AlienGui::CheckboxParameters()
                        .name("Tag for attackers")
                        .label(tr("node.label.tagforattackers"))
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorTagForAttackers)),
                    &sensor._tagForAttackers);

                // Mode selection
                auto mode = sensor.getMode();
                if (AlienGui::Combo(
                        AlienGui::ComboParameters()
                            .name("Mode")
                            .label(tr("node.label.mode"))
                            .values(Const::SensorModeStrings)
                            .displayValues(GenomeEditorPresentation::sensorModes())
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorMode)),
                        mode)) {
                    sensor._mode = createSensorModeGenomeDesc(mode);
                }

                // Mode-specific parameters
                if (mode == SensorMode_DetectSolid) {
                    // No parameters
                } else if (mode == SensorMode_DetectEnergy) {
                    AlienGui::BeginIndent();
                    auto& detectEnergy = std::get<DetectEnergyGenomeDesc>(sensor._mode);
                    AlienGui::InputFloat(
                        AlienGui::InputFloatParameters()
                            .name("Min density")
                            .label(tr("node.label.mindensity"))
                            .step(0.05f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorEnergyMinDensity)),
                        detectEnergy._minDensity);
                    AlienGui::EndIndent();
                } else if (mode == SensorMode_DetectFreeCell) {
                    AlienGui::BeginIndent();
                    auto& detectFreeCell = std::get<DetectFreeCellGenomeDesc>(sensor._mode);
                    AlienGui::InputFloat(
                        AlienGui::InputFloatParameters()
                            .name("Min density")
                            .label(tr("node.label.mindensity"))
                            .step(0.05f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorFreeCellMinDensity)),
                        detectFreeCell._minDensity);
                    AlienGui::ColorCheckboxes(
                        AlienGui::ColorCheckboxesParameters()
                            .customizationColors(customizationColors)
                            .name("Restrict to colors")
                            .label(tr("node.label.restricttocolors"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorRestrictToColors)),
                        detectFreeCell._restrictToColors);
                    AlienGui::EndIndent();
                } else if (mode == SensorMode_DetectCreature) {
                    AlienGui::BeginIndent();
                    auto& detectCreature = std::get<DetectCreatureGenomeDesc>(sensor._mode);
                    AlienGui::InputOptionalInt(
                        AlienGui::InputIntParameters()
                            .name("Min creature cells")
                            .label(tr("node.label.mincreaturecells"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorMinNumCells)),
                        detectCreature._minNumCells);
                    AlienGui::InputOptionalInt(
                        AlienGui::InputIntParameters()
                            .name("Max creature cells")
                            .label(tr("node.label.maxcreaturecells"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorMaxNumCells)),
                        detectCreature._maxNumCells);
                    AlienGui::ColorCheckboxes(
                        AlienGui::ColorCheckboxesParameters()
                            .customizationColors(customizationColors)
                            .name("Restrict to colors")
                            .label(tr("node.label.restricttocolors"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorRestrictToColors)),
                        detectCreature._restrictToColors);
                    AlienGui::Combo(
                        AlienGui::ComboParameters()
                            .name("Restrict to lineage")
                            .label(tr("node.label.restricttolineage"))
                            .values({"No", "Same lineage", "Other lineage"})
                            .displayValues({tr("node.option.lineage.any"), tr("node.option.lineage.related"), tr("node.option.lineage.unrelated")})
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorRestrictToLineage)),
                        detectCreature._restrictToLineage);
                    AlienGui::EndIndent();
                }

                // Minimum range
                AlienGui::SliderInt(
                    AlienGui::SliderIntParameters()
                        .name("Min range")
                        .label(tr("node.label.minrange"))
                        .min(Const::SensorRange_Min)
                        .max(Const::SensorRange_Max)
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorMinRange)),
                    &sensor._minRange);

                // Maximum range
                AlienGui::SliderInt(
                    AlienGui::SliderIntParameters()
                        .name("Max range")
                        .label(tr("node.label.maxrange"))
                        .min(Const::SensorRange_Min)
                        .max(Const::SensorRange_Max)
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::SensorMaxRange)),
                    &sensor._maxRange);

                ImGui::PopID();

            } else if (nodeType == CellType_Generator) {

                auto& generator = std::get<GeneratorGenomeDesc>(cellTypeNode._cellType);

                // Additive
                AlienGui::Checkbox(
                    AlienGui::CheckboxParameters()
                        .name("Additive")
                        .label(tr("node.label.additive"))
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneratorAdditive)),
                    &generator._additive);

                // Value range
                AlienGui::InputFloat(
                    AlienGui::InputFloatParameters()
                        .name("Min value")
                        .label(tr("node.label.minvalue"))
                        .format("%.2f")
                        .step(0.05f)
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneratorMinValue)),
                    generator._minValue);
                AlienGui::InputFloat(
                    AlienGui::InputFloatParameters()
                        .name("Max value")
                        .label(tr("node.label.maxvalue"))
                        .format("%.2f")
                        .step(0.05f)
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneratorMaxValue)),
                    generator._maxValue);

                // Time offset
                AlienGui::InputInt(
                    AlienGui::InputIntParameters()
                        .name("Time offset")
                        .label(tr("node.label.timeoffset"))
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneratorTimeOffset)),
                    generator._timeOffset);

                // Mode
                auto mode = generator.getMode();
                if (AlienGui::Combo(
                        AlienGui::ComboParameters()
                            .name("Mode")
                            .label(tr("node.label.mode"))
                            .values(Const::GeneratorModeStrings)
                            .displayValues(GenomeEditorPresentation::generatorModes())
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneratorMode)),
                        mode)) {
                    generator._mode = createGeneratorModeGenomeDesc(mode);
                }

                if (mode == GeneratorMode_SquareSignal) {
                    AlienGui::BeginIndent();

                    auto& squareSignal = std::get<SquareSignalGenomeDesc>(generator._mode);
                    AlienGui::InputInt(
                        AlienGui::InputIntParameters()
                            .name("Period")
                            .label(tr("node.label.period"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneratorPeriod)),
                        squareSignal._period);

                    AlienGui::EndIndent();
                } else if (mode == GeneratorMode_SawtoothSignal) {
                    AlienGui::BeginIndent();

                    auto& sawtoothSignal = std::get<SawtoothSignalGenomeDesc>(generator._mode);
                    AlienGui::InputInt(
                        AlienGui::InputIntParameters()
                            .name("Period")
                            .label(tr("node.label.period"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneratorPeriod)),
                        sawtoothSignal._period);

                    AlienGui::EndIndent();
                }

            } else if (nodeType == CellType_Attacker) {

                auto& attacker = std::get<AttackerGenomeDesc>(cellTypeNode._cellType);
                auto mode = attacker.getMode();
                if (AlienGui::Combo(
                        AlienGui::ComboParameters()
                            .name("Mode")
                            .label(tr("node.label.mode"))
                            .values(Const::AttackerModeStrings)
                            .displayValues(GenomeEditorPresentation::attackerModes())
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::AttackerMode)),
                        mode)) {
                    attacker._mode = createAttackerModeGenomeDesc(mode);
                }

                if (mode == AttackerMode_FreeCell) {
                    AlienGui::BeginIndent();

                    auto& attackFreeCell = std::get<AttackFreeCellGenomeDesc>(attacker._mode);
                    AlienGui::ColorCheckboxes(
                        AlienGui::ColorCheckboxesParameters()
                            .customizationColors(customizationColors)
                            .name("Restrict to colors")
                            .label(tr("node.label.restricttocolors"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::AttackerRestrictToColors)),
                        attackFreeCell._restrictToColors);

                    AlienGui::EndIndent();
                }
            } else if (nodeType == CellType_Injector) {

                // Gene index
                auto& injector = std::get<InjectorGenomeDesc>(cellTypeNode._cellType);
                AlienGui::InputInt(
                    AlienGui::InputIntParameters()
                        .name("Gene index")
                        .label(tr("node.label.geneindex"))
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::InjectorGeneIndex)),
                    injector._geneIndex);

            } else if (nodeType == CellType_Muscle) {

                // Mode
                auto& muscle = std::get<MuscleGenomeDesc>(cellTypeNode._cellType);
                auto mode = muscle.getMode();
                if (AlienGui::Combo(
                        AlienGui::ComboParameters()
                            .name("Mode")
                            .label(tr("node.label.mode"))
                            .values(Const::MuscleModeStrings)
                            .displayValues(GenomeEditorPresentation::muscleModes())
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MuscleMode)),
                        mode)) {
                    muscle._mode = createMuscleModeGenomeDesc(mode);
                }

                if (mode == MuscleMode_AutoBending) {
                    AlienGui::BeginIndent();

                    // Max angle deviation
                    auto& autoBending = std::get<AutoBendingGenomeDesc>(muscle._mode);
                    AlienGui::SliderFloat(
                        AlienGui::SliderFloatParameters()
                            .name("Max angle deviation")
                            .label(tr("node.label.maxangledeviation"))
                            .min(0.0f)
                            .max(1.0f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MuscleMaxAngleDeviation)),
                        &autoBending._maxAngleDeviation);

                    // Front back ratio
                    AlienGui::SliderFloat(
                        AlienGui::SliderFloatParameters()
                            .name("Forward backward ratio")
                            .label(tr("node.label.forwardbackwardratio"))
                            .min(0.0f)
                            .max(1.0f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MuscleForwardBackwardRatio)),
                        &autoBending._forwardBackwardRatio);

                    AlienGui::EndIndent();

                } else if (mode == MuscleMode_ManualBending) {
                    AlienGui::BeginIndent();

                    // Max angle deviation
                    auto& manualBending = std::get<ManualBendingGenomeDesc>(muscle._mode);
                    AlienGui::SliderFloat(
                        AlienGui::SliderFloatParameters()
                            .name("Max angle deviation")
                            .label(tr("node.label.maxangledeviation"))
                            .min(0.0f)
                            .max(1.0f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MuscleMaxAngleDeviation)),
                        &manualBending._maxAngleDeviation);

                    // Front back ratio
                    AlienGui::SliderFloat(
                        AlienGui::SliderFloatParameters()
                            .name("Forward backward ratio")
                            .label(tr("node.label.forwardbackwardratio"))
                            .min(0.0f)
                            .max(1.0f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MuscleForwardBackwardRatio)),
                        &manualBending._forwardBackwardRatio);

                    AlienGui::EndIndent();

                } else if (mode == MuscleMode_AngleBending) {
                    AlienGui::BeginIndent();

                    // Max angle deviation
                    auto& angleBending = std::get<AngleBendingGenomeDesc>(muscle._mode);
                    AlienGui::SliderFloat(
                        AlienGui::SliderFloatParameters()
                            .name("Max angle deviation")
                            .label(tr("node.label.maxangledeviation"))
                            .min(0.0f)
                            .max(1.0f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MuscleMaxAngleDeviation)),
                        &angleBending._maxAngleDeviation);

                    // Front back ratio
                    AlienGui::InputFloat(
                        AlienGui::InputFloatParameters()
                            .name("Attraction repulsion ratio")
                            .label(tr("node.label.attractionrepulsionratio"))
                            .format("%.2f")
                            .step(0.05f)
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MuscleAttractionRepulsionRatio)),
                        angleBending._attractionRepulsionRatio);

                    AlienGui::EndIndent();

                } else if (mode == MuscleMode_AutoCrawling) {
                    AlienGui::BeginIndent();

                    // Max angle deviation
                    auto& autoCrawling = std::get<AutoCrawlingGenomeDesc>(muscle._mode);
                    AlienGui::SliderFloat(
                        AlienGui::SliderFloatParameters()
                            .name("Max distance deviation")
                            .label(tr("node.label.maxdistancedeviation"))
                            .min(0.0f)
                            .max(1.0f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MuscleMaxDistanceDeviation)),
                        &autoCrawling._maxDistanceDeviation);

                    // Front back ratio
                    AlienGui::SliderFloat(
                        AlienGui::SliderFloatParameters()
                            .name("Forward backward ratio")
                            .label(tr("node.label.forwardbackwardratio"))
                            .min(0.0f)
                            .max(1.0f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MuscleForwardBackwardRatio)),
                        &autoCrawling._forwardBackwardRatio);

                    AlienGui::EndIndent();

                } else if (mode == MuscleMode_ManualCrawling) {
                    AlienGui::BeginIndent();

                    // Max angle deviation
                    auto& manualCrawling = std::get<ManualCrawlingGenomeDesc>(muscle._mode);
                    AlienGui::SliderFloat(
                        AlienGui::SliderFloatParameters()
                            .name("Max distance deviation")
                            .label(tr("node.label.maxdistancedeviation"))
                            .min(0.0f)
                            .max(1.0f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MuscleMaxDistanceDeviation)),
                        &manualCrawling._maxDistanceDeviation);

                    // Front back ratio
                    AlienGui::SliderFloat(
                        AlienGui::SliderFloatParameters()
                            .name("Forward backward ratio")
                            .label(tr("node.label.forwardbackwardratio"))
                            .min(0.0f)
                            .max(1.0f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MuscleForwardBackwardRatio)),
                        &manualCrawling._forwardBackwardRatio);

                    AlienGui::EndIndent();

                } else if (mode == MuscleMode_DirectMovement) {
                }

            } else if (nodeType == CellType_Defender) {

                // Defender mode
                auto& defender = std::get<DefenderGenomeDesc>(cellTypeNode._cellType);
                AlienGui::Combo(
                    AlienGui::ComboParameters()
                        .name("Mode")
                        .label(tr("node.label.mode"))
                        .values(Const::DefenderModeStrings)
                        .displayValues(GenomeEditorPresentation::defenderModes())
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::DefenderMode)),
                    defender._mode);

            } else if (nodeType == CellType_Reconnector) {

                // Mode selection
                auto& reconnector = std::get<ReconnectorGenomeDesc>(cellTypeNode._cellType);
                auto mode = reconnector.getMode();
                if (AlienGui::Combo(
                        AlienGui::ComboParameters()
                            .name("Mode")
                            .label(tr("node.label.mode"))
                            .values(Const::ReconnectorModeStrings)
                            .displayValues(GenomeEditorPresentation::reconnectorModes())
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ReconnectorMode)),
                        mode)) {
                    reconnector._mode = createReconnectorModeGenomeDesc(mode);
                }

                // Mode-specific parameters
                if (mode == ReconnectorMode_Solid) {
                    // No parameters
                } else if (mode == ReconnectorMode_FreeCell) {
                    AlienGui::BeginIndent();
                    auto& freeCell = std::get<ReconnectFreeCellGenomeDesc>(reconnector._mode);
                    AlienGui::ColorCheckboxes(
                        AlienGui::ColorCheckboxesParameters()
                            .customizationColors(customizationColors)
                            .name("Restrict to colors")
                            .label(tr("node.label.restricttocolors"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ReconnectorRestrictToColors)),
                        freeCell._restrictToColors);
                    AlienGui::EndIndent();
                } else if (mode == ReconnectorMode_Creature) {
                    AlienGui::BeginIndent();
                    auto& creature = std::get<ReconnectCreatureGenomeDesc>(reconnector._mode);
                    AlienGui::InputOptionalInt(
                        AlienGui::InputIntParameters()
                            .name("Min creature cells")
                            .label(tr("node.label.mincreaturecells"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ReconnectorMinNumCells)),
                        creature._minNumCells);
                    AlienGui::InputOptionalInt(
                        AlienGui::InputIntParameters()
                            .name("Max creature cells")
                            .label(tr("node.label.maxcreaturecells"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ReconnectorMaxNumCells)),
                        creature._maxNumCells);
                    AlienGui::ColorCheckboxes(
                        AlienGui::ColorCheckboxesParameters()
                            .customizationColors(customizationColors)
                            .name("Restrict to colors")
                            .label(tr("node.label.restricttocolors"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ReconnectorRestrictToColors)),
                        creature._restrictToColors);
                    AlienGui::Combo(
                        AlienGui::ComboParameters()
                            .name("Restrict to lineage")
                            .label(tr("node.label.restricttolineage"))
                            .values({"No", "Same lineage", "Other lineage"})
                            .displayValues({tr("node.option.lineage.any"), tr("node.option.lineage.related"), tr("node.option.lineage.unrelated")})
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::ReconnectorRestrictToLineage)),
                        creature._restrictToLineage);
                    AlienGui::EndIndent();
                }

            } else if (nodeType == CellType_Detonator) {

                // Countdown
                auto& detonator = std::get<DetonatorGenomeDesc>(cellTypeNode._cellType);
                AlienGui::InputInt(
                    AlienGui::InputIntParameters()
                        .name("Countdown")
                        .label(tr("node.label.countdown"))
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::DetonatorCountdown)),
                    detonator._countdown);
            } else if (nodeType == CellType_Digestor) {

                auto& digestor = std::get<DigestorGenomeDesc>(cellTypeNode._cellType);
                AlienGui::SliderFloat(
                    AlienGui::SliderFloatParameters()
                        .name("Energy conductivity")
                        .label(tr("node.label.energyconductivity"))
                        .max(1.0f)
                        .format("%.2f")
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::DigestorEnergyConductivity)),
                    &digestor._rawEnergyConductivity);
                auto rawEnergyConversionRate = digestor.getRawEnergyConversionRate();
                AlienGui::SliderFloat(
                    AlienGui::SliderFloatParameters()
                        .name("Energy conversion")
                        .label(tr("node.label.energyconversion"))
                        .max(1.0f)
                        .format("%.2f")
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::DigestorEnergyConversion)),
                    &rawEnergyConversionRate);
                digestor.setRawEnergyConversionRate(rawEnergyConversionRate);
            } else if (nodeType == CellType_Memory) {

                // Mode selection
                auto& memory = std::get<MemoryGenomeDesc>(cellTypeNode._cellType);
                auto mode = memory.getMode();
                if (AlienGui::Combo(
                        AlienGui::ComboParameters()
                            .name("Mode")
                            .label(tr("node.label.mode"))
                            .values(Const::MemoryModeStrings)
                            .displayValues(GenomeEditorPresentation::memoryModes())
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MemoryMode)),
                        mode)) {
                    memory._mode = createMemoryModeGenomeDesc(mode);
                    if (mode == MemoryMode_SignalRecorder || mode == MemoryMode_SignalStorage) {
                        memory._signalEntries.resize(8, SignalEntryGenomeDesc());
                    }
                }

                // Mode-specific parameters
                if (mode == MemoryMode_SignalDelay) {
                    AlienGui::BeginIndent();
                    auto& signalDelay = std::get<SignalDelayGenomeDesc>(memory._mode);
                    AlienGui::InputInt(
                        AlienGui::InputIntParameters()
                            .name("Delay")
                            .label(tr("node.label.delay"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MemoryDelay)),
                        signalDelay._delay);
                    AlienGui::EndIndent();
                } else if (mode == MemoryMode_SignalRecorder) {
                    AlienGui::BeginIndent();
                    auto& signalRecorder = std::get<SignalRecorderGenomeDesc>(memory._mode);
                    AlienGui::Checkbox(
                        AlienGui::CheckboxParameters()
                            .name("Read only")
                            .label(tr("node.label.readonly"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MemoryReadOnly)),
                        &signalRecorder._readOnly);
                    if (AlienGui::Button(AlienGui::ButtonParameters()
                                             .buttonText("Edit")
                                             .buttonLabel(tr("node.edit"))
                                             .name("Signal buffer")
                                             .label(tr("node.label.signalbuffer"))
                                             .textWidth(rightColumnWidth)
                                             .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MemorySignalBuffer)))) {
                        SignalsBufferDialog::get().open(
                            memory._signalEntries, [&memory](std::vector<SignalEntryGenomeDesc> const& entries) { memory._signalEntries = entries; });
                    }
                    AlienGui::EndIndent();
                } else if (mode == MemoryMode_SignalStorage) {
                    AlienGui::BeginIndent();
                    auto& signalStorage = std::get<SignalStorageGenomeDesc>(memory._mode);
                    AlienGui::Checkbox(
                        AlienGui::CheckboxParameters()
                            .name("Read only")
                            .label(tr("node.label.readonly"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MemoryReadOnly)),
                        &signalStorage._readOnly);
                    if (AlienGui::Button(AlienGui::ButtonParameters()
                                             .buttonText("Edit")
                                             .buttonLabel(tr("node.edit"))
                                             .name("Signal buffer")
                                             .label(tr("node.label.signalbuffer"))
                                             .textWidth(rightColumnWidth)
                                             .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MemorySignalBuffer)))) {
                        SignalsBufferDialog::get().open(
                            memory._signalEntries, [&memory](std::vector<SignalEntryGenomeDesc> const& entries) { memory._signalEntries = entries; });
                    }
                    AlienGui::EndIndent();
                } else if (mode == MemoryMode_SignalIntegrator) {
                    AlienGui::BeginIndent();
                    auto& signalIntegrator = std::get<SignalIntegratorGenomeDesc>(memory._mode);
                    AlienGui::SliderFloat(
                        AlienGui::SliderFloatParameters()
                            .name("New signal weight")
                            .label(tr("node.label.newsignalweight"))
                            .max(1.0f)
                            .format("%.2f")
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MemoryNewSignalWeight)),
                        &signalIntegrator._newSignalWeight);
                    AlienGui::EndIndent();
                }

                bool bit[STANDARD_NEURONS_PER_CELL];
                for (int i = 0; i < STANDARD_NEURONS_PER_CELL; ++i) {
                    bit[i] = (memory._channelBitMask & (1 << i)) != 0;
                }
                AlienGui::MultiCheckboxes(
                    AlienGui::MultiCheckboxesParameters()
                        .name("Channel mask bit 0-3")
                        .label(tr("node.label.channelmaskbit03"))
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MemoryChannelMask)),
                    bit[0],
                    bit[1],
                    bit[2],
                    bit[3]);
                AlienGui::MultiCheckboxes(
                    AlienGui::MultiCheckboxesParameters()
                        .name("Channel mask bit 4-7")
                        .label(tr("node.label.channelmaskbit47"))
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::MemoryChannelMask)),
                    bit[4],
                    bit[5],
                    bit[6],
                    bit[7]);
                memory._channelBitMask = 0;
                for (int i = 0; i < STANDARD_NEURONS_PER_CELL; ++i) {
                    if (bit[i]) {
                        memory._channelBitMask |= 1 << i;
                    }
                }

            } else if (nodeType == CellType_Communicator) {

                // Mode selection
                auto& communicator = std::get<CommunicatorGenomeDesc>(cellTypeNode._cellType);
                auto mode = communicator.getMode();
                if (AlienGui::Combo(
                        AlienGui::ComboParameters()
                            .name("Mode")
                            .label(tr("node.label.mode"))
                            .values(Const::CommunicatorModeStrings)
                            .displayValues(GenomeEditorPresentation::communicatorModes())
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::CommunicatorMode)),
                        mode)) {
                    communicator._mode = createCommunicatorModeGenomeDesc(mode);
                }

                // Mode-specific parameters
                if (mode == CommunicatorMode_Sender) {
                    AlienGui::BeginIndent();
                    auto& sender = std::get<SenderGenomeDesc>(communicator._mode);
                    AlienGui::SliderInt(
                        AlienGui::SliderIntParameters()
                            .name("Range")
                            .label(tr("node.label.range"))
                            .min(0)
                            .max(20)
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::CommunicatorRange)),
                        &sender._range);
                    AlienGui::Checkbox(
                        AlienGui::CheckboxParameters()
                            .name("One-way")
                            .label(tr("node.label.oneway"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::CommunicatorOneway)),
                        &sender._oneway);
                    AlienGui::EndIndent();
                } else if (mode == CommunicatorMode_Receiver) {
                    AlienGui::BeginIndent();
                    auto& receiver = std::get<ReceiverGenomeDesc>(communicator._mode);
                    AlienGui::ColorCheckboxes(
                        AlienGui::ColorCheckboxesParameters()
                            .customizationColors(customizationColors)
                            .name("Restrict to colors")
                            .label(tr("node.label.restricttocolors"))
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::CommunicatorRestrictToColors)),
                        receiver._restrictToColors);
                    AlienGui::Combo(
                        AlienGui::ComboParameters()
                            .name("Restrict to lineage")
                            .label(tr("node.label.restricttolineage"))
                            .values({"No", "Same lineage", "Other lineage"})
                            .displayValues({tr("node.option.lineage.any"), tr("node.option.lineage.related"), tr("node.option.lineage.unrelated")})
                            .textWidth(rightColumnWidth)
                            .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::CommunicatorRestrictToLineage)),
                        receiver._restrictToLineage);

                    AlienGui::EndIndent();
                }
            }

            table.next();
            table.end();
        }
    }
    ImGui::EndChild();
}

void _NodeEditorWidget::processNoSelection()
{
    AlienGui::Group(AlienGui::GroupParameters().text("Nothing selected").label(tr("node.group.nothing")));
    if (ImGui::BeginChild("overlay", ImVec2(0, 0), 0)) {
        auto startPos = ImGui::GetCursorScreenPos();
        auto size = ImGui::GetContentRegionAvail();
        AlienGui::DisabledField();
        auto text = tr("node.noselection");
        auto textSize = ImGui::CalcTextSize(text.c_str());
        ImVec2 textPos(startPos.x + size.x / 2 - textSize.x / 2, startPos.y + size.y / 2 - textSize.y / 2);
        ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), text.c_str());
    }
    ImGui::EndChild();
}

void _NodeEditorWidget::processNeuralNetEditor()
{
    AlienGui::MoveTickUp();
    AlienGui::MoveTickUp();
    if (AlienGui::Group(AlienGui::GroupParameters().text("Neural network").label(tr("neural.title")).highlighted(true).expandButton(true))) {
        _neuralNetWidget->openDialog();
    }

    auto& node = _editData->getSelectedNodeRef();
    _neuralNetWidget->process(
        node._neuralNetwork._weights,
        node._neuralNetwork._biases,
        node._neuralNetwork._activationFunctions,
        node._neuralNetwork._connectionWeights,
        CellFunctionChannels::getModules(node));
}
