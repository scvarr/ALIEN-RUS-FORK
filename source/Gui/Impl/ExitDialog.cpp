#include "ExitDialog.h"

#include <imgui.h>

#include <Base/LocalizationService.h>

#include "AlienGui.h"
#include "LocalizationGui.h"
#include "MainLoopController.h"

ExitDialog::ExitDialog()
    : AlienDialog("Exit", {520.0f, 210.0f})
{
    setTitleKey("exit.title");
}

void ExitDialog::processIntern()
{
    ImGui::TextWrapped("%s", tr("exit.confirm").c_str());

    LocalizationGui::languageSelector();

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienGui::Separator();

    if (AlienGui::Button(tr("common.ok") + "###OK")) {
        MainLoopController::get().scheduleClosing();
        close();
    }
    ImGui::SameLine();
    if (AlienGui::Button(tr("common.cancel") + "###Cancel")) {
        close();
    }
    ImGui::SetItemDefaultFocus();
}
