#include <gtest/gtest.h>
#include <imgui_internal.h>

#include <Engine/Impl/SimulationFacadeImpl.h>
#include <Persister/Interface/TaskProcessor.h>
#include <Gui/Impl/AccountPresentation.h>
#include <Gui/Impl/ActivateUserDialog.h>
#include <Gui/Impl/BrowserController.h>
#include <Gui/Impl/BrowserData.h>
#include <Gui/Impl/BrowserMetadataSearch.h>
#include <Gui/Impl/CreateUserDialog.h>
#include <Gui/Impl/DelayedExecutionController.h>
#include <Gui/Impl/DeleteUserDialog.h>
#include <Gui/Impl/LoginController.h>
#include <Gui/Impl/LoginDialog.h>
#include <Gui/Impl/NewPasswordDialog.h>
#include <Gui/Impl/ResetPasswordDialog.h>
#include <Gui/Impl/WindowController.h>
#include <Base/Tests/SettingsTestScope.h>

#include "AccountFacadeStubs.h"
#include "AccountNetworkStub.h"

class AccountLocalizationTests : public ::testing::Test
{
protected:
    class World : public _SimulationFacadeImpl
    {
    public:
        std::string getGpuName() const override { return "GPU/Ёж 100% {gpu} ### literal"; }
    };

    void SetUp() override
    {
        AccountNetworkStub::calls.clear();
        AccountNetworkStub::arguments.clear();
        AccountNetworkStub::success = false;
        AccountNetworkStub::loggedIn = true;
        originalScale = WindowController::get().getContentScaleFactor();
        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().DisplaySize = {4000, 3000};
        StyleService::get().setup();
        LocalizationService::get().load("resources/locales");
        select("en");
        _SimulationFacadeImpl::set(std::make_shared<World>());
        AccountRenderStub::set(std::make_shared<AccountRenderStub>());
        _PersisterFacadeImpl::set(persister);
        BrowserController::get().setup();
        auto& login = LoginController::get();
        login._shareGpuInfo = false;
        login._remember = true;
        login._taskProcessor = _TaskProcessor::createTaskProcessor(persister);
    }

    void TearDown() override
    {
        frame([&] {
            for (auto dialog : dialogs()) {
                dialog->_modalWindow.process([&] { dialog->_modalWindow.close(); });
            }
            auto& message = GenericMessageDialog::get();
            message._modalWindow.process([&] { message._modalWindow.close(); });
        });
        LoginController::get()._taskProcessor.reset();
        static_cast<MainLoopEntity&>(BrowserController::get()).shutdown();
        _PersisterFacadeImpl::set(originalPersister);
        AccountRenderStub::set(originalRendering);
        _SimulationFacadeImpl::set(originalWorld);
        WindowController::get()._contentScaleFactor = originalScale;
        ImGui::DestroyContext();
    }

    std::vector<AlienDialog*> dialogs()
    {
        return {
            &LoginDialog::get(),
            &CreateUserDialog::get(),
            &ActivateUserDialog::get(),
            &ResetPasswordDialog::get(),
            &NewPasswordDialog::get(),
            &DeleteUserDialog::get()};
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
        ImGui::SetNextWindowSize({1500, 1000});
        ImGui::Begin("Account host");
        draw();
        ImGui::End();
        ImGui::Render();
        EXPECT_EQ(0, GImGui->ErrorCountCurrentFrame);
    }

    void populate()
    {
        auto& login = LoginController::get();
        login.setUserName(AccountNetworkStub::user);
        login.setPassword(AccountNetworkStub::password);
        CreateUserDialog::get().open(AccountNetworkStub::user, AccountNetworkStub::password, {});
        CreateUserDialog::get()._email = "MiXeD+Ёж@example.invalid";
        ActivateUserDialog::get().open(AccountNetworkStub::user, AccountNetworkStub::password, {});
        ActivateUserDialog::get()._confirmationCode = "CoDe/Ёж";
        ResetPasswordDialog::get().open(AccountNetworkStub::user, {});
        ResetPasswordDialog::get()._email = "MiXeD+Ёж@example.invalid";
        NewPasswordDialog::get().open(AccountNetworkStub::user, {});
        NewPasswordDialog::get()._newPassword = AccountNetworkStub::password;
        NewPasswordDialog::get()._confirmationCode = "CoDe/Ёж";
        DeleteUserDialog::get()._reenteredPassword = AccountNetworkStub::password;
    }

