#include "SelectionWidget.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include <Fonts/AlienIconFont.h>
#include <Fonts/IconsFontAwesome5.h>

#include <Base/Math.h>
#include <Base/StringHelper.h>

#include <Engine/Interface/SimulationFacade.h>

#include "AlienGui.h"
#include "EditorController.h"
#include "EditorModel.h"
#include "InspectionController.h"
#include "MultiplierWidget.h"
#include "SimulationInteractionController.h"
#include "SimulationView.h"
#include "StyleService.h"
#include "Viewport.h"
#include "WorldToolPresentation.h"

namespace
{
    auto constexpr FramePadding = 10.0f;
    auto constexpr FrameThickness = 1.0f;
    auto constexpr CornerHandleSize = 7.0f;
    auto constexpr HandleRadius = 7.0f;
    auto constexpr HandleHitMargin = 4.0f;
    auto constexpr HandleThickness = 1.5f;
    auto constexpr RotationHandleDistance = 30.0f;
    auto constexpr SummarySpacing = 8.0f;
    auto constexpr SummaryPaddingX = 8.0f;
    auto constexpr SummaryPaddingY = 3.0f;
    auto constexpr SummaryRounding = 10.0f;
    auto constexpr ActionBarSpacing = 10.0f;
    auto constexpr ActionBarPadding = 5.0f;
    auto constexpr ActionBarMargin = 10.0f;
    auto constexpr ColorFieldSize = 24.0f;
    auto constexpr MultiplyPopupWidth = 470.0f;
    auto constexpr MorePopupWidth = 360.0f;
}

void SelectionWidget::process()
{
    auto const& model = EditorModel::get();
    if (!SimulationInteractionController::get().isEditMode() || !SimulationView::get().isRenderSimulation() || model.getTool() != EditTool_Select
        || model.isSelectionEmpty()) {
        _lastRotationAngle.reset();
        return;
    }

    auto bounds = calcViewBounds();
    processFrame(bounds);
    processRotationHandle(bounds);
    processSummary(bounds);
    processActionBar(bounds);
}

SelectionWidget::ViewBounds SelectionWidget::calcViewBounds() const
{
    auto const& model = EditorModel::get();
    auto selectionBounds = model.getSelectionBounds(model.isApplyToNetworks());
    auto zoom = Viewport::get().getZoomFactor();
    auto borderlessRendering = _SimulationFacade::get()->getSimulationParameters().borderlessRendering.value;
    auto center = Viewport::get().mapWorldToViewPosition(selectionBounds.center, borderlessRendering);
    auto padding = RealVector2D{scale(FramePadding), scale(FramePadding)};
    return ViewBounds{
        .center = center,
        .topLeft = center + (selectionBounds.topLeft - selectionBounds.center) * zoom - padding,
        .bottomRight = center + (selectionBounds.bottomRight - selectionBounds.center) * zoom + padding,
    };
}

void SelectionWidget::processFrame(ViewBounds const& bounds) const
{
    auto drawList = ImGui::GetBackgroundDrawList();
    drawList->AddRect(
        {bounds.topLeft.x, bounds.topLeft.y}, {bounds.bottomRight.x, bounds.bottomRight.y}, Const::SelectionFrameColor, 0, 0, scale(FrameThickness));

    auto halfSize = scale(CornerHandleSize) / 2;
    for (auto const& corner : {
             bounds.topLeft,
             RealVector2D{bounds.bottomRight.x, bounds.topLeft.y},
             RealVector2D{bounds.topLeft.x, bounds.bottomRight.y},
             bounds.bottomRight,
         }) {
        drawList->AddRectFilled({corner.x - halfSize, corner.y - halfSize}, {corner.x + halfSize, corner.y + halfSize}, Const::SelectionHandleFillColor);
        drawList->AddRect(
            {corner.x - halfSize, corner.y - halfSize}, {corner.x + halfSize, corner.y + halfSize}, Const::SelectionHandleColor, 0, 0, scale(HandleThickness));
    }
}

