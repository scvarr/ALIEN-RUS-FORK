#include <gtest/gtest.h>
#include <imgui_internal.h>

#include <boost/json.hpp>
#include <future>

#include <McpTools/Impl/McpToolsFacadeImpl.h>
#include <Gui/Impl/GenericMessageDialog.h>
#include <Gui/Impl/LocalizationGuiText.h>
#include <Gui/Impl/McpController.h>
#include <Gui/Impl/McpPresentation.h>
#include <Gui/Impl/McpSettingsDialog.h>
#include <Gui/Impl/McpWindow.h>
#include <Gui/Impl/OverlayController.h>
#include <Base/Tests/SettingsTestScope.h>

#include "McpServerStub.h"

class McpLocalizationTests : public ::testing::Test
{
protected:
    class Tools : public _McpToolsFacadeImpl
    {
    public:
        std::string getInstructions() override { return "Machine instructions / {url} ### literal"; }
        std::vector<McpTool> getTools(McpToolContext& context) override { return {}; }
    };

    void SetUp() override
    {
        originalTools = _McpToolsFacade::get();
        _McpToolsFacadeImpl::set(std::make_shared<Tools>());
        originalScale = WindowController::get().getContentScaleFactor();
        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().DisplaySize = {4000, 3000};
        StyleService::get().setup();
        LocalizationService::get().load("resources/locales");
        select("en");
        McpServerStub::available = true;
        McpServerStub::starts.clear();
        McpServerStub::stops = 0;
        McpController::get().setPort(8765);
        McpController::get().clearCommandLog();
    }

    void TearDown() override
    {
        McpController::get().stopServer();
        McpController::get().clearCommandLog();
        McpServerStub::tools.clear();
        frame([&] {
            auto& settings = McpSettingsDialog::get();
            settings._modalWindow.process([&] { settings._modalWindow.close(); });
            auto& message = GenericMessageDialog::get();
            message._modalWindow.process([&] { message._modalWindow.close(); });
        });
        WindowController::get()._contentScaleFactor = originalScale;
        OverlayController::get()._messageStartTimepoint.reset();
        _McpToolsFacadeImpl::set(originalTools);
        ImGui::DestroyContext();
    }

    void select(std::string const& language)
    {
        LocalizationService::get().requestLanguage(language);
        LocalizationService::get().applyPendingLanguage();
    }

    void scaleForTest(float factor)
    {
        WindowController::get()._contentScaleFactor = factor;
        ImGui::GetIO().Fonts->Clear();
        ImGui::GetIO().FontDefault = nullptr;
        ImGui::GetStyle() = ImGuiStyle();
        StyleService::get().setup();
    }

