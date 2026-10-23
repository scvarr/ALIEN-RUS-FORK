#include "CreateUserDialog.h"

#include "AccountPresentation.h"

#include <imgui.h>

#include <Fonts/IconsFontAwesome5.h>

#include <Network/NetworkService.h>

#include "ActivateUserDialog.h"
#include "AlienGui.h"
#include "GenericMessageDialog.h"

CreateUserDialog::CreateUserDialog()
    : AlienDialog("Create user", {600.0f, 300.0f})
{
    setTitleKey("account.create");
}

void CreateUserDialog::open(std::string const& userName, std::string const& password, UserInfo const& userInfo)
{
    AlienDialog::open();
    _userName = userName;
    _password = password;
    _email.clear();
    _userInfo = userInfo;
}

void CreateUserDialog::processIntern()
{
    AlienGui::Text(AccountPresentation::text("Security information"));
    AlienGui::HelpMarker(AccountPresentation::text(
        "The data transfer to the server is encrypted via https. On the server side, the email address is not stored in cleartext, but "
        "as a SHA-256 hash value in the database."));
    AlienGui::Text(AccountPresentation::text("Data privacy policy"));
    AlienGui::HelpMarker(AccountPresentation::text(
        "The entered e-mail address will not be passed on to third parties and is used only for the following two reasons: 1) To send "
        "the confirmation code. "
        "2) A SHA-256 hash value of the email address is stored on the server to verify that it is not yet in use."));
    AlienGui::Separator();
    AlienGui::Text(AccountPresentation::text("Please enter your email address to receive the\nconfirmation code for the new user."));
    AlienGui::Separator();
    AlienGui::InputText(AlienGui::InputTextParameters().hint(AccountPresentation::text("Email")).hintId("Email").textWidth(0), _email);

    AlienGui::Separator();

    ImGui::BeginDisabled(_email.empty());
    if (AccountPresentation::button("Create user")) {
        close();

        onCreateUser();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AccountPresentation::button("Cancel")) {
        close();
    }
}

void CreateUserDialog::onCreateUser()
{
    if (NetworkService::get().createUser(_userName, _password, _email)) {
        ActivateUserDialog::get().open(_userName, _password, _userInfo);
    } else {
        AccountPresentation::information("createerror");
    }
}