namespace
{
    struct HandleState
    {
        bool hovered = false;
        bool active = false;
    };

    HandleState processHandle(char const* id, RealVector2D const& pos, std::string const& tooltip)
    {
        auto size = 2 * scale(HandleRadius + HandleHitMargin);
        ImGui::SetNextWindowPos({pos.x - size / 2, pos.y - size / 2}, ImGuiCond_Always);
        ImGui::SetNextWindowSize({size, size}, ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.0f, 1.0f));
        HandleState result;
        auto flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (ImGui::Begin(id, nullptr, flags)) {
            ImGui::InvisibleButton("##handle", {size, size});
            result.hovered = ImGui::IsItemHovered();
            result.active = ImGui::IsItemActive();
            if (!result.active) {
                AlienGui::Tooltip(AlienGui::TooltipParameters().text(tooltip).delay(false));
            }
        }
        ImGui::End();
        ImGui::PopStyleVar(3);
        return result;
    }

    void drawHandle(RealVector2D const& pos, HandleState const& state, ImColor const& color)
    {
        auto drawList = ImGui::GetBackgroundDrawList();
        auto borderColor = state.hovered || state.active ? Const::SelectionHandleHoveredColor : color;
        drawList->AddCircleFilled({pos.x, pos.y}, scale(HandleRadius), Const::SelectionHandleFillColor);
        drawList->AddCircle({pos.x, pos.y}, scale(HandleRadius), borderColor, 0, scale(HandleThickness));
    }

    float calcAngle(RealVector2D const& from, ImVec2 const& to)
    {
        return toFloat(std::atan2(to.y - from.y, to.x - from.x) * Const::RadToDeg);
    }
}

void SelectionWidget::processRotationHandle(ViewBounds const& bounds)
{
    auto frameTop = RealVector2D{(bounds.topLeft.x + bounds.bottomRight.x) / 2, bounds.topLeft.y};
    auto handlePos = RealVector2D{frameTop.x, frameTop.y - scale(RotationHandleDistance)};

    ImGui::GetBackgroundDrawList()->AddLine(
        {frameTop.x, frameTop.y}, {handlePos.x, handlePos.y + scale(HandleRadius)}, Const::SelectionFrameColor, scale(FrameThickness));

    auto state = processHandle("##selectionRotationHandle", handlePos, tr("worldtools.dragtorotatetheselection"));
    drawHandle(handlePos, state, Const::SelectionHandleColor);

    if (!state.active) {
        _lastRotationAngle.reset();
        return;
    }
    auto angle = calcAngle(bounds.center, ImGui::GetMousePos());
    if (_lastRotationAngle.has_value()) {
        auto angleDelta = Math::getNormalizedAngle(angle - *_lastRotationAngle, -180.0f);
        if (std::abs(angleDelta) > NEAR_ZERO) {
            EditorController::get().onRotateSelectedObjects(angleDelta);
        }
    }
    _lastRotationAngle = angle;
}

void SelectionWidget::processSummary(ViewBounds const& bounds) const
{
    auto const& model = EditorModel::get();
    auto const& selection = model.getSelectionShallowData();
    auto numObjects = model.isApplyToNetworks() ? selection.numClusterCells : selection.numObjects;

    std::vector<std::string> parts;
    if (numObjects > 0) {
        parts.emplace_back(WorldToolPresentation::selectionCount("worldtools.count.objects", numObjects));
    }
    if (selection.numCreatures > 0) {
        parts.emplace_back(WorldToolPresentation::selectionCount("worldtools.count.creatures", selection.numCreatures));
    }
    if (selection.numEnergyParticles > 0) {
        parts.emplace_back(WorldToolPresentation::selectionCount("worldtools.count.particles", selection.numEnergyParticles));
    }
    std::string text;
    for (auto const& part : parts) {
        text += text.empty() ? part : "  \xC2\xB7  " + part;
    }

    auto textSize = ImGui::CalcTextSize(text.c_str());
    auto topLeft = ImVec2{bounds.topLeft.x, bounds.bottomRight.y + scale(SummarySpacing)};
    auto bottomRight = ImVec2{topLeft.x + textSize.x + 2 * scale(SummaryPaddingX), topLeft.y + textSize.y + 2 * scale(SummaryPaddingY)};
    auto drawList = ImGui::GetBackgroundDrawList();
    drawList->AddRectFilled(topLeft, bottomRight, Const::FloatingCardBackgroundColor, scale(SummaryRounding));
    drawList->AddRect(topLeft, bottomRight, Const::FloatingCardBorderColor, scale(SummaryRounding));
    drawList->AddText({topLeft.x + scale(SummaryPaddingX), topLeft.y + scale(SummaryPaddingY)}, Const::SelectionChipTextColor, text.c_str());
}

