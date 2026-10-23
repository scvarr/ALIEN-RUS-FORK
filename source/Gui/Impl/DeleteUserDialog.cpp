#include "DeleteUserDialog.h"

#include "AccountPresentation.h"

#include <imgui.h>

#include <Network/NetworkService.h>

#include "AlienGui.h"
#include "BrowserWindow.h"
#include "CreateUserDialog.h"
#include "GenericMessageDialog.h"

DeleteUserDialog::DeleteUserDialog()
    : AlienDialog("Delete user", {700.0f, 260.0f})
{
    setTitleKey("account.deleteuser");
}

void DeleteUserDialog::processIntern()
{
    AlienGui::Text(LocalizationGuiText::format("account.deletewarning", {{"user", *NetworkService::get().getLoggedInUserName()}}));
    AlienGui::Separator();

    AlienGui::InputText(
        AlienGui::InputTextParameters().hint(AccountPresentation::text("Re-enter password")).hintId("Re-enter password").password(true).textWidth(0),
        _reenteredPassword);
    AlienGui::Separator();

    ImGui::BeginDisabled(_reenteredPassword.empty());
    if (AccountPresentation::button("Delete")) {
        close();
        if (_reenteredPassword == *NetworkService::get().getPassword()) {
            onDelete();
        } else {
            AccountPresentation::information("mismatch");
        }
        _reenteredPassword.clear();
    }
    ImGui::EndDisabled();
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AccountPresentation::button("Cancel")) {
        close();
        _reenteredPassword.clear();
    }
}

void DeleteUserDialog::onDelete()
{
    auto userName = *NetworkService::get().getLoggedInUserName();
    if (NetworkService::get().deleteUser()) {
        BrowserWindow::get().onRefresh();
        AccountPresentation::information("deleted", {{"user", userName}});
    } else {
        AccountPresentation::information("servererror");
    }
}
