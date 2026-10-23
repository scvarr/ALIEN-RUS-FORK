#include "CreatorWidget.h"

#include <imgui.h>

#include <Engine/Interface/SimulationFacade.h>

#include "AlienGui.h"
#include "EditorModel.h"
#include "HelpStrings.h"
#include "InspectionPresentation.h"
#include "WorldToolPresentation.h"

namespace
{
    auto constexpr PointButtonWidth = 70.0f;
}

void CreatorWidget::process()
{
    auto& controller = CreatorController::get();
    auto parameters = controller.getParameters();
    auto tool = EditorModel::get().getTool();

    processColorWidget();
    if (tool == EditTool_Freehand) {
        processPencilWidget(parameters);
    }
    processMaterialWidgets(parameters);
    processShapeWidgets(parameters);
    if (tool != EditTool_Object && tool != EditTool_Freehand) {
        processObjectDistanceWidget(parameters);
    }
    if (tool != EditTool_Object) {
        processStickyWidget(parameters);
    }
    processStaticWidget(parameters);
    controller.setParameters(parameters);

    if (tool == EditTool_Line || tool == EditTool_Curve || tool == EditTool_Polygon) {
        processPointButtons();
    }
}

void CreatorWidget::processColorWidget()
{
    auto color = EditorModel::get().getDefaultColorCode();
    AlienGui::ComboColor(
        AlienGui::ComboColorParameters()
            .customizationColors(_SimulationFacade::get()->getSimulationParameters().customizationColors.value)
            .name("Color")
            .label(tr("parameters.color.color"))
            .textWidth(WorldToolPresentation::creatorTextWidth())
            .tooltip(InspectionPresentation::attributeDescription(EntityAttribute::Color)),
        color);
    EditorModel::get().setDefaultColorCode(color);
}

void CreatorWidget::processPencilWidget(CreatorParameters& parameters)
{
    AlienGui::SliderFloat(
        AlienGui::SliderFloatParameters()
            .name("Pencil radius")
            .label(tr("worldtools.pencilradius"))
            .min(1.0f)
            .max(8.0f)
            .textWidth(WorldToolPresentation::creatorTextWidth())
            .format("%.1f")
            .tooltip(tr("worldtools.help.creatorpencilradiustooltip")),
        &parameters.pencilWidth);
}

void CreatorWidget::processMaterialWidgets(CreatorParameters& parameters)
{
    AlienGui::Switcher(
        AlienGui::SwitcherParameters()
            .name("Material")
            .label(tr("worldtools.material"))
            .textWidth(WorldToolPresentation::creatorTextWidth())
            .values({"Solid", "Fluid", "Free cells", "Energy particles"})
            .displayValues({tr("inspection.label.solid"), tr("inspection.label.fluid"), tr("worldtools.freecells"), tr("worldtools.energyparticles")})
            .tooltip(tr("worldtools.help.creatormaterialtooltip")),
        &parameters.material);
    AlienGui::InputFloat(
        AlienGui::InputFloatParameters()
            .name("Energy")
            .label(tr("new.energy"))
            .format("%.2f")
            .textWidth(WorldToolPresentation::creatorTextWidth())
            .tooltip(tr("worldtools.help.creatorenergytooltip")),
        parameters.energy);
    if (parameters.material == CreationMaterial_Fluid) {
        AlienGui::SliderFloat(
            AlienGui::SliderFloatParameters()
                .name("Glow")
                .label(tr("parameters.visualization.glow.name"))
                .min(0)
                .max(1.0f)
                .format("%.2f")
                .textWidth(WorldToolPresentation::creatorTextWidth())
                .tooltip(InspectionPresentation::attributeDescription(EntityAttribute::FluidGlow)),
            &parameters.glow);
    }
    if (parameters.material != CreationMaterial_EnergyParticle && parameters.material != CreationMaterial_Fluid) {
        AlienGui::SliderFloat(
            AlienGui::SliderFloatParameters()
                .name("Stiffness")
                .label(tr("genome.field.gene.stiffness"))
                .max(1.0f)
                .min(0.0f)
                .textWidth(WorldToolPresentation::creatorTextWidth())
                .tooltip(InspectionPresentation::attributeDescription(EntityAttribute::Stiffness)),
            &parameters.stiffness);
    }
}