void SelectionWidget::processActionBar(ViewBounds const& bounds)
{
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    auto& controller = EditorController::get();
    auto const& model = EditorModel::get();
    auto popupButton = [](std::string const& icon, std::string const& name, char const* popupId) {
        return AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters().icon(icon).name(name).action([popupId] { ImGui::OpenPopup(popupId); }));
    };
    auto actionButton = [](std::string const& icon, std::string const& name, std::function<void()> const& action, bool disabled = false) {
        return AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters().icon(icon).name(name).disabled(disabled).action(action));
    };
    std::vector<AlienGui::ToolbarItem> items = {
        popupButton(ICON_COLOR, tr("parameters.color.color"), "##hudColor"),
        popupButton(ICON_STICKY, tr("worldtools.stickiness"), "##hudSticky"),
        popupButton(ICON_FIXED, tr("inspection.label.static"), "##hudStatic"),
        AlienGui::ToolbarItem::createSeparator(),
        actionButton(ICON_UNIFORM_VELOCITY, tr("worldtools.makeuniformvelocities"), [&controller] { controller.onUniformVelocities(); }),
        actionButton(
            ICON_RELEASE_STRESSES, tr("worldtools.releasestresses"), [&controller] { controller.onReleaseStresses(); }, model.isCellSelectionEmpty()),
        actionButton(
            ICON_GLUE_SELECTION,
            tr("worldtools.glueselectionconnectsneighboringobjectswithintheselection"),
            [&controller] { controller.onGlueSelectedObjects(); },
            model.isCellSelectionEmpty()),
        AlienGui::ToolbarItem::createSeparator(),
        popupButton(ICON_MULTIPLY, tr("worldtools.multiply"), "##hudMultiply"),
        AlienGui::ToolbarItem::createSeparator(),
        actionButton(ICON_FA_COPY, tr("worldtools.copyctrlc"), [&controller] { controller.onCopy(); }),
        actionButton(
            ICON_FA_TRASH, tr("worldtools.deletedel"), [&controller] { controller.onDelete(); }, !controller.isDeletingPossible()),
        AlienGui::ToolbarItem::createSeparator(),
        popupButton(ICON_INSPECT, tr("worldtools.inspect"), "##hudInspect"),
        popupButton(ICON_MORE, tr("worldtools.transformpositionvelocityandrotation"), "##hudMore"),
    };

    auto viewport = ImGui::GetMainViewport();
    auto width = AlienGui::CalcToolbarWidth(items) + 2 * scale(ActionBarPadding) + 1.0f;
    auto height = AlienGui::CalcToolbarHeight() + 2 * scale(ActionBarPadding);
    auto margin = scale(ActionBarMargin);
    auto posX = (bounds.topLeft.x + bounds.bottomRight.x) / 2 - width / 2;
    posX = std::clamp(posX, viewport->WorkPos.x + margin, std::max(viewport->WorkPos.x + margin, viewport->WorkPos.x + viewport->WorkSize.x - width - margin));
    auto posY = bounds.topLeft.y - scale(RotationHandleDistance + HandleRadius + ActionBarSpacing) - height;
    if (posY < viewport->WorkPos.y + margin) {
        posY = bounds.bottomRight.y + scale(SummarySpacing) + ImGui::GetTextLineHeight() + 2 * scale(SummaryPaddingY + ActionBarSpacing);
    }
    posY = std::clamp(posY, viewport->WorkPos.y + margin, std::max(viewport->WorkPos.y + margin, viewport->WorkPos.y + viewport->WorkSize.y - height - margin));

    ImGui::SetNextWindowPos({posX, posY}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({width, height}, ImGuiCond_Always);
    AlienGui::PushFloatingCardStyle(ActionBarPadding);
    auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing;
    if (ImGui::Begin("##selectionActionBar", nullptr, flags)) {
        AlienGui::PopFloatingCardStyle();
        AlienGui::Toolbar(AlienGui::ToolbarParameters().id("SelectionActions").bottomSeparator(false).overflowTooltip(tr("parameters.overflow")), items);
        processColorPopup();
        processStickyPopup();
        processStaticPopup();
        processMultiplyPopup();
        processInspectPopup();
        processMorePopup();
    } else {
        AlienGui::PopFloatingCardStyle();
    }
    ImGui::End();
}

