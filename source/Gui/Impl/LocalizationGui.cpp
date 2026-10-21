#include "LocalizationGui.h"

#include <Base/LocalizationService.h>

#include "AlienGui.h"

void LocalizationGui::languageSelector(float textWidth)
{
    auto& localization = LocalizationService::get();
    auto language = localization.getLanguage() == "ru" ? 1 : 0;
    auto parameters = AlienGui::ComboParameters()
                          .name("settings.language")
                          .label(tr("settings.language"))
                          .textWidth(textWidth)
                          .values({tr("language.english"), tr("language.russian")});
    if (AlienGui::Combo(parameters, language)) {
        localization.requestLanguage(language == 1 ? "ru" : "en");
    }
}
