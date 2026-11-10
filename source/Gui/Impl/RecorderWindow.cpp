#include "RecorderWindow.h"

#include "LocalizationGuiText.h"

#include <filesystem>

#include <boost/algorithm/string.hpp>

#include <imgui.h>

#include <Fonts/AlienIconFont.h>
#include <Fonts/IconsFontAwesome5.h>

#include <Base/GlobalSettings.h>
#include <Base/StringHelper.h>

#include <Engine/Interface/SimulationFacade.h>

#include "AlienGui.h"
#include "FileTransferController.h"
#include "GenericFileDialog.h"
#include "GenericMessageDialog.h"
#include "RecorderController.h"
#include "SimulationView.h"
#include "StyleService.h"

namespace
{
    auto constexpr RightColumnWidth = 280.0f;
    auto constexpr DefaultSettingsHeight = 222.0f;
    auto constexpr RowHeight = 23.0f;
    auto constexpr DefaultDuration = 5.0f;

    bool inputCell(std::string const& id, float& value, char const* format, bool& activated)
    {
        ImGui::SetNextItemWidth(-1);
        auto result = ImGui::InputFloat(id.c_str(), &value, 0, 0, format);
        activated |= ImGui::IsItemActivated();
        return result;
    }

    std::string formatCount(int value)
    {
        return StringHelper::format(static_cast<uint64_t>(value));
    }
}

RecorderWindow::RecorderWindow()
    : AlienWindow("Recorder", "windows.recorder", false, true, {60.0f, 80.0f}, {700.0f, 572.0f})
{}

void RecorderWindow::initIntern()
{
    setTitleKey("localui.sync.recorder");
    _settingsOpen = GlobalSettings::get().getValue("windows.recorder.settings.open", _settingsOpen);
    _settingsHeight = GlobalSettings::get().getValue("windows.recorder.settings.height", scale(DefaultSettingsHeight))
        * WindowController::get().getContentScaleCorrection();
    _videoDirectory = GlobalSettings::get().getValue("windows.recorder.video directory", std::filesystem::current_path().string());
}

void RecorderWindow::shutdownIntern()
{
    GlobalSettings::get().setValue("windows.recorder.settings.open", _settingsOpen);
    GlobalSettings::get().setValue("windows.recorder.settings.height", _settingsHeight);
    GlobalSettings::get().setValue("windows.recorder.video directory", _videoDirectory);
}

void RecorderWindow::processIntern()
{
    auto numWaypoints = toInt(RecorderController::get().getWaypoints().size());
    if (_selectedWaypoint && *_selectedWaypoint >= numWaypoints) {
        _selectedWaypoint.reset();
    }

    processToolbar();

    if (ImGui::BeginChild("##child1", {0, -scale(44.0f)})) {
        if (ImGui::BeginChild("##child2", {0, _settingsOpen ? -_settingsHeight : -scale(35.0f)})) {
            processWaypointTable();
        }
        ImGui::EndChild();

        processSettings();
    }
    ImGui::EndChild();

    processStatusBar();
}

void RecorderWindow::processToolbar()
{
    auto& controller = RecorderController::get();
    auto busy = controller.isPlaying() || controller.isRecording();
    auto hasPath = controller.getWaypoints().size() >= 2;

    AlienGui::Toolbar(
        AlienGui::ToolbarParameters().id("Recorder"),
        {AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_FOLDER_OPEN)
                                                 .name(tr("localui.recorder.openflight"))
                                                 .tooltip(tr("localui.recorder.loadhelp"))
                                                 .disabled(controller.isRecording())
                                                 .action([&] { onOpenFlight(); })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_SAVE)
                                                 .name(tr("localui.recorder.saveflight"))
                                                 .tooltip(tr("localui.recorder.savehelp"))
                                                 .disabled(controller.getWaypoints().empty())
                                                 .action([&] { onSaveFlight(); })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_PLUS)
                                                 .name(tr("localui.recorder.addpoint"))
                                                 .tooltip(tr("localui.recorder.addhelp"))
                                                 .disabled(controller.isRecording())
                                                 .action([&] { onAddWaypoint(); })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_MINUS)
                                                 .name(tr("localui.recorder.deletepoint"))
                                                 .disabled(controller.isRecording() || !_selectedWaypoint.has_value())
                                                 .action([&] { onDeleteWaypoint(); })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_CLEAR)
                                                 .name(tr("localui.recorder.deleteall"))
                                                 .disabled(controller.isRecording() || controller.getWaypoints().empty())
                                                 .action([&] {
                                                     GenericMessageDialog::get().yesNo(tr("browser.delete"), tr("localui.recorder.deleteconfirm"), [&] {
                                                         RecorderController::get().setWaypoints({});
                                                         _selectedWaypoint.reset();
                                                     });
                                                 })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_PLAY)
                                                 .name(tr("localui.recorder.play"))
                                                 .tooltip(tr("localui.recorder.playhelp"))
                                                 .disabled(busy || !hasPath)
                                                 .action([&] { RecorderController::get().startPlayback(); })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_VIDEO)
                                                 .name(tr("localui.recorder.record"))
                                                 .tooltip(tr("localui.recorder.recordhelp"))
                                                 .disabled(busy || !hasPath)
                                                 .action([&] { onRecordVideo(); })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_STOP)
                                                 .name(tr("localui.recorder.stop"))
                                                 .tooltip(tr("localui.recorder.stophelp"))
                                                 .disabled(!busy)
                                                 .action([&] { RecorderController::get().stop(); }))});
}

