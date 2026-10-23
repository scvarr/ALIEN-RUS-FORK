#pragma once

#include "AlienGui.h"
#include "GenericMessageDialog.h"
#include "LocalizationGuiText.h"

namespace AccountPresentation
{
    inline std::string text(std::string const& canonical)
    {
        static std::map<std::string, std::string> const keys = {
            {"Login or register", "account.login.title"},
            {"Create user", "account.create"},
            {"Activate user", "account.activate"},
            {"Reset password", "account.reset"},
            {"New password", "account.newpassword"},
            {"Delete user", "account.deleteuser"},
            {"How to create a new user?", "account.howcreate"},
            {"Forgot your password?", "account.forgot"},
            {"Security information", "account.security"},
            {"Data privacy policy", "account.privacy"},
            {"User name", "account.username"},
            {"Password", "account.password"},
            {"Remember", "account.remember"},
            {"Share GPU model info", "account.sharegpu"},
            {"Login", "account.login"},
            {"Cancel", "account.cancel"},
            {"Email", "account.email"},
            {"Code (case sensitive)", "account.code"},
            {"OK", "account.ok"},
            {"Resend", "account.resend"},
            {"Resend to other email address", "account.resendother"},
            {"Re-enter password", "account.reenter"},
            {"Delete", "account.delete"},
            {"Please enter the desired user name and password and proceed by clicking the 'Create user' button.", "account.help.loginhowtocreatenewusetooltip"},
            {"Please enter the user name and proceed by clicking the 'Reset password' button.", "account.help.loginforgotyourpasswordtooltip"},
            {"The data transfer to the server is encrypted via https. On the server side, the password is not stored in cleartext, but as a salted SHA-256 "
             "hash value in the database. If the toggle 'Remember' is activated, the password will be stored in the Windows registry under the path "
             "'HKEY_CURRENT_USER\\SOFTWARE\\alien' or, in the case of other OS, in 'settings.json' on your local machine.",
             "account.help.loginsecurityinformationtooltip"},
            {"If the toggle 'Remember' is activated, the password will be stored in the Windows registry under the path 'HKEY_CURRENT_USER\\SOFTWARE\\alien' "
             "or, in the case of other OS, in 'settings.json' on your local machine. It is recommended not to choose a password that is used elsewhere.",
             "account.help.loginremembertooltip"},
            {"The data transfer to the server is encrypted via https. On the server side, the email address is not stored in cleartext, but as a SHA-256 hash "
             "value in the database.",
             "account.emailsecurity"},
            {"The entered e-mail address will not be passed on to third parties and is used only for the following two reasons: 1) To send the confirmation "
             "code. 2) A SHA-256 hash value of the email address is stored on the server to verify that it is not yet in use.",
             "account.createprivacy"},
            {"The entered e-mail address will not be passed on to third parties and is used only for the following two reasons: 1) To send the confirmation "
             "code. 2) Its SHA-256 hash value is compared with the one stored for your account on the server.",
             "account.resetprivacy"},
            {"Please enter your email address to receive the\nconfirmation code for the new user.", "account.createprompt"},
            {"Please enter your email address to receive the\nconfirmation code to reset the password.", "account.resetprompt"},
            {"Please enter the confirmation code sent to your email address.", "account.activateprompt"},
            {"Please check your spam folder if you did not find an email. If you did not receive an email there, try signing up with possibly another email "
             "address. If this still does not work, please contact info@alien-project.org.",
             "account.activatehelp"},
            {"The data transfer to the server is encrypted via https. On the server side, the password is not stored in cleartext, but as a salted SHA-256 "
             "hash value in the database.",
             "account.passwordsecurity"},
            {"Please enter a new password and the confirmation code\nsent to your email address.", "account.newprompt"},
            {"An error occurred on the server. This could be related to the fact that\n your user name or email address is already in use,\n or your "
             "user name contains white spaces.",
             "account.createerror"},
            {"An error occurred on the server. Your entered code may be incorrect.\nPlease try to register again.", "account.activateerror"},
            {"An error occurred on the server. This could be related to the fact that the\nemail address is wrong.", "account.reseterror"},
            {"An error occurred on the server. Your entered code may be incorrect.\nPlease try to reset the password again.", "account.newerror"},
            {"The password has been successfully set.\nYou are logged in.", "account.newsuccess"},
            {"The password does not match.", "account.mismatch"},
            {"An error occurred on the server.", "account.servererror"},
        };
        return tr(keys.at(canonical));
    }

    inline bool button(std::string const& canonical)
    {
        auto label = text(canonical);
        if (ImGui::CalcTextSize(label.c_str()).x + ImGui::GetStyle().FramePadding.x * 2 > ImGui::GetContentRegionAvail().x) {
            ImGui::NewLine();
        }
        return AlienGui::DisplayButton(canonical, label);
    }

    inline void information(std::string const& operation, std::map<std::string, std::string> const& arguments = {})
    {
        auto success = operation == "activated" || operation == "newsuccess" || operation == "deleted";
        GenericMessageDialog::get().localizedInformation(
            success ? "Information" : "Error", success ? "account.information" : "localui.error", "account." + operation, arguments);
    }
}
