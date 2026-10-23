#include "LoginDialog.h"

#include "AccountPresentation.h"

#include <imgui.h>

#include <Base/GlobalSettings.h>

#include <Network/NetworkService.h>

#include <Engine/Interface/SimulationFacade.h>

#include "ActivateUserDialog.h"
#include "AlienGui.h"
#include "BrowserWindow.h"
#include "CreateUserDialog.h"
#include "GenericMessageDialog.h"
#include "HelpStrings.h"
#include "LoginController.h"
#include "ResetPasswordDialog.h"
#include "StyleService.h"
#include <Engine/Interface/SimulationFacade.h>

void LoginDialog::initIntern()
{

}

LoginDialog::LoginDialog()
    : AlienDialog("Login or register", {640.0f, 450.0f})
{
    setTitleKey("account.login.title");
}

void LoginDialog::processIntern()
{
    AlienGui::Text(AccountPresentation::text("How to create a new user?"));
    AlienGui::HelpMarker(AccountPresentation::text(Const::LoginHowToCreateNewUseTooltip));

    AlienGui::Text(AccountPresentation::text("Forgot your password?"));
    AlienGui::HelpMarker(AccountPresentation::text(Const::LoginForgotYourPasswordTooltip));

    AlienGui::Text(AccountPresentation::text("Security information"));
    AlienGui::HelpMarker(AccountPresentation::text(Const::LoginSecurityInformationTooltip));

    AlienGui::Separator();

    auto& loginController = LoginController::get();
    auto userName = loginController.getUserName();
    AlienGui::InputText(AlienGui::InputTextParameters().hint(AccountPresentation::text("User name")).hintId("User name").textWidth(0), userName);
    loginController.setUserName(userName);

    auto password = loginController.getPassword();
    AlienGui::InputText(AlienGui::InputTextParameters().hint(AccountPresentation::text("Password")).hintId("Password").password(true).textWidth(0), password);
    loginController.setPassword(password);

    AlienGui::Separator();
    ImGui::Spacing();

    auto remember = loginController.isRemember();
    AlienGui::ToggleButton(AlienGui::ToggleButtonParameters().name("Remember").label(AccountPresentation::text("Remember")).tooltip(AccountPresentation::text(Const::LoginRememberTooltip)), remember);
    loginController.setRemember(remember);

    auto shareGpuInfo = loginController.shareGpuInfo();
    AlienGui::ToggleButton(
        AlienGui::ToggleButtonParameters()
            .name("Share GPU model info").label(AccountPresentation::text("Share GPU model info"))
            .tooltip(LocalizationGuiText::format("account.help.sharegpu", {{"gpu", _SimulationFacade::get()->getGpuName()}})),
        shareGpuInfo);
    loginController.setShareGpuInfo(shareGpuInfo);

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienGui::Separator();

    ImGui::BeginDisabled(userName.empty() || password.empty());
    if (AccountPresentation::button("Login")) {
        close();
        loginController.onLogin();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    AlienGui::VerticalSeparator();

    ImGui::SameLine();
    ImGui::BeginDisabled(userName.empty() || password.empty());
    if (AccountPresentation::button("Create user")) {
        close();
        CreateUserDialog::get().open(userName, password, LoginController::get().getUserInfo());
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(userName.empty());
    if (AccountPresentation::button("Reset password")) {
        close();
        ResetPasswordDialog::get().open(userName, LoginController::get().getUserInfo());
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    AlienGui::VerticalSeparator();

    ImGui::SameLine();
    if (AccountPresentation::button("Cancel")) {
        close();
    }
}
