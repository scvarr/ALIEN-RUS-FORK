#include "AutosaveWindow.h"

#include <filesystem>

#include <Fonts/AlienIconFont.h>
#include <Fonts/IconsFontAwesome5.h>

#include <Base/GlobalSettings.h>
#include <Base/Resources.h>
#include <Base/StringHelper.h>

#include <Persister/Interface/SavepointTableService.h>

#include "AlienGui.h"
#include "AutosaveController.h"
#include "FileTransferController.h"
#include "GenericMessageDialog.h"
#include "LocalDialogPresentation.h"
#include "StyleService.h"

namespace
{
    auto constexpr RightColumnWidth = 280.0f;
    auto constexpr DefaultSettingsHeight = 130.0f;
}

AutosaveWindow::AutosaveWindow()
    : AlienWindow("Autosave", "windows.autosave", false, true, {49.0f, 71.0f}, {680.0f, 480.0f})
{
    setTitleKey("menu.autosave");
}

void AutosaveWindow::initIntern()
{
    _settingsOpen = GlobalSettings::get().getValue("windows.autosave.settings.open", _settingsOpen);
    _settingsHeight =
        GlobalSettings::get().getValue("windows.autosave.settings.height", scale(DefaultSettingsHeight)) * WindowController::get().getContentScaleCorrection();

    _origAutosaveInterval = GlobalSettings::get().getValue("windows.autosave.interval", _origAutosaveInterval);
    _origSaveMode = GlobalSettings::get().getValue("windows.autosave.mode", _origSaveMode);
    _origNumberOfFiles = GlobalSettings::get().getValue("windows.autosave.number of files", _origNumberOfFiles);
    _origDirectory = GlobalSettings::get().getValue("windows.autosave.directory", (std::filesystem::current_path() / Const::AutosavePath).string());
}

void AutosaveWindow::shutdownIntern()
{
    GlobalSettings::get().setValue("windows.autosave.settings.open", _settingsOpen);
    GlobalSettings::get().setValue("windows.autosave.settings.height", _settingsHeight);
}

void AutosaveWindow::processIntern()
{
    try {
        processToolbar();

        if (ImGui::BeginChild("##child1", {0, -scale(70.0f)})) {
            processHeader();

            //AlienGui::Separator();
            if (ImGui::BeginChild("##child2", {0, _settingsOpen ? -_settingsHeight : -scale(35.0f)})) {
                processTable();
            }
            ImGui::EndChild();

            processSettings();
        }
        ImGui::EndChild();

        processStatusBar();
    } catch (std::runtime_error const& error) {
        LocalDialogPresentation::error("localui.error.details", error.what());
    }
}

void AutosaveWindow::processToolbar()
{
    auto const& savepointTable = AutosaveController::get().getSavepointTable();

    AlienGui::Toolbar(
        AlienGui::ToolbarParameters().id("Autosave"),
        {AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_PLUS).name(tr("localui.autosave.create")).disabled(!savepointTable.has_value()).action([&] {
                 AutosaveController::get().onCreateSavepoint(false);
             })),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_MINUS).name(tr("localui.autosave.delete")).disabled(!static_cast<bool>(_selectedEntry)).action([&] {
                 AutosaveController::get().onDeleteSavepoint(_selectedEntry);
                 _selectedEntry.reset();
             })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_CLEAR)
                                                 .name(tr("localui.autosave.deleteall"))
                                                 .disabled(!savepointTable.has_value() || savepointTable->isEmpty())
                                                 .action([&] {
                                                     GenericMessageDialog::get().localizedYesNo(
                                                         "Delete", "menu.delete", "localui.autosave.confirmdelete", [&]() {
                                                             AutosaveController::get().scheduleCleanup();
                                                             _selectedEntry.reset();
                                                         });
                                                 }))});
}

void AutosaveWindow::processHeader() {}