    void frame(std::function<void()> const& draw)
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({1800, 2400});
        ImGui::Begin("MCP test host");
        draw();
        ImGui::End();
        ImGui::Render();
        EXPECT_EQ(0, GImGui->ErrorCountCurrentFrame);
    }

    void checkLifecycle()
    {
        auto& controller = McpController::get();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            controller.setServerRunning(true);
            EXPECT_TRUE(controller.isServerRunning());
            EXPECT_EQ(LocalizationGuiText::format("mcp.started", {{"url", controller.getServerUrl()}}), OverlayController::get()._message);
            EXPECT_EQ("alien", McpServerStub::name);
            EXPECT_EQ("Machine instructions / {url} ### literal", McpServerStub::instructions);
            auto starts = McpServerStub::starts.size();
            controller.setServerRunning(true);
            controller.setPort(controller.getPort());
            EXPECT_EQ(starts, McpServerStub::starts.size());
            controller.setPort(9000 + static_cast<int>(starts));
            EXPECT_EQ(starts + 1, McpServerStub::starts.size());
            EXPECT_EQ(controller.getPort(), McpServerStub::starts.back());
            controller.setServerRunning(false);
            EXPECT_FALSE(controller.isServerRunning());
            EXPECT_EQ(tr("mcp.serverstopped"), OverlayController::get()._message);
        }
        controller.setPort(-100);
        EXPECT_EQ(1, controller.getPort());
        controller.setPort(70000);
        EXPECT_EQ(65535, controller.getPort());
        EXPECT_EQ("http://127.0.0.1:65535/mcp", controller.getServerUrl());
        McpServerStub::available = false;
        controller.setServerRunning(true);
        EXPECT_FALSE(controller.isServerRunning());
        auto& message = GenericMessageDialog::get();
        EXPECT_EQ("65535", message._messageArguments.at("port"));
        ImGuiID id = 0;
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            frame([&] {
                message._modalWindow.process([&] {
                    if (!id) {
                        id = ImGui::GetCurrentWindow()->ID;
                    }
                    EXPECT_EQ(id, ImGui::GetCurrentWindow()->ID);
                    message.processIntern();
                });
            });
            EXPECT_TRUE(message.isOpen());
            auto text = LocalizationGuiText::format(message._messageKey, message._messageArguments);
            EXPECT_NE(std::string::npos, text.find("65535"));
        }
    }

    void checkWindowAndSettings()
    {
        auto& controller = McpController::get();
        auto& settings = McpSettingsDialog::get();
        controller._toolGroups = {{"Technical group", {"alien_get_simulation", "alien_set_view"}}};
        controller.setPort(8765);
        McpWindow::get().setOn(true);
        for (auto factor : {1.0f, 1.5f, 2.0f}) {
            scaleForTest(factor);
            settings.open();
            settings._port = 12345;
            ImGuiID modalId = 0;
            ImGuiID inputId = 0;
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                for (int pass = 0; pass < 3; ++pass) {
                    frame([&] {
                        ImGui::SetNextWindowSize({620 * factor, 700 * factor});
                        static_cast<MainLoopEntity&>(McpWindow::get()).process();
                        settings._modalWindow.process([&] {
                            auto window = ImGui::GetCurrentWindow();
                            if (!modalId) {
                                modalId = window->ID;
                            }
                            EXPECT_EQ(modalId, window->ID);
                            ImGui::PushID("Port");
                            auto id = ImGui::GetID("###input");
                            ImGui::PopID();
                            if (!inputId) {
                                inputId = id;
                                ImGui::SetKeyboardFocusHere();
                            }
                            EXPECT_EQ(inputId, id);
                            settings.processIntern();
                            EXPECT_LE(GImGui->LastItemData.Rect.Max.x, window->WorkRect.Max.x + 1);
                            EXPECT_LE(GImGui->LastItemData.Rect.Max.y, window->WorkRect.Max.y + 1);
                        });
                    });
                }
                EXPECT_EQ(inputId, GImGui->ActiveId);
                EXPECT_EQ(12345, settings._port);
                EXPECT_EQ(8765, controller.getPort());
                EXPECT_TRUE(settings.isOpen());
                EXPECT_TRUE(McpWindow::get().isOn());
                EXPECT_TRUE(McpServerStub::starts.empty());
            }
            frame([&] { settings._modalWindow.process([&] { settings._modalWindow.close(); }); });
        }
    }

    void checkCommands()
    {
        auto& controller = McpController::get();
        boost::json::object arguments{{"text", "Мир/User 100% {tools} ### literal"}, {"port", 12345}, {"enabled", true}};
        auto serialized = boost::json::serialize(arguments);
        auto raw = std::string("Result/Ёж 100% {url} ### literal\nHTTP 400");
        for (auto factor : {1.0f, 1.5f, 2.0f}) {
            scaleForTest(factor);
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                for (bool error : {false, true}) {
                    McpTool tool{.name = "alien_test", .handler = [&](boost::json::object const& actual) {
                                     EXPECT_EQ(serialized, boost::json::serialize(actual));
                                     return McpToolResult{.text = raw, .images = {{"image/png", "literal/base64=="}}, .isError = error};
                                 }};
                    auto wrapped = controller.wrapTool(tool);
                    auto pending = std::async(std::launch::async, [&] { return wrapped.handler(arguments); });
                    // Pump only this controller's queue; no simulation or world facade is called.
                    while (pending.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
                        decltype(controller._pendingTasks) tasks;
                        {
                            std::lock_guard lock(controller._pendingTasksMutex);
                            tasks.swap(controller._pendingTasks);
                        }
                        for (auto const& task : tasks) {
                            task();
                        }
                    }
                    auto result = pending.get();
                    EXPECT_EQ(raw, result.text);
                    EXPECT_EQ(error, result.isError);
                    ASSERT_EQ(1, result.images.size());
                    EXPECT_EQ("literal/base64==", result.images.front().data);
                    auto const& entry = controller.getCommandLog().back();
                    EXPECT_EQ("alien_test " + serialized, entry.command);
                    EXPECT_EQ(raw, entry.result);
                    EXPECT_EQ(error, entry.isError);
                    auto time = entry.time;
                    frame([&] { McpWindow::get().processIntern(); });
                    EXPECT_EQ(time, controller.getCommandLog().back().time);
                }
            }
        }
        auto count = controller.getCommandLog().size();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            frame([&] { McpWindow::get().processIntern(); });
            EXPECT_EQ(count, controller.getCommandLog().size());
        }
        controller.clearCommandLog();
        frame([&] { McpWindow::get().processIntern(); });
        EXPECT_TRUE(controller.getCommandLog().empty());
        controller._stopping = true;
        EXPECT_EQ("The MCP server has been stopped.", controller.executeOnMainThread([](auto const&) {}).text);
        controller._stopping = false;
    }

    void pressSettingsButton(std::string const& canonical)
    {
        auto& settings = McpSettingsDialog::get();
        ImVec2 target;
        auto draw = [&] { settings.processIntern(); };
        frame([&] {
            auto id = ImGui::GetID(canonical.c_str());
            ImGui::FocusWindow(ImGui::GetCurrentWindow());
            ImGui::SetNavID(id, ImGuiNavLayer_Main, 0, ImRect());
            draw();
            auto window = ImGui::GetCurrentWindow();
            target = ImGui::WindowRectRelToAbs(window, window->NavRectRel[ImGuiNavLayer_Main]).GetCenter();
            ImGui::ClearActiveID();
        });
        auto& io = ImGui::GetIO();
        io.AddMousePosEvent(target.x, target.y);
        frame(draw);
        io.AddMouseButtonEvent(0, true);
        frame(draw);
        io.AddMouseButtonEvent(0, false);
        frame(draw);
    }

    void checkSettingsActionsAndGuideLayout()
    {
        auto& controller = McpController::get();
        auto& settings = McpSettingsDialog::get();
        controller.setServerRunning(true);
        for (float factor : {1.0f, 1.5f, 2.0f}) {
            scaleForTest(factor);
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                settings.open();
                auto port = controller.getPort() + 1;
                settings._port = port;
                auto starts = McpServerStub::starts.size();
                pressSettingsButton("Cancel");
                EXPECT_EQ(port - 1, controller.getPort());
                EXPECT_EQ(starts, McpServerStub::starts.size());
                settings.open();
                settings._port = port;
                pressSettingsButton("Adopt");
                EXPECT_EQ(port, controller.getPort());
                EXPECT_EQ(starts + 1, McpServerStub::starts.size());
                for (int pass = 0; pass < 3; ++pass) {
                    frame([&] {
                        ImGui::BeginChild("MCP guide", {600 * factor, 600 * factor});
                        McpWindow::get().processConnectionGuide();
                        auto window = ImGui::GetCurrentWindow();
                        EXPECT_LE(GImGui->LastItemData.Rect.Max.x, window->WorkRect.Max.x + 1);
                        EXPECT_LE(window->ContentSize.x, window->InnerRect.GetWidth() + 1);
                        ImGui::EndChild();
                    });
                }
            }
        }
    }

    void checkPendingDeferredCommand()
    {
        auto& controller = McpController::get();
        McpToolCompletion completion;
        boost::json::object arguments{{"name", "Ёж/{tools} ### 100%"}};
        auto serialized = boost::json::serialize(arguments);
        McpTool tool{.name = "alien_deferred", .deferredHandler = [&](auto const& actual, auto const& callback) {
                         EXPECT_EQ(serialized, boost::json::serialize(actual));
                         completion = callback;
                     }};
        auto wrapped = controller.wrapTool(tool);
        auto pending = std::async(std::launch::async, [&] { return wrapped.handler(arguments); });
        while (!completion) {
            decltype(controller._pendingTasks) tasks;
            {
                std::lock_guard lock(controller._pendingTasksMutex);
                tasks.swap(controller._pendingTasks);
            }
            for (auto const& task : tasks) {
                task();
            }
            std::this_thread::yield();
        }
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            frame([&] { McpWindow::get().processIntern(); });
            EXPECT_EQ(std::future_status::timeout, pending.wait_for(std::chrono::milliseconds(0)));
            EXPECT_TRUE(controller.getCommandLog().empty());
        }
        completion(McpToolResult{.text = "literal result {url} ### 100%", .isError = true});
        EXPECT_EQ("literal result {url} ### 100%", pending.get().text);
        ASSERT_EQ(1, controller.getCommandLog().size());
        EXPECT_EQ("alien_deferred " + serialized, controller.getCommandLog().back().command);
        EXPECT_EQ("literal result {url} ### 100%", controller.getCommandLog().back().result);
    }

    void checkLogScrollAndColumnState()
    {
        auto& controller = McpController::get();
        for (int i = 0; i < 100; ++i) {
            controller.addCommandLogEntry("alien_test", {{"index", i}}, McpToolResult{.text = "literal result"});
        }
        ImGuiTable* originalTable = nullptr;
        ImGuiID originalId = 0;
        auto draw = [&] {
            ImGui::BeginChild("Log viewport", {600, 350});
            auto id = ImGui::GetID("##mcpCommands");
            McpWindow::get().processCommandLog();
            auto table = ImGui::TableFindByID(id);
            ASSERT_NE(nullptr, table);
            if (!originalTable) {
                originalTable = table;
                originalId = id;
            }
            EXPECT_EQ(originalTable, table);
            EXPECT_EQ(originalId, id);
            ImGui::EndChild();
        };
        frame(draw);
        frame(draw);
        ASSERT_NE(nullptr, originalTable);
        originalTable->Columns[1].StretchWeight = 2.5f;
        ImGui::SetScrollY(originalTable->InnerWindow, 200);
        frame(draw);
        auto scroll = originalTable->InnerWindow->Scroll.y;
        auto weight = originalTable->Columns[1].StretchWeight;
        EXPECT_GT(scroll, 0);
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            frame(draw);
            EXPECT_FLOAT_EQ(scroll, originalTable->InnerWindow->Scroll.y);
            EXPECT_FLOAT_EQ(weight, originalTable->Columns[1].StretchWeight);
            EXPECT_EQ(100, controller.getCommandLog().size());
            EXPECT_EQ("alien_test {\"index\":99}", controller.getCommandLog().back().command);
        }
    }

    SettingsTestScope settingsScope;
    McpToolsFacade originalTools;
    float originalScale = 1;
};

