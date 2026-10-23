#include "AboutDialog.h"

#include <algorithm>

#include <imgui.h>

#include <Base/Resources.h>

#include "AlienGui.h"
#include "LocalizationGuiText.h"
#include "StyleService.h"

AboutDialog::AboutDialog()
    : AlienDialog("About", {500.0f, 240.0f})
{
    setTitleKey("menu.about");
}

void AboutDialog::processIntern()
{
    auto text = LocalizationGuiText::format("localui.about.text", {{"version", Const::ProgramVersion}});
    ImGui::TextWrapped("%s", text.c_str());

    ImGui::Dummy({0, std::max(0.0f, ImGui::GetContentRegionAvail().y - scale(50.0f))});
    AlienGui::Separator();

    if (AlienGui::Button(tr("common.ok") + "###OK")) {
        close();
    }
    ImGui::SetItemDefaultFocus();
}
