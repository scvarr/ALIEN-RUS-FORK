#include "EditSimulationDialog.h"

#include "BrowserPresentation.h"

#include <imgui.h>

#include <Network/NetworkResourceService.h>
#include <Network/NetworkService.h>
#include <Network/NetworkValidationService.h>

#include "AlienGui.h"
#include "BrowserWindow.h"
#include "DelayedExecutionController.h"
#include "GenericMessageDialog.h"
#include "HelpStrings.h"
#include "NetworkTransferController.h"
#include "OverlayController.h"
#include "StyleService.h"

void EditSimulationDialog::openForLeaf(NetworkResourceTreeTO const& treeTO)
{
    changeTitle("Change name or description");
    setTitleKey("browser.changenameordescription");
    AlienDialog::open();
    _treeTO = treeTO;

    auto& rawTO = _treeTO->getLeaf().rawTO;
    _newName = rawTO->resourceName;
    _newDescription = rawTO->description;
}

void EditSimulationDialog::openForFolder(NetworkResourceTreeTO const& treeTO, std::vector<NetworkResourceRawTO> const& rawTOs)
{
    changeTitle("Change folder name");
    setTitleKey("browser.changefoldername");
    AlienDialog::open();
    _treeTO = treeTO;
    _rawTOs = rawTOs;

    _newName = NetworkResourceService::get().concatenateFolderName(treeTO->folderNames, false);
    _origFolderName = _newName;
}

EditSimulationDialog::EditSimulationDialog()
    : AlienDialog("")
{}

void EditSimulationDialog::processIntern()
{
    if (_treeTO->isLeaf()) {
        processForLeaf();
    } else {
        processForFolder();
    }
}

void EditSimulationDialog::processForLeaf()
{
    auto& rawTO = _treeTO->getLeaf().rawTO;
    std::string resourceTypeString = rawTO->resourceType == NetworkResourceType_Simulation ? "simulation" : "genome";

    AlienGui::InputText(AlienGui::InputTextParameters().textWidth(0).hint(BrowserPresentation::text("Name")).hintId("Name"), _newName);

    AlienGui::Separator();

    ImGui::PushID("description");
    AlienGui::InputTextMultiline(
        AlienGui::InputTextMultilineParameters().hint(tr("genome.upload.description")).textWidth(0).height(ImGui::GetContentRegionAvail().y - scale(50.0f)),
        _newDescription);
    ImGui::PopID();

    AlienGui::Separator();

    ImGui::BeginDisabled(_newName.empty());
    if (AlienGui::DisplayButton("OK", BrowserPresentation::text("OK"))) {
        if (NetworkValidationService::get().isStringValidForDatabase(_newName) && NetworkValidationService::get().isStringValidForDatabase(_newDescription)) {
            EditNetworkResourceRequestData::Entry entry{.resourceId = rawTO->id, .newName = _newName, .newDescription = _newDescription};
            NetworkTransferController::get().onEdit(EditNetworkResourceRequestData{.entries = std::vector{entry}});
            close();
        } else {
            GenericMessageDialog::get().localizedInformation("Error", "common.error", "genome.upload.invalid");
        }
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienGui::DisplayButton("Cancel", BrowserPresentation::text("Cancel"))) {
        close();
    }
}

void EditSimulationDialog::processForFolder()
{
    if (ImGui::BeginChild("##Folder", {0, -scale(50.0f)})) {
        AlienGui::InputText(AlienGui::InputTextParameters().textWidth(0).hint(BrowserPresentation::text("Folder name")).hintId("Folder name"), _newName);
    }
    ImGui::EndChild();

    AlienGui::Separator();

    ImGui::BeginDisabled(_newName.empty());
    if (AlienGui::DisplayButton("OK", BrowserPresentation::text("OK"))) {
        if (NetworkValidationService::get().isStringValidForDatabase(_newName)) {

            EditNetworkResourceRequestData requestData;
            for (auto const& rawTO : _rawTOs) {
                auto nameWithoutOldFolder = rawTO->resourceName.substr(_origFolderName.size() + 1);
                auto newName = NetworkResourceService::get().concatenateFolderName({_newName, nameWithoutOldFolder}, false);
                requestData.entries.emplace_back(rawTO->id, newName, rawTO->description);
            }
            NetworkTransferController::get().onEdit(requestData);
            close();
        } else {
            GenericMessageDialog::get().localizedInformation("Error", "common.error", "genome.upload.invalid");
        }
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienGui::DisplayButton("Cancel", BrowserPresentation::text("Cancel"))) {
        close();
    }
}
