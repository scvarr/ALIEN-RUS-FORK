#include "SavePictureDialog.h"

#include <algorithm>
#include <filesystem>

#include <imgui.h>

#include <Base/LocalizationService.h>

#include <Base/AlienExceptions.h>
#include <Base/GlobalSettings.h>
#include <Base/LoggingService.h>

#include "AlienGui.h"
#include "DelayedExecutionController.h"
#include "GenericFileDialog.h"
#include "GenericMessageDialog.h"
#include "LocalizationGui.h"
#include "OverlayController.h"
#include "PictureGuiService.h"
#include "SimulationView.h"
#include "Viewport.h"

namespace
{
    auto constexpr TextWidth = 150.0f;
}

SavePictureDialog::SavePictureDialog()
    : AlienDialog("Save picture", {560.0f, 260.0f})
{
    setTitleKey("picture.title");
}

void SavePictureDialog::initIntern()
{
    _width = GlobalSettings::get().getValue("dialogs.save picture.width", _width);
    _height = GlobalSettings::get().getValue("dialogs.save picture.height", _height);
    _referencePath = GlobalSettings::get().getValue("dialogs.save picture.reference path", _referencePath);
}

void SavePictureDialog::shutdownIntern()
{
    GlobalSettings::get().setValue("dialogs.save picture.width", _width);
    GlobalSettings::get().setValue("dialogs.save picture.height", _height);
    GlobalSettings::get().setValue("dialogs.save picture.reference path", _referencePath);
}

void SavePictureDialog::openIntern()
{
    if (_width <= 0 || _height <= 0) {
        auto viewSize = Viewport::get().getViewSize();
        _width = viewSize.x;
        _height = viewSize.y;
    }
}

void SavePictureDialog::processIntern()
{
    AlienGui::InputInt(AlienGui::InputIntParameters().name("Width").label(tr("common.width")).textWidth(TextWidth), _width);
    AlienGui::InputInt(AlienGui::InputIntParameters().name("Height").label(tr("common.height")).textWidth(TextWidth), _height);
    if (AlienGui::Button(tr("picture.viewsize") + "###Adopt view size")) {
        auto viewSize = Viewport::get().getViewSize();
        _width = viewSize.x;
        _height = viewSize.y;
    }
    _width = std::max(1, _width);
    _height = std::max(1, _height);

    LocalizationGui::languageSelector();

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienGui::Separator();

    if (AlienGui::Button(tr("common.ok") + "###OK")) {
        close();
        delayedExecution([this] { onSavePicture(); });
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienGui::Button(tr("common.cancel") + "###Cancel")) {
        close();
    }
}

void SavePictureDialog::onSavePicture()
{
    GenericFileDialog::get().showSaveFileDialog(
        tr("picture.title"), tr("file.picture") + " (*.png){.png},.*", _referencePath, [this](std::filesystem::path const& path) {
            auto referencePath = path;
            _referencePath = referencePath.remove_filename().string();

            auto filename = path;
            if (filename.extension().empty()) {
                filename.replace_extension(".png");
            }
            try {
                PictureGuiService::get().savePng(SimulationView::get().savePicture({_width, _height}), filename);
                printOverlayMessage(filename.filename().string());
            } catch (AlienException const& exception) {
                log(Priority::Important, std::string("Picture save failed: ") + exception.what());
                GenericMessageDialog::get().localizedInformation("Save picture error", "picture.title", "picture.error");
            }
        });
}
