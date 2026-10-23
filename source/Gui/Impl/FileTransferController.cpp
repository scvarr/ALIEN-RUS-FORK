#include "FileTransferController.h"

#include <Base/GlobalSettings.h>
#include <Base/LocalizationService.h>
#include <Base/PathHelper.h>

#include <Engine/Interface/SimulationFacade.h>

#include <Persister/Interface/TaskProcessor.h>

#include "GenericFileDialog.h"
#include "GenericMessageDialog.h"
#include "LocalDialogPresentation.h"
#include "NewSimulationService.h"
#include "OverlayController.h"
#include "Viewport.h"

#include <ImFileDialog.h>
#include <Engine/Interface/SimulationFacade.h>
#include <Persister/Interface/PersisterFacade.h>

#include "Persister/Interface/SerializerService.h"

namespace
{
    std::string fileDialogDirectory(std::string const& referencePath)
    {
        try {
            return PathHelper::toLegacyString(PathHelper::fromUtf8(referencePath)).value_or("");
        } catch (std::system_error const&) {
            return {};
        }
    }
}

void FileTransferController::onOpenSimulationDialog()
{
    GenericFileDialog::get().showOpenFileDialog(
        tr("file.open"), tr("file.simulation") + " (*.sim){.sim},.*", fileDialogDirectory(_referencePath), [&](std::filesystem::path const& filename) {
            _referencePath = PathHelper::toUtf8(filename.parent_path());
            onOpenSimulation(filename);
        });
}

void FileTransferController::onOpenSimulation(std::filesystem::path const& filename)
{
    printOverlayMessage(tr("localui.loading"));

    _openSimulationProcessor->executeTask(
        [&](auto const& senderId) {
            auto senderInfo = SenderInfo{.senderId = senderId, .wishResultData = true, .wishErrorInfo = true};
            auto readData = ReadSimulationRequestData{.filename = filename, .initSimulation = false};
            return _PersisterFacade::get()->scheduleReadSimulation(senderInfo, readData);
        },
        [&](auto const& requestId) {
            auto const& data = _PersisterFacade::get()->fetchReadSimulationData(requestId);
            if (auto errorMessage = NewSimulationService::get().loadSimulation(data.simulationDesc)) {
                LocalDialogPresentation::simulationLoadError(*errorMessage);
            }
            printOverlayMessage(PathHelper::toUtf8(data.filename));
        },
        [](auto const& criticalErrors) { LocalDialogPresentation::error("localui.error.open", criticalErrors); });
}

void FileTransferController::onSaveSimulationDialog()
{
    GenericFileDialog::get().showSaveFileDialog(tr("file.save"), tr("file.simulation") + " (*.sim){.sim},.*", fileDialogDirectory(_referencePath), [&](std::filesystem::path const& path) {
        auto firstFilename = path;
        _referencePath = PathHelper::toUtf8(firstFilename.parent_path());
        printOverlayMessage(tr("localui.saving"));
        _saveSimulationProcessor->executeTask(
            [&, firstFilename = firstFilename](auto const& senderId) {
                auto senderInfo = SenderInfo{.senderId = senderId, .wishResultData = false, .wishErrorInfo = true};
                auto readData = SaveSimulationRequestData{firstFilename, Viewport::get().getZoomFactor(), Viewport::get().getCenterInWorldPos()};
                return _PersisterFacade::get()->scheduleSaveSimulation(senderInfo, readData);
            },
            [](auto const&) {},
            [](auto const& criticalErrors) { LocalDialogPresentation::error("localui.error.save", criticalErrors); });
    });
}

void FileTransferController::onOpenGenomeDialog(std::function<void(GenomeDesc const&)> const& openFunc)
{
    GenericFileDialog::get().showOpenFileDialog(
        tr("genome.action.open"),
        tr("genome.file.filter") + " (*.genome){.genome},.*",
        fileDialogDirectory(_referencePath),
        [&, openFunc = openFunc](std::filesystem::path const& path) {
            auto firstFilename = ifd::FileDialog::Instance().GetResult();
            _referencePath = PathHelper::toUtf8(firstFilename.parent_path());
            GenomeDesc genome;
            if (!SerializerService::get().deserializeGenomeFromFile(genome, firstFilename.string())) {
                GenericMessageDialog::get().localizedInformation("Open genome", "genome.action.open", "genome.file.openerror");
            } else {
                openFunc(genome);
            }
        });
}

void FileTransferController::onSaveGenomeDialog(GenomeDesc const& genome, std::function<void()> const& afterSaveFunc)
{
    GenericFileDialog::get().showSaveFileDialog(
        tr("genome.action.save"), tr("genome.file.filter") + " (*.genome){.genome},.*", fileDialogDirectory(_referencePath), [&, genome = genome, afterSaveFunc = afterSaveFunc](std::filesystem::path const& path) {
            auto firstFilename = ifd::FileDialog::Instance().GetResult();
            _referencePath = PathHelper::toUtf8(firstFilename.parent_path());
            if (!SerializerService::get().serializeGenomeToFile(firstFilename.string(), genome)) {
                GenericMessageDialog::get().localizedInformation("Save genome", "genome.action.save", "genome.file.saveerror");
            } else if (afterSaveFunc) {
                afterSaveFunc();
            }
        });
}

void FileTransferController::init()
{

    _openSimulationProcessor = _TaskProcessor::createTaskProcessor(_PersisterFacade::get());
    _saveSimulationProcessor = _TaskProcessor::createTaskProcessor(_PersisterFacade::get());

    _referencePath = GlobalSettings::get().getValue("dialogs.directory.reference path", _referencePath);
}

void FileTransferController::process()
{
    _openSimulationProcessor->process();
    _saveSimulationProcessor->process();
}

void FileTransferController::shutdown()
{
    GlobalSettings::get().setValue("dialogs.directory.reference path", _referencePath);
}