void SelectionWidget::processColorPopup()
{
    if (!ImGui::BeginPopup("##hudColor")) {
        return;
    }
    auto const& customizationColors = _SimulationFacade::get()->getSimulationParameters().customizationColors.value;
    for (int i = 0; i < MAX_COLORS; ++i) {
        if (i > 0) {
            ImGui::SameLine();
        }
        ImGui::PushID(i);
        if (AlienGui::ColorField(customizationColors.values[i].toRgbColor(), ColorFieldSize, ColorFieldSize)) {
            EditorController::get().onColorSelectedObjects(i);
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopID();
    }
    ImGui::EndPopup();
}

void SelectionWidget::processStickyPopup()
{
    if (!ImGui::BeginPopup("##hudSticky")) {
        return;
    }
    if (WorldToolPresentation::selectable("Make sticky", tr("worldtools.makesticky"))) {
        EditorController::get().onSetSticky(true);
    }
    if (WorldToolPresentation::selectable("Make unsticky", tr("worldtools.makeunsticky"))) {
        EditorController::get().onSetSticky(false);
    }
    ImGui::EndPopup();
}

void SelectionWidget::processStaticPopup()
{
    if (!ImGui::BeginPopup("##hudStatic")) {
        return;
    }
    if (WorldToolPresentation::selectable("Make static", tr("worldtools.makestatic"))) {
        EditorController::get().onSetStatic(true);
    }
    if (WorldToolPresentation::selectable("Make non-static", tr("worldtools.makenonstatic"))) {
        EditorController::get().onSetStatic(false);
    }
    ImGui::EndPopup();
}

void SelectionWidget::processMultiplyPopup()
{
    ImGui::SetNextWindowSize(
        {scale(std::max({MultiplyPopupWidth, WorldToolPresentation::randomTextWidth() + 190.0f, WorldToolPresentation::gridTextWidth() + 260.0f})), 0.0f});
    if (!ImGui::BeginPopup("##hudMultiply")) {
        return;
    }
    MultiplierWidget::get().processContent();
    ImGui::EndPopup();
}

void SelectionWidget::processInspectPopup()
{
    if (!ImGui::BeginPopup("##hudInspect")) {
        return;
    }
    auto& controller = InspectionController::get();
    if (WorldToolPresentation::selectable(
            "Objects (ALT+N)", tr("worldtools.objectsaltn"), controller.isObjectInspectionPossible() ? 0 : ImGuiSelectableFlags_Disabled)) {
        controller.onInspectSelectedObjects();
    }
    if (WorldToolPresentation::selectable(
            "Genomes (ALT+F)", tr("worldtools.genomesaltf"), controller.isGenomeInspectionPossible() ? 0 : ImGuiSelectableFlags_Disabled)) {
        controller.onInspectSelectedGenomes();
    }
    if (WorldToolPresentation::selectable(
            "Creatures (ALT+P)", tr("worldtools.creaturesaltp"), controller.isCreatureInspectionPossible() ? 0 : ImGuiSelectableFlags_Disabled)) {
        controller.onInspectSelectedCreatures();
    }
    ImGui::EndPopup();
}

void SelectionWidget::processMorePopup()
{
    auto const& model = EditorModel::get();
    auto const& selection = model.getSelectionShallowData();
    if (_lastSelection.has_value()
        && (_lastSelection->numObjects != selection.numObjects || _lastSelection->numEnergyParticles != selection.numEnergyParticles)) {
        _angle = 0;
        _angularVelocity = 0;
    }
    _lastSelection = selection;

    ImGui::SetNextWindowSize({scale(std::max(MorePopupWidth, WorldToolPresentation::transformTextWidth() + 170.0f)), 0.0f});
    if (!ImGui::BeginPopup("##hudMore")) {
        return;
    }
    auto& controller = EditorController::get();
    auto bounds = model.getSelectionBounds(model.isApplyToNetworks());

    AlienGui::Group(AlienGui::GroupParameters().text("Center position and velocity").label(tr("worldtools.centerpositionandvelocity")));
    auto position = bounds.center;
    AlienGui::InputFloat(
        AlienGui::InputFloatParameters()
            .name("Position X")
            .label(tr("worldtools.positionx"))
            .textWidth(WorldToolPresentation::transformTextWidth())
            .format("%.3f"),
        position.x);
    AlienGui::InputFloat(
        AlienGui::InputFloatParameters()
            .name("Position Y")
            .label(tr("worldtools.positiony"))
            .textWidth(WorldToolPresentation::transformTextWidth())
            .format("%.3f"),
        position.y);
    if (position != bounds.center) {
        controller.onMoveSelectedObjectsBy(position - bounds.center);
    }

    auto velocity = bounds.velocity;
    AlienGui::InputFloat(
        AlienGui::InputFloatParameters()
            .name("Velocity X")
            .label(tr("worldtools.velocityx"))
            .textWidth(WorldToolPresentation::transformTextWidth())
            .step(0.1f)
            .format("%.3f"),
        velocity.x);
    AlienGui::InputFloat(
        AlienGui::InputFloatParameters()
            .name("Velocity Y")
            .label(tr("worldtools.velocityy"))
            .textWidth(WorldToolPresentation::transformTextWidth())
            .step(0.1f)
            .format("%.3f"),
        velocity.y);
    if (velocity != bounds.velocity) {
        controller.onSetVelocityOfSelectedObjects(velocity);
    }

    AlienGui::Group(AlienGui::GroupParameters().text("Center rotation").label(tr("worldtools.centerrotation")));
    auto origAngle = _angle;
    AlienGui::SliderInputFloat(
        AlienGui::SliderInputFloatParameters()
            .name("Angle")
            .label(tr("parameters.forcefield.layerlinearforcefieldangle.name"))
            .textWidth(WorldToolPresentation::transformTextWidth())
            .inputWidth(50.0f)
            .min(-180.0f)
            .max(180.0f)
            .format("%.1f"),
        _angle);
    if (_angle != origAngle) {
        controller.onRotateSelectedObjects(_angle - origAngle);
    }

    auto origAngularVelocity = _angularVelocity;
    AlienGui::InputFloat(
        AlienGui::InputFloatParameters()
            .name("Angular velocity")
            .label(tr("worldtools.angularvelocity"))
            .textWidth(WorldToolPresentation::transformTextWidth())
            .step(0.01f)
            .format("%.2f"),
        _angularVelocity);
    if (_angularVelocity != origAngularVelocity) {
        controller.onSetAngularVelocityOfSelectedObjects(_angularVelocity);
    }
    ImGui::EndPopup();
}
