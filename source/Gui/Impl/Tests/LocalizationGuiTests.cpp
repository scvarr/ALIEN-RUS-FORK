#include <gtest/gtest.h>

#include <imgui_internal.h>

#include <Fonts/IconsFontAwesome5.h>

#include <Base/GlobalSettings.h>
#include <Base/LocalizationService.h>
#include <Base/Tests/SettingsTestScope.h>

#include <Gui/Impl/AlienDialog.h>
#include <Gui/Impl/AlienGui.h>
#include <Gui/Impl/AlienWindow.h>
#include <Gui/Impl/ModalWindow.h>
#include <Gui/Impl/StyleService.h>
#include <Gui/Impl/TextFontService.h>

class LocalizationGuiTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        _language = GlobalSettings::get().getValue("settings.language", std::string("en"));
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = {1920, 1080};
        StyleService::get().setup();
        LocalizationService::get().load("resources/locales");
        select("en");
    }

    void TearDown() override
    {
        ImGui::DestroyContext();
        GlobalSettings::get().setValue("settings.language", _language);
    }

    void select(std::string const& language)
    {
        LocalizationService::get().requestLanguage(language);
        LocalizationService::get().applyPendingLanguage();
    }

    void frame(std::function<void()> const& content)
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowSize({1000, 800});
        ImGui::Begin("Host");
        content();
        ImGui::End();
        ImGui::Render();
    }

    std::string _language;
    SettingsTestScope _settingsScope;
};

TEST_F(LocalizationGuiTests, fontsAreReadyBeforeSimulationRenderingStarts)
{
    auto& style = StyleService::get();
    auto atlas = ImGui::GetIO().Fonts;
    ASSERT_TRUE(atlas->IsBuilt());
    for (auto font :
         {style.getDefaultFont(),
          style.getTinyFont(),
          style.getSmallBoldFont(),
          style.getMediumBoldFont(),
          style.getMediumFont(),
          style.getLargeFont(),
          style.getMonospaceMediumFont(),
          style.getMonospaceLargeFont(),
          style.getIconFont(),
          style.getReefMediumFont(),
          style.getReefLargeFont()}) {
        ASSERT_EQ(atlas, font->ContainerAtlas);
        EXPECT_TRUE(font->IsLoaded());
    }

    auto labelFont = style.getDefaultFont();
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    labelFont->ContainerAtlas->GetTexDataAsAlpha8(&pixels, &width, &height);
    EXPECT_NE(nullptr, pixels);
    EXPECT_GT(width, 0);
    EXPECT_GT(height, 0);
    EXPECT_NE(nullptr, labelFont->FindGlyphNoFallback('A'));
}

TEST_F(LocalizationGuiTests, textRolesContainRussianPunctuationAndIcons)
{
    auto& style = StyleService::get();
    for (auto font :
         {style.getDefaultFont(),
          style.getTinyFont(),
          style.getSmallBoldFont(),
          style.getMediumBoldFont(),
          style.getMediumFont(),
          style.getLargeFont(),
          style.getMonospaceMediumFont(),
          style.getMonospaceLargeFont()}) {
        for (ImWchar c = 0x0410; c <= 0x044f; ++c) {
            EXPECT_NE(nullptr, font->FindGlyphNoFallback(c)) << c;
        }
        for (ImWchar c : {0x0401, 0x0451, 0x2116, 0x2013, 0x2014, 0x00ab, 0x00bb, 0x201c, 0x201d}) {
            EXPECT_NE(nullptr, font->FindGlyphNoFallback(c)) << c;
        }
    }
    for (auto font : {style.getDefaultFont(), style.getTinyFont(), style.getIconFont()}) {
        EXPECT_NE(nullptr, font->FindGlyphNoFallback(0xf013));  // FontAwesome cog.
    }
    EXPECT_GT(style.getIconFont()->Glyphs.Size, 100);
    auto atlas = ImGui::GetIO().Fonts;
    auto textureSize = ImVec2(static_cast<float>(atlas->TexWidth), static_cast<float>(atlas->TexHeight));
    select("ru");
    EXPECT_EQ(atlas, ImGui::GetIO().Fonts);
    EXPECT_EQ(textureSize.x, atlas->TexWidth);
    EXPECT_EQ(textureSize.y, atlas->TexHeight);
}

