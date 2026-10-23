#include "EditorWidget.h"

#include <imgui.h>

#include <Fonts/AlienIconFont.h>
#include <Fonts/IconsFontAwesome5.h>

#include <Engine/Interface/SimulationFacade.h>

#include "AlienGui.h"
#include "CreatorController.h"
#include "CreatorWidget.h"
#include "EditorController.h"
#include "ImageToPatternDialog.h"
#include "McpWindow.h"
#include "ScissorsController.h"
#include "SimulationInteractionController.h"
#include "SimulationView.h"
#include "StyleService.h"
#include "WorldToolPresentation.h"

namespace
{
    auto constexpr DockSpacing = 14.0f;
    auto constexpr DockPadding = 6.0f;
    auto constexpr OptionsSpacing = 8.0f;
    auto constexpr OptionsPadding = 10.0f;
    auto constexpr OptionsWidth = 380.0f;
    auto constexpr OptionsTextWidth = 140.0f;
    auto constexpr FloatingCardFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing;

    struct ToolDefinition
    {
        EditTool tool;
        std::string icon;
        std::string name;
    };
    std::vector<std::vector<ToolDefinition>> const ToolGroups = {
        {
            {EditTool_Select, ICON_SELECTION, "Select and move"},
            {EditTool_Force, ICON_FORCE, "Apply forces"},
            {EditTool_Scissors, ICON_SCISSORS, "Scissors: cut connections"},
            {EditTool_Freehand, ICON_FREEHAND, "Draw freehand"},
        },
        {
            {EditTool_Object, ICON_SINGLE_OBJECT, "Single object"},
            {EditTool_Rectangle, ICON_RECTANGLE_NETWORK, "Rectangular object network"},
            {EditTool_Hexagon, ICON_HEXAGON_NETWORK, "Hexagonal object network"},
            {EditTool_Disc, ICON_DISC_NETWORK, "Disc-shaped object network"},
            {EditTool_Line, ICON_LINE_NETWORK, "Object network along a line"},
            {EditTool_Curve, ICON_CURVE_NETWORK, "Object network along a Bezier curve"},
            {EditTool_Polygon, ICON_POLYGON_NETWORK, "Polygon-shaped object network"},
        },
    };
}

void EditorWidget::process()
{
    if (!SimulationInteractionController::get().isEditMode() || !SimulationView::get().isRenderSimulation()) {
        return;
    }
    processDock();
    processToolOptions();
    processShortcuts();
}

void EditorWidget::processDock()
{
    auto& model = EditorModel::get();
    auto simulationRunning = _SimulationFacade::get()->isSimulationRunning();
    if (model.getTool() == EditTool_Force && !simulationRunning) {
        selectTool(EditTool_Select);
    }

    std::vector<AlienGui::ToolbarItem> items;
    for (auto const& group : ToolGroups) {
        for (auto const& definition : group) {
            auto parameters = AlienGui::ToolbarItemParameters()
                                  .icon(definition.icon)
                                  .name(WorldToolPresentation::text(definition.name))
                                  .selected(model.getTool() == definition.tool)
                                  .action([this, tool = definition.tool] { selectTool(tool); });
            if (definition.tool == EditTool_Force) {
                parameters.disabled(!simulationRunning).tooltip(tr("worldtools.applyforcesavailablewhilethesimulationisrunning"));
            }
            items.emplace_back(AlienGui::ToolbarItem::createButton(parameters));
        }
        items.emplace_back(AlienGui::ToolbarItem::createSeparator());
    }
    items.emplace_back(AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                               .icon(ICON_AGENT)
                                                               .name(tr("menu.mcp"))
                                                               .tooltip(tr("worldtools.connectyouraiagenttoalien"))
                                                               .selected(McpWindow::get().isOn())
                                                               .action([] { McpWindow::get().setOn(!McpWindow::get().isOn()); })));
    items.emplace_back(AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                               .icon(ICON_IMAGE_PATTERN)
                                                               .name(tr("worldtools.patternfromimage"))
                                                               .tooltip(tr("worldtools.createapatternfromanimage"))
                                                               .action([] { ImageToPatternDialog::get().show(); })));
    items.emplace_back(AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                               .icon(ICON_FA_PASTE)
                                                               .name(tr("menu.paste"))
                                                               .tooltip(tr("worldtools.pastethecopiedselectionctrlv"))
                                                               .disabled(!EditorController::get().isPastingPossible())
                                                               .action([] { EditorController::get().onPaste(); })));
    items.emplace_back(AlienGui::ToolbarItem::createSeparator());
    items.emplace_back(AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                               .icon(ICON_SCOPE_OBJECTS)
                                                               .name(tr("worldtools.applytoselectedobjects"))
                                                               .tooltip(tr("worldtools.scopehelp0"))
                                                               .selected(!model.isApplyToNetworks())
                                                               .action([&model] { model.setApplyToNetworks(false); })));
    items.emplace_back(AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                               .icon(ICON_SCOPE_NETWORKS)
                                                               .name(tr("worldtools.applytoentirenetworks"))
                                                               .tooltip(tr("worldtools.scopehelp1"))
                                                               .selected(model.isApplyToNetworks())
                                                               .action([&model] { model.setApplyToNetworks(true); })));
    items.emplace_back(AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                               .icon(ICON_GLUE)
                                                               .name(tr("worldtools.glueoncontact"))
                                                               .tooltip(tr("worldtools.scopehelp2"))
                                                               .selected(model.isGlueOnContact())
                                                               .action([&model] { model.setGlueOnContact(!model.isGlueOnContact()); })));

    auto anchor = SimulationInteractionController::get().getEditToggleAnchor();
    auto width = AlienGui::CalcToolbarWidth(items) + 2 * scale(DockPadding) + 1.0f;
    auto height = AlienGui::CalcToolbarHeight() + 2 * scale(DockPadding);
    _dockTop = anchor.y - height / 2;

    ImGui::SetNextWindowPos({anchor.x + scale(DockSpacing), _dockTop}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({width, height}, ImGuiCond_Always);
    AlienGui::PushFloatingCardStyle(DockPadding);
    if (ImGui::Begin("##editDock", nullptr, FloatingCardFlags)) {
        AlienGui::Toolbar(AlienGui::ToolbarParameters().id("EditDock").bottomSeparator(false).overflowTooltip(tr("parameters.overflow")), items);
    }
    ImGui::End();
    AlienGui::PopFloatingCardStyle();
}

