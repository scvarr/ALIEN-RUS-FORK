#include "McpWindow.h"

#include <algorithm>
#include <ranges>
#include <format>

#include <boost/algorithm/string/join.hpp>

#include <imgui.h>

#include <Fonts/AlienIconFont.h>
#include <Fonts/IconsFontAwesome5.h>

#include <Base/StringHelper.h>

#include "AlienGui.h"
#include "LocalizationGuiText.h"
#include "McpController.h"
#include "McpPresentation.h"
#include "McpSettingsDialog.h"
#include "OverlayController.h"
#include "StyleService.h"

namespace
{
    auto constexpr StatusChipPadding = 34.0f;
    auto constexpr StepNumberRadius = 9.0f;
    auto constexpr StepNumberSpacing = 8.0f;
    auto constexpr TimeColumnSample = "0000-00-00 00:00:00";
    auto constexpr ResultColumnWeight = 1.4f;
    auto constexpr CopyButtonText = ICON_FA_COPY "  Copy";
}

McpWindow::McpWindow()
    : AlienWindow("MCP server", "windows.mcp server", false, false, {60.0f, 60.0f}, {620.0f, 700.0f})
{
    setTitleKey("mcp.title");
}

void McpWindow::processIntern()
{
    processToolbar();

    if (ImGui::BeginChild("##content", {0, 0})) {
        processConnectionGuide();
        processCommandLog();
    }
    ImGui::EndChild();
}

namespace
{
    std::string getStatusText(bool running)
    {
        return tr(running ? "mcp.running" : "mcp.stopped");
    }

    void processStatusChip(bool running)
    {
        AlienGui::Chip(AlienGui::ChipParameters()
                           .text(getStatusText(running))
                           .textColor(running ? Const::McpSuccessColor : Const::TextDimColor)
                           .backgroundColor(running ? Const::McpRunningBadgeColor : Const::RaisedColor)
                           .dotColor(running ? Const::McpSuccessColor : Const::TextFaintColor));
    }
}

void McpWindow::processToolbar()
{
    auto& controller = McpController::get();
    auto running = controller.isServerRunning();
    AlienGui::Toolbar(
        AlienGui::ToolbarParameters()
            .id("McpServer")
            .overflowTooltip(tr("mcp.more"))
            .trailing([running] { processStatusChip(running); })
            .trailingWidth(scaleInverse(ImGui::CalcTextSize(getStatusText(running).c_str()).x) + StatusChipPadding),
        {AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_PLAY).name(tr("mcp.start")).disabled(running).action([&] { controller.setServerRunning(true); })),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_STOP).name(tr("mcp.stop")).disabled(!running).action([&] { controller.setServerRunning(false); })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_CLEAR).name(tr("mcp.clear")).disabled(controller.getCommandLog().empty()).action([&] {
                 controller.clearCommandLog();
             })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_COG).name(tr("mcp.settings")).action([&] { McpSettingsDialog::get().open(); }))});
}

void McpWindow::processConnectionGuide()
{
    auto& controller = McpController::get();
    auto const& style = ImGui::GetStyle();

    ImGui::Spacing();
    AlienGui::Text(AlienGui::TextParameters().text(tr("mcp.connect")).style(AlienGui::TextStyle::Bold));

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, Const::TextInfoColor.Value);
    ImGui::TextUnformatted(ICON_FA_QUESTION_CIRCLE);
    ImGui::PopStyleColor();
    AlienGui::Tooltip(AlienGui::TooltipParameters()
                          .textFunc([&] {
                              std::string toolGroups;
                              for (auto const& toolGroup : controller.getToolGroups()) {
                                  toolGroups += std::format(
                                      "\n\n{}:\n{}", McpPresentation::groupCaption(toolGroup.name), boost::algorithm::join(toolGroup.toolNames, ", "));
                              }
                              return LocalizationGuiText::format("mcp.help", {{"tools", toolGroups}});
                          })
                          .delay(false));

    processStepNumber(1);
    ImGui::TextWrapped("%s", tr("mcp.step1").c_str());

    processStepNumber(2);
    ImGui::TextWrapped("%s", tr("mcp.step2").c_str());

    auto url = controller.getServerUrl();
    auto copyLabel = std::string(ICON_FA_COPY "  ") + tr("mcp.copy");
    auto copyButtonWidth = ImGui::CalcTextSize(copyLabel.c_str()).x + style.FramePadding.x * 2;
    ImGui::PushFont(StyleService::get().getMonospaceMediumFont());
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Const::BackgroundColor.Value);
    ImGui::PushStyleColor(ImGuiCol_Border, Const::LineColor.Value);
    ImGui::PushStyleColor(ImGuiCol_Text, Const::AccentColor.Value);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    auto urlWidth = ImGui::CalcTextSize(url.c_str()).x + style.FramePadding.x * 2;
    ImGui::SetNextItemWidth(std::max(1.0f, std::min(urlWidth, ImGui::GetContentRegionAvail().x - copyButtonWidth - style.ItemSpacing.x)));
    ImGui::InputText("##url", url.data(), url.size() + 1, ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::PopFont();

    ImGui::SameLine();
    if (AlienGui::DisplayButton(CopyButtonText, copyLabel)) {
        ImGui::SetClipboardText(url.c_str());
        printOverlayMessage(tr("mcp.copied"));
    }
}

