#include "NewPasswordDialog.h"

#include "AccountPresentation.h"

#include <imgui.h>

#include <Network/NetworkService.h>

#include <Engine/Interface/SimulationFacade.h>

#include "AlienGui.h"
#include "BrowserWindow.h"
#include "GenericMessageDialog.h"

void NewPasswordDialog::initIntern()
{

}

void NewPasswordDialog::open(std::string const& userName, UserInfo const& userInfo)
{
    AlienDialog::open();
    _userName = userName;
    _newPassword.clear();
    _confirmationCode.clear();
    _userInfo = userInfo;
}

NewPasswordDialog::NewPasswordDialog()
    : AlienDialog("New password", {600.0f, 300.0f})
{
    setTitleKey("account.newpassword");
}

void NewPasswordDialog::processIntern()
{
    AlienGui::Text(AccountPresentation::text("Security information"));
    AlienGui::HelpMarker(AccountPresentation::text(
        "The data transfer to the server is encrypted via https. On the server side, the password is not stored in cleartext, but as a salted SHA-256 hash "
        "value in the database."));

    AlienGui::Separator();

    AlienGui::Text(AccountPresentation::text("Please enter a new password and the confirmation code\nsent to your email address."));
    AlienGui::Separator();
    AlienGui::InputText(AlienGui::InputTextParameters().hint(AccountPresentation::text("New password")).hintId("New password").password(true).textWidth(0), _newPassword);
    AlienGui::InputText(AlienGui::InputTextParameters().hint(AccountPresentation::text("Code (case sensitive)")).hintId("Code (case sensitive)").textWidth(0), _confirmationCode);

    AlienGui::Separator();

    ImGui::BeginDisabled(_confirmationCode.empty());
    if (AccountPresentation::button("OK")) {
        close();
        onNewPassword();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AccountPresentation::button("Cancel")) {
        close();
    }
}

void NewPasswordDialog::onNewPassword()
{
    auto result = NetworkService::get().setNewPassword(_userName, _newPassword, _confirmationCode);
    if (result) {
        LoginErrorCode errorCode;
        result |= NetworkService::get().login(errorCode, _userName, _newPassword, _userInfo);
    }
    if (!result) {
        AccountPresentation::information("newerror");
        return;
    }
    AccountPresentation::information("newsuccess");
    BrowserWindow::get().onRefresh();
}