void RecorderWindow::processWaypointTable()
{
    auto& controller = RecorderController::get();
    auto waypoints = controller.getWaypoints();
    auto changed = false;

    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV
        | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX;

    ImGui::BeginDisabled(controller.isRecording());
    if (ImGui::BeginTable("Waypoints", 7, flags, ImVec2(-1, -1), 0.0f)) {
        ImGui::TableSetupColumn((tr("localui.recorder.waypoint") + "###" + "Waypoint").c_str(), ImGuiTableColumnFlags_WidthFixed, scale(70.0f));
        ImGui::TableSetupColumn((tr("localui.recorder.centerx") + "###" + "Center x").c_str(), ImGuiTableColumnFlags_WidthFixed, scale(80.0f));
        ImGui::TableSetupColumn((tr("localui.recorder.centery") + "###" + "Center y").c_str(), ImGuiTableColumnFlags_WidthFixed, scale(80.0f));
        ImGui::TableSetupColumn((tr("localui.recorder.zoom") + "###" + "Zoom").c_str(), ImGuiTableColumnFlags_WidthFixed, scale(65.0f));
        ImGui::TableSetupColumn((tr("localui.recorder.duration") + "###" + "Duration").c_str(), ImGuiTableColumnFlags_WidthFixed, scale(75.0f));
        ImGui::TableSetupColumn((tr("localui.recorder.timelapse") + "###" + "Time-lapse").c_str(), ImGuiTableColumnFlags_WidthFixed, scale(80.0f));
        ImGui::TableSetupColumn((tr("localui.recorder.view") + "###" + "View").c_str(), ImGuiTableColumnFlags_WidthFixed, scale(60.0f));
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Const::TableHeaderColor);

        for (int row = 0; row < toInt(waypoints.size()); ++row) {
            auto& waypoint = waypoints.at(row);

            ImGui::PushID(row);
            ImGui::TableNextRow(0, scale(RowHeight));

            ImGui::TableNextColumn();
            auto selected = _selectedWaypoint == row;
            if (AlienGui::TableRowSelectable("", &selected, ImGuiSelectableFlags_None, RealVector2D(0, scale(RowHeight) - ImGui::GetStyle().FramePadding.y))) {
                _selectedWaypoint = selected ? std::make_optional(row) : std::nullopt;
            }
            ImGui::SameLine();
            AlienGui::Text(std::to_string(row + 1));

            auto selectRow = false;

            ImGui::TableNextColumn();
            changed |= inputCell("##centerX", waypoint.center.x, "%.1f", selectRow);

            ImGui::TableNextColumn();
            changed |= inputCell("##centerY", waypoint.center.y, "%.1f", selectRow);

            ImGui::TableNextColumn();
            changed |= inputCell("##zoom", waypoint.zoom, "%.2f", selectRow);

            // The duration leads from the previous waypoint to this one
            ImGui::TableNextColumn();
            if (row > 0) {
                changed |= inputCell("##duration", waypoint.duration, "%.1f s", selectRow);
            } else {
                AlienGui::Text(AlienGui::TextParameters().text(tr("localui.recorder.start")).style(AlienGui::TextStyle::Decent));
            }

            ImGui::TableNextColumn();
            changed |= inputCell("##timeLapse", waypoint.timeLapse, "%.2fx", selectRow);

            ImGui::TableNextColumn();
            if (AlienGui::ActionButton(AlienGui::ActionButtonParameters().buttonText(ICON_FA_EYE))) {
                controller.moveViewToWaypoint(waypoint);
            }
            AlienGui::Tooltip(AlienGui::TooltipParameters().text(tr("localui.recorder.showpoint")).delay(false));
            ImGui::SameLine();
            if (AlienGui::ActionButton(AlienGui::ActionButtonParameters().buttonText(ICON_FA_CROSSHAIRS))) {
                auto view = controller.createWaypointFromView();
                waypoint.center = view.center;
                waypoint.zoom = view.zoom;
                changed = true;
                selectRow = true;
            }
            AlienGui::Tooltip(AlienGui::TooltipParameters().text(tr("localui.recorder.takeview")).delay(false));

            if (selectRow) {
                _selectedWaypoint = row;
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::EndDisabled();

    if (changed) {
        controller.setWaypoints(waypoints);
    }
}

void RecorderWindow::processSettings()
{
    ImGui::Spacing();
    ImGui::Spacing();
    if (_settingsOpen) {
        AlienGui::MovableHorizontalSeparator(AlienGui::MovableHorizontalSeparatorParameters().additive(false), _settingsHeight);
    }

    _settingsOpen = AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Settings").label(tr("localui.recorder.settings")).rank(AlienGui::TreeNodeRank::High).defaultOpen(_settingsOpen));
    if (_settingsOpen) {
        if (ImGui::BeginChild("##recorderSettings", {scale(0), 0})) {
            auto& controller = RecorderController::get();
            ImGui::BeginDisabled(controller.isRecording());

            auto smoothing = controller.getSmoothing();
            if (AlienGui::SliderFloat(
                    AlienGui::SliderFloatParameters()
                        .name("Smoothing").label(tr("localui.recorder.smoothing"))
                        .textWidth(RightColumnWidth)
                        .min(0)
                        .max(10.0f)
                        .format("%.2f s")
                        .defaultValue(&RecorderController::DefaultSmoothing)
                        .tooltip(tr("localui.recorder.smoothhelp")),
                    &smoothing)) {
                controller.setSmoothing(smoothing);
            }

            auto fps = controller.getFps();
            if (AlienGui::InputInt(
                    AlienGui::InputIntParameters().name("Frames per second").label(tr("localui.recorder.fps")).textWidth(RightColumnWidth).defaultValue(RecorderController::DefaultFps),
                    fps)) {
                controller.setFps(fps);
            }

            auto bitRate = controller.getBitRate();
            if (AlienGui::InputInt(
                    AlienGui::InputIntParameters()
                        .name("Bit rate (MBit/s)").label(tr("localui.recorder.bitrate"))
                        .textWidth(RightColumnWidth)
                        .defaultValue(RecorderController::DefaultBitRate)
                        .tooltip(tr("localui.recorder.bithelp")),
                    bitRate)) {
                controller.setBitRate(bitRate);
            }

            auto timestepsPerSecond = controller.getTimestepsPerSecond();
            if (AlienGui::InputInt(
                    AlienGui::InputIntParameters()
                        .name("Time steps per second at 1x").label(tr("localui.recorder.steps"))
                        .textWidth(RightColumnWidth)
                        .defaultValue(RecorderController::DefaultTimestepsPerSecond)
                        .tooltip(tr("localui.recorder.stephelp")),
                    timestepsPerSecond)) {
                controller.setTimestepsPerSecond(timestepsPerSecond);
            }

            auto timeLapseAndZoomShown = controller.isTimeLapseAndZoomShown();
            if (AlienGui::Checkbox(
                    AlienGui::CheckboxParameters()
                        .name("Show time-lapse and zoom").label(tr("localui.recorder.showfactors"))
                        .textWidth(RightColumnWidth)
                        .defaultValue(&RecorderController::DefaultTimeLapseAndZoomShown)
                        .tooltip(tr("localui.recorder.factorshelp")),
                    &timeLapseAndZoomShown)) {
                controller.setTimeLapseAndZoomShown(timeLapseAndZoomShown);
            }

            auto simulationStarted = controller.isSimulationStartedWithFlight();
            if (AlienGui::Checkbox(
                    AlienGui::CheckboxParameters()
                        .name("Run simulation during flight").label(tr("localui.recorder.run"))
                        .textWidth(RightColumnWidth)
                        .defaultValue(&RecorderController::DefaultSimulationStartedWithFlight)
                        .tooltip(tr("localui.recorder.runhelp")),
                    &simulationStarted)) {
                controller.setSimulationStartedWithFlight(simulationStarted);
            }

            ImGui::EndDisabled();
        }
        ImGui::EndChild();
    }
    AlienGui::EndTreeNode();
}

void RecorderWindow::processStatusBar()
{
    auto& controller = RecorderController::get();

    std::vector<std::string> statusItems;
    std::optional<std::string> warningItem;
    if (auto progress = controller.getRecordingProgress()) {
        statusItems.emplace_back(LocalizationGuiText::format("localui.recorder.frame", {{"current", formatCount(progress->frame + 1)}, {"total", formatCount(progress->numFrames)}}));
        statusItems.emplace_back(std::to_string(progress->resolution.x) + " x " + std::to_string(progress->resolution.y));
        if (!SimulationView::get().isRenderSimulation()) {
            warningItem = tr("localui.recorder.notrendered");
        }
    } else if (auto playbackTime = controller.getPlaybackTime()) {
        statusItems.emplace_back(
            LocalizationGuiText::format("localui.recorder.playtime", {{"current", StringHelper::format(*playbackTime, 1)}, {"total", StringHelper::format(controller.createPath().getDuration(), 1)}}));
    } else {
        auto numWaypoints = controller.getWaypoints().size();
        statusItems.emplace_back(LocalizationGuiText::format("localui.recorder.points", {{"count", std::to_string(numWaypoints)}}));
        if (numWaypoints >= 2) {
            updateVideoInfo();
            statusItems.emplace_back(LocalizationGuiText::format("localui.recorder.seconds", {{"count", StringHelper::format(controller.createPath().getDuration(), 1)}}));
            statusItems.emplace_back(LocalizationGuiText::format("localui.recorder.frames", {{"count", formatCount(_videoInfo->numFrames)}}));
            statusItems.emplace_back(LocalizationGuiText::format("localui.recorder.stepscount", {{"count", StringHelper::format(_videoInfo->numTimesteps)}}));
        }
    }

    AlienGui::StatusBar(AlienGui::StatusBarParameters().infoItems(statusItems).warningItem(warningItem));
}

void RecorderWindow::onOpenFlight()
{
    FileTransferController::get().onOpenCameraFlightDialog([this] { _selectedWaypoint.reset(); });
}

void RecorderWindow::onSaveFlight()
{
    FileTransferController::get().onSaveCameraFlightDialog();
}

void RecorderWindow::onAddWaypoint()
{
    auto& controller = RecorderController::get();
    auto waypoints = controller.getWaypoints();
    auto index = _selectedWaypoint ? *_selectedWaypoint + 1 : toInt(waypoints.size());

    // The new waypoint continues the time-lapse of its predecessor
    auto waypoint = controller.createWaypointFromView();
    waypoint.duration = index >= 2 ? waypoints.at(index - 1).duration : DefaultDuration;
    if (index >= 1) {
        waypoint.timeLapse = waypoints.at(index - 1).timeLapse;
    }
    waypoints.insert(waypoints.begin() + index, waypoint);
    controller.setWaypoints(waypoints);
    _selectedWaypoint = index;
}

void RecorderWindow::onDeleteWaypoint()
{
    auto& controller = RecorderController::get();
    auto waypoints = controller.getWaypoints();
    waypoints.erase(waypoints.begin() + *_selectedWaypoint);
    controller.setWaypoints(waypoints);
    if (*_selectedWaypoint >= toInt(waypoints.size())) {
        _selectedWaypoint = waypoints.empty() ? std::nullopt : std::make_optional(toInt(waypoints.size()) - 1);
    }
}

void RecorderWindow::onRecordVideo()
{
    GenericFileDialog::get().showSaveFileDialog(tr("localui.recorder.record"), tr("localui.recorder.videofilter") + " (*.mp4){.mp4},.*", _videoDirectory, [this](std::filesystem::path const& path) {
        // Videos are always written as MP4 files, so that a file selected by mistake, e.g. a program, is not overwritten
        auto filename = path;
        if (!boost::iequals(filename.extension().string(), ".mp4")) {
            filename += ".mp4";
        }
        _videoDirectory = std::filesystem::path(filename).remove_filename().string();
        try {
            RecorderController::get().startRecording(filename);
        } catch (AlienException const& exception) {
            GenericMessageDialog::get().information(tr("localui.recorder.record"), std::string(tr("localui.recorder.recorderror")) + exception.what());
        }
    });
}

void RecorderWindow::updateVideoInfo()
{
    auto& controller = RecorderController::get();
    auto isUpToDate = _videoInfo && _videoInfo->waypoints == controller.getWaypoints() && _videoInfo->smoothing == controller.getSmoothing()
        && _videoInfo->fps == controller.getFps() && _videoInfo->timestepsPerSecond == controller.getTimestepsPerSecond();

    // While a value is edited, the previous key figures are kept, so that typing or dragging does not recalculate a long schedule each frame
    if (isUpToDate || (_videoInfo && ImGui::IsAnyItemActive())) {
        return;
    }
    auto schedule = controller.createRecordingSchedule();
    _videoInfo = VideoInfo{
        .waypoints = controller.getWaypoints(),
        .smoothing = controller.getSmoothing(),
        .fps = controller.getFps(),
        .timestepsPerSecond = controller.getTimestepsPerSecond(),
        .numFrames = schedule.getNumFrames(),
        .numTimesteps = schedule.getTotalTimesteps(),
    };
}
