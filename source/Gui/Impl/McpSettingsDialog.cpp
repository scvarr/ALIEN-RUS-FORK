#include "McpSettingsDialog.h"

#include <imgui.h>

#include "AlienGui.h"
#include "LocalizationGuiText.h"
#include "McpController.h"
#include "StyleService.h"

namespace
{
    auto constexpr RightColumnWidth = 150.0f;
}

McpSettingsDialog::McpSettingsDialog()
    : AlienDialog("MCP server settings", {530.0f, 180.0f})
{
    setTitleKey("mcp.settings.title");
}

void McpSettingsDialog::processIntern()
{
    auto& controller = McpController::get();

    AlienGui::InputInt(
        AlienGui::InputIntParameters()
            .name("Port")
            .label(tr("mcp.port"))
            .textWidth(RightColumnWidth)
            .defaultValue(controller.getDefaultPort())
            .tooltip(tr("mcp.port.help")),
        _port);

    ImGui::Dummy({0, std::max(0.0f, ImGui::GetContentRegionAvail().y - scale(50.0f))});
    AlienGui::Separator();

    if (AlienGui::DisplayButton("Adopt", tr("common.apply"))) {
        close();
        controller.setPort(_port);
    }
    ImGui::SetItemDefaultFocus();

    if (ImGui::GetContentRegionAvail().x > ImGui::CalcTextSize(tr("common.cancel").c_str()).x + scale(140.0f)) {
        ImGui::SameLine();
    }
    if (AlienGui::DisplayButton("Cancel", tr("common.cancel"))) {
        close();
    }
}

void McpSettingsDialog::openIntern()
{
    _port = McpController::get().getPort();
}
