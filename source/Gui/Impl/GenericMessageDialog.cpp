#include "GenericMessageDialog.h"

#include <algorithm>

#include <boost/algorithm/string.hpp>

#include <imgui.h>

#include <Base/LocalizationService.h>
#include <Base/LoggingService.h>
#include <Base/MarkdownParser.h>
#include <Base/WebLinkHelper.h>

#include "AlienGui.h"
#include "LocalizationGuiText.h"
#include "StyleService.h"

void GenericMessageDialog::processIntern()
{
    switch (_dialogType) {
    case DialogType::Information:
        processInformation();
        break;
    case DialogType::YesNo:
        processYesNo();
        break;
    }
}

void GenericMessageDialog::information(std::string const& title, std::string const& message)
{
    setTitleKey("");
    _messageKey.clear();
    _messageArguments.clear();
    _title = title;
    _message = message;
    _markdownMessage.reset();
    _dialogType = DialogType::Information;
    log(Priority::Important, "message dialog showing: '" + message + "'");

    AlienDialog::open();
    changeTitle(title);
}

void GenericMessageDialog::information(std::string const& title, std::vector<PersisterErrorInfo> const& errors)
{
    std::vector<std::string> errorMessages;
    for (auto const& error : errors) {
        errorMessages.emplace_back(error.message);
    }
    GenericMessageDialog::get().information(title, boost::join(errorMessages, "\n\n"));
}

void GenericMessageDialog::markdownInformation(std::string const& title, std::string const& markdownMessage)
{
    information(title, markdownMessage);
    _markdownMessage = MarkdownParser::parse(markdownMessage);
}

void GenericMessageDialog::yesNo(std::string const& title, std::string const& message, std::function<void()> const& yesFunction)
{
    setTitleKey("");
    _messageKey.clear();
    _messageArguments.clear();
    _title = title;
    _message = message;
    _markdownMessage.reset();
    _dialogType = DialogType::YesNo;
    _execFunction = yesFunction;

    AlienDialog::open();
    changeTitle(title);
}

void GenericMessageDialog::localizedInformation(std::string const& stableTitle, std::string const& titleKey, std::string const& messageKey)
{
    localizedInformation(stableTitle, titleKey, messageKey, {});
}

void GenericMessageDialog::localizedInformation(
    std::string const& stableTitle,
    std::string const& titleKey,
    std::string const& messageKey,
    std::map<std::string, std::string> const& arguments)
{
    information(stableTitle, LocalizationGuiText::format(messageKey, arguments));
    _messageArguments = arguments;
    setTitleKey(titleKey);
    _messageKey = messageKey;
}

void GenericMessageDialog::localizedYesNo(
    std::string const& stableTitle,
    std::string const& titleKey,
    std::string const& messageKey,
    std::function<void()> const& yesFunction)
{
    yesNo(stableTitle, tr(messageKey), yesFunction);
    setTitleKey(titleKey);
    _messageKey = messageKey;
}

GenericMessageDialog::GenericMessageDialog()
    : AlienDialog("Message")
{}

void GenericMessageDialog::processInformation()
{
    processMessageText();
    AlienGui::Separator();

    if (AlienGui::Button(_messageKey.empty() ? "OK" : tr("common.ok") + "###OK")) {
        close();
    }
}

void GenericMessageDialog::processYesNo()
{
    processMessageText();
    AlienGui::Separator();

    if (AlienGui::Button(_messageKey.empty() ? "Yes" : tr("common.yes") + "###Yes")) {
        close();
        _execFunction();
    }
    ImGui::SameLine();
    if (AlienGui::Button(_messageKey.empty() ? "No" : tr("common.no") + "###No")) {
        close();
    }
}

void GenericMessageDialog::processMessageText()
{
    auto messageHeight = std::max(scale(20.0f), ImGui::GetContentRegionAvail().y - scale(50.0f));
    ImGui::BeginChild("MessageText", {0, messageHeight});
    if (_markdownMessage.has_value()) {
        if (auto clickedLink = _markdownRenderer.render(*_markdownMessage, {})) {
            WebLinkHelper::openInBrowser(*clickedLink);
        }
    } else {
        auto message = _messageKey.empty() ? _message : LocalizationGuiText::format(_messageKey, _messageArguments);
        ImGui::TextWrapped("%s", message.c_str());
    }
    ImGui::EndChild();
}
