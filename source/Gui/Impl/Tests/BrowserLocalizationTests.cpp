#include <gtest/gtest.h>
#include <imgui_internal.h>
#include <Base/LocalizationService.h>
#include <Persister/Impl/PersisterFacadeImpl.h>
#include <Rendering/Interface/RenderingFacade.h>
#include <Gui/Impl/BrowserController.h>
#include <Gui/Impl/BrowserGalleryWidget.h>
#include <Gui/Impl/BrowserLoginBannerWidget.h>
#include <Gui/Impl/BrowserLoginHintWidget.h>
#include <Gui/Impl/BrowserMetadataSearch.h>
#include <Gui/Impl/BrowserPresentation.h>
#include <Gui/Impl/BrowserTableWidget.h>
#include <Gui/Impl/BrowserUserListWidget.h>
#include <Gui/Impl/BrowserWindow.h>
#include <Gui/Impl/EditSimulationDialog.h>
#include <Gui/Impl/GenericMessageDialog.h>
#include <Gui/Impl/MainLoopEntity.h>
#include <Gui/Impl/NetworkTransferController.h>
#include <Gui/Impl/OverlayController.h>
#include <Gui/Impl/ReplaceSimulationDialog.h>
#include <Gui/Impl/StyleService.h>
#include <Gui/Impl/UploadSimulationDialog.h>
#include <Gui/Impl/WindowController.h>
#include <Gui/Impl/RecorderController.h>
#include <Gui/Impl/RecorderWindow.h>
#include <Gui/Impl/ConsoleModeController.h>
#include <Base/Tests/SettingsTestScope.h>

class BrowserLocalizationTests : public ::testing::Test
{
protected:
    void initTransfers() { NetworkTransferController::get().init(); }
    void clearTransfers()
    {
        auto& controller = NetworkTransferController::get();
        controller._downloadProcessor.reset();
        controller._uploadProcessor.reset();
        controller._replaceProcessor.reset();
        controller._deleteProcessor.reset();
        controller._editProcessor.reset();
        controller._moveProcessor.reset();
    }
    void initBrowser()
    {
        BrowserController::get().init();
        auto& browser = BrowserWindow::get();
        browser._data = BrowserController::get().getData();
        browser._galleryWidget = _BrowserGalleryWidget::create(browser._data);
        browser._tableWidget = _BrowserTableWidget::create(browser._data);
        browser._userListWidget = _BrowserUserListWidget::create(browser._data);
        browser._loginHintWidget = _BrowserLoginHintWidget::create();
        browser._loginBannerWidget = _BrowserLoginBannerWidget::create();
        browser._userTableWidth = 300;
    }