void McpWindow::processStepNumber(int number)
{
    ImGui::AlignTextToFramePadding();
    auto radius = scale(StepNumberRadius);
    auto pos = ImGui::GetCursorScreenPos();
    ImVec2 center{pos.x + radius, pos.y + ImGui::GetFrameHeight() / 2};
    auto drawList = ImGui::GetWindowDrawList();
    drawList->AddCircleFilled(center, radius, Const::RaisedColor);

    auto text = std::to_string(number);
    ImGui::PushFont(StyleService::get().getSmallBoldFont());
    auto textSize = ImGui::CalcTextSize(text.c_str());
    drawList->AddText({center.x - textSize.x / 2, center.y - textSize.y / 2}, Const::AccentColor, text.c_str());
    ImGui::PopFont();

    ImGui::Dummy({radius * 2, ImGui::GetFrameHeight()});
    ImGui::SameLine(0, scale(StepNumberSpacing));
}

void McpWindow::processCommandLog()
{
    auto const& commandLog = McpController::get().getCommandLog();

    ImGui::Spacing();
    AlienGui::Group(AlienGui::GroupParameters().text(LocalizationGuiText::format("mcp.log", {{"count", std::to_string(commandLog.size())}})));

    auto flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##mcpCommands", 3, flags, ImVec2(-1, -1))) {
        auto timeColumnWidth =
            std::max(ImGui::CalcTextSize(TimeColumnSample).x, ImGui::CalcTextSize(tr("mcp.timestamp").c_str()).x) + ImGui::GetStyle().CellPadding.x * 2;
        ImGui::TableSetupColumn(tr("mcp.timestamp").c_str(), ImGuiTableColumnFlags_WidthFixed, timeColumnWidth);
        ImGui::TableSetupColumn(tr("mcp.command").c_str(), ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn(tr("mcp.result").c_str(), ImGuiTableColumnFlags_WidthStretch, ResultColumnWeight);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Const::TableHeaderColor);

        ImGuiListClipper clipper;
        clipper.Begin(toInt(commandLog.size()));
        while (clipper.Step()) {
            for (auto row : std::views::iota(clipper.DisplayStart, clipper.DisplayEnd)) {
                auto const& entry = commandLog.at(commandLog.size() - 1 - row);
                ImGui::PushID(row);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                AlienGui::Text(StringHelper::format(entry.time));

                ImGui::TableNextColumn();
                ImGui::PushFont(StyleService::get().getMonospaceMediumFont());
                ImGui::PushStyleColor(ImGuiCol_Text, Const::AccentColor.Value);
                AlienGui::Text(AlienGui::TextParameters().text(entry.command).truncate(true));
                ImGui::PopStyleColor();
                ImGui::PopFont();
                AlienGui::Tooltip(AlienGui::TooltipParameters().text(entry.command));

                ImGui::TableNextColumn();
                if (entry.isError) {
                    ImGui::PushStyleColor(ImGuiCol_Text, Const::WarningColor.Value);
                }
                AlienGui::Text(AlienGui::TextParameters().text(entry.result).truncate(true));
                if (entry.isError) {
                    ImGui::PopStyleColor();
                }
                AlienGui::Tooltip(AlienGui::TooltipParameters().text(entry.result));

                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}