TEST_F(LocalizationGuiTests, scaledFontRolesKeepRequiredGlyphs)
{
    for (auto factor : {1.0f, 1.5f, 2.0f, 4.0f}) {
        ImFontAtlas atlas;
        for (auto role : {TextFontRole::Regular, TextFontRole::Bold, TextFontRole::Monospace}) {
            TextFontService::addFont(atlas, role, 16.0f * factor);
        }
        ASSERT_TRUE(atlas.Build());
        for (auto font : atlas.Fonts) {
            EXPECT_FLOAT_EQ(16.0f * factor, font->FontSize);
            EXPECT_NE(nullptr, font->FindGlyphNoFallback(0x0401));
            EXPECT_NE(nullptr, font->FindGlyphNoFallback(0x0451));
            EXPECT_NE(nullptr, font->FindGlyphNoFallback(0x2116));
        }
    }
}

TEST_F(LocalizationGuiTests, openModalKeepsIdentityGeometryAndEditsAcrossLanguages)
{
    ModalWindow modal("Display settings", {760, 350});
    modal.setTitleKey("display.title");
    modal.open();
    ImGuiID windowId = 0;
    ImGuiID childId = 0;
    ImVec2 position;
    ImVec2 size;
    int pendingValue = 73;
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        frame([&] {
            modal.process([&] {
                auto window = ImGui::GetCurrentWindow();
                if (windowId == 0) {
                    ImGui::SetWindowPos({210, 180});
                    windowId = window->ID;
                    childId = ImGui::GetID("pending.edit");
                    position = window->Pos;
                    size = window->Size;
                }
                EXPECT_EQ(windowId, window->ID);
                EXPECT_EQ(childId, ImGui::GetID("pending.edit"));
                EXPECT_EQ(position.x, window->Pos.x);
                EXPECT_EQ(position.y, window->Pos.y);
                EXPECT_EQ(size.x, window->Size.x);
                EXPECT_EQ(size.y, window->Size.y);
                ImGui::InputInt("##pending.edit", &pendingValue);
            });
        });
        EXPECT_TRUE(modal.isOpen());
        EXPECT_EQ(73, pendingValue);
        EXPECT_EQ(1, ImGui::GetCurrentContext()->OpenPopupStack.Size);
    }
    EXPECT_NE(0, windowId);
}