    void checkValues()
    {
        EXPECT_EQ(AccountNetworkStub::user, LoginController::get().getUserName());
        EXPECT_EQ(AccountNetworkStub::password, LoginController::get().getPassword());
        EXPECT_EQ("MiXeD+Ёж@example.invalid", CreateUserDialog::get()._email);
        EXPECT_EQ("MiXeD+Ёж@example.invalid", ResetPasswordDialog::get()._email);
        EXPECT_EQ("CoDe/Ёж", ActivateUserDialog::get()._confirmationCode);
        EXPECT_EQ("CoDe/Ёж", NewPasswordDialog::get()._confirmationCode);
        EXPECT_EQ(AccountNetworkStub::password, NewPasswordDialog::get()._newPassword);
        EXPECT_EQ(AccountNetworkStub::password, DeleteUserDialog::get()._reenteredPassword);
    }

    std::string message()
    {
        auto& dialog = GenericMessageDialog::get();
        return LocalizationGuiText::format(dialog._messageKey, dialog._messageArguments);
    }

    void press(AlienDialog& dialog, std::string const& button)
    {
        ImVec2 target;
        auto draw = [&] {
            ImGui::PushID(&dialog);
            dialog.processIntern();
            ImGui::PopID();
        };
        frame([&] {
            ImGui::PushID(&dialog);
            auto id = ImGui::GetID(button.c_str());
            ImGui::FocusWindow(ImGui::GetCurrentWindow());
            ImGui::SetNavID(id, ImGuiNavLayer_Main, 0, ImRect());
            dialog.processIntern();
            auto window = ImGui::GetCurrentWindow();
            auto rect = window->NavRectRel[ImGuiNavLayer_Main];
            target = ImGui::WindowRectRelToAbs(window, rect).GetCenter();
            ImGui::ClearActiveID();
            ImGui::PopID();
        });
        auto& io = ImGui::GetIO();
        io.AddMousePosEvent(target.x, target.y);
        frame(draw);
        io.AddMouseButtonEvent(0, true);
        frame(draw);
        io.AddMouseButtonEvent(0, false);
        frame(draw);
    }

