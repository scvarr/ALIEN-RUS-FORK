#include "ActivateUserDialog.h"

#include "AccountPresentation.h"

#include <imgui.h>

#include <Network/NetworkService.h>

#include <Engine/Interface/SimulationFacade.h>

#include "AlienGui.h"
#include "BrowserWindow.h"
#include "CreateUserDialog.h"
#include "GenericMessageDialog.h"
#include "StyleService.h"

void ActivateUserDialog::initIntern()
{

}

void ActivateUserDialog::open(std::string const& userName, std::string const& password, UserInfo const& userInfo)
{
    AlienDialog::open();
    _userName = userName;
    _password = password;
    _userInfo = userInfo;
}

ActivateUserDialog::ActivateUserDialog()
    : AlienDialog("Activate user", {750.0f, 260.0f})
{
    setTitleKey("account.activate");
}

void ActivateUserDialog::processIntern()
{
    AlienGui::Text(AccountPresentation::text("Please enter the confirmation code sent to your email address."));
    AlienGui::HelpMarker(AccountPresentation::text(
        "Please check your spam folder if you did not find an email. If you did not receive an email there, try signing up with possibly another "
        "email address. If this still does not work, please contact info@alien-project.org."));
    AlienGui::Separator();
    AlienGui::InputText(AlienGui::InputTextParameters().hint(AccountPresentation::text("Code (case sensitive)")).hintId("Code (case sensitive)").textWidth(0), _confirmationCode);

    AlienGui::Separator();

    ImGui::BeginDisabled(_confirmationCode.empty());
    if (AccountPresentation::button("OK")) {
        close();
        onActivateUser();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    AlienGui::VerticalSeparator();

    ImGui::SameLine();
    if (AccountPresentation::button("Resend")) {
        CreateUserDialog::get().onCreateUser();
    }

    ImGui::SameLine();
    if (AccountPresentation::button("Resend to other email address")) {
        close();
        CreateUserDialog::get().open(_userName, _password, _userInfo);
    }

    ImGui::SameLine();
    AlienGui::VerticalSeparator();

    ImGui::SameLine();
    if (AccountPresentation::button("Cancel")) {
        close();
    }
}

void ActivateUserDialog::onActivateUser()
{
    auto result = NetworkService::get().activateUser(_userName, _password, _userInfo, _confirmationCode);
    if (result) {
        LoginErrorCode errorCode;
        result |= NetworkService::get().login(errorCode, _userName, _password, _userInfo);
    }
    if (!result) {
        AccountPresentation::information("activateerror");
    } else {
        AccountPresentation::information("activated", {{"user", _userName}});
        BrowserWindow::get().onRefresh();
    }
}
