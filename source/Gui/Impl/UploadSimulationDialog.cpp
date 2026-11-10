#include "UploadSimulationDialog.h"

#include <algorithm>

#include <imgui.h>

#include <Base/GlobalSettings.h>
#include <Base/LocalizationService.h>
#include <Fonts/IconsFontAwesome5.h>

#include <Data/SimulationParameters.h>

#include <Engine/Interface/SimulationFacade.h>

#include <Network/NetworkResourceService.h>
#include <Network/NetworkService.h>
#include <Network/NetworkValidationService.h>

#include <Persister/Interface/SerializerService.h>

#include "AlienGui.h"
#include "BrowserWindow.h"
#include "EditorController.h"
#include "GenericMessageDialog.h"
#include "GenomeEditorWindow.h"
#include "HelpStrings.h"
#include "LoginDialog.h"
#include "NetworkTransferController.h"
#include "StyleService.h"
#include "Viewport.h"

namespace
{
    auto constexpr HelpMarkerWidth = 24.0f;
    auto constexpr BottomSpace = 90.0f;
    auto constexpr MinDescriptionHeight = 50.0f;

    std::map<NetworkResourceType, std::string> const BrowserDataTypeToLowerString = {
        {NetworkResourceType_Simulation, "simulation"},
        {NetworkResourceType_Genome, "genome"}};
    std::map<NetworkResourceType, std::string> const BrowserDataTypeToUpperString = {
        {NetworkResourceType_Simulation, "Simulation"},
        {NetworkResourceType_Genome, "Genome"}};
}

namespace
{
    std::string getSuggestedNameWithoutFolder(NetworkResourceType resourceType)
    {
        if (resourceType == NetworkResourceType_Genome) {
            return GenomeEditorWindow::get().getCurrentGenome()._name;
        }
        auto projectName = std::string(_SimulationFacade::get()->getSimulationParameters().projectName.value);
        if (projectName == SimulationParameters().projectName.value) {
            return {};
        }
        return projectName;
    }

    std::string getSuggestedResourceName(NetworkResourceType resourceType, std::string const& folder)
    {
        auto name = getSuggestedNameWithoutFolder(resourceType);
        auto userName = NetworkService::get().getLoggedInUserName();
        if (!folder.empty() || !userName.has_value()) {
            return name;
        }
        return *userName + "/" + name;
    }
}

void UploadSimulationDialog::open(NetworkResourceType resourceType, std::string const& folder, std::string const& resourceName)
{
    if (NetworkService::get().getLoggedInUserName()) {
        changeTitle("Upload " + BrowserDataTypeToLowerString.at(resourceType));
        setTitleKey(resourceType == NetworkResourceType_Genome ? "genome.upload.title" : "browser.upload.title");
        _resourceType = resourceType;
        _folder = folder;
        _resourceName = resourceName.empty() ? getSuggestedResourceName(resourceType, folder) : resourceName;
        _resourceDescription = _resourceDescriptionByTypeAndFolder[{_resourceType, _folder}];
        if (_resourceType == NetworkResourceType_Simulation) {
            _preview.createForSimulation();
        } else {
            _preview.createForGenome(GenomeEditorWindow::get().getCurrentPreviewDescs());
        }
        AlienDialog::open();
    } else {
        LoginDialog::get().open();
    }
}

UploadSimulationDialog::UploadSimulationDialog()
    : AlienDialog("", {640.0f, 810.0f})
{}