    BrowserData browserData() { return BrowserWindow::get()._data; }
    BrowserGalleryWidget gallery() { return BrowserWindow::get()._galleryWidget; }
    void browserView(bool galleryView)
    {
        BrowserWindow::get()._galleryView = galleryView;
        BrowserWindow::get().processIntern();
    }
    void browserPage(int page) { gallery()->_page = page; }
    int browserPage() { return gallery()->_page; }
    void browserSort(int sorting) { gallery()->_sorting = sorting; }
    int browserSort() { return gallery()->_sorting; }
    std::vector<NetworkResourceRawTO> browserSorted() { return gallery()->getSortedEntries(); }
    void destroyBrowser()
    {
        auto& browser = BrowserWindow::get();
        browser._galleryWidget.reset();
        browser._tableWidget.reset();
        browser._userListWidget.reset();
        browser._loginHintWidget.reset();
        browser._loginBannerWidget.reset();
        browser._data.reset();
        BrowserController::get().shutdown();
    }
    void browserEdit(NetworkResourceTreeTO const& tree, bool folder)
    {
        auto& dialog = EditSimulationDialog::get();
        dialog._treeTO = tree;
        dialog._newName = "Папка/User 100% {name} ### literal";
        dialog._newDescription = "Описание / https://example.invalid/ ### literal";
        dialog.changeTitle(folder ? "Change folder name" : "Change name or description");
        dialog.setTitleKey(folder ? "browser.changefoldername" : "browser.changenameordescription");
        openWithoutSimulation(dialog);
    }
    void browserUpload(int type)
    {
        auto& dialog = UploadSimulationDialog::get();
        dialog._resourceType = type;
        dialog._resourceName = "Мир/User 100% {name} ### literal";
        dialog._resourceDescription = "Описание / https://example.invalid/ ### literal";
        dialog._folder = "Папка/Folder";
        dialog.changeTitle(type == NetworkResourceType_Simulation ? "Upload simulation" : "Upload genome");
        dialog.setTitleKey(type == NetworkResourceType_Simulation ? "browser.upload.title" : "genome.upload.title");
        openWithoutSimulation(dialog);
    }
    void browserReplace(BrowserLeaf const& leaf, int type)
    {
        auto& dialog = ReplaceSimulationDialog::get();
        dialog._leaf = leaf;
        dialog._resourceType = type;
        dialog.changeTitle(type == NetworkResourceType_Simulation ? "Replace simulation" : "Replace genome");
        dialog.setTitleKey(type == NetworkResourceType_Simulation ? "browser.replacesimulation" : "browser.replacegenome");
        openWithoutSimulation(dialog);
    }
    void verifyConsoleRecordingGuard()
    {
        auto& recorder = RecorderController::get();
        recorder.setWaypoints({CameraWaypoint{}, CameraWaypoint{}});
        // State-only fixture: no encoder is created and no MP4 success is claimed.
        recorder._recording = RecorderController::Recording{.schedule = recorder.createRecordingSchedule()};
        EXPECT_TRUE(recorder.isRecording());
        ConsoleModeController::get().activate();
        EXPECT_FALSE(ConsoleModeController::get().isActive());
        recorder._recording.reset();
        recorder.setWaypoints({});
    }
    void submitUpload(WorkspaceType workspace) { UploadSimulationDialog::get().onUpload(workspace); }
    void validUploadData()
    {
        UploadSimulationDialog::get()._resourceName = "Water world";
        UploadSimulationDialog::get()._resourceDescription = "Experiment";
    }
    void invalidUploadName() { UploadSimulationDialog::get()._resourceName = "bad\\name"; }
    std::string confirmation() { return GenericMessageDialog::get()._message; }
    bool hasConfirmationAction() { return GenericMessageDialog::get()._actionText.has_value(); }
    void checkBrowserDialogData()
    {
        EXPECT_EQ("Папка/User 100% {name} ### literal", EditSimulationDialog::get()._newName);
        EXPECT_EQ("Мир/User 100% {name} ### literal", UploadSimulationDialog::get()._resourceName);
        EXPECT_EQ("Описание / https://example.invalid/ ### literal", UploadSimulationDialog::get()._resourceDescription);
    }

    void SetUp() override
    {
        originalScale = WindowController::get().getContentScaleFactor();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = {4000, 3000};
        StyleService::get().setup();
        LocalizationService::get().load("resources/locales");
        select("en");
    }
    void TearDown() override
    {
        WindowController::get()._contentScaleFactor = originalScale;
        ImGui::DestroyContext();
    }
    void select(std::string const& language)
    {
        LocalizationService::get().requestLanguage(language);
        LocalizationService::get().applyPendingLanguage();
    }
    void setTestScale(float factor)
    {
        WindowController::get()._contentScaleFactor = factor;
        ImGui::GetIO().Fonts->Clear();
        ImGui::GetIO().FontDefault = nullptr;
        ImGui::GetStyle() = ImGuiStyle();
        StyleService::get().setup();
    }
    void frame(std::function<void()> const& content)
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos({0, 0});
        auto factor = WindowController::get().getContentScaleFactor();
        ImGui::SetNextWindowSize({1500 * factor, 1000 * factor});
        ImGui::Begin("Browser host");
        content();
        ImGui::End();
        ImGui::Render();
    }
    void openWithoutSimulation(AlienDialog& dialog) { dialog._modalWindow.open(); }
    void closeImmediately(AlienDialog& dialog)
    {
        frame([&] { dialog._modalWindow.process([&] { dialog._modalWindow.close(); }); });
    }
    std::string localErrorDetails() { return GenericMessageDialog::get()._messageArguments.at("details"); }
    float originalScale = 1;
    SettingsTestScope isolatedSettings;
};

