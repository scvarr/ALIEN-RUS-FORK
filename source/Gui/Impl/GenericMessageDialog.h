#pragma once

#include <map>
#include <optional>

#include <Base/MarkdownDocument.h>
#include <Base/Singleton.h>

#include <Engine/Interface/Definitions.h>

#include <Persister/Interface/PersisterErrorInfo.h>

#include "AlienDialog.h"
#include "Definitions.h"
#include "MainLoopEntity.h"
#include "MarkdownRenderer.h"

class GenericMessageDialog : public AlienDialog
{
    friend class McpLocalizationTests;
    friend class AccountLocalizationTests;

    friend class LocalizationGuiTests;
    friend class BrowserLocalizationTests;

    MAKE_SINGLETON_NO_DEFAULT_CONSTRUCTION(GenericMessageDialog);

public:
    void information(std::string const& title, std::string const& message);
    void information(std::string const& title, std::vector<PersisterErrorInfo> const& errors);
    void markdownInformation(std::string const& title, std::string const& markdownMessage);
    void yesNo(std::string const& title, std::string const& message, std::function<void()> const& yesFunction);

    void localizedInformation(std::string const& stableTitle, std::string const& titleKey, std::string const& messageKey);
    void localizedInformation(
        std::string const& stableTitle,
        std::string const& titleKey,
        std::string const& messageKey,
        std::map<std::string, std::string> const& arguments);

    void localizedMarkdownInformation(std::string const& stableTitle, std::string const& titleKey, std::string const& messageKey);

    void localizedYesNo(std::string const& stableTitle, std::string const& titleKey, std::string const& messageKey, std::function<void()> const& yesFunction);

private:
    GenericMessageDialog();

    void open() override {}
    void processIntern() override;

    void processInformation();
    void processYesNo();
    void processMessageText();

    enum class DialogType
    {
        Information,
        YesNo
    };
    DialogType _dialogType = DialogType::Information;
    std::string _title;
    std::string _message;
    std::string _messageKey;
    std::map<std::string, std::string> _messageArguments;
    std::optional<MarkdownDocument> _markdownMessage;
    MarkdownRenderer _markdownRenderer;
    std::function<void()> _execFunction;
};

inline void showMessage(std::string const& title, std::string const& message)
{
    GenericMessageDialog::get().information(title, message);
}
