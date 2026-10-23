#include "NetworkTransferController.h"

#include "BrowserPresentation.h"
#include "LocalDialogPresentation.h"

#include <Base/VersionParserService.h>

#include <Engine/Interface/SimulationFacade.h>

#include <Persister/Interface/TaskProcessor.h>

#include "BrowserWindow.h"
#include "EditorController.h"
#include "GenericMessageDialog.h"
#include "GenomeEditorWindow.h"
#include "NewSimulationService.h"
#include "OverlayController.h"
#include <Engine/Interface/SimulationFacade.h>
#include <Persister/Interface/PersisterFacade.h>

void NetworkTransferController::init()
{

    _downloadProcessor = _TaskProcessor::createTaskProcessor(_PersisterFacade::get());
    _uploadProcessor = _TaskProcessor::createTaskProcessor(_PersisterFacade::get());
    _replaceProcessor = _TaskProcessor::createTaskProcessor(_PersisterFacade::get());
    _deleteProcessor = _TaskProcessor::createTaskProcessor(_PersisterFacade::get());
    _editProcessor = _TaskProcessor::createTaskProcessor(_PersisterFacade::get());
    _moveProcessor = _TaskProcessor::createTaskProcessor(_PersisterFacade::get());
}

void NetworkTransferController::onDownload(DownloadNetworkResourceRequestData const& requestData)
{
    printOverlayMessage(BrowserPresentation::text("Downloading ..."));

    _downloadProcessor->executeTask(
        [&](auto const& senderId) {
            return _PersisterFacade::get()->scheduleDownloadNetworkResource(
                SenderInfo{.senderId = senderId, .wishResultData = true, .wishErrorInfo = true}, requestData);
        },
        [&](auto const& requestId) {
            auto data = _PersisterFacade::get()->fetchDownloadNetworkResourcesData(requestId);

            if (data.resourceType == NetworkResourceType_Simulation) {
                if (auto errorMessage = NewSimulationService::get().loadSimulation(std::get<SimulationDesc>(data.resourceData))) {
                    LocalDialogPresentation::simulationLoadError(*errorMessage);
                }
                printOverlayMessage(data.resourceName);
            } else {
                EditorController::get().setOn(true);
                GenomeEditorWindow::get().openTab(std::get<GenomeDesc>(data.resourceData), false);
            }
            if (VersionParserService::get().isVersionNewer(data.resourceVersion)) {
                GenericMessageDialog::get().localizedMarkdownInformation(
                    "Warning", "browser.warning",
                    data.resourceType == NetworkResourceType_Simulation ? "browser.newerversion.simulation" : "browser.newerversion.genome");
            }
        },
        [](auto const& errors) { LocalDialogPresentation::error("browser.error.download", errors); });
}

void NetworkTransferController::onUpload(UploadNetworkResourceRequestData const& requestData)
{
    printOverlayMessage(BrowserPresentation::text("Uploading ..."));

    _uploadProcessor->executeTask(
        [&](auto const& senderId) {
            return _PersisterFacade::get()->scheduleUploadNetworkResource(
                SenderInfo{.senderId = senderId, .wishResultData = true, .wishErrorInfo = true}, requestData);
        },
        [&](auto const& requestId) {
            _PersisterFacade::get()->fetchUploadNetworkResourcesData(requestId);
            BrowserWindow::get().onRefresh();
        },
        [](auto const& errors) { LocalDialogPresentation::error("browser.error.upload", errors); });
}

void NetworkTransferController::onReplace(ReplaceNetworkResourceRequestData const& requestData)
{
    printOverlayMessage(BrowserPresentation::text("Replacing ..."));

    _replaceProcessor->executeTask(
        [&](auto const& senderId) {
            return _PersisterFacade::get()->scheduleReplaceNetworkResource(
                SenderInfo{.senderId = senderId, .wishResultData = true, .wishErrorInfo = true}, requestData);
        },
        [&, resourceId = requestData.resourceId](auto const& requestId) {
            _PersisterFacade::get()->fetchReplaceNetworkResourcesData(requestId);
            BrowserWindow::get().onPreviewPictureChanged(resourceId);
            BrowserWindow::get().onRefresh();
        },
        [](auto const& errors) { LocalDialogPresentation::error("browser.error.replace", errors); });
}

void NetworkTransferController::onDelete(DeleteNetworkResourceRequestData const& requestData)
{
    printOverlayMessage(BrowserPresentation::text("Deleting ..."));

    _deleteProcessor->executeTask(
        [&](auto const& senderId) {
            return _PersisterFacade::get()->scheduleDeleteNetworkResource(
                SenderInfo{.senderId = senderId, .wishResultData = true, .wishErrorInfo = true}, requestData);
        },
        [&](auto const& requestId) {
            _PersisterFacade::get()->fetchDeleteNetworkResourcesData(requestId);
            BrowserWindow::get().onRefresh();
        },
        [](auto const& errors) { LocalDialogPresentation::error("browser.error.delete", errors); });
}

void NetworkTransferController::onEdit(EditNetworkResourceRequestData const& requestData)
{
    printOverlayMessage(BrowserPresentation::text("Applying changes ..."));

    _editProcessor->executeTask(
        [&](auto const& senderId) {
            return _PersisterFacade::get()->scheduleEditNetworkResource(SenderInfo{.senderId = senderId, .wishResultData = true, .wishErrorInfo = true}, requestData);
        },
        [&](auto const& requestId) {
            _PersisterFacade::get()->fetchEditNetworkResourcesData(requestId);
            BrowserWindow::get().onRefresh();
        },
        [](auto const& errors) { LocalDialogPresentation::error("browser.error.edit", errors); });
}

void NetworkTransferController::onMove(MoveNetworkResourceRequestData const& requestData)
{
    printOverlayMessage(BrowserPresentation::text("Changing visibility ..."));

    _moveProcessor->executeTask(
        [&](auto const& senderId) {
            return _PersisterFacade::get()->scheduleMoveNetworkResource(SenderInfo{.senderId = senderId, .wishResultData = true, .wishErrorInfo = true}, requestData);
        },
        [&](auto const& requestId) {
            _PersisterFacade::get()->fetchMoveNetworkResourcesData(requestId);
            BrowserWindow::get().onRefresh();
        },
        [](auto const& errors) { LocalDialogPresentation::error("browser.error.move", errors); });
}

void NetworkTransferController::process()
{
    _downloadProcessor->process();
    _uploadProcessor->process();
    _replaceProcessor->process();
    _deleteProcessor->process();
    _editProcessor->process();
    _moveProcessor->process();
}