void UploadSimulationDialog::processIntern()
{
    auto genomeText = [&](std::string const&, std::string const& key) {
        return _resourceType == NetworkResourceType_Genome ? tr(key)
                                                           : tr(key == "genome.field.name"              ? "browser.upload.name"
                                                                    : key.starts_with("genome.upload.") ? "browser.upload." + key.substr(14)
                                                                                                        : key);
    };

    _preview.process();

    AlienGui::Separator();

    if (!_folder.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDimColor.Value);
        AlienGui::Text(ICON_FA_FOLDER);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        AlienGui::Text(AlienGui::TextParameters().text(_folder).style(AlienGui::TextStyle::Bold));
        AlienGui::Tooltip(
            AlienGui::TooltipParameters().text(tr("localui.sync.folder")));
    }

    AlienGui::InputText(
        AlienGui::InputTextParameters()
            .hint(genomeText("", "genome.field.name"))
            .hintId(BrowserDataTypeToUpperString.at(_resourceType) + " name")
            .textWidth(HelpMarkerWidth)
            .tooltip(
                tr("localui.sync.folders")),
        _resourceName);

    ImGui::PushID("description");
    AlienGui::InputTextMultiline(
        AlienGui::InputTextMultilineParameters()
            .hint(
                _resourceType == NetworkResourceType_Simulation ? tr("localui.sync.desc.sim")
                                                                : tr("localui.sync.desc.genome"))
            .textWidth(0)
            .height(std::max(MinDescriptionHeight, scaleInverse(ImGui::GetContentRegionAvail().y) - BottomSpace)),
        _resourceDescription);
    ImGui::PopID();

    ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDimColor.Value);
    AlienGui::Text(ICON_FA_INFO_CIRCLE);
    ImGui::SameLine();
    AlienGui::Text(
        tr("localui.sync.sharing"));
    ImGui::PopStyleColor();

    AlienGui::Separator();

    ImGui::BeginDisabled(NetworkResourceService::get().removeFoldersFromName(_resourceName).empty());
    if (AlienGui::ActionButton(AlienGui::ActionButtonParameters()
                                   .buttonText(std::string(ICON_FA_UPLOAD "  ") + tr("localui.sync.share"))
                                   .highlighted(true)
                                   .frame(true)
                                   .transparentBackground(false)
                                   .tooltip(tr("localui.sync.sharehelp")))) {
        onUpload(WorkspaceType_Public);
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (AlienGui::ActionButton(AlienGui::ActionButtonParameters()
                                   .buttonText(std::string(ICON_FA_LOCK "  ") + tr("localui.sync.private"))
                                   .transparentBackground(false)
                                   .tooltip(
                                       tr("localui.sync.privatehelp")))) {
        onUpload(WorkspaceType_Private);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (AlienGui::DisplayButton("Cancel", genomeText("Cancel", "common.cancel"))) {
        close();
    }

    ImGui::SameLine();
    auto privacyText = genomeText("", "genome.upload.privacy");
    auto privacyWidth = ImGui::CalcTextSize(privacyText.c_str()).x + ImGui::GetStyle().ItemSpacing.x + ImGui::CalcTextSize(ICON_FA_QUESTION_CIRCLE).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.0f, ImGui::GetContentRegionAvail().x - privacyWidth));
    AlienGui::Text(AlienGui::TextParameters().text(privacyText).style(AlienGui::TextStyle::Decent));
    AlienGui::HelpMarker(
        genomeText("", "genome.upload.privacy.help"));
}

void UploadSimulationDialog::onUpload(WorkspaceType workspaceType)
{
    _resourceDescriptionByTypeAndFolder[{_resourceType, _folder}] = _resourceDescription;
    if (!NetworkValidationService::get().isStringValidForDatabase(_resourceName)
        || !NetworkValidationService::get().isStringValidForDatabase(_resourceDescription)) {
        GenericMessageDialog::get().localizedInformation("Error", "common.error", "genome.upload.invalid");
        return;
    }
    close();

    auto data = [&]() -> std::variant<UploadNetworkResourceRequestData::SimulationData, UploadNetworkResourceRequestData::CreatureData> {
        if (_resourceType == NetworkResourceType_Simulation) {
            return UploadNetworkResourceRequestData::SimulationData{
                .zoom = Viewport::get().getZoomFactor(), .center = Viewport::get().getCenterInWorldPos(), .jpg = _preview.getJpg()};
        } else {
            return UploadNetworkResourceRequestData::CreatureData{.description = GenomeEditorWindow::get().getCurrentGenome(), .jpg = _preview.getJpg()};
        }
    }();
    NetworkTransferController::get().onUpload(UploadNetworkResourceRequestData{
        .folderName = _folder,
        .resourceWithoutFolderName = _resourceName,
        .resourceDescription = _resourceDescription,
        .workspaceType = workspaceType,
        .downloadCache = BrowserWindow::get().getSimulationCache(),
        .data = data});
}
