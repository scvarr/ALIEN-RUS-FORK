#include "NetworkSettingsDialog.h"

#include <algorithm>

#include <imgui.h>

#include <Base/LocalizationService.h>

#include <Network/NetworkService.h>

#include "AlienGui.h"
#include "BrowserWindow.h"
#include "HelpStrings.h"
#include "StyleService.h"

namespace
{
    auto const RightColumnWidth = 150.0f;
}

NetworkSettingsDialog::NetworkSettingsDialog()
    : AlienDialog("Network settings", {650.0f, 240.0f})
{
    setTitleKey("settings.network");
}

void NetworkSettingsDialog::processIntern()
{
    auto labelWidth = std::max(
        {RightColumnWidth,
         ImGui::CalcTextSize(tr("localui.network.address").c_str()).x / scale(1.0f) + 60.0f,
         ImGui::CalcTextSize(tr("localui.network.active").c_str()).x / scale(1.0f) + 60.0f});
    AlienGui::InputText(
        AlienGui::InputTextParameters()
            .name("Blocks")
            .defaultValue(_origServerAddress)
            .name("Server address")
            .label(tr("localui.network.address"))
            .textWidth(labelWidth),
        _serverAddress);
    AlienGui::Checkbox(
        AlienGui::CheckboxParameters()
            .name("Count me as active")
            .label(tr("localui.network.active"))
            .textWidth(labelWidth)
            .tooltip(tr(Const::NetworkCountAsActiveTooltip)),
        &_countAsActive);

    ImGui::Dummy({0, std::max(0.0f, ImGui::GetContentRegionAvail().y - scale(50.0f))});
    AlienGui::Separator();

    if (AlienGui::Button(tr("common.apply") + "###Adopt")) {
        close();
        onChangeSettings();
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienGui::Button(tr("common.cancel") + "###Cancel")) {
        close();
    }
}

void NetworkSettingsDialog::openIntern()
{
    _origServerAddress = NetworkService::get().getServerAddress();
    _serverAddress = _origServerAddress;
    _countAsActive = NetworkService::get().isCountAsActive();
}

void NetworkSettingsDialog::onChangeSettings()
{
    NetworkService::get().setServerAddress(_serverAddress);
    NetworkService::get().setCountAsActive(_countAsActive);
    BrowserWindow::get().onRefresh();
}