TEST_F(LocalizationGuiTests, widgetIdsRetainLegacyScopesWhenLabelsChange)
{
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        frame([&] {
            auto window = ImGui::GetCurrentWindow();
            auto verify = [&](ImGuiID id, auto const& draw) {
                ImGui::SetActiveID(id, window);
                ImGui::GetCurrentContext()->ActiveIdIsAlive = 0;
                draw();
                EXPECT_EQ(id, ImGui::GetCurrentContext()->ActiveIdIsAlive);
                ImGui::ClearActiveID();
            };
            int resolution = 0;
            verify(ImGui::GetID("##Resolution"), [&] {
                auto parameters = AlienGui::ComboParameters().name("Resolution").label(tr("display.resolution")).values({tr("display.desktop")});
                AlienGui::Combo(parameters, resolution);
            });
            bool fullscreen = false;
            verify(ImGui::GetID("Full screen"), [&] {
                AlienGui::ToggleButton(AlienGui::ToggleButtonParameters().name("Full screen").label(tr("display.fullscreen")), fullscreen);
            });
            ImGui::PushID("Adopt scaling from OS");
            ImGui::PushID(0);
            auto checkboxId = ImGui::GetID("##checkbox");
            ImGui::PopID();
            ImGui::PopID();
            bool automatic = false;
            verify(checkboxId, [&] {
                AlienGui::Checkbox(AlienGui::CheckboxParameters().name("Adopt scaling from OS").label(tr("display.autoscale")), &automatic);
            });
            for (auto const& name : {"Frames per second", "Content scaling"}) {
                ImGui::PushID(name);
                ImGui::PushID("");
                ImGui::PushID(0);
                auto sliderId = ImGui::GetID("##slider");
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                verify(sliderId, [&] {
                    if (std::string(name) == "Frames per second") {
                        int fps = 40;
                        AlienGui::SliderInt(AlienGui::SliderIntParameters().name(name).label(tr("display.fps")).min(20).max(100), &fps);
                    } else {
                        float factor = 1.5f;
                        AlienGui::SliderFloat(AlienGui::SliderFloatParameters().name(name).label(tr("display.scale")).min(1).max(4), &factor);
                    }
                });
            }
            for (auto const& [key, id] : std::vector<std::pair<std::string, std::string>>{
                     {"settings.menu", "settings.menu"},
                     {"settings.saveonexit", "settings.saveonexit"},
                     {"settings.darkmode", "settings.darkmode"},
                     {"settings.display", "settings.display"},
                     {"settings.network", "settings.network"},
                     {"settings.debug", "settings.debug"},
                     {"common.apply", "Adopt"},
                     {"common.cancel", "Cancel"},
                     {"common.yes", "Yes"},
                     {"common.no", "No"}}) {
                auto label = tr(key) + "###" + id;
                EXPECT_EQ(ImGui::GetID(("###" + id).c_str()), ImGui::GetID(label.c_str()));
            }
        });
    }
}

TEST_F(LocalizationGuiTests, alienWindowKeepsOriginalIdentityWithLocalizedTitle)
{
    class TestWindow : public AlienWindow
    {
    public:
        TestWindow()
            : AlienWindow("Legacy window", "localization.test", false)
        {}
        using AlienWindow::setTitleKey;

    private:
        void processIntern() override {}
    } window;
    window.setTitleKey("display.title");
    window.setOn(true);
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        frame([&] { static_cast<MainLoopEntity&>(window).process(); });
        ASSERT_NE(nullptr, ImGui::FindWindowByName("Legacy window"));
        EXPECT_EQ(ImHashStr("Legacy window"), ImGui::FindWindowByName("Legacy window")->ID);
        EXPECT_TRUE(window.isOn());
    }
}

TEST_F(LocalizationGuiTests, alienDialogRetainsOpenState)
{
    class TestDialog : public AlienDialog
    {
    public:
        TestDialog()
            : AlienDialog("Localized test dialog")
        {
            setTitleKey("display.title");
        }
    } dialog;
    dialog.open();
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        frame([&] { static_cast<MainLoopEntity&>(dialog).process(); });
        EXPECT_TRUE(dialog.isOpen());
    }
}

TEST_F(LocalizationGuiTests, legacyDisplayIniGeometryMigratesWithoutChangingSettingsKey)
{
    auto& settings = GlobalSettings::get();
    auto original = settings.getValue("gui.imgui settings", std::string());
    settings.setValue("gui.imgui settings", std::string("[Window][Display settings]\nPos=123,234\nSize=810,420\nCollapsed=0\n\n"));
    settings.loadImGuiSettings();
    auto migrated = ImGui::FindWindowSettingsByID(ImHashStr("###Display settings"));
    EXPECT_NE(nullptr, migrated);
    if (migrated) {
        EXPECT_EQ(123, migrated->Pos.x);
        EXPECT_EQ(234, migrated->Pos.y);
        EXPECT_EQ(810, migrated->Size.x);
        EXPECT_EQ(420, migrated->Size.y);
    }
    settings.setValue("gui.imgui settings", original);
}