namespace
{
    class BrowserRenderStub : public _RenderingFacade
    {
    public:
        static void set(RenderingFacade const& value) { _instance = value; }
        void setWindowHints() override {}
        void setup(GLFWwindow* window) override {}
        void setupSimulationRendering(ImFont* labelFont) override {}
        void shutdown() override {}
        void newFrame() override {}
        void drawSimulation(RenderView const& view) override {}
        void clearScreen(FloatColorRGB const& color) override {}
        void render(ImDrawData* drawData) override {}
        void captureSimulationFrame(int numPrecedingRenderings) override
        {
            ADD_FAILURE() << "Unexpected frame capture in localization fixture";
        }
        std::optional<PictureData> fetchCapturedSimulationFrame() override
        {
            ADD_FAILURE() << "Unexpected frame capture result in localization fixture";
            return std::nullopt;
        }
        PictureData renderSimulationPicture(RenderView const& view) override { return {}; }
        PictureData renderDrawList(ImDrawList* drawList, IntVector2D const& resolution, ImColor const& backgroundColor, int supersampling) override
        {
            return {};
        }
        TextureData loadTexture(std::filesystem::path const& filename) override { return {}; }
        TextureData loadTextureFromMemory(std::string const& encodedImage) override { return {}; }
        TextureData createTexture(uint8_t const* pixels, int width, int height, TextureFormat format, TextureFilter filter) override { return {}; }
        void deleteTexture(TextureData const& texture) override {}
        void deleteTexture(ImTextureID textureId) override {}
    };

