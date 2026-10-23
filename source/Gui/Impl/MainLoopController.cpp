#include "MainLoopController.h"

#include <ranges>
#include <thread>

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include <Fonts/IconsFontAwesome5.h>

#include <Base/Definitions.h>
#include <Base/GlobalSettings.h>
#include <Base/LocalizationService.h>
#include <Base/LoggingService.h>
#include <Base/Resources.h>

#include <Engine/Interface/SimulationFacade.h>
#include <Engine/Interface/TemporalControlService.h>

#include <Persister/Interface/PersisterFacade.h>
#include <Persister/Interface/SerializerService.h>
#include <Persister/Interface/TaskProcessor.h>

#include <Rendering/Interface/RenderingFacade.h>

#include "AboutDialog.h"
#include "AlienGui.h"
#include "AutosaveWindow.h"
#include "BrowserWindow.h"
#include "ConsoleModeController.h"
#include "DeleteUserDialog.h"
#include "DisplaySettingsDialog.h"
#include "DocumentationWindow.h"
#include "EditorController.h"
#include "EvolutionDashboardWindow.h"
#include "ExitDialog.h"
#include "FileTransferController.h"
#include "FpsController.h"
#include "GenericMessageDialog.h"
#include "GenomeEditorWindow.h"
#include "ImageToPatternDialog.h"
#include "InspectionController.h"
#include "LocalizationGui.h"
#include "LocalizationGuiText.h"
#include "LogWindow.h"
#include "LoginDialog.h"
#include "MassEditDialog.h"
#include "McpWindow.h"
#include "NetworkSettingsDialog.h"
#include "NewSimulationDialog.h"
#include "OverlayController.h"
#include "SavePictureDialog.h"
#include "SimulationInteractionController.h"
#include "SimulationParametersMainWindow.h"
#include "SimulationView.h"
#include "SpatialControlWindow.h"
#include "StyleService.h"
#include "TemporalControlWindow.h"
#include "UiController.h"
#include "UploadSimulationDialog.h"
#include "Viewport.h"

#include <Engine/Interface/SimulationFacade.h>
#include <Persister/Interface/PersisterFacade.h>
#include <GLFW/glfw3.h>

namespace
{
    std::chrono::milliseconds::rep const FadeOutDuration = 500;
    std::chrono::milliseconds::rep const FadeInDuration = 500;

    auto const StartupSenderId = "Startup";
}

void MainLoopController::setup()
{
    GlobalSettings::get().loadImGuiSettings();

    _logo = _RenderingFacade::get()->loadTexture(Const::LogoFilename);
    _saveOnExit = GlobalSettings::get().getValue("controllers.main loop.save on exit", _saveOnExit);

    _autosaveProcessor = _TaskProcessor::createTaskProcessor(_PersisterFacade::get());
    _searchProcessor = _TaskProcessor::createTaskProcessor(_PersisterFacade::get());
    _downloadProcessor = _TaskProcessor::createTaskProcessor(_PersisterFacade::get());
}

void MainLoopController::process()
{
    LocalizationService::get().applyPendingLanguage();

    glfwPollEvents();
    _SimulationFacade::get()->checkAndThrowException();

    if (ConsoleModeController::get().isActive()) {
        ConsoleModeController::get().process();
        return;
    }

    StyleService::get().process();

    _RenderingFacade::get()->newFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //   ImGui::ShowDemoWindow(NULL);

    if (_programState == ProgramState::FirstTick) {
        processFirstTick();
    } else if (_programState == ProgramState::LoadingScreen) {
        processLoadingScreen();
    } else if (_programState == ProgramState::FadeOutLoadingScreen) {
        processFadeOutLoadingScreen();
    } else if (_programState == ProgramState::FadeInUI) {
        processFadeInUI();
    } else if (_programState == ProgramState::OperatingMode) {
        processOperatingMode();
    } else if (_programState == ProgramState::ScheduleExit) {
        processScheduleExit();
    } else if (_programState == ProgramState::Exiting) {
        processExiting();
    }

    ImGui::Render();
    _RenderingFacade::get()->render(ImGui::GetDrawData());

    GlobalSettings::get().saveImGuiSettingsIfDirty();
}