    void checkEmptyFieldsMismatchAndResend()
    {
        populate();
        auto& login = LoginController::get();
        login.setUserName("");
        login.setPassword("");
        CreateUserDialog::get()._email.clear();
        ActivateUserDialog::get()._confirmationCode.clear();
        ResetPasswordDialog::get()._email.clear();
        NewPasswordDialog::get()._confirmationCode.clear();
        DeleteUserDialog::get()._reenteredPassword.clear();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            press(LoginDialog::get(), "Login");
            press(LoginDialog::get(), "Create user");
            press(LoginDialog::get(), "Reset password");
            press(CreateUserDialog::get(), "Create user");
            press(ActivateUserDialog::get(), "OK");
            press(ResetPasswordDialog::get(), "OK");
            press(NewPasswordDialog::get(), "OK");
            press(DeleteUserDialog::get(), "Delete");
            EXPECT_TRUE(AccountNetworkStub::calls.empty());
            EXPECT_EQ(0, persister->scheduled);
        }
        populate();
        DeleteUserDialog::get()._reenteredPassword = "wrong CASE";
        press(DeleteUserDialog::get(), "Delete");
        EXPECT_TRUE(AccountNetworkStub::calls.empty());
        EXPECT_EQ(tr("account.mismatch"), message());
        EXPECT_TRUE(DeleteUserDialog::get()._reenteredPassword.empty());
        press(ActivateUserDialog::get(), "Resend");
        EXPECT_EQ((std::vector<std::string>{"create"}), AccountNetworkStub::calls);
        EXPECT_EQ("MiXeD+Ёж@example.invalid", AccountNetworkStub::arguments.back());
        press(ActivateUserDialog::get(), "Resend to other email address");
        EXPECT_TRUE(CreateUserDialog::get().isOpen());
        EXPECT_TRUE(CreateUserDialog::get()._email.empty());
        EXPECT_EQ(1, AccountNetworkStub::calls.size());
        ResetPasswordDialog::get()._email = "invalid-address";
        press(ResetPasswordDialog::get(), "OK");
        EXPECT_EQ("reset", AccountNetworkStub::calls.back());
        EXPECT_EQ("invalid-address", AccountNetworkStub::arguments.back());
        EXPECT_EQ(tr("account.reseterror"), message());
        login.setUserName(AccountNetworkStub::user);
        login.setPassword(AccountNetworkStub::password);
        press(LoginDialog::get(), "Login");
        EXPECT_EQ(1, persister->scheduled);
        EXPECT_EQ(AccountNetworkStub::password, persister->loginRequest.password);
        frame([&] { static_cast<MainLoopEntity&>(DelayedExecutionController::get()).process(); });
        frame([&] { static_cast<MainLoopEntity&>(DelayedExecutionController::get()).process(); });
    }

    SettingsTestScope settings;
    PersisterFacade originalPersister = _PersisterFacade::get();
    RenderingFacade originalRendering = _RenderingFacade::get();
    SimulationFacade originalWorld = _SimulationFacade::get();
    std::shared_ptr<AccountPersisterStub> persister = std::make_shared<AccountPersisterStub>();
    float originalScale = 1;
    void checkFormsAcrossLanguagesAndScales()
    {
        auto names = std::vector<std::string>{"User name", "Email", "Code (case sensitive)", "Email", "New password", "Re-enter password"};
        for (float factor : {1.0f, 1.5f, 2.0f}) {
            scaleForTest(factor);
            populate();
            auto forms = dialogs();
            for (size_t i = 0; i < forms.size(); ++i) {
                auto& dialog = *forms.at(i);
                dialog._modalWindow.open();
                ImGuiID windowId = 0;
                ImGuiID inputId = 0;
                for (auto const& language : {"en", "ru", "en"}) {
                    select(language);
                    for (int pass = 0; pass < 3; ++pass) {
                        frame([&] {
                            ImGui::SetNextWindowSize({750 * factor, 500 * factor}, ImGuiCond_Always);
                            dialog._modalWindow.process([&] {
                                auto window = ImGui::GetCurrentWindow();
                                auto id = ImGui::GetID(("##" + names.at(i)).c_str());
                                if (windowId == 0) {
                                    windowId = window->ID;
                                    inputId = id;
                                    ImGui::SetKeyboardFocusHere();
                                }
                                EXPECT_EQ(windowId, window->ID);
                                EXPECT_EQ(inputId, id);
                                dialog.processIntern();
                                EXPECT_LE(GImGui->LastItemData.Rect.Max.x, window->WorkRect.Max.x + 1);
                                EXPECT_LE(GImGui->LastItemData.Rect.Max.y, window->WorkRect.Max.y + 1);
                            });
                        });
                    }
                    EXPECT_EQ(inputId, GImGui->ActiveId);
                    EXPECT_EQ(inputId, GImGui->InputTextState.ID);
                    EXPECT_TRUE(dialog.isOpen());
                    checkValues();
                    EXPECT_TRUE(AccountNetworkStub::calls.empty());
                    EXPECT_EQ(0, persister->scheduled);
                }
                frame([&] { dialog._modalWindow.process([&] { dialog._modalWindow.close(); }); });
            }
        }
    }
    void checkIsolatedOperationResponses()
    {
        populate();
        select("ru");
        CreateUserDialog::get().onCreateUser();
        EXPECT_EQ((std::vector<std::string>{"create"}), AccountNetworkStub::calls);
        EXPECT_EQ(
            (std::vector<std::string>{AccountNetworkStub::user, AccountNetworkStub::password, "MiXeD+Ёж@example.invalid"}), AccountNetworkStub::arguments);
        EXPECT_EQ(tr("account.createerror"), message());
        ActivateUserDialog::get().onActivateUser();
        EXPECT_EQ("CoDe/Ёж", AccountNetworkStub::arguments.back());
        EXPECT_EQ(tr("account.activateerror"), message());
        ResetPasswordDialog::get().onResetPassword();
        EXPECT_EQ(tr("account.reseterror"), message());
        NewPasswordDialog::get().onNewPassword();
        EXPECT_EQ(tr("account.newerror"), message());
        DeleteUserDialog::get().onDelete();
        EXPECT_EQ(tr("account.servererror"), message());
        AccountNetworkStub::success = true;
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            auto before = AccountNetworkStub::calls.size();
            CreateUserDialog::get().onCreateUser();
            EXPECT_TRUE(ActivateUserDialog::get().isOpen());
            EXPECT_EQ(before + 1, AccountNetworkStub::calls.size());
        }
        ActivateUserDialog::get().onActivateUser();
        EXPECT_EQ("activate", AccountNetworkStub::calls.at(AccountNetworkStub::calls.size() - 2));
        EXPECT_EQ("login", AccountNetworkStub::calls.back());
        EXPECT_TRUE(AccountNetworkStub::loggedIn);
        EXPECT_NE(std::string::npos, message().find(AccountNetworkStub::user));
        select("ru");
        EXPECT_EQ(LocalizationGuiText::format("account.activated", {{"user", AccountNetworkStub::user}}), message());
        ResetPasswordDialog::get().onResetPassword();
        EXPECT_TRUE(NewPasswordDialog::get().isOpen());
        NewPasswordDialog::get()._newPassword = AccountNetworkStub::password;
        NewPasswordDialog::get()._confirmationCode = "CoDe/Ёж";
        NewPasswordDialog::get().onNewPassword();
        EXPECT_EQ(tr("account.newsuccess"), message());
        DeleteUserDialog::get().onDelete();
        EXPECT_FALSE(AccountNetworkStub::loggedIn);
        EXPECT_EQ(LocalizationGuiText::format("account.deleted", {{"user", AccountNetworkStub::user}}), message());
    }
    void checkPendingLoginAndDiagnostics()
    {
        populate();
        auto& login = LoginController::get();
        login.onLogin();
        EXPECT_EQ(AccountNetworkStub::user, persister->loginRequest.userName);
        EXPECT_EQ(AccountNetworkStub::password, persister->loginRequest.password);
        for (auto const& language : {"en", "ru", "en", "ru"}) {
            select(language);
            login.process();
            EXPECT_EQ(1, persister->scheduled);
        }
        std::string raw = "HTTP 401: User/Ёж 100% {details} ### literal";
        persister->state = PersisterRequestState::Error;
        persister->errors = {{raw}};
        login.process();
        EXPECT_EQ(LocalizationGuiText::format("account.loginerror", {{"details", raw}}), message());
        EXPECT_EQ(raw, GenericMessageDialog::get()._messageArguments.at("details"));
        select("en");
        EXPECT_EQ(LocalizationGuiText::format("account.loginerror", {{"details", raw}}), message());
        ImGuiID messageWindowId = 0;
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            frame([&] {
                auto& dialog = GenericMessageDialog::get();
                dialog._modalWindow.process([&] {
                    auto id = ImGui::GetCurrentWindow()->ID;
                    if (messageWindowId == 0) {
                        messageWindowId = id;
                    }
                    EXPECT_EQ(messageWindowId, id);
                    dialog.processIntern();
                });
            });
            EXPECT_TRUE(GenericMessageDialog::get().isOpen());
            EXPECT_EQ(raw, GenericMessageDialog::get()._messageArguments.at("details"));
            EXPECT_EQ(1, persister->scheduled);
        }
        login.process();
        EXPECT_EQ(1, persister->scheduled);
    }
    void checkBrowserMetadata()
    {
        _NetworkResourceRawTO raw{};
        raw.resourceName = "МИР/ГеНоМ/ЁЖ World";
        raw.userName = "Author";
        raw.description = "prefix МИР middle ГЕНОМ suffix ёж";
        raw.timestamp = "2026-10-23";
        raw.version = "4.12.0";
        raw.numDownloads = 123;
        raw.width = 456;
        raw.height = 789;
        raw.particles = 2468;
        raw.contentSize = 13579;
        auto before = raw.resourceName;
        raw.description = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
        EXPECT_TRUE(BrowserMetadataSearch::matches(raw, "абвгдеёжзийклмнопрстуфхцчшщъыьэюя"));
        raw.description = "prefix МИР middle ГЕНОМ suffix ёж";
        for (auto query : {"МИР", "мир", "ГЕНОМ", "геном", "ЁЖ", "ёж", "МиР", "гЕнОм", "ёЖ", "wOrLd", "AUTHOR"}) {
            EXPECT_TRUE(BrowserMetadataSearch::matches(raw, query)) << query;
        }
        EXPECT_FALSE(BrowserMetadataSearch::matches(raw, "еж"));
        EXPECT_FALSE(BrowserMetadataSearch::matches(raw, "ЕЖ"));
        for (auto query : {"", "2026", "Author", "World", "prefix", "suffix", "123", "456", "789", "2468", "13579", "4.12.0", "absent"}) {
            EXPECT_EQ(raw.matchWithFilter(query), BrowserMetadataSearch::matches(raw, query)) << query;
        }
        EXPECT_EQ(before, raw.resourceName);
        for (auto const& field : {raw.userName, raw.timestamp, raw.version, std::string("World")}) {
            for (size_t start = 0; start < field.size(); ++start) {
                for (size_t length = 1; length <= field.size() - start; ++length) {
                    auto query = field.substr(start, length);
                    EXPECT_EQ(raw.matchWithFilter(query), BrowserMetadataSearch::matches(raw, query));
                }
            }
        }
        raw.resourceName = "ЕЖ";
        raw.description.clear();
        EXPECT_FALSE(BrowserMetadataSearch::matches(raw, "ёж"));
        for (auto text : {"мир хвост", "начало МиР хвост", "начало МИР"}) {
            raw.resourceName = text;
            raw.description.clear();
            EXPECT_TRUE(BrowserMetadataSearch::matches(raw, "мир"));
        }
        auto data = BrowserController::get().getData();
        for (int type : {NetworkResourceType_Simulation, NetworkResourceType_Genome}) {
            auto& workspace = data->workspaces.at({type, WorkspaceType_Public});
            auto item = std::make_shared<_NetworkResourceRawTO>(raw);
            item->resourceType = type;
            workspace.rawTOs = {item};
            data->filter = "мир";
            data->createTreeTOs(workspace);
            EXPECT_EQ(1, workspace.treeTOs.size());
        }
    }
    void checkMalformedUtf8()
    {
        _NetworkResourceRawTO raw{};
        raw.resourceName = "ЁЖ";
        for (std::string bad :
             {std::string("\x80"),
              std::string("\xd0"),
              std::string("\xc0\xaf"),
              std::string("\xed\xa0\x80"),
              std::string("\xf4\x90\x80\x80"),
              std::string("\xf0\x9f\x92"),
              std::string("\xd0\x41"),
              std::string("\x81")}) {
            EXPECT_FALSE(BrowserMetadataSearch::matches(raw, bad));
            raw.description = bad + "Ascii";
            EXPECT_FALSE(BrowserMetadataSearch::matches(raw, "ascii"));
            EXPECT_TRUE(BrowserMetadataSearch::matches(raw, "ёж"));
            EXPECT_TRUE(BrowserMetadataSearch::matches(raw, ""));
        }
        for (int byte = 0; byte <= 255; ++byte) {
            auto text = std::string(1, static_cast<char>(byte));
            auto valid = Utf8Search::decode(text).has_value();
            EXPECT_EQ(byte < 128, valid);
        }
    }
};

TEST_F(AccountLocalizationTests, checkFormsAcrossLanguagesAndScales)
{
    checkFormsAcrossLanguagesAndScales();
}

TEST_F(AccountLocalizationTests, checkIsolatedOperationResponses)
{
    checkIsolatedOperationResponses();
}

TEST_F(AccountLocalizationTests, checkPendingLoginAndDiagnostics)
{
    checkPendingLoginAndDiagnostics();
}

TEST_F(AccountLocalizationTests, checkBrowserMetadata)
{
    checkBrowserMetadata();
}

TEST_F(AccountLocalizationTests, checkMalformedUtf8)
{
    checkMalformedUtf8();
}

TEST_F(AccountLocalizationTests, checkEmptyFieldsMismatchAndResend)
{
    checkEmptyFieldsMismatchAndResend();
}