    class BrowserPersisterStub : public _PersisterFacade
    {
    public:
        GetNetworkResourcesResultData result;
        std::optional<PersisterRequestState> state = PersisterRequestState::InProgress;
        std::vector<PersisterErrorInfo> errors;
        int scheduled = 0;
        std::optional<UploadNetworkResourceRequestData> uploaded;
        void setup() override {}
        void shutdown() override {}
        void restart() override {}
        bool isBusy() const override { return {}; }
        std::optional<PersisterRequestState> getRequestState(PersisterRequestId const& id) const override { return state; }
        PersisterRequestResult fetchPersisterRequestResult(PersisterRequestId const& id) override { return {}; }
        std::vector<PersisterErrorInfo> fetchAllErrorInfos(SenderId const& senderId) override { return std::exchange(errors, {}); }
        PersisterErrorInfo fetchError(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleSaveSimulation(SenderInfo const& senderInfo, SaveSimulationRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        SaveSimulationResultData fetchSaveSimulationData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleReadSimulation(SenderInfo const& senderInfo, ReadSimulationRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        ReadSimulationResultData fetchReadSimulationData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleLogin(SenderInfo const& senderInfo, LoginRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        LoginResultData fetchLoginData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleGetNetworkResources(SenderInfo const& senderInfo, GetNetworkResourcesRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        GetNetworkResourcesResultData fetchGetNetworkResourcesData(PersisterRequestId const& id) override { return result; }
        PersisterRequestId scheduleDownloadNetworkResource(SenderInfo const& senderInfo, DownloadNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        DownloadNetworkResourceResultData fetchDownloadNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleUploadNetworkResource(SenderInfo const& senderInfo, UploadNetworkResourceRequestData const& data) override
        {
            uploaded = data;
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        UploadNetworkResourceResultData fetchUploadNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleReplaceNetworkResource(SenderInfo const& senderInfo, ReplaceNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        ReplaceNetworkResourceResultData fetchReplaceNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleGetSimulationPictures(SenderInfo const& senderInfo, GetSimulationPicturesRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        GetSimulationPicturesResultData fetchGetSimulationPicturesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleGetUserNamesForReaction(SenderInfo const& senderInfo, GetUserNamesForReactionRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        GetUserNamesForReactionResultData fetchGetUserNamesForReactionData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleDeleteNetworkResource(SenderInfo const& senderInfo, DeleteNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        DeleteNetworkResourceResultData fetchDeleteNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleEditNetworkResource(SenderInfo const& senderInfo, EditNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        EditNetworkResourceResultData fetchEditNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleMoveNetworkResource(SenderInfo const& senderInfo, MoveNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        MoveNetworkResourceResultData fetchMoveNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleToggleReactionNetworkResource(SenderInfo const& senderInfo, ToggleReactionNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        ToggleReactionNetworkResourceResultData fetchToggleReactionNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleGetPeakSimulation(SenderInfo const& senderInfo, GetPeakSimulationRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        GetPeakSimulationResultData fetchGetPeakSimulationData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleSaveDeserializedSimulation(SenderInfo const& senderInfo, SaveDeserializedSimulationRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        SaveDeserializedSimulationResultData fetchSaveDeserializedSimulationData(PersisterRequestId const& id) override { return {}; }
    };

    struct BrowserServiceScope
    {
        RenderingFacade rendering = _RenderingFacade::get();
        PersisterFacade persister = _PersisterFacade::get();
        std::shared_ptr<BrowserPersisterStub> stub = std::make_shared<BrowserPersisterStub>();
        BrowserServiceScope()
        {
            BrowserRenderStub::set(std::make_shared<BrowserRenderStub>());
            _PersisterFacadeImpl::set(stub);
        }
        ~BrowserServiceScope()
        {
            BrowserRenderStub::set(rendering);
            _PersisterFacadeImpl::set(persister);
        }
    };
}

TEST_F(BrowserLocalizationTests, l6bBrowserKeepsWorkspaceTableSortSelectionFilterAndPageAtEveryScale)
{
    BrowserServiceScope services;
    initBrowser();
    auto data = browserData();
    for (int type : {NetworkResourceType_Simulation, NetworkResourceType_Genome}) {
        for (int workspace : {WorkspaceType_AlienProject, WorkspaceType_Public, WorkspaceType_Private}) {
            for (int i = 0; i < 105; ++i) {
                auto raw = std::make_shared<_NetworkResourceRawTO>();
                raw->id = std::to_string(type) + "/" + std::to_string(workspace) + "/" + std::to_string(i);
                raw->resourceType = type;
                raw->workspaceType = workspace;
                raw->resourceName = "Folder/Папка/Мир 100% {name} ### " + std::to_string(i);
                raw->description = "Описание пользователя https://example.invalid/";
                raw->userName = "User/Ёж";
                raw->version = "4.12.0";
                raw->timestamp = "2026-10-23 12:00:00";
                raw->numDownloads = i;
                raw->width = 100;
                raw->height = 200;
                raw->particles = 12345;
                raw->contentSize = 123456;
                auto resources = data->getWorkspace({type, workspace}).rawTOs;
                resources.emplace_back(raw);
                data->setRawTOs({type, workspace}, resources);
            }
        }
    }
    auto& io = ImGui::GetIO();
    io.DisplaySize = {4000, 3000};
    std::map<int, ImGuiTable*> tables;
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (int type : {NetworkResourceType_Simulation, NetworkResourceType_Genome}) {
            for (int workspace : {WorkspaceType_AlienProject, WorkspaceType_Public, WorkspaceType_Private}) {
                data->setCurrentWorkspaceId({type, workspace});
                auto const& current = data->getCurrentWorkspace();
                data->setCollapsedFolderNames(data->getCurrentWorkspaceId(), {{"Folder", "Папка"}});
                frame([&] { browserView(false); });
                frame([&] { browserView(false); });
                data->setSelectedTreeTO(current.treeTOs.back());
                auto selected = data->getSelectedTreeTO();
                data->setFilter("100%");
                browserSort(GallerySorting_MostDownloads);
                browserPage(1);
                ImGuiTable* originalTable = nullptr;
                float originalWidth = 0;
                for (auto language : {"en", "ru", "en"}) {
                    select(language);
                    for (bool galleryView : {true, false}) {
                        for (int pass = 0; pass < 2; ++pass) {
                            ImGui::NewFrame();
                            ImGui::SetNextWindowPos({0, 0});
                            ImGui::SetNextWindowSize({1500 * factor, 900 * factor});
                            ImGui::Begin("Browser host");
                            browserView(galleryView);
                            ImGui::End();
                            ImGui::Render();
                            EXPECT_EQ(0, GImGui->ErrorCountCurrentFrame);
                        }
                        if (!galleryView) {
                            ImGuiTable* resourceTable = nullptr;
                            for (int index = 0; index < GImGui->Tables.GetMapSize(); ++index) {
                                auto table = GImGui->Tables.GetByIndex(index);
                                if (table->LastFrameActive == GImGui->FrameCount && table->ColumnsCount > 5) {
                                    resourceTable = table;
                                }
                            }
                            ASSERT_NE(nullptr, resourceTable);
                            if (!originalTable) {
                                originalTable = resourceTable;
                                originalWidth = resourceTable->Columns[0].WidthGiven;
                            }
                            EXPECT_EQ(originalTable, resourceTable);
                            EXPECT_NEAR(originalWidth, resourceTable->Columns[0].WidthGiven, 1);
                            ASSERT_FALSE(current.sortSpecs.empty());
                            EXPECT_EQ(NetworkResourceColumnId_Timestamp, current.sortSpecs.front().ColumnUserID);
                        }
                        EXPECT_EQ(type, data->getCurrentWorkspaceId().resourceType);
                        EXPECT_EQ(workspace, data->getCurrentWorkspaceId().workspaceType);
                        EXPECT_TRUE(data->isSelected(selected));
                        EXPECT_EQ("100%", data->getFilter());
                        EXPECT_EQ((std::set<std::vector<std::string>>{{"Folder", "Папка"}}), current.collapsedFolderNames);
                        EXPECT_EQ(1, browserPage());
                        EXPECT_EQ(GallerySorting_MostDownloads, browserSort());
                        EXPECT_EQ(105, browserSorted().size());
                        EXPECT_EQ(104, browserSorted().front()->numDownloads);
                        EXPECT_EQ("User/Ёж", current.rawTOs.front()->userName);
                        EXPECT_FALSE(data->isOwner(current.rawTOs.front()));
                    }
                }
            }
        }
    }
    // Picture requests remain pending across switches and use the isolated facade.
    EXPECT_GT(services.stub->scheduled, 0);
    destroyBrowser();
}

TEST_F(BrowserLocalizationTests, l6bEmptyBrowserAndPendingRefreshErrorUseIsolatedResponses)
{
    BrowserServiceScope services;
    initBrowser();
    auto& controller = BrowserController::get();
    controller.refresh(true);
    EXPECT_TRUE(controller.isRefreshing());
    auto scheduled = services.stub->scheduled;
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            for (int workspace : {WorkspaceType_AlienProject, WorkspaceType_Public, WorkspaceType_Private}) {
                browserData()->setCurrentWorkspaceId({browserData()->getCurrentWorkspaceId().resourceType, workspace});
                frame([&] { browserView(false); });
                frame([&] { browserView(true); });
                EXPECT_TRUE(controller.isRefreshing());
                EXPECT_EQ(scheduled, services.stub->scheduled);
                EXPECT_EQ(0, GImGui->ErrorCountCurrentFrame);
            }
        }
    }
    services.stub->state.reset();
    services.stub->errors = {{"403 / Нет доступа 100% {details} ### literal"}};
    controller.process();
    EXPECT_FALSE(controller.isRefreshing());
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        frame([&] { static_cast<MainLoopEntity&>(GenericMessageDialog::get()).process(); });
        EXPECT_EQ("403 / Нет доступа 100% {details} ### literal", localErrorDetails());
        EXPECT_EQ(0, GImGui->ErrorCountCurrentFrame);
    }
    closeImmediately(GenericMessageDialog::get());
    destroyBrowser();
}

TEST_F(BrowserLocalizationTests, l6bOpenResourceDialogsKeepIdentityAndUserTextAtEveryScale)
{
    auto raw = std::make_shared<_NetworkResourceRawTO>();
    raw->resourceName = "Папка/Мир 100% {name} ### literal";
    auto tree = std::make_shared<_NetworkResourceTreeTO>();
    tree->node = BrowserLeaf{.leafName = raw->resourceName, .rawTO = raw};
    ImGui::GetIO().DisplaySize = {4000, 3000};
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (int type : {NetworkResourceType_Simulation, NetworkResourceType_Genome}) {
            raw->resourceType = type;
            browserUpload(type);
            browserEdit(tree, false);
            browserReplace(tree->getLeaf(), type);
            for (AlienDialog* dialog :
                 {static_cast<AlienDialog*>(&UploadSimulationDialog::get()),
                  static_cast<AlienDialog*>(&EditSimulationDialog::get()),
                  static_cast<AlienDialog*>(&ReplaceSimulationDialog::get())}) {
                ImGuiWindow* original = nullptr;
                for (auto language : {"en", "ru", "en"}) {
                    select(language);
                    frame([&] { static_cast<MainLoopEntity&>(*dialog).process(); });
                    frame([&] { static_cast<MainLoopEntity&>(*dialog).process(); });
                    EXPECT_TRUE(dialog->isOpen());
                    auto stableTitle = dialog == &UploadSimulationDialog::get()
                        ? (type == NetworkResourceType_Simulation ? "Upload simulation" : "Upload genome")
                        : dialog == &EditSimulationDialog::get() ? "Change name or description"
                        : type == NetworkResourceType_Simulation ? "Replace simulation"
                                                                 : "Replace genome";
                    auto currentWindow = ImGui::FindWindowByName((std::string("###") + stableTitle).c_str());
                    ASSERT_NE(nullptr, currentWindow);
                    if (!original) {
                        original = currentWindow;
                    }
                    EXPECT_EQ(original, currentWindow);
                    checkBrowserDialogData();
                    EXPECT_EQ(0, GImGui->ErrorCountCurrentFrame);
                }
                closeImmediately(*dialog);
            }
        }
    }
}

TEST_F(BrowserLocalizationTests, l6bRussianCountFormsAndLocalizedMarkdownKeepLiteralPayloads)
{
    select("ru");
    for (auto const& [number, noun] : std::vector<std::pair<int, std::string>>{
             {0, "миров"}, {1, "мир"}, {2, "мира"}, {5, "миров"}, {11, "миров"}, {12, "миров"}, {21, "мир"}, {22, "мира"}, {25, "миров"}, {111, "миров"}}) {
        EXPECT_EQ(std::to_string(number) + " " + noun, BrowserPresentation::count("browser.folder.simulation", number));
    }
    auto& message = GenericMessageDialog::get();
    message.localizedMarkdownInformation("Warning", "browser.warning", "browser.newerversion.genome");
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        frame([&] { static_cast<MainLoopEntity&>(message).process(); });
        EXPECT_TRUE(message.isOpen());
        EXPECT_EQ(0, GImGui->ErrorCountCurrentFrame);
    }
    closeImmediately(message);
}

TEST_F(BrowserLocalizationTests, l6bAllResourceOperationsKeepPendingTasksAndShowRawFailureDetails)
{
    BrowserServiceScope services;
    initTransfers();
    initBrowser();
    auto& controller = NetworkTransferController::get();
    std::vector<std::function<void()>> operations{
        [&] { controller.onDownload({}); },
        [&] { controller.onUpload({}); },
        [&] { controller.onReplace({}); },
        [&] { controller.onDelete({}); },
        [&] { controller.onEdit({}); },
        [&] { controller.onMove({}); }};
    for (auto const& operation : operations) {
        services.stub->state = PersisterRequestState::InProgress;
        operation();
        auto scheduled = services.stub->scheduled;
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            static_cast<MainLoopEntity&>(controller).process();
            EXPECT_EQ(scheduled, services.stub->scheduled);
        }
        services.stub->state.reset();
        services.stub->errors = {{"403 / Нет доступа 100% {details} ### literal"}};
        static_cast<MainLoopEntity&>(controller).process();
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            frame([&] { static_cast<MainLoopEntity&>(GenericMessageDialog::get()).process(); });
            EXPECT_TRUE(GenericMessageDialog::get().isOpen());
            EXPECT_EQ("403 / Нет доступа 100% {details} ### literal", localErrorDetails());
        }
        closeImmediately(GenericMessageDialog::get());
    }
    destroyBrowser();
    clearTransfers();
}

TEST_F(BrowserLocalizationTests, l6cGalleryAndTableUseTheSameUnicodeFilterWithoutChangingSortOrder)
{
    BrowserServiceScope services;
    initBrowser();
    auto data = browserData();
    for (int type : {NetworkResourceType_Simulation, NetworkResourceType_Genome}) {
        data->setCurrentWorkspaceId({type, WorkspaceType_Public});
        auto const& workspace = data->getCurrentWorkspace();
        for (int i = 0; i < 3; ++i) {
            auto item = std::make_shared<_NetworkResourceRawTO>();
            item->id = std::to_string(i);
            item->resourceName = i == 0 ? "МИР/ГЕНОМ/ЁЖ" : i == 1 ? "мир/геном/ёж" : "Other";
            item->numLikesByEmojiType = {{1, i}};
            item->resourceType = type;
            item->workspaceType = WorkspaceType_Public;
            auto resources = workspace.rawTOs;
            resources.emplace_back(item);
            data->setRawTOs(data->getCurrentWorkspaceId(), resources);
        }
        browserSort(0);
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            for (auto query : {"мИр", "ГеНоМ", "ёЖ"}) {
                data->setFilter(query);
                auto sorted = browserSorted();
                ASSERT_EQ(2, sorted.size());
                EXPECT_EQ("1", sorted.at(0)->id);
                EXPECT_EQ("0", sorted.at(1)->id);
                EXPECT_EQ("МИР/ГЕНОМ/ЁЖ", workspace.rawTOs.at(0)->resourceName);
                EXPECT_EQ(2, std::count_if(workspace.treeTOs.begin(), workspace.treeTOs.end(), [](auto const& tree) { return tree->isLeaf(); }));
            }
            data->setFilter("ЕЖ");
            EXPECT_TRUE(workspace.treeTOs.empty());
            EXPECT_TRUE(browserSorted().empty());
        }
    }
    destroyBrowser();
}

TEST_F(BrowserLocalizationTests, syncUploadVisibilityValidationAndConfirmation)
{
    BrowserServiceScope services;
    initBrowser();
    initTransfers();
    for (auto workspace : {WorkspaceType_Public, WorkspaceType_Private}) {
        browserUpload(NetworkResourceType_Simulation);
        validUploadData();
        services.stub->state = PersisterRequestState::InProgress;
        submitUpload(workspace);
        ASSERT_TRUE(services.stub->uploaded);
        EXPECT_EQ(workspace, services.stub->uploaded->workspaceType);
        EXPECT_EQ("Папка/Folder", services.stub->uploaded->folderName);
        EXPECT_EQ("Water world", services.stub->uploaded->resourceWithoutFolderName);
        EXPECT_TRUE(std::holds_alternative<UploadNetworkResourceRequestData::SimulationData>(services.stub->uploaded->data));
        services.stub->state = PersisterRequestState::Finished;
        static_cast<MainLoopEntity&>(NetworkTransferController::get()).process();
        EXPECT_TRUE(GenericMessageDialog::get().isOpen());
        EXPECT_TRUE(hasConfirmationAction());
        EXPECT_NE(std::string::npos, confirmation().find("Water world"));
        closeImmediately(GenericMessageDialog::get());
    }
    auto scheduled = services.stub->scheduled;
    browserUpload(NetworkResourceType_Simulation);
    invalidUploadName();
    submitUpload(WorkspaceType_Public);
    EXPECT_EQ(scheduled, services.stub->scheduled);
    EXPECT_TRUE(GenericMessageDialog::get().isOpen());
    closeImmediately(GenericMessageDialog::get());
    destroyBrowser();
    clearTransfers();
}

TEST_F(BrowserLocalizationTests, syncRefreshPreservesResourceSelectionAcrossReplacementObjects)
{
    BrowserServiceScope services;
    initBrowser();
    auto data = browserData();
    data->setCurrentWorkspaceId({NetworkResourceType_Simulation, WorkspaceType_Public});
    auto first = std::make_shared<_NetworkResourceRawTO>();
    first->id = "stable-resource";
    first->resourceName = "Folder/Мир";
    data->setRawTOs(data->getCurrentWorkspaceId(), {first});
    auto selected = data->getCurrentWorkspace().treeTOs.back();
    ASSERT_TRUE(selected->isLeaf());
    data->setSelectedTreeTO(selected);
    auto replacement = std::make_shared<_NetworkResourceRawTO>(*first);
    replacement->description = "Обновлено";
    data->setRawTOs(data->getCurrentWorkspaceId(), {replacement});
    ASSERT_TRUE(data->getSelectedTreeTO());
    EXPECT_TRUE(data->isSelected(replacement));
    EXPECT_EQ(replacement, data->getSelectedTreeTO()->getLeaf().rawTO);
    data->setRawTOs(data->getCurrentWorkspaceId(), {});
    EXPECT_FALSE(data->getSelectedTreeTO());
    destroyBrowser();
}

TEST_F(BrowserLocalizationTests, syncRecorderPlaybackTransitionsAndWindowVisibility)
{
    auto& recorder = RecorderController::get();
    recorder.setSimulationStartedWithFlight(false);
    recorder.setWaypoints({});
    recorder.startPlayback();
    EXPECT_FALSE(recorder.isPlaying());
    EXPECT_THROW(recorder.startRecording("unused.mp4"), std::exception);
    EXPECT_FALSE(recorder.isRecording());
    recorder.setWaypoints({CameraWaypoint{}, CameraWaypoint{}});
    recorder.startPlayback();
    EXPECT_TRUE(recorder.isPlaying());
    EXPECT_TRUE(recorder.getPlaybackTime());
    recorder.setWaypoints({});
    EXPECT_FALSE(recorder.isPlaying());
    EXPECT_FALSE(recorder.getPlaybackTime());
    recorder.stop();
    auto& window = RecorderWindow::get();
    window.setOn(true);
    EXPECT_TRUE(window.isOn());
    window.setOn(false);
    EXPECT_FALSE(window.isOn());
    recorder.setSimulationStartedWithFlight(true);
}

TEST_F(BrowserLocalizationTests, syncConsoleRejectsActivationWhileRecording)
{
    verifyConsoleRecordingGuard();
}
