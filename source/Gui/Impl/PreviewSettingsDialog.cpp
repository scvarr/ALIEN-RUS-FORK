#include "PreviewSettingsDialog.h"

#include <imgui.h>

#include <Base/LocalizationService.h>
#include "AlienGui.h"
#include "GenomeTabEditData.h"
#include "GenomeWindowEditData.h"

PreviewSettingsDialog::PreviewSettingsDialog()
    : AlienDialog("Preview settings")
{
    setTitleKey("neural.preview.settings");
}

void PreviewSettingsDialog::setEditData(GenomeWindowEditData const& genomeEditData, GenomeTabEditData const& editData)
{
    _genomeEditData = genomeEditData;
    _editData = editData;
}

void PreviewSettingsDialog::processIntern()
{
    // Convert boolean to switcher index: 0 = Node index, 1 = Cell type
    int displayMode = _showNodeIndex ? 0 : 1;

    AlienGui::Switcher(
        AlienGui::SwitcherParameters()
            .name("Display mode")
            .label(tr("neural.preview.mode"))
            .textWidth(170.0f)
            .values({tr("neural.preview.node"), tr("neural.preview.cell")}),
        &displayMode);

    // Convert switcher index back to boolean
    _showNodeIndex = (displayMode == 0);

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienGui::Separator();

    if (AlienGui::DisplayButton("Adopt", tr("neural.adopt"))) {
        close();
        _editData->showNodeIndex = _showNodeIndex;
        _genomeEditData->defaultShowNodeIndex = _showNodeIndex;
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienGui::DisplayButton("Cancel", tr("neural.cancel"))) {
        close();
    }
}

void PreviewSettingsDialog::openIntern()
{
    _showNodeIndex = _editData->showNodeIndex;
}
