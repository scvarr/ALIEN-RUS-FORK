#include "DisplaySettingsDialog.h"

#include <algorithm>
#include <sstream>

#include <imgui.h>

#include <Base/LocalizationService.h>
#include <Base/LoggingService.h>

#include "AlienGui.h"
#include "GenericMessageDialog.h"
#include "LocalizationGui.h"
#include "MainLoopController.h"
#include "StyleService.h"
#include "WindowController.h"

#include <GLFW/glfw3.h>

namespace
{
    auto const RightColumnWidth = 310.0f;
}

DisplaySettingsDialog::DisplaySettingsDialog()
    : AlienDialog("Display settings", {760.0f, 350.0f})
{
    setTitleKey("display.title");
}

DisplaySettingsDialog::~DisplaySettingsDialog() = default;

void DisplaySettingsDialog::processIntern()
{
    LocalizationGui::languageSelector(RightColumnWidth);
    _videoModeStrings = createVideoModeStrings();

    auto isFullscreen = _pendingIsFullscreen;

    if (AlienGui::ToggleButton(AlienGui::ToggleButtonParameters().name("Full screen").label(tr("display.fullscreen")), isFullscreen)) {
        _pendingIsFullscreen = isFullscreen;
    }

    ImGui::BeginDisabled(!_pendingIsFullscreen);

    AlienGui::Combo(
        AlienGui::ComboParameters()
            .name("Resolution")
            .label(tr("display.resolution"))
            .textWidth(RightColumnWidth)
            .defaultValue(_origSelectionIndex)
            .values(_videoModeStrings),
        _pendingSelectionIndex);

    ImGui::EndDisabled();

    AlienGui::SliderInt(
        AlienGui::SliderIntParameters()
            .name("Frames per second")
            .label(tr("display.fps"))
            .textWidth(RightColumnWidth)
            .defaultValue(&_origFps)
            .min(20)
            .max(100)
            .tooltip(tr("display.fps.tooltip")),
        &_pendingFps);

    AlienGui::Checkbox(
        AlienGui::CheckboxParameters()
            .name("Adopt scaling from OS")
            .label(tr("display.autoscale"))
            .textWidth(RightColumnWidth)
            .defaultValue(&_origAutoContentScaleFactor)
            .tooltip(tr("display.autoscale.tooltip")),
        &_pendingAutoContentScaleFactor);

    ImGui::BeginDisabled(_pendingAutoContentScaleFactor);
    AlienGui::SliderFloat(
        AlienGui::SliderFloatParameters()
            .name("Content scaling")
            .label(tr("display.scale"))
            .textWidth(RightColumnWidth)
            .defaultValue(&_origContentScaleFactor)
            .min(WindowController::MinContentScaleFactor)
            .max(WindowController::MaxContentScaleFactor)
            .format("%.2f"),
        &_pendingContentScaleFactor);
    ImGui::EndDisabled();

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienGui::Separator();

    if (AlienGui::Button(tr("common.apply") + "###Adopt")) {
        close();
        if (_pendingIsFullscreen) {
            setFullscreen(_pendingSelectionIndex);
        } else {
            WindowController::get().setWindowedMode();
        }
        WindowController::get().setFps(_pendingFps);
        _selectionIndex = _pendingSelectionIndex;

        if (isContentScalingChanged()) {
            auto automatic = _pendingAutoContentScaleFactor;
            auto scaleFactor = _pendingContentScaleFactor;
            delayedExecution([this, automatic, scaleFactor] { onChangeContentScaling(automatic, scaleFactor); });
        }
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienGui::Button(tr("common.cancel") + "###Cancel")) {
        close();
    }
}

void DisplaySettingsDialog::openIntern()
{
    _videoModes.clear();
    if (auto primaryMonitor = glfwGetPrimaryMonitor()) {
        auto videoModesCount = 0;
        auto videoModes = glfwGetVideoModes(primaryMonitor, &videoModesCount);
        _videoModes.assign(videoModes, videoModes + videoModesCount);
    }
    _videoModeStrings = createVideoModeStrings();

    _selectionIndex = getSelectionIndex();
    _origSelectionIndex = _selectionIndex;
    _origFps = WindowController::get().getFps();
    _pendingIsFullscreen = !WindowController::get().isWindowedMode();
    _origAutoContentScaleFactor = WindowController::get().isAutoContentScaleFactor();
    _origContentScaleFactor = WindowController::get().getUserDefinedContentScaleFactor();
    _pendingSelectionIndex = _selectionIndex;
    _pendingFps = _origFps;
    _pendingAutoContentScaleFactor = _origAutoContentScaleFactor;
    _pendingContentScaleFactor = _origContentScaleFactor;
}

bool DisplaySettingsDialog::isContentScalingChanged() const
{
    if (_pendingAutoContentScaleFactor != _origAutoContentScaleFactor) {
        return true;
    }
    return !_pendingAutoContentScaleFactor && _pendingContentScaleFactor != _origContentScaleFactor;
}

void DisplaySettingsDialog::onChangeContentScaling(bool automatic, float scaleFactor) const
{
    GenericMessageDialog::get().localizedYesNo("Content scaling", "display.scale", "display.scale.confirm", [automatic, scaleFactor] {
        WindowController::get().setAutoContentScaleFactor(automatic);
        if (!automatic) {
            WindowController::get().setUserDefinedContentScaleFactor(scaleFactor);
        }
        MainLoopController::get().scheduleClosing();
    });
}

void DisplaySettingsDialog::setFullscreen(int selectionIndex)
{
    if (0 == selectionIndex) {
        WindowController::get().setDesktopMode();
    } else {
        WindowController::get().setUserDefinedResolution(_videoModes.at(selectionIndex - 1));
    }
}

namespace
{
    bool operator==(GLFWvidmode const& m1, GLFWvidmode const& m2)
    {
        return m1.width == m2.width && m1.height == m2.height && m1.redBits == m2.redBits && m1.greenBits == m2.greenBits && m1.blueBits == m2.blueBits
            && m1.refreshRate == m2.refreshRate;
    }
}

int DisplaySettingsDialog::getSelectionIndex() const
{
    auto result = 0;
    if (!WindowController::get().isWindowedMode() && !WindowController::get().isDesktopMode()) {
        auto userMode = WindowController::get().getUserDefinedResolution();
        auto it = std::ranges::find_if(_videoModes, [&](auto const& videoMode) { return videoMode == userMode; });
        if (it != _videoModes.end()) {
            return toInt(std::distance(_videoModes.begin(), it)) + 1;
        }
    }
    return result;
}

namespace
{
    std::string createVideoModeString(GLFWvidmode const& videoMode)
    {
        std::stringstream ss;
        ss << videoMode.width << " x " << videoMode.height << " @ " << videoMode.refreshRate << tr("display.hertz");
        return ss.str();
    }
}

std::vector<std::string> DisplaySettingsDialog::createVideoModeStrings() const
{
    std::vector<std::string> result;
    result.emplace_back(tr("display.desktop"));
    for (auto const& videoMode : _videoModes) {
        result.emplace_back(createVideoModeString(videoMode));
    }

    return result;
}
