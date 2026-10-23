#pragma once

#include <string>
#include <vector>

#include "GenericMessageDialog.h"
#include "LocalizationGuiText.h"

namespace LocalDialogPresentation
{
    // Resolve owned prose at display time; diagnostic arguments remain event-time data.
    inline void error(std::string const& messageKey, std::string const& details)
    {
        GenericMessageDialog::get().localizedInformation("Error", "localui.error", messageKey, {{"details", details}});
    }

    inline void error(std::string const& messageKey, std::vector<PersisterErrorInfo> const& errors)
    {
        std::string details;
        for (auto const& entry : errors) {
            if (!details.empty()) {
                details += "\n\n";
            }
            details += entry.message;
        }
        error(messageKey, details);
    }
    inline void simulationLoadError(std::string const& details)
    {
        if (details == "Failed to load simulation.") {
            GenericMessageDialog::get().localizedInformation("Error", "localui.error", "localui.error.load");
        } else {
            error("localui.error.open", details);
        }
    }
}