void CreatorWidget::processShapeWidgets(CreatorParameters& parameters)
{
    auto tool = EditorModel::get().getTool();
    if (tool == EditTool_Rectangle) {
        AlienGui::InputInt(
            AlienGui::InputIntParameters()
                .name("Horizontal objects")
                .label(tr("worldtools.horizontalobjects"))
                .textWidth(WorldToolPresentation::creatorTextWidth())
                .tooltip(tr("worldtools.help.creatorrectanglewidthtooltip")),
            parameters.rectHorizontalObjects);
        AlienGui::InputInt(
            AlienGui::InputIntParameters()
                .name("Vertical objects")
                .label(tr("worldtools.verticalobjects"))
                .textWidth(WorldToolPresentation::creatorTextWidth())
                .tooltip(tr("worldtools.help.creatorrectangleheighttooltip")),
            parameters.rectVerticalObjects);
    }
    if (tool == EditTool_Hexagon) {
        AlienGui::InputInt(
            AlienGui::InputIntParameters()
                .name("Layers")
                .label(tr("worldtools.layers"))
                .textWidth(WorldToolPresentation::creatorTextWidth())
                .tooltip(tr("worldtools.help.creatorhexagonlayerstooltip")),
            parameters.layers);
    }
    if (tool == EditTool_Disc) {
        AlienGui::InputFloat(
            AlienGui::InputFloatParameters()
                .name("Outer radius")
                .label(tr("worldtools.outerradius"))
                .textWidth(WorldToolPresentation::creatorTextWidth())
                .format("%.0f")
                .tooltip(tr("worldtools.help.creatordiscouterradiustooltip")),
            parameters.outerRadius);
        AlienGui::InputFloat(
            AlienGui::InputFloatParameters()
                .name("Inner radius")
                .label(tr("worldtools.innerradius"))
                .textWidth(WorldToolPresentation::creatorTextWidth())
                .format("%.0f")
                .tooltip(tr("worldtools.help.creatordiscinnerradiustooltip")),
            parameters.innerRadius);
    }
}

void CreatorWidget::processObjectDistanceWidget(CreatorParameters& parameters)
{
    AlienGui::InputFloat(
        AlienGui::InputFloatParameters()
            .name("Object distance")
            .label(tr("worldtools.objectdistance"))
            .format("%.2f")
            .step(0.1)
            .textWidth(WorldToolPresentation::creatorTextWidth())
            .tooltip(tr("worldtools.help.creatordistancetooltip")),
        parameters.objectDistance);
}

void CreatorWidget::processStickyWidget(CreatorParameters& parameters)
{
    AlienGui::Checkbox(
        AlienGui::CheckboxParameters()
            .name("Sticky")
            .label(tr("inspection.label.sticky"))
            .textWidth(WorldToolPresentation::creatorTextWidth())
            .tooltip(InspectionPresentation::attributeDescription(EntityAttribute::Sticky)),
        &parameters.sticky);
}

void CreatorWidget::processStaticWidget(CreatorParameters& parameters)
{
    if (parameters.material != CreationMaterial_EnergyParticle) {
        AlienGui::Checkbox(
            AlienGui::CheckboxParameters()
                .name("Static")
                .label(tr("inspection.label.static"))
                .textWidth(WorldToolPresentation::creatorTextWidth())
                .tooltip(InspectionPresentation::attributeDescription(EntityAttribute::Static)),
            &parameters.isStatic);
    }
}

void CreatorWidget::processPointButtons()
{
    auto& controller = CreatorController::get();
    AlienGui::Separator();

    ImGui::BeginDisabled(!controller.isFinishingPointsPossible());
    if (AlienGui::DisplayButton(
            "Finish",
            tr("worldtools.finish"),
            {std::max(scale(PointButtonWidth), ImGui::CalcTextSize(tr("worldtools.finish").c_str()).x + ImGui::GetStyle().FramePadding.x * 2), 0})) {
        controller.onFinishPoints();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!controller.hasPoints());
    if (AlienGui::DisplayButton(
            "Abort",
            tr("worldtools.abort"),
            {std::max(scale(PointButtonWidth), ImGui::CalcTextSize(tr("worldtools.finish").c_str()).x + ImGui::GetStyle().FramePadding.x * 2), 0})) {
        controller.onAbortPoints();
    }
    ImGui::EndDisabled();
}