namespace
{
    ToolDefinition const& getToolDefinition(EditTool tool)
    {
        for (auto const& group : ToolGroups) {
            for (auto const& definition : group) {
                if (definition.tool == tool) {
                    return definition;
                }
            }
        }
        return ToolGroups.front().front();
    }
}

void EditorWidget::processToolOptions()
{
    auto const& model = EditorModel::get();
    auto tool = model.getTool();
    if (tool == EditTool_Select) {
        return;
    }

    auto anchor = SimulationInteractionController::get().getEditToggleAnchor();
    ImGui::SetNextWindowPos({anchor.x + scale(DockSpacing), _dockTop - scale(OptionsSpacing)}, ImGuiCond_Always, {0.0f, 1.0f});
    ImGui::SetNextWindowSizeConstraints(
        {scale(std::max(OptionsWidth, WorldToolPresentation::creatorTextWidth() + 190.0f)), 0.0f},
        {scale(std::max(OptionsWidth, WorldToolPresentation::creatorTextWidth() + 190.0f)), FLT_MAX});
    AlienGui::PushFloatingCardStyle(OptionsPadding);
    if (ImGui::Begin("##editToolOptions", nullptr, FloatingCardFlags | ImGuiWindowFlags_AlwaysAutoResize)) {
        AlienGui::Group(AlienGui::GroupParameters().text(getToolDefinition(tool).name).label(WorldToolPresentation::text(getToolDefinition(tool).name)));

        std::string hint;
        if (tool == EditTool_Scissors) {
            auto& scissorsController = ScissorsController::get();
            auto cutOnlyInSelection = scissorsController.isCutOnlyInSelection();
            AlienGui::Checkbox(
                AlienGui::CheckboxParameters()
                    .name("Only in selection")
                    .label(tr("worldtools.onlyinselection"))
                    .textWidth(WorldToolPresentation::textWidth(OptionsTextWidth, {"worldtools.onlyinselection"}))
                    .tooltip(tr("worldtools.ifenabledonlyconnectionsbetweenselectedobjectsarecut")),
                &cutOnlyInSelection);
            scissorsController.setCutOnlyInSelection(cutOnlyInSelection);
            hint = tr("worldtools.holdtheleftmousebuttonanddragacrossconnectionstocutthem");
        } else if (tool == EditTool_Force) {
            hint = tr("worldtools.holdtheleftmousebuttonanddragtopushtheobjectsunderthecursor");
        } else {
            CreatorWidget::get().process();
            if (tool == EditTool_Line || tool == EditTool_Curve || tool == EditTool_Polygon) {
                hint = tr("worldtools.leftclickaddpointrightclickremovelastpointenterfinishescabort");
            } else if (tool == EditTool_Freehand) {
                hint = tr("worldtools.holdtheleftmousebuttontodraw");
            } else {
                hint = tr("worldtools.leftclickinthesimulationtoplace");
            }
        }
        ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDecentColor.Value);
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextUnformatted(hint.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
    }
    ImGui::End();
    AlienGui::PopFloatingCardStyle();
}

void EditorWidget::processShortcuts()
{
    auto const& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard || io.WantTextInput || io.KeyCtrl || io.KeyAlt || io.KeySuper) {
        return;
    }
    if (ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)) {
        return;
    }

    auto& creatorController = CreatorController::get();
    if (ImGui::IsKeyPressed(ImGuiKey_Enter, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false)) {
        creatorController.onFinishPoints();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Escape, false) && creatorController.hasPoints()) {
        creatorController.onAbortPoints();
    }
}

void EditorWidget::selectTool(EditTool tool)
{
    EditorModel::get().setTool(tool);
}