void MainLoopController::shutdown()
{
    GlobalSettings::get().setValue("controllers.main loop.save on exit", _saveOnExit);
    GlobalSettings::get().saveImGuiSettings();
}

void MainLoopController::scheduleClosing()
{
    _programState = ProgramState::ScheduleExit;
}

bool MainLoopController::shouldClose() const
{
    return _programState == ProgramState::Finished;
}

bool MainLoopController::isSaveOnExit() const
{
    return _saveOnExit;
}

bool MainLoopController::isOperatingMode() const
{
    return _programState == ProgramState::OperatingMode;
}

void MainLoopController::processFirstTick()
{
    drawLoadingScreen();

    if (std::filesystem::exists(Const::AutosaveFile)) {
        scheduleReadingAutosave();
    } else {
        scheduleSearchingStartupSimulation();
    }
    _programState = ProgramState::LoadingScreen;

    OverlayController::get().process();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void MainLoopController::processLoadingScreen()
{
    drawLoadingScreen();

    _autosaveProcessor->process();
    _searchProcessor->process();
    _downloadProcessor->process();

    OverlayController::get().process();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void MainLoopController::scheduleReadingAutosave()
{
    _autosaveProcessor->executeTask(
        [](auto const& senderId) {
            return _PersisterFacade::get()->scheduleReadSimulation(
                SenderInfo{.senderId = senderId, .wishResultData = true, .wishErrorInfo = true},
                ReadSimulationRequestData{.filename = Const::AutosaveFile, .initSimulation = true});
        },
        [this](auto const& requestId) {
            auto const& data = _PersisterFacade::get()->fetchReadSimulationData(requestId);
            _loadedSimulationName = Const::AutosaveFileWithoutPath.string();
            finishSimulationLoading(data.simulationDesc);
        },
        [this](auto const&) { scheduleSearchingStartupSimulation(); });
}

void MainLoopController::scheduleSearchingStartupSimulation()
{
    _searchProcessor->executeTask(
        [](auto const& senderId) {
            return _PersisterFacade::get()->scheduleGetNetworkResources(
                SenderInfo{.senderId = senderId, .wishResultData = true, .wishErrorInfo = true}, GetNetworkResourcesRequestData());
        },
        [this](auto const& requestId) {
            auto const& data = _PersisterFacade::get()->fetchGetNetworkResourcesData(requestId);
            auto startupSimulation = std::ranges::find_if(data.resourceTOs, [](auto const& resourceTO) {
                return resourceTO->resourceType == NetworkResourceType_Simulation && resourceTO->resourceName == Const::StartupSimulationResourceName;
            });
            if (startupSimulation != data.resourceTOs.end()) {
                scheduleDownloadingStartupSimulation(*startupSimulation);
            } else {
                setupEmptySimulation();
            }
        },
        [this](auto const&) { setupEmptySimulation(); });
}

void MainLoopController::scheduleDownloadingStartupSimulation(NetworkResourceRawTO const& resourceTO)
{
    _downloadProcessor->executeTask(
        [&](auto const& senderId) {
            return _PersisterFacade::get()->scheduleDownloadNetworkResource(
                SenderInfo{.senderId = senderId, .wishResultData = true, .wishErrorInfo = true},
                DownloadNetworkResourceRequestData{
                    .resourceId = resourceTO->id,
                    .resourceName = resourceTO->resourceName,
                    .resourceVersion = resourceTO->version,
                    .resourceType = NetworkResourceType_Simulation,
                    .downloadCache = BrowserWindow::get().getSimulationCache()});
        },
        [this](auto const& requestId) {
            auto const& data = _PersisterFacade::get()->fetchDownloadNetworkResourcesData(requestId);
            auto const& deserializedSim = std::get<SimulationDesc>(data.resourceData);
            _loadedSimulationName = data.resourceName;
            setupSimulation(deserializedSim);
            finishSimulationLoading(deserializedSim);
        },
        [this](auto const&) { setupEmptySimulation(); });
}

void MainLoopController::setupSimulation(SimulationDesc const& simulationDesc)
{
    _SimulationFacade::get()->newSimulation(simulationDesc._timestep, simulationDesc._worldSize, simulationDesc._simulationParameters);
    _SimulationFacade::get()->setSimulationData(simulationDesc._mainData);
    _SimulationFacade::get()->setStatisticsHistory(simulationDesc._statistics);
    _SimulationFacade::get()->setRealTime(simulationDesc._realTime);
}

void MainLoopController::setupEmptySimulation()
{
    GenericMessageDialog::get().localizedInformation("Error", "localui.error", "localui.startup.failure");

    SimulationDesc emptySim;
    emptySim.worldSize({1000, 500}).timestep(0).zoom(12.0f).center({500.0f, 250.0f}).realTime(std::chrono::milliseconds(0));

    setupSimulation(emptySim);
    finishSimulationLoading(emptySim);
}

void MainLoopController::finishSimulationLoading(SimulationDesc const& simulationDesc)
{
    Viewport::get().setCenterInWorldPos(simulationDesc._center);
    Viewport::get().setZoomFactor(simulationDesc._zoom);
    TemporalControlService::get().createFlashback();

    _simulationLoadedTimepoint = std::chrono::steady_clock::now();
    _programState = ProgramState::FadeOutLoadingScreen;
}

void MainLoopController::processFadeOutLoadingScreen()
{
    drawLoadingScreen();

    SimulationView::get().draw();

    decreaseAlphaForFadeOutLoadingScreen();
    if (ImGui::GetStyle().Alpha == 0.0f) {
        _programState = ProgramState::FadeInUI;
        _fadedOutTimepoint = std::chrono::steady_clock::now();
    }
}

void MainLoopController::processFadeInUI()
{
    SimulationView::get().draw();

    pushGlobalStyle();
    processMenubar();
    MainLoopEntityController::get().process();
    OverlayController::get().process();
    SimulationView::get().processSimulationScrollbars();
    popGlobalStyle();

    increaseAlphaForFadeInUI();
    if (ImGui::GetStyle().Alpha == 1.0f) {
        if (!_loadedSimulationName.empty()) {
            printOverlayMessage(_loadedSimulationName);
        }
        _programState = ProgramState::OperatingMode;
    }
}

void MainLoopController::processOperatingMode()
{
    SimulationView::get().draw();

    pushGlobalStyle();
    processMenubar();
    MainLoopEntityController::get().process();
    OverlayController::get().process();
    SimulationView::get().processSimulationScrollbars();
    popGlobalStyle();

    FpsController::get().processForceFps(WindowController::get().getFps());

    auto parameters = _SimulationFacade::get()->getSimulationParameters();
    WindowController::get().updateWindowTitle(parameters.projectName.value);

    if (glfwWindowShouldClose(WindowController::get().getWindowData().window)) {
        scheduleClosing();
    }
}

void MainLoopController::processScheduleExit()
{
    if (_saveOnExit) {
        printOverlayMessage(tr("localui.savingonexit"));

        auto senderInfo = SenderInfo{.senderId = SenderId{StartupSenderId}, .wishResultData = true, .wishErrorInfo = true};
        auto saveData = SaveSimulationRequestData{Const::AutosaveFile, Viewport::get().getZoomFactor(), Viewport::get().getCenterInWorldPos()};
        _saveSimRequestId = _PersisterFacade::get()->scheduleSaveSimulation(senderInfo, saveData);
        _programState = ProgramState::Exiting;
    } else {
        _programState = ProgramState::Finished;
    }
}

void MainLoopController::processExiting()
{
    SimulationView::get().draw();

    pushGlobalStyle();
    processMenubar();
    MainLoopEntityController::get().process();
    OverlayController::get().process();
    SimulationView::get().processSimulationScrollbars();
    popGlobalStyle();

    FpsController::get().processForceFps(WindowController::get().getFps());

    if (auto requestedSimState = _PersisterFacade::get()->getRequestState(_saveSimRequestId)) {
        if (requestedSimState.value() == PersisterRequestState::Finished) {
            _PersisterFacade::get()->fetchSaveSimulationData(_saveSimRequestId);
            _programState = ProgramState::Finished;
        } else if (requestedSimState.value() == PersisterRequestState::Error) {
            _programState = ProgramState::Finished;
        }
    }
}

void MainLoopController::drawLoadingScreen()
{
    // Background color
    _RenderingFacade::get()->clearScreen({0, 0, 0.1f});

    auto& styleRep = StyleService::get();
    auto center = ImGui::GetMainViewport()->GetCenter();
    auto bottom = ImGui::GetMainViewport()->Pos.y + ImGui::GetMainViewport()->Size.y;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    auto imageScale = scale(1.0f);
    ImGui::SetNextWindowSize(ImVec2(_logo.width * imageScale + 10.0f, _logo.height * imageScale + 10.0f));

    ImGuiWindowFlags windowFlags = 0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;
    ImGui::Begin("##startup", NULL, windowFlags);
    ImGui::Image(_logo.textureId, ImVec2(_logo.width * imageScale, _logo.height * imageScale));
    ImGui::End();

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImColor textColor = Const::ProgramVersionTextColor;
    textColor.Value.w *= ImGui::GetStyle().Alpha;

    drawList->AddText(styleRep.getReefLargeFont(), scale(48.0f), {center.x - scale(175), bottom - scale(200)}, textColor, "Artificial Life Environment");

    auto versionString = LocalizationGuiText::format("localui.version", {{"version", Const::ProgramVersion}});
    auto versionFont = LocalizationService::get().getLanguage() == "ru" ? styleRep.getMediumFont() : styleRep.getReefMediumFont();
    auto versionWidth = versionFont->CalcTextSizeA(scale(24.0f), FLT_MAX, 0, versionString.c_str()).x;
    drawList->AddText(versionFont, scale(24.0f), {center.x - versionWidth / 2, bottom - scale(140)}, textColor, versionString.c_str());

    if (GlobalSettings::get().isDebugMode()) {
        drawList->AddText(styleRep.getReefMediumFont(), scale(24.0f), {center.x - scale(24.0f), bottom - scale(100)}, textColor, "DEBUG");
    }
}

void MainLoopController::decreaseAlphaForFadeOutLoadingScreen()
{
    auto now = std::chrono::steady_clock::now();
    auto millisecSinceActivation = std::chrono::duration_cast<std::chrono::milliseconds>(now - *_simulationLoadedTimepoint).count();
    millisecSinceActivation = std::min(FadeOutDuration, millisecSinceActivation);
    auto alphaFactor = 1.0f - toFloat(millisecSinceActivation) / FadeOutDuration;
    ImGui::GetStyle().Alpha = alphaFactor;
}

void MainLoopController::increaseAlphaForFadeInUI()
{
    auto now = std::chrono::steady_clock::now();
    auto millisecSinceActivation = std::chrono::duration_cast<std::chrono::milliseconds>(now - *_fadedOutTimepoint).count();
    millisecSinceActivation = std::min(FadeInDuration, millisecSinceActivation);
    auto alphaFactor = toFloat(millisecSinceActivation) / FadeInDuration;
    ImGui::GetStyle().Alpha = alphaFactor;
}

void MainLoopController::processMenubar()
{
    auto& io = ImGui::GetIO();

    AlienGui::BeginMenuBar();
    AlienGui::MenuShutdownButton([&] { ExitDialog::get().open(); });
    ImGui::Dummy(ImVec2(scale(10.0f), 0.0f));

    AlienGui::BeginMenu(" " ICON_FA_GAMEPAD "  " + tr("menu.simulation") + " ###menu.simulation", _simulationMenuOpened);
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters().name(tr("menu.new") + "###menu.new").keyCtrl(true).key(ImGuiKey_N), [&] { NewSimulationDialog::get().open(); });
    AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr("menu.open") + "###menu.open").keyCtrl(true).key(ImGuiKey_O), [&] {
        FileTransferController::get().onOpenSimulationDialog();
    });
    AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr("menu.save") + "###menu.save").keyCtrl(true).key(ImGuiKey_S), [&] {
        FileTransferController::get().onSaveSimulationDialog();
    });
    AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr("menu.savepicture") + "###menu.savepicture"), [&] { SavePictureDialog::get().open(); });
    AlienGui::MenuSeparator();
    auto running = _SimulationFacade::get()->isSimulationRunning();
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters().name(tr("menu.run") + "###menu.run").key(ImGuiKey_Space).disabled(running).closeMenuWhenItemClicked(false), [&] {
            _SimulationFacade::get()->runSimulation();
            printOverlayMessage(tr("overlay.run"));
        });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters().name(tr("menu.pause") + "###menu.pause").key(ImGuiKey_Space).disabled(!running).closeMenuWhenItemClicked(false), [&] {
            _SimulationFacade::get()->pauseSimulation();
            printOverlayMessage(tr("overlay.pause"));
        });
    AlienGui::EndMenu();

    AlienGui::BeginMenu(" " ICON_FA_GLOBE "  " + tr("menu.network") + " ###menu.network", _networkMenuOpened);
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.browser") + "###menu.browser")
            .keyAlt(true)
            .key(ImGuiKey_B)
            .closeMenuWhenItemClicked(false)
            .selected(BrowserWindow::get().isOn()),
        [&] { BrowserWindow::get().setOn(!BrowserWindow::get().isOn()); });
    AlienGui::MenuSeparator();
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters().name(tr("menu.login") + "###menu.login").keyAlt(true).key(ImGuiKey_L).disabled(NetworkService::get().isLoggedIn()),
        [&] { LoginDialog::get().open(); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters().name(tr("menu.logout") + "###menu.logout").closeMenuWhenItemClicked(false).disabled(!NetworkService::get().isLoggedIn()),
        [&] {
            NetworkService::get().logout();
            BrowserWindow::get().onRefresh();
        });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.uploadsimulation") + "###menu.uploadsimulation")
            .keyAlt(true)
            .key(ImGuiKey_D)
            .disabled(!NetworkService::get().isLoggedIn()),
        [&] { UploadSimulationDialog::get().open(NetworkResourceType_Simulation); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.uploadgenome") + "###menu.uploadgenome")
            .keyAlt(true)
            .key(ImGuiKey_Q)
            .disabled(!NetworkService::get().isLoggedIn()),
        [&] { UploadSimulationDialog::get().open(NetworkResourceType_Genome); });
    AlienGui::MenuSeparator();
    AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr("menu.deleteuser") + "###menu.deleteuser").disabled(!NetworkService::get().isLoggedIn()), [&] {
        DeleteUserDialog::get().open();
    });
    AlienGui::EndMenu();

    AlienGui::BeginMenu(" " ICON_FA_WINDOW_RESTORE "  " + tr("menu.windows") + " ###menu.windows", _windowMenuOpened);
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.temporal") + "###menu.temporal")
            .keyAlt(true)
            .key(ImGuiKey_1)
            .selected(TemporalControlWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { TemporalControlWindow::get().setOn(!TemporalControlWindow::get().isOn()); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.spatial") + "###menu.spatial")
            .keyAlt(true)
            .key(ImGuiKey_2)
            .selected(SpatialControlWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { SpatialControlWindow::get().setOn(!SpatialControlWindow::get().isOn()); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.evolution") + "###menu.evolution")
            .keyAlt(true)
            .key(ImGuiKey_3)
            .selected(EvolutionDashboardWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { EvolutionDashboardWindow::get().setOn(!EvolutionDashboardWindow::get().isOn()); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.parameters") + "###menu.parameters")
            .keyAlt(true)
            .key(ImGuiKey_4)
            .selected(SimulationParametersMainWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { SimulationParametersMainWindow::get().setOn(!SimulationParametersMainWindow::get().isOn()); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.autosave") + "###menu.autosave")
            .keyAlt(true)
            .key(ImGuiKey_5)
            .selected(AutosaveWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { AutosaveWindow::get().setOn(!AutosaveWindow::get().isOn()); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.mcp") + "###menu.mcp")
            .keyAlt(true)
            .key(ImGuiKey_6)
            .selected(McpWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { McpWindow::get().setOn(!McpWindow::get().isOn()); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.log") + "###menu.log")
            .keyAlt(true)
            .key(ImGuiKey_7)
            .selected(LogWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { LogWindow::get().setOn(!LogWindow::get().isOn()); });
    AlienGui::EndMenu();

    AlienGui::BeginMenu(" " ICON_FA_PEN_ALT "  " + tr("menu.editor") + " ###menu.editor", _editorMenuOpened);
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.genomeeditor") + "###menu.genomeeditor")
            .keyAlt(true)
            .key(ImGuiKey_W)
            .selected(GenomeEditorWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { GenomeEditorWindow::get().setOn(!GenomeEditorWindow::get().isOn()); });
    AlienGui::MenuSeparator();
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.editobjects") + "###menu.editobjects")
            .keyAlt(true)
            .key(ImGuiKey_E)
            .selected(SimulationInteractionController::get().isEditMode())
            .closeMenuWhenItemClicked(false),
        [&] { SimulationInteractionController::get().setEditMode(!SimulationInteractionController::get().isEditMode()); });
    AlienGui::MenuSeparator();
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.inspectobjects") + "###menu.inspectobjects")
            .keyAlt(true)
            .key(ImGuiKey_N)
            .disabled(!SimulationInteractionController::get().isEditMode() || !InspectionController::get().isObjectInspectionPossible()),
        [&] { InspectionController::get().onInspectSelectedObjects(); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.inspectgenomes") + "###menu.inspectgenomes")
            .keyAlt(true)
            .key(ImGuiKey_F)
            .disabled(!SimulationInteractionController::get().isEditMode() || !InspectionController::get().isGenomeInspectionPossible()),
        [&] { InspectionController::get().onInspectSelectedGenomes(); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.inspectcreatures") + "###menu.inspectcreatures")
            .keyAlt(true)
            .key(ImGuiKey_P)
            .disabled(!SimulationInteractionController::get().isEditMode() || !InspectionController::get().isCreatureInspectionPossible()),
        [&] { InspectionController::get().onInspectSelectedCreatures(); });
    auto inspectionWindowsActive = InspectionController::get().areInspectionWindowsActive();
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.closeinspections") + "###menu.closeinspections")
            .key(ImGuiKey_Escape)
            .disabled(!SimulationInteractionController::get().isEditMode() || !inspectionWindowsActive),
        [&] { InspectionController::get().onCloseAllInspectorWindows(); });
    AlienGui::MenuSeparator();
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.deselect") + "###menu.deselect")
            .key(ImGuiKey_Escape)
            .disabled(!SimulationInteractionController::get().isEditMode() || !EditorController::get().isDeselectingPossible() || inspectionWindowsActive),
        [&] { EditorController::get().onDeselect(); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.copy") + "###menu.copy")
            .keyCtrl(true)
            .key(ImGuiKey_C)
            .disabled(!SimulationInteractionController::get().isEditMode() || !EditorController::get().isCopyingPossible()),
        [&] { EditorController::get().onCopy(); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.paste") + "###menu.paste")
            .keyCtrl(true)
            .key(ImGuiKey_V)
            .disabled(!SimulationInteractionController::get().isEditMode() || !EditorController::get().isPastingPossible()),
        [&] { EditorController::get().onPaste(); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.delete") + "###menu.delete")
            .key(ImGuiKey_Delete)
            .disabled(!SimulationInteractionController::get().isEditMode() || !EditorController::get().isCopyingPossible()),
        [&] { EditorController::get().onDelete(); });
    AlienGui::EndMenu();

    AlienGui::BeginMenu(" " ICON_FA_EYE "  " + tr("menu.view") + " ###menu.view", _viewMenuOpened);
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.celloverlay") + "###menu.celloverlay")
            .keyAlt(true)
            .key(ImGuiKey_O)
            .selected(SimulationView::get().isOverlayActive())
            .closeMenuWhenItemClicked(false),
        [&] { SimulationView::get().setOverlayActive(!SimulationView::get().isOverlayActive()); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.messageoverlay") + "###menu.messageoverlay")
            .keyAlt(true)
            .key(ImGuiKey_X)
            .selected(OverlayController::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { OverlayController::get().setOn(!OverlayController::get().isOn()); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.renderui") + "###menu.renderui")
            .keyAlt(true)
            .key(ImGuiKey_U)
            .selected(UiController::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { UiController::get().setOn(!UiController::get().isOn()); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.rendersimulation") + "###menu.rendersimulation")
            .keyAlt(true)
            .key(ImGuiKey_I)
            .selected(SimulationView::get().isRenderSimulation())
            .closeMenuWhenItemClicked(false),
        [&] { SimulationView::get().setRenderSimulation(!SimulationView::get().isRenderSimulation()); });
    AlienGui::MenuSeparator();
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.console") + "###menu.console")
            .keyAlt(true)
            .key(ImGuiKey_C)
            .disabled(_programState != ProgramState::OperatingMode),
        [&] { ConsoleModeController::get().activate(); });
    AlienGui::EndMenu();

    AlienGui::BeginMenu(" " ICON_FA_TOOLS "  " + tr("menu.tools") + " ###menu.tools", _toolsMenuOpened);
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters().name(tr("menu.mass") + "###menu.mass").keyAlt(true).key(ImGuiKey_H), [&] { MassEditDialog::get().open(); });
    AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr("menu.imageconverter") + "###menu.imageconverter").keyAlt(true).key(ImGuiKey_Y), [&] {
        ImageToPatternDialog::get().show();
    });
    AlienGui::EndMenu();

    AlienGui::BeginMenu(" " ICON_FA_COG "  " + tr("settings.menu") + " ###settings.menu", _settingsMenuOpened, false);
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters().name(tr("settings.saveonexit") + "###settings.saveonexit").selected(_saveOnExit).closeMenuWhenItemClicked(false),
        [&] { _saveOnExit = !_saveOnExit; });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("settings.darkmode") + "###settings.darkmode")
            .selected(StyleService::get().isDarkMode())
            .closeMenuWhenItemClicked(false),
        [&] { StyleService::get().setDarkMode(!StyleService::get().isDarkMode()); });
    AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr("settings.display") + "###settings.display"), [&] { DisplaySettingsDialog::get().open(); });
    AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr("settings.network") + "###settings.network"), [&] { NetworkSettingsDialog::get().open(); });
    if (_settingsMenuOpened) {
        ImGui::Dummy({scale(280.0f), 0});
        LocalizationGui::languageSelector();
    }
    AlienGui::MenuSeparator();
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("settings.debug") + "###settings.debug")
            .selected(GlobalSettings::get().isDebugMode())
            .closeMenuWhenItemClicked(false),
        [&] { _SimulationFacade::get()->setDebugMode(!GlobalSettings::get().isDebugMode()); });
    AlienGui::EndMenu();

    AlienGui::BeginMenu(" " ICON_FA_LIFE_RING "  " + tr("menu.help") + " ###menu.help", _helpMenuOpened);
    AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr("menu.about") + "###menu.about"), [&] { AboutDialog::get().open(); });
    AlienGui::MenuItem(
        AlienGui::MenuItemParameters()
            .name(tr("menu.documentation") + "###menu.documentation")
            .key(ImGuiKey_F1)
            .selected(DocumentationWindow::get().isOn())
            .closeMenuWhenItemClicked(false),
        [&] { DocumentationWindow::get().setOn(!DocumentationWindow::get().isOn()); });
    AlienGui::EndMenu();
    AlienGui::EndMenuBar();

    // Further hotkeys
    if (!io.WantCaptureKeyboard) {
        if (ImGui::IsKeyPressed(ImGuiKey_F7)) {
            if (WindowController::get().isDesktopMode()) {
                WindowController::get().setWindowedMode();
            } else {
                WindowController::get().setDesktopMode();
            }
        }
    }
}

void MainLoopController::pushGlobalStyle()
{
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, Const::SliderBarWidth);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, Const::WindowsRounding);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)Const::HeaderHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, (ImVec4)Const::HeaderActiveColor);
    ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)Const::HeaderColor);
}

void MainLoopController::popGlobalStyle()
{
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}
