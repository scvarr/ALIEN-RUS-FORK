#include "ResetPasswordDialog.h"

#include "AccountPresentation.h"

#include <imgui.h>

#include "AlienGui.h"
#include "GenericMessageDialog.h"
#include "NewPasswordDialog.h"

ResetPasswordDialog::ResetPasswordDialog()
    : AlienDialog("Reset password", {600.0f, 300.0f})
{
    setTitleKey("account.reset");
}

void ResetPasswordDialog::open(std::string const& userName, UserInfo const& userInfo)
{
    AlienDialog::open();
    _userName = userName;
    _email.clear();
    _userInfo = userInfo;
}

void ResetPasswordDialog::processIntern()
{
    AlienGui::Text(AccountPresentation::text("Security information"));
    AlienGui::HelpMarker(AccountPresentation::text(
        "The data transfer to the server is encrypted via https. On the server side, the email address is not stored in cleartext, but "
        "as a SHA-256 hash value in the database."));
    AlienGui::Text(AccountPresentation::text("Data privacy policy"));
    AlienGui::HelpMarker(AccountPresentation::text(
        "The entered e-mail address will not be passed on to third parties and is used only for the following two reasons: 1) To send the confirmation code. "
        "2) Its SHA-256 hash value is compared with the one stored for your account on the server."));
    AlienGui::Separator();
    AlienGui::Text(AccountPresentation::text("Please enter your email address to receive the\nconfirmation code to reset the password."));
    AlienGui::Separator();
    AlienGui::InputText(AlienGui::InputTextParameters().hint(AccountPresentation::text("Email")).hintId("Email").textWidth(0), _email);

    AlienGui::Separator();

    ImGui::BeginDisabled(_email.empty());
    if (AccountPresentation::button("OK")) {
        close();

        onResetPassword();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AccountPresentation::button("Cancel")) {
        close();
    }
}

void ResetPasswordDialog::onResetPassword()
{
    if (NetworkService::get().resetPassword(_userName, _email)) {
        NewPasswordDialog::get().open(_userName, _userInfo);
    } else {
        AccountPresentation::information("reseterror");
    }
}