TEST_F(McpLocalizationTests, isolatedLifecycleAndStartupFailure)
{
    checkLifecycle();
}

TEST_F(McpLocalizationTests, openWindowAndPortInputRetainStateAcrossLanguagesAndScales)
{
    checkWindowAndSettings();
}

TEST_F(McpLocalizationTests, argumentsResultsAndCommandLogRemainLiteral)
{
    checkCommands();
}

TEST_F(McpLocalizationTests, settingsApplyCancelAndGuideFitAcrossLanguagesAndScales)
{
    checkSettingsActionsAndGuideLayout();
}

TEST_F(McpLocalizationTests, languageSwitchDoesNotCompleteOrRescheduleDeferredCommand)
{
    checkPendingDeferredCommand();
}

TEST_F(McpLocalizationTests, populatedLogRetainsScrollAndResizedColumnsAcrossLanguages)
{
    checkLogScrollAndColumnState();
}

TEST_F(McpLocalizationTests, groupCaptionsLocalizeWithoutChangingMachineMetadata)
{
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        EXPECT_EQ(tr("mcp.group.files"), McpPresentation::groupCaption("Simulations and files"));
        EXPECT_EQ(tr("mcp.group.temporal"), McpPresentation::groupCaption("Temporal control"));
        EXPECT_EQ(tr("mcp.group.editing"), McpPresentation::groupCaption("World editing"));
        EXPECT_EQ(tr("mcp.group.inspection"), McpPresentation::groupCaption("Inspection and view"));
        EXPECT_EQ(tr("mcp.group.parameters"), McpPresentation::groupCaption("Simulation parameters"));
        EXPECT_EQ(tr("mcp.group.documentation"), McpPresentation::groupCaption("Documentation"));
        EXPECT_EQ("Unknown/{tools} ### literal", McpPresentation::groupCaption("Unknown/{tools} ### literal"));
    }
}