void AutosaveWindow::processTable()
{
    auto const& savepointTable = AutosaveController::get().getSavepointTable();
    if (!savepointTable.has_value()) {
        ImGui::TextWrapped("%s", tr("localui.autosave.directoryerror").c_str());
        return;
    }
    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX;

    if (ImGui::BeginTable("Save files", 3, flags, ImVec2(-1, -1), 0.0f)) {
        ImGui::TableSetupColumn(
            (tr("localui.autosave.simulation") + "###Simulation").c_str(), ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, scale(140.0f));
        ImGui::TableSetupColumn(
            (tr("localui.timestamp") + "###Timestamp").c_str(), ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, scale(140.0f));
        ImGui::TableSetupColumn(
            (tr("localui.autosave.timestep") + "###Time step").c_str(), ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, scale(100.0f));
        //ImGui::TableSetupColumn("Peak value", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, scale(200.0f));
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Const::TableHeaderColor);

        ImGuiListClipper clipper;
        clipper.Begin(savepointTable->getSize());
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                auto const& entry = savepointTable->at(row);

                ImGui::PushID(row);
                ImGui::TableNextRow(0, scale(23.0f));

                // Project name
                ImGui::TableNextColumn();
                if (entry->state == SavepointState_InQueue) {
                    ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDecentColor.Value);
                    AlienGui::Text(tr("localui.autosave.queued"));
                    ImGui::PopStyleColor();
                } else if (entry->state == SavepointState_InProgress) {
                    ImGui::PushStyleColor(ImGuiCol_Text, Const::TextDecentColor.Value);
                    AlienGui::Text(tr("localui.autosave.progress"));
                    ImGui::PopStyleColor();
                } else if (entry->state == SavepointState_Persisted) {
                    auto triggerLoadSavepoint = AlienGui::ActionButton(AlienGui::ActionButtonParameters().buttonText(ICON_FA_DOWNLOAD));
                    AlienGui::Tooltip(AlienGui::TooltipParameters().text(tr("localui.autosave.load")).delay(false));
                    if (triggerLoadSavepoint) {
                        onLoadSavepoint(entry);
                    }

                    ImGui::SameLine();
                    AlienGui::Text(entry->name);
                } else if (entry->state == SavepointState_Error) {
                    AlienGui::Text(tr("localui.error"));
                }
                ImGui::SameLine();
                ImGui::Dummy({0, scale(22.0f)});

                ImGui::SameLine();
                auto selected = _selectedEntry == entry;
                if (AlienGui::TableRowSelectable(
                        "",
                        &selected,
                        ImGuiSelectableFlags_None,
                        RealVector2D(0, scale(ImGui::GetTextLineHeightWithSpacing()) - ImGui::GetStyle().FramePadding.y))) {
                    _selectedEntry = selected ? entry : nullptr;
                }

                // Timestamp
                ImGui::TableNextColumn();
                if (entry->state == SavepointState_Persisted) {
                    AlienGui::Text(entry->timestamp);
                }

                // Timestep
                ImGui::TableNextColumn();
                if (entry->state == SavepointState_Persisted) {
                    AlienGui::Text(AlienGui::TextParameters().text(StringHelper::format(entry->timestep)).rightAligned(true));
                }

                // Peak
                //ImGui::TableNextColumn();
                //AlienGui::Text(entry->peak);

                //if (!entry->peakType.empty()) {
                //    ImGui::SameLine();
                //    AlienGui::Text(AlienGui::TextParameters().text(" (" + entry->peakType + ")").style(AlienGui::TextStyle::Decent));
                //}

                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}

