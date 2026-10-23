#include "BrowserLoginBannerWidget.h"

#include "BrowserPresentation.h"

#include <string>

#include <imgui.h>

#include <Fonts/IconsFontAwesome5.h>

#include <Network/NetworkService.h>

#include "AlienGui.h"
#include "LoginDialog.h"
#include "StyleService.h"

namespace
{
    auto constexpr PaddingX = 12.0f;
    auto constexpr PaddingY = 6.0f;
    auto constexpr BarWidth = 3.0f;
    auto constexpr IconSpacing = 9.0f;
    auto constexpr TextSpacing = 12.0f;

    auto const Icon = std::string(ICON_FA_USER_PLUS);
}

BrowserLoginBannerWidget _BrowserLoginBannerWidget::create()
{
    return BrowserLoginBannerWidget(new _BrowserLoginBannerWidget());
}

void _BrowserLoginBannerWidget::process()
{
    auto const headline = BrowserPresentation::text("Join the ALIEN community");
    auto const subText = BrowserPresentation::text("Upload your own simulations, react to those of others and get a private workspace. It's free.");
    auto const buttonText = BrowserPresentation::text("Log in or register");
    if (!isVisible()) {
        return;
    }

    auto const& style = ImGui::GetStyle();
    auto width = ImGui::GetContentRegionAvail().x;
    auto height = ImGui::GetFrameHeight() + scale(PaddingY) * 2;

    auto pos = ImGui::GetCursorScreenPos();
    auto drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(pos, {pos.x + width, pos.y + height}, Const::BrowserLoginBannerColor);
    drawList->AddRectFilled(pos, {pos.x + scale(BarWidth), pos.y + height}, Const::BrowserLoginBannerBarColor);

    auto loginButtonWidth = ImGui::CalcTextSize(buttonText.c_str()).x + style.FramePadding.x * 2;
    auto dismissButtonWidth = ImGui::CalcTextSize(ICON_FA_TIMES).x + style.FramePadding.x * 2;
    auto dismissButtonPosX = pos.x + width - scale(PaddingX) - dismissButtonWidth;

    auto textPosY = pos.y + (height - ImGui::GetTextLineHeight()) / 2;
    auto textPosX = pos.x + scale(PaddingX) + scale(BarWidth);
    drawList->AddText({textPosX, textPosY}, Const::BrowserLoginBannerBarColor, Icon.c_str());

    textPosX += ImGui::CalcTextSize(Icon.c_str()).x + scale(IconSpacing);
    drawList->AddText({textPosX, textPosY}, Const::TextBaseColor, headline.c_str());

    textPosX += ImGui::CalcTextSize(headline.c_str()).x + scale(TextSpacing);
    auto subTextWidth = ImGui::CalcTextSize(subText.c_str()).x;
    if (textPosX + subTextWidth + scale(TextSpacing) + loginButtonWidth < dismissButtonPosX - style.ItemSpacing.x) {
        drawList->AddText({textPosX, textPosY}, Const::TextDecentColor, subText.c_str());
        textPosX += subTextWidth + scale(TextSpacing);
    }

    ImGui::SetCursorScreenPos({textPosX, pos.y + scale(PaddingY)});
    if (AlienGui::ActionButton(
            AlienGui::ActionButtonParameters().buttonText(buttonText + "###loginInvitation").highlighted(true).frame(true).transparentBackground(false))) {
        LoginDialog::get().open();
    }

    ImGui::SetCursorScreenPos({dismissButtonPosX, pos.y + scale(PaddingY)});
    if (AlienGui::ActionButton(AlienGui::ActionButtonParameters()
                                   .buttonText(ICON_FA_TIMES)
                                   .tooltip(BrowserPresentation::text("Hide the invitation until the next program start")))) {
        _dismissed = true;
    }

    ImGui::SetCursorScreenPos({pos.x, pos.y + height + style.ItemSpacing.y});
}

bool _BrowserLoginBannerWidget::isVisible() const
{
    return !_dismissed && !NetworkService::get().getLoggedInUserName();
}