void AutosaveWindow::processSettings()
{
    ImGui::Spacing();
    ImGui::Spacing();
    if (_settingsOpen) {
        AlienGui::MovableHorizontalSeparator(AlienGui::MovableHorizontalSeparatorParameters().additive(false), _settingsHeight);
    }

    _settingsOpen = AlienGui::BeginTreeNode(
        AlienGui::TreeNodeParameters().name("Settings").label(tr("localui.autosave.settings")).rank(AlienGui::TreeNodeRank::High).defaultOpen(_settingsOpen));
    if (_settingsOpen) {
        if (ImGui::BeginChild("##autosaveSettings", {scale(0), 0})) {
            auto autosaveEnabled = AutosaveController::get().isAutosaveEnabled();
            auto autosaveInterval = AutosaveController::get().getAutosaveInterval();
            if (AlienGui::InputInt(
                    AlienGui::InputIntParameters()
                        .name("Autosave interval (min)")
                        .label(tr("localui.autosave.interval"))
                        .textWidth(RightColumnWidth)
                        .defaultValue(_origAutosaveInterval),
                    autosaveInterval,
                    &autosaveEnabled)) {
                AutosaveController::get().setAutosaveInterval(autosaveInterval);
                AutosaveController::get().setAutosaveEnabled(autosaveEnabled);
            }
            //if (AlienGui::Switcher(
            //        AlienGui::SwitcherParameters()
            //            .name("Catch peaks")
            //            .textWidth(RightColumnWidth)
            //            .defaultValue(_origCatchPeaks)
            //            .readOnly(!_autosaveEnabled)
            //            .values({
            //                "None",
            //                "Genome complexity variance",
            //            })
            //            .tooltip("If activated, the simulation is monitored continuously. When the autosave interval expires, the time at which the selected "
            //                     "measured value was particularly high is saved."),
            //        _catchPeaks)) {
            //    _peakDeserializedSimulation->setDeserializedSimulation(SimulationDesc());
            //}

            auto directory = AutosaveController::get().getDirectory();
            if (AlienGui::InputText(
                    AlienGui::InputTextParameters()
                        .name("Directory")
                        .label(tr("localui.autosave.directory"))
                        .textWidth(RightColumnWidth)
                        .defaultValue(_origDirectory)
                        .folderButton(true)
                        .folderDialogTitle(tr("localui.autosave.selectdirectory"))
                        .tooltip(tr("localui.autosave.directoryhelp")),
                    directory)) {
                AutosaveController::get().setDirectory(directory);
                _selectedEntry.reset();
            }

            auto saveMode = AutosaveController::get().getSaveMode();
            if (AlienGui::Switcher(
                    AlienGui::SwitcherParameters()
                        .name("Mode")
                        .label(tr("localui.autosave.mode"))
                        .values({"Limited save files", "Unlimited save files"})
                        .displayValues({tr("localui.autosave.limited"), tr("localui.autosave.unlimited")})
                        .textWidth(RightColumnWidth)
                        .defaultValue(_origSaveMode),
                    &saveMode)) {
                AutosaveController::get().setSaveMode(saveMode);
            }

            if (saveMode == AutosaveController::SaveMode_Circular) {
                auto numberOfFiles = AutosaveController::get().getNumberOfFiles();
                if (AlienGui::InputInt(
                        AlienGui::InputIntParameters()
                            .name("Number of files")
                            .label(tr("localui.autosave.files"))
                            .textWidth(RightColumnWidth)
                            .defaultValue(_origNumberOfFiles),
                        numberOfFiles)) {
                    AutosaveController::get().setNumberOfFiles(numberOfFiles);
                }
            }
        }
        ImGui::EndChild();
    }
    AlienGui::EndTreeNode();
}

void AutosaveWindow::processStatusBar()
{
    auto const& savepointTable = AutosaveController::get().getSavepointTable();

    std::vector<std::string> statusItems;
    if (!savepointTable.has_value()) {
        statusItems.emplace_back(tr("localui.autosave.invalid"));
    } else if (!AutosaveController::get().isAutosaveEnabled()) {
        statusItems.emplace_back(tr("localui.autosave.unscheduled"));
    } else {
        statusItems.emplace_back(LocalizationGuiText::format(
            "localui.autosave.next", {{"duration", StringHelper::format(AutosaveController::get().getDurationUntilNextAutosave())}}));
    }
    if (savepointTable.has_value()) {
        statusItems.emplace_back(LocalizationGuiText::format("localui.autosave.count", {{"count", std::to_string(savepointTable->getSize())}}));
    }

    AlienGui::Separator();
    for (auto const& item : statusItems) {
        ImGui::TextWrapped("%s", item.c_str());
    }
}

void AutosaveWindow::onLoadSavepoint(SavepointEntry const& entry)
{
    auto path = SavepointTableService::get().calcAbsolutePath(AutosaveController::get().getSavepointTable().value(), entry);
    FileTransferController::get().onOpenSimulation(path);
}
