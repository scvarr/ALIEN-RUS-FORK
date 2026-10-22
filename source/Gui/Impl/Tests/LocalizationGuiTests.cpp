#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <ranges>
#include <set>

#include <imgui_internal.h>

#include <Fonts/IconsFontAwesome5.h>

#include <Base/GlobalSettings.h>
#include <Base/LocalizationService.h>
#include <Base/PathHelper.h>
#include <Base/Tests/SettingsTestScope.h>

#include <Data/ParametersValidationService.h>
#include <Data/SimulationParameters.h>
#include <Data/SpecificationEvaluationService.h>
#include <Engine/Impl/SimulationFacadeImpl.h>
#include <Persister/Interface/ReadSimulationRequestData.h>
#include <Persister/Interface/ReadSimulationResultData.h>
#include <Persister/Interface/SerializerService.h>
#include <Gui/Impl/AlienDialog.h>
#include <Gui/Impl/AlienGui.h>
#include <Gui/Impl/AlienWindow.h>
#include <Gui/Impl/DelayedExecutionController.h>
#include <Gui/Impl/ExitDialog.h>
#include <Gui/Impl/GenericMessageDialog.h>
#include <Gui/Impl/LocalizationGui.h>
#include <Gui/Impl/ModalWindow.h>
#include <Gui/Impl/NewSimulationDialog.h>
#include <Gui/Impl/OverlayController.h>
#include <Gui/Impl/SavePictureDialog.h>
#include <Gui/Impl/SimulationParametersBaseWidget.h>
#include <Gui/Impl/SimulationParametersGuiText.h>
#include <Gui/Impl/SimulationParametersMainWindow.h>
#include <Gui/Impl/SimulationParametersPresentation.h>
#include <Gui/Impl/SpecificationGuiService.h>
#include <Gui/Impl/StyleService.h>
#include <Gui/Impl/Utf8Search.h>
#include <Gui/Impl/WindowController.h>

namespace
{
    ParameterGroupSpec const& parameterGroup(ParametersSpec const& spec, std::string const& name)
    {
        return *std::ranges::find_if(spec._groups, [&](auto const& group) { return group._name == name; });
    }

    size_t visibleParameters(ParameterGroupSpec const& group)
    {
        return std::ranges::count_if(group._parameters, [](auto const& parameter) { return parameter._visible; });
    }
}
#include <Gui/Impl/TextFontService.h>

class LocalizationGuiTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        _language = GlobalSettings::get().getValue("settings.language", std::string("en"));
        _originalScale = WindowController::get().getContentScaleFactor();
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
        WindowController::get()._contentScaleFactor = _originalScale;
        OverlayController::get()._messageStartTimepoint.reset();
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

    void openWithoutSimulation(AlienDialog& dialog) { dialog._modalWindow.open(); }

    void closeImmediately(AlienDialog& dialog)
    {
        frame([&] { dialog._modalWindow.process([&] { dialog._modalWindow.close(); }); });
    }

    ImRect processDialogButton(AlienDialog& dialog)
    {
        ImRect button;
        dialog._modalWindow.process([&] {
            dialog.processIntern();
            button = GImGui->LastItemData.Rect;
        });
        return button;
    }

    void prepareParametersTable()
    {
        auto& window = SimulationParametersMainWindow::get();
        window._locations = {{"Base", LocationType::Base, "-", "20.0%"}, {"User layer", LocationType::Layer, "(12, 34)", "50.0%"}};
        window._selectedOrderNumber = 1;
    }

    void parametersTable() { SimulationParametersMainWindow::get().processLocationTable(); }
    void parametersToolbar() { SimulationParametersMainWindow::get().processToolbar(); }
    void verifyParametersSelection()
    {
        auto& window = SimulationParametersMainWindow::get();
        EXPECT_EQ(1, window._selectedOrderNumber);
        EXPECT_EQ("User layer", window._locations.at(1).name);
        EXPECT_EQ("(12, 34)", window._locations.at(1).position);
        EXPECT_EQ("50.0%", window._locations.at(1).strength);
    }

    void parametersGroup(std::vector<ParameterSpec> const& specs, SimulationParameters& values, SimulationParameters& defaults, int orderNumber)
    {
        SpecificationGuiService::get().createWidgetsForParameterGroup(specs, true, values, defaults, orderNumber, {});
    }

    void setTestScale(float factor)
    {
        WindowController::get()._contentScaleFactor = factor;
        ImGui::GetIO().Fonts->Clear();
        ImGui::GetIO().FontDefault = nullptr;
        ImGui::GetStyle() = ImGuiStyle();
        StyleService::get().setup();
    }

    void switcherFrame(float width, std::function<void()> const& content)
    {
        ImGui::GetIO().DisplaySize = {3000, 2200};
        ImGui::NewFrame();
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({width, 1900});
        ImGui::Begin("Switcher host");
        content();
        ImGui::End();
        ImGui::Render();
    }

    std::string overlayMessage() const { return OverlayController::get()._message; }
    void renderOverlay() { OverlayController::get().processMessage(); }

    float _originalScale = 1;
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

TEST_F(LocalizationGuiTests, languagePopupKeepsMenuOpenAndAcceptsMouseSelection)
{
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    bool menuOpen = true;
    ImRect comboRect;
    ImGuiWindow* menuWindow = nullptr;
    auto draw = [&] {
        LocalizationService::get().applyPendingLanguage();
        frame([&] {
            AlienGui::BeginMenuBar();
            AlienGui::BeginMenu(tr("settings.menu") + "###settings.menu", menuOpen, false);
            if (menuOpen) {
                for (auto const& key : {"settings.saveonexit", "settings.darkmode", "settings.display", "settings.network"}) {
                    AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr(key) + "###" + key), [] {});
                }
                ImGui::Dummy({scale(280.0f), 0});
                auto pos = ImGui::GetCursorScreenPos();
                comboRect = ImRect(pos, ImVec2(pos.x + ImGui::GetContentRegionAvail().x - scale(120.0f), pos.y + ImGui::GetFrameHeight()));
                LocalizationGui::languageSelector();
                AlienGui::MenuSeparator();
                AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr("settings.debug") + "###settings.debug"), [] {});
                menuWindow = ImGui::GetCurrentWindow();
            }
            AlienGui::EndMenu();
            AlienGui::EndMenuBar();
        });
    };

    io.AddMousePosEvent(10, 10);
    for (int i = 0; i < 3; ++i) {
        draw();
    }
    ASSERT_TRUE(menuOpen);
    ASSERT_NE(nullptr, menuWindow);
    for (auto const& [index, language] : std::vector<std::pair<int, std::string>>{{1, "ru"}, {0, "en"}}) {
        io.AddMousePosEvent(10, 10);
        draw();
        draw();
        ASSERT_TRUE(menuOpen);
        auto center = comboRect.GetCenter();
        io.AddMousePosEvent(center.x, center.y);
        draw();
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
        draw();
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
        draw();
        draw();
        draw();
        auto& popups = ImGui::GetCurrentContext()->OpenPopupStack;
        ASSERT_EQ(1, popups.Size);
        auto popup = popups.back().Window;
        ASSERT_NE(nullptr, popup);
        auto pos = popup->DC.CursorStartPos;
        pos.x += 20;
        pos.y += ImGui::GetTextLineHeight() * 0.5f + static_cast<float>(index) * ImGui::GetTextLineHeightWithSpacing();
        if (language == "ru") {
            auto popupEdge = ImVec2(pos.x, popup->Rect().Max.y - 1);
            ASSERT_GT(popupEdge.y, menuWindow->Rect().Max.y);
            io.AddMousePosEvent(popupEdge.x, popupEdge.y);
            draw();
            ASSERT_TRUE(menuOpen);
        }
        io.AddMousePosEvent(pos.x, pos.y);
        draw();
        ASSERT_TRUE(menuOpen);
        ASSERT_EQ(1, popups.Size);
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
        draw();
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
        draw();
        draw();
        EXPECT_EQ(language, LocalizationService::get().getLanguage());
        EXPECT_TRUE(popups.empty());
    }

    io.AddMousePosEvent(1800, 900);
    draw();
    EXPECT_FALSE(menuOpen);
}

TEST_F(LocalizationGuiTests, l2aDialogsKeepActualWindowsAndEditedInputAcrossLanguages)
{
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (auto const& [dialog, title, hasWidth] : std::vector<std::tuple<AlienDialog*, std::string, bool>>{
             {&NewSimulationDialog::get(), "New simulation", true}, {&ExitDialog::get(), "Exit", false}, {&SavePictureDialog::get(), "Save picture", true}}) {
        SCOPED_TRACE(title);
        openWithoutSimulation(*dialog);
        ImGuiWindow* original = nullptr;
        ImGuiID inputId = 0;
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            frame([&] { static_cast<MainLoopEntity&>(*dialog).process(); });
            ASSERT_TRUE(dialog->isOpen());
            auto window = ImGui::FindWindowByName(("###" + title).c_str());
            ASSERT_NE(nullptr, window);
            if (!original) {
                original = window;
            }
            EXPECT_EQ(original, window);
            EXPECT_EQ(1, ImGui::GetCurrentContext()->OpenPopupStack.Size);
            if (hasWidth) {
                auto scope = ImHashStr(title.c_str(), 0, window->ID);
                auto widthScope = ImHashStr("Width", 0, scope);
                auto id = ImHashStr("###input", 0, widthScope);
                if (inputId == 0) {
                    inputId = id;
                    frame([&] { static_cast<MainLoopEntity&>(*dialog).process(); });
                    frame([&] { static_cast<MainLoopEntity&>(*dialog).process(); });
                    auto pos = window->DC.CursorStartPos;
                    pos.x += 40;
                    auto row = title == "New simulation" ? 1.0f : 0.0f;
                    pos.y += ImGui::GetFrameHeight() * (row + 0.5f) + ImGui::GetStyle().ItemSpacing.y * row;
                    io.AddMousePosEvent(pos.x, pos.y);
                    frame([&] { static_cast<MainLoopEntity&>(*dialog).process(); });
                    io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
                    frame([&] { static_cast<MainLoopEntity&>(*dialog).process(); });
                    io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
                    frame([&] { static_cast<MainLoopEntity&>(*dialog).process(); });
                    ImGui::GetIO().AddKeyEvent(ImGuiMod_Ctrl, true);
                    ImGui::GetIO().AddKeyEvent(ImGuiKey_A, true);
                    frame([&] { static_cast<MainLoopEntity&>(*dialog).process(); });
                    ImGui::GetIO().AddKeyEvent(ImGuiMod_Ctrl, false);
                    ImGui::GetIO().AddKeyEvent(ImGuiKey_A, false);
                    ImGui::GetIO().AddInputCharactersUTF8("73");
                    frame([&] { static_cast<MainLoopEntity&>(*dialog).process(); });
                }
                EXPECT_EQ(inputId, ImGui::GetActiveID());
                auto state = ImGui::GetInputTextState(inputId);
                ASSERT_NE(nullptr, state);
                EXPECT_STREQ("73", state->TextA.Data);
            }
        }
        closeImmediately(*dialog);
        EXPECT_FALSE(dialog->isOpen());
    }
}

TEST_F(LocalizationGuiTests, l2aInputLabelsRetainLegacyIdsAndValues)
{
    int width = 73;
    float energy = 1234;
    char name[64] = "Edited project";
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        frame([&] {
            auto verify = [&](ImGuiID id, auto const& draw) {
                ImGui::SetActiveID(id, ImGui::GetCurrentWindow());
                ImGui::GetCurrentContext()->ActiveIdIsAlive = 0;
                draw();
                EXPECT_EQ(id, ImGui::GetCurrentContext()->ActiveIdIsAlive);
                ImGui::ClearActiveID();
            };
            ImGui::PushID("Width");
            auto widthId = ImGui::GetID("###input");
            ImGui::PopID();
            verify(widthId, [&] { AlienGui::InputInt(AlienGui::InputIntParameters().name("Width").label(tr("common.width")), width); });
            ImGui::PushID("");
            auto energyId = ImGui::GetID("##Energy");
            ImGui::PopID();
            verify(energyId, [&] { AlienGui::InputFloat(AlienGui::InputFloatParameters().name("Energy").label(tr("new.energy")), energy); });
            verify(ImGui::GetID("##Project name"), [&] {
                AlienGui::InputText(AlienGui::InputTextParameters().name("Project name").label(tr("new.project")), name, sizeof(name));
            });
        });
        EXPECT_EQ(73, width);
        EXPECT_FLOAT_EQ(1234, energy);
        EXPECT_STREQ("Edited project", name);
    }
}

TEST_F(LocalizationGuiTests, l2aAllMenusKeepWindowsAndDistinctItemIds)
{
    for (auto const& key : {"menu.simulation", "menu.network", "menu.windows", "menu.editor", "menu.view", "menu.tools", "settings.menu", "menu.help"}) {
        bool open = true;
        ImGuiWindow* original = nullptr;
        ImGui::GetIO().AddMousePosEvent(10, 10);
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            frame([&] {
                AlienGui::BeginMenuBar();
                AlienGui::BeginMenu(tr(key) + "###" + key, open, false);
                if (open) {
                    auto window = ImGui::GetCurrentWindow();
                    if (!original) {
                        original = window;
                    }
                    EXPECT_EQ(original, window);
                    AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr("menu.save") + "###menu.save"), [] {});
                    auto first = ImGui::GetItemID();
                    AlienGui::MenuItem(AlienGui::MenuItemParameters().name(tr("menu.save") + "###menu.savepicture"), [] {});
                    EXPECT_NE(first, ImGui::GetItemID());
                }
                AlienGui::EndMenu();
                AlienGui::EndMenuBar();
            });
            EXPECT_TRUE(open);
        }
        EXPECT_NE(nullptr, original);
    }
}

TEST_F(LocalizationGuiTests, l2aLocalizedErrorKeepsIdentityAcrossLanguages)
{
    auto& dialog = GenericMessageDialog::get();
    dialog.localizedInformation("Save picture error", "picture.title", "picture.error");
    ImGuiWindow* original = nullptr;
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        frame([&] { static_cast<MainLoopEntity&>(dialog).process(); });
        auto window = ImGui::FindWindowByName("###Save picture error");
        ASSERT_NE(nullptr, window);
        if (!original) {
            original = window;
        }
        EXPECT_EQ(original, window);
        EXPECT_TRUE(dialog.isOpen());
    }
    closeImmediately(dialog);
}

TEST_F(LocalizationGuiTests, l2aLegacyDialogGeometryMigrates)
{
    auto& settings = GlobalSettings::get();
    for (auto const& title : {"New simulation", "Exit", "Save picture"}) {
        settings.setValue("gui.imgui settings", "[Window][" + std::string(title) + "]\nPos=123,234\nSize=710,400\nCollapsed=0\n\n");
        settings.loadImGuiSettings();
        auto migrated = ImGui::FindWindowSettingsByID(ImHashStr(("###" + std::string(title)).c_str()));
        ASSERT_NE(nullptr, migrated);
        EXPECT_EQ(123, migrated->Pos.x);
        EXPECT_EQ(710, migrated->Size.x);
    }
}

TEST_F(LocalizationGuiTests, l2b1TreeLabelsPreserveLegacyIdsAndSavedState)
{
    for (auto const& name : {"Overview", "Expert settings", "###parameters"}) {
        ImGuiID original = 0;
        bool first = true;
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            frame([&] {
                auto flags = AlienGui::TreeNodeParameters().name(name).defaultOpen(false);
                if (!first) {
                    flags.label(tr("parameters.overview"));
                }
                bool open = AlienGui::BeginTreeNode(flags);
                auto id = ImGui::GetItemID();
                if (first) {
                    original = id;
                    // Simulate a user opening the actual tree, then save it in the wrapper.
                    ImGui::GetCurrentWindow()->DC.StateStorage->SetInt(id, 1);
                } else {
                    EXPECT_EQ(original, id);
                    EXPECT_TRUE(open);
                }
                AlienGui::EndTreeNode();
            });
            first = false;
        }
        // A new parent scope must recover the same saved state by the stable name.
        frame([&] {
            ImGui::PushID("new parent");
            EXPECT_TRUE(AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label("Another caption").defaultOpen(false)));
            AlienGui::EndTreeNode();
            ImGui::PopID();
        });
    }
}

TEST_F(LocalizationGuiTests, l2b1ProductionTableRetainsLayoutSelectionAndData)
{
    prepareParametersTable();
    ImGuiTable* original = nullptr;
    ImGuiID headers[5] = {};
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        for (int pass = 0; pass < 2; ++pass) {
            frame([&] {
                auto id = ImGui::GetID("Locations");
                parametersTable();
                auto table = GImGui->Tables.GetByKey(id);
                ASSERT_NE(nullptr, table);
                if (!original) {
                    original = table;
                }
                EXPECT_EQ(original, table);
                for (int column = 0; column < 5; ++column) {
                    auto& c = table->Columns[column];
                    EXPECT_EQ(0u, c.UserID);
                    auto label = table->ColumnsNames.Buf.Data + c.NameOffset;
                    ImGui::PushID(column);
                    auto headerId = ImGui::GetID(label);
                    ImGui::PopID();
                    if (headers[column] == 0) {
                        headers[column] = headerId;
                    }
                    EXPECT_EQ(headers[column], headerId);
                }
                if (language == std::string("en") && pass == 0) {
                    table->Columns[0].WidthRequest = 207;
                    table->Columns[4].IsUserEnabledNextFrame = false;
                    table->Columns[0].DisplayOrder = 1;
                    table->Columns[1].DisplayOrder = 0;
                    table->DisplayOrderToIndex[0] = 1;
                    table->DisplayOrderToIndex[1] = 0;
                    table->IsSettingsDirty = true;
                }
                if (language == std::string("ru")) {
                    EXPECT_EQ(1, table->Columns[0].DisplayOrder);
                    EXPECT_EQ(0, table->Columns[1].DisplayOrder);
                    EXPECT_FLOAT_EQ(207, table->Columns[0].WidthRequest);
                    EXPECT_FALSE(table->Columns[4].IsEnabled);
                }
            });
            verifyParametersSelection();
        }
    }
}

TEST_F(LocalizationGuiTests, l2b1DynamicTitlesKeepUserTextAndNumericWindowIdentity)
{
    _SimulationParametersBaseWidget base;
    ImGuiWindow* original = nullptr;
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        EXPECT_EQ(tr("parameters.window.base"), base.getLocationName());
        auto name = std::string("User {name} 100% ### text");
        for (auto const& key : {"parameters.window.layer", "parameters.window.source"}) {
            EXPECT_NE(std::string::npos, SimulationParametersGuiText::named(key, name).find(name));
        }
        EXPECT_NE(std::string::npos, SimulationParametersGuiText::section(name, false, true).find(name));
        frame([&] {
            auto title = SimulationParametersGuiText::named("parameters.window.layer", name) + "###973";
            ImGui::Begin(title.c_str());
            auto window = ImGui::GetCurrentWindow();
            if (!original) {
                original = window;
                ImGui::SetWindowPos({123, 234});
            }
            EXPECT_EQ(original, window);
            EXPECT_EQ(123, window->Pos.x);
            EXPECT_EQ(234, window->Pos.y);
            ImGui::End();
        });
    }
}

TEST_F(LocalizationGuiTests, l2b1ProductionToolbarOverflowSurvivesLanguageSwitch)
{
    prepareParametersTable();
    ImGuiWindow* original = nullptr;
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        frame([&] {
            ImGui::BeginChild("narrow toolbar", {240, 600});
            ImGui::PushID("SimulationParameters");
            ImGui::OpenPopup("##toolbarMenu");
            ImGui::PopID();
            parametersToolbar();
            ImGui::PushID("SimulationParameters");
            EXPECT_TRUE(ImGui::IsPopupOpen("##toolbarMenu"));
            ImGui::PopID();
            ImGui::EndChild();
        });
        auto popup = GImGui->OpenPopupStack.back().Window;
        ASSERT_NE(nullptr, popup);
        if (!original) {
            original = popup;
        }
        EXPECT_EQ(original, popup);
        verifyParametersSelection();
    }
}

TEST_F(LocalizationGuiTests, l2b1LocalizedErrorsRemainOpenAndConfirmable)
{
    auto& dialog = GenericMessageDialog::get();
    for (auto const& key :
         {"parameters.file.openerror", "parameters.file.saveerror", "parameters.reference.error", "parameters.limit.layers", "parameters.limit.sources"}) {
        dialog.localizedInformation("L2B1 error", "common.error", key);
        ImGuiWindow* original = nullptr;
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            frame([&] { static_cast<MainLoopEntity&>(dialog).process(); });
            auto window = ImGui::FindWindowByName("###L2B1 error");
            ASSERT_NE(nullptr, window);
            if (!original) {
                original = window;
            }
            EXPECT_EQ(original, window);
            EXPECT_TRUE(dialog.isOpen());
        }
        ImRect button;
        auto draw = [&] {
            frame([&] {
                button = processDialogButton(dialog);
                static_cast<MainLoopEntity&>(DelayedExecutionController::get()).process();
            });
        };
        draw();
        auto center = button.GetCenter();
        ImGui::GetIO().AddMousePosEvent(center.x, center.y);
        draw();
        ImGui::GetIO().AddMouseButtonEvent(0, true);
        draw();
        ImGui::GetIO().AddMouseButtonEvent(0, false);
        draw();
        draw();
        draw();
        EXPECT_FALSE(dialog.isOpen());
    }
}

TEST_F(LocalizationGuiTests, l2b2BindingsUseMembersAndKeepFourContextDescriptions)
{
    auto const& original = SimulationParameters::getSpec();
    auto const& general = parameterGroup(original, "General");
    auto const& location = parameterGroup(original, "Location");
    ASSERT_EQ(5, general._parameters.size());
    ASSERT_EQ(4, location._parameters.size());
    std::vector<std::string> slugs = {
        "general.project",
        "general.layer",
        "general.source",
        "general.opacity",
        "general.share",
        "location.layerposition",
        "location.layervelocity",
        "location.sourceposition",
        "location.sourcevelocity"};
    std::vector<ParameterSpec> parameters = general._parameters;
    parameters.insert(parameters.end(), location._parameters.begin(), location._parameters.end());
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        for (size_t i = 0; i < parameters.size(); ++i) {
            auto const& parameter = parameters.at(i);
            auto key = "parameters." + slugs.at(i);
            EXPECT_EQ(tr(key + ".name"), SimulationParametersPresentation::label(parameter));
            EXPECT_EQ(tr(key + ".description"), SimulationParametersPresentation::description(parameter));
            if (std::string(language) == "en") {
                EXPECT_EQ(parameter._name, SimulationParametersPresentation::label(parameter));
                EXPECT_EQ(parameter._description, SimulationParametersPresentation::description(parameter));
            }
            auto renamed = parameter;
            renamed._name = "Same canonical display name";
            EXPECT_EQ(SimulationParametersPresentation::label(parameter), SimulationParametersPresentation::label(renamed));
        }
        EXPECT_NE(
            SimulationParametersPresentation::description(location._parameters.at(0)),
            SimulationParametersPresentation::description(location._parameters.at(2)));
        EXPECT_NE(
            SimulationParametersPresentation::description(location._parameters.at(1)),
            SimulationParametersPresentation::description(location._parameters.at(3)));
    }
    select("ru");
    EXPECT_EQ("Общие", SimulationParametersPresentation::label(general));
    EXPECT_EQ("Расположение", SimulationParametersPresentation::label(location));
    auto const& other = parameterGroup(original, "Cell type: Reconnector");
    EXPECT_EQ(other._name, SimulationParametersPresentation::label(other));
    for (auto const& parameter : other._parameters) {
        EXPECT_EQ(parameter._name, SimulationParametersPresentation::label(parameter));
        EXPECT_EQ(parameter._description, SimulationParametersPresentation::description(parameter));
    }
    auto const& untouched = parameterGroup(SimulationParameters::getSpec(), "Location");
    EXPECT_EQ("Position (x,y)", untouched._parameters.at(0)._name);
    EXPECT_EQ("Center of the layer in world coordinates.", untouched._parameters.at(0)._description);
    EXPECT_EQ("%.2f", std::get<Float2Spec>(untouched._parameters.at(0)._reference)._format);
    EXPECT_EQ(-4.0f, std::get<RealVector2D>(std::get<Float2Spec>(untouched._parameters.at(1)._reference)._min).x);
    EXPECT_EQ(4.0f, std::get<RealVector2D>(std::get<Float2Spec>(untouched._parameters.at(1)._reference)._max).y);
}

TEST_F(LocalizationGuiTests, l2b2BilingualFilterPreservesIndicesReferencesAndGroupMatches)
{
    auto const& original = SimulationParameters::getSpec();
    select("ru");
    for (auto const& [query, group, count] : std::vector<std::tuple<std::string, std::string, size_t>>{
             {"оБщИе", "General", 5},
             {"GENERAL", "General", 5},
             {"название", "General", 3},
             {"Opacity", "General", 1},
             {"дОлЯ", "General", 1},
             {"расположение", "Location", 4},
             {"поЛоЖеНие", "Location", 4},
             {"Положение (", "Location", 2},
             {"POSITION", "Location", 2},
             {"СКОРОСТЬ", "Location", 2},
             {"velocity", "Location", 2},
             {"Visualization", "Visualization", 13},
             {"Glow", "Visualization", 1},
             {"no such parameter", "General", 0}}) {
        SCOPED_TRACE(query);
        ParametersFilter queryFilter{.containedText = query};
        auto filtered = SimulationParametersPresentation::filter(original, queryFilter);
        EXPECT_EQ(original._groups.size(), filtered._groups.size());
        auto const& result = parameterGroup(filtered, group);
        EXPECT_EQ(count, visibleParameters(result));
        auto const& source = parameterGroup(original, group);
        ASSERT_EQ(source._parameters.size(), result._parameters.size());
        for (size_t i = 0; i < source._parameters.size(); ++i) {
            EXPECT_EQ(source._parameters.at(i)._name, result._parameters.at(i)._name);
            EXPECT_EQ(source._parameters.at(i)._description, result._parameters.at(i)._description);
        }
        EXPECT_EQ(query, queryFilter.containedText);
    }
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        for (auto const& query : {ParametersFilter{}, ParametersFilter{.containedText = ""}}) {
            auto full = SimulationParametersPresentation::filter(original, query);
            for (size_t i = 0; i < original._groups.size(); ++i) {
                EXPECT_EQ(original._groups.at(i)._name, full._groups.at(i)._name);
                EXPECT_EQ(original._groups.at(i)._parameters.size(), full._groups.at(i)._parameters.size());
                for (size_t j = 0; j < full._groups.at(i)._parameters.size(); ++j) {
                    EXPECT_EQ(original._groups.at(i)._parameters.at(j)._visible, full._groups.at(i)._parameters.at(j)._visible);
                }
            }
        }
        auto filtered = SimulationParametersPresentation::filter(original, {.containedText = "Доля"});
        EXPECT_EQ(std::string(language) == "ru" ? 1 : 0, visibleParameters(parameterGroup(filtered, "General")));
    }
    auto nested = ParameterSpec().name("Parent").reference(AlternativeSpec().alternatives({{"Mode", {ParameterSpec().name("Child")}}}));
    auto tree = ParametersSpec().groups({ParameterGroupSpec().name("Group").parameters({nested})});
    auto filtered = SimulationParametersPresentation::filter(tree, {.containedText = "cHiLd"});
    auto const& parent = filtered._groups.at(0)._parameters.at(0);
    EXPECT_TRUE(parent._visible);
    EXPECT_TRUE(std::get<AlternativeSpec>(parent._reference)._alternatives.at(0).second.at(0)._visible);
    filtered = SimulationParametersPresentation::filter(tree, {.containedText = "gRoUp"});
    EXPECT_TRUE(std::get<AlternativeSpec>(filtered._groups.at(0)._parameters.at(0)._reference)._alternatives.at(0).second.at(0)._visible);
}

TEST_F(LocalizationGuiTests, l2b2Utf8SearchAndHighlightUseCompleteCharacters)
{
    for (auto const& query : {"Ёж", "ёЖ"}) {
        auto found = Utf8Search::find("АБ Ёжик xyz", query);
        ASSERT_TRUE(found);
        EXPECT_EQ("Ёж", std::string("АБ Ёжик xyz").substr(found->begin, found->end - found->begin));
    }
    EXPECT_TRUE(Utf8Search::find("MiXeD ПОЛОЖЕНИЕ", "mixed положение"));
    EXPECT_FALSE(Utf8Search::find("Еж", "ёж"));
    for (auto const& invalid :
         {std::string("\xd0"), std::string("\xc0\x80"), std::string("\xed\xa0\x80"), std::string("\xf4\x90\x80\x80"), std::string("\x80")}) {
        EXPECT_FALSE(Utf8Search::find("Положение", invalid));
        EXPECT_FALSE(Utf8Search::find(invalid, "a"));
    }
    for (bool tree : {false, true}) {
        auto vertices = [&](std::optional<std::string> query) {
            int result = 0;
            frame([&] {
                auto draw = ImGui::GetWindowDrawList();
                auto before = draw->VtxBuffer.Size;
                if (tree) {
                    AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name("Highlight").label("АБ Ёжик xyz").highlightedSubString(query));
                    result = draw->VtxBuffer.Size - before;
                    AlienGui::EndTreeNode();
                } else {
                    AlienGui::Text(AlienGui::TextParameters().text("АБ Ёжик xyz").highlightedSubString(query));
                    result = draw->VtxBuffer.Size - before;
                }
            });
            return result;
        };
        vertices(std::nullopt);
        auto plain = vertices(std::nullopt);
        EXPECT_EQ(plain + 16, vertices("ёЖ"));  // Two complete glyphs drawn twice.
        EXPECT_EQ(plain, vertices(std::string("\x81")));
    }
}

TEST_F(LocalizationGuiTests, l2b2ProductionTextAndVelocityRetainIdsEditsAndValues)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    values->numLayers = defaults->numLayers = 1;
    values->layerOrderNumbers[0] = defaults->layerOrderNumbers[0] = 1;
    values->layerVelocity.layerValues[0] = {1.25f, -2.5f};
    std::string name = "Layer draft";
    std::copy(name.begin(), name.end(), values->layerName.layerValues[0]);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ParametersFilter filter{.containedText = "Layer name"};
    ImGuiID id = 0;
    auto draw = [&](bool focus) {
        frame([&] {
            auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), filter);
            if (focus) {
                ImGui::SetKeyboardFocusHere();
            }
            parametersGroup(parameterGroup(filtered, "General")._parameters, *values, *defaults, 1);
            ImGui::PushID(1);  // Original index of Layer name, even when other parameters are hidden.
            auto current = ImGui::GetID("##Layer name");
            ImGui::PopID();
            if (!id) {
                id = current;
            }
            EXPECT_EQ(id, current);
        });
    };
    draw(false);
    draw(true);
    draw(false);
    ASSERT_EQ(id, GImGui->ActiveId);
    io.AddInputCharactersUTF8("Edited layer");
    draw(false);
    auto draft = std::string(values->layerName.layerValues[0]);
    EXPECT_NE(name, draft);
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        draw(false);
        EXPECT_EQ(id, GImGui->ActiveId);
        EXPECT_EQ(id, GImGui->InputTextState.ID);
        EXPECT_EQ(draft, std::string(values->layerName.layerValues[0]));
        EXPECT_EQ("Layer name", filter.containedText);
    }
    ImGui::ClearActiveID();
    for (int order : {1, 2}) {
        values->numSources = defaults->numSources = 1;
        values->sourceOrderNumbers[0] = defaults->sourceOrderNumbers[0] = 2;
        values->sourceVelocity.sourceValues[0] = {-1.125f, 2.25f};
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            auto before = *values;
            frame([&] {
                auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = "Velocity"});
                parametersGroup(parameterGroup(filtered, "Location")._parameters, *values, *defaults, order);
            });
            EXPECT_EQ(before, *values);
        }
    }
}

TEST_F(LocalizationGuiTests, l2b2GroupStateSurvivesLanguageAndCanonicalNameStaysEnglish)
{
    for (auto const& name :
         {"General",
          "Location",
          "Shape",
          "Force field",
          "Physics: Motion",
          "Physics: Thresholds",
          "Radiation",
          "Cell life cycle",
          "Cell construction",
          "Mutations"}) {
        auto const& group = parameterGroup(SimulationParameters::getSpec(), name);
        ImGuiID originalId = 0;
        int iteration = 0;
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            frame([&] {
                ImGui::PushID(name);
                bool open = AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(SimulationParametersPresentation::label(group)));
                auto current = ImGui::GetItemID();
                if (iteration++ == 0) {
                    originalId = current;
                    ImGui::GetStateStorage()->SetInt(current, 0);
                } else {
                    EXPECT_EQ(originalId, current);
                    EXPECT_FALSE(open);
                }
                AlienGui::EndTreeNode();
                ImGui::PopID();
            });
        }
    }
}

TEST_F(LocalizationGuiTests, l2b2PickerKeepsCallbacksValuesAndSliderIdentity)
{
    auto const& spec = parameterGroup(SimulationParameters::getSpec(), "Location")._parameters.at(0);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    float x = 12.5f, y = 34.25f;
    bool enabled = false;
    int transitions = 0;
    std::optional<RealVector2D> selected;
    ImVec2 button;
    ImVec2 revert;
    ImGuiID sliderY = 0;
    auto draw = [&] {
        frame([&] {
            auto pos = ImGui::GetCursorScreenPos();
            auto width = ImGui::GetContentRegionAvail().x - scale(287) - scale(22) - ImGui::GetStyle().FramePadding.x;
            button = {pos.x + width + scale(11), pos.y + ImGui::GetFrameHeight() * 0.5f};
            auto const& style = ImGui::GetStyle();
            revert = {
                pos.x + width + style.ItemSpacing.x - style.FramePadding.x + scale(22) + style.ItemSpacing.x
                    + (ImGui::CalcTextSize(ICON_FA_UNDO).x + style.FramePadding.x * 2) * 0.5f,
                button.y};
            AlienGui::SliderFloat2(
                AlienGui::SliderFloat2Parameters()
                    .name(spec._name)
                    .label(SimulationParametersPresentation::label(spec))
                    .min({0, 0})
                    .max({100, 100})
                    .format("%.2f")
                    .defaultValue(RealVector2D{9, 10})
                    .textWidth(287)
                    .tooltip(SimulationParametersPresentation::description(spec))
                    .mousePickerTooltip(tr("parameters.location.pick.tooltip"))
                    .mousePickerMessage(tr("parameters.location.pick.message"))
                    .getMousePickerEnabledFunc([&] { return enabled; })
                    .setMousePickerEnabledFunc([&](bool value) {
                        enabled = value;
                        ++transitions;
                    })
                    .getMousePickerPositionFunc([&] { return selected; }),
                x,
                y);
            if (!sliderY) {
                sliderY = ImGui::GetItemID();
            }
            EXPECT_EQ(sliderY, ImGui::GetItemID());
        });
    };
    draw();
    draw();
    io.AddMousePosEvent(button.x, button.y);
    draw();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
    draw();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
    draw();
    ASSERT_TRUE(enabled);
    EXPECT_EQ(1, transitions);
    selected = RealVector2D{47.5f, 62.25f};
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        draw();
        EXPECT_FLOAT_EQ(47.5f, x);
        EXPECT_FLOAT_EQ(62.25f, y);
        EXPECT_TRUE(enabled);
        EXPECT_EQ(1, transitions);
    }
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
    draw();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
    draw();
    EXPECT_FALSE(enabled);
    selected = RealVector2D{1, 2};
    draw();
    EXPECT_FLOAT_EQ(47.5f, x);
    EXPECT_FLOAT_EQ(62.25f, y);
    EXPECT_EQ(2, transitions);
    select("ru");
    draw();
    io.AddMousePosEvent(revert.x, revert.y);
    draw();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
    draw();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
    draw();
    EXPECT_FLOAT_EQ(9, x);
    EXPECT_FLOAT_EQ(10, y);
    EXPECT_EQ(2, transitions);
    AlienGui::SliderFloat2Parameters legacy;
    EXPECT_EQ("Select a position with the mouse", legacy._mousePickerTooltip);
    EXPECT_EQ("Select a position in the simulation view", legacy._mousePickerMessage);
}

TEST_F(LocalizationGuiTests, l2b2FilterRetainsActiveInputAndUtf8QueryAcrossLanguages)
{
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    std::string query;
    ImGuiID original = 0;
    auto draw = [&](bool focus) {
        frame([&] {
            if (focus) {
                ImGui::SetKeyboardFocusHere();
            }
            AlienGui::InputFilter(AlienGui::InputFilterParameters().id("Filter (case insensitive)").hint(tr("parameters.filter")), query);
            auto id = ImGui::GetID("##Filter (case insensitive)");
            if (!original) {
                original = id;
            }
            EXPECT_EQ(original, id);
        });
    };
    draw(false);
    draw(true);
    draw(false);
    ASSERT_EQ(original, GImGui->ActiveId);
    io.AddInputCharactersUTF8("ПоЛоЖеНие Ёё");
    draw(false);
    ASSERT_EQ("ПоЛоЖеНие Ёё", query);
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        draw(false);
        EXPECT_EQ(original, GImGui->ActiveId);
        EXPECT_EQ(original, GImGui->InputTextState.ID);
        EXPECT_EQ("ПоЛоЖеНие Ёё", query);
    }
    ImGui::ClearActiveID();
    // Without the additive ID, existing callers still derive identity from their hint.
    frame([&] {
        ImGui::SetKeyboardFocusHere();
        AlienGui::InputFilter(AlienGui::InputFilterParameters().hint("Legacy search"), query);
    });
    frame([&] { AlienGui::InputFilter(AlienGui::InputFilterParameters().hint("Legacy search"), query); });
    frame([&] {
        AlienGui::InputFilter(AlienGui::InputFilterParameters().hint("Legacy search"), query);
        EXPECT_EQ(ImGui::GetID("##Legacy search"), GImGui->ActiveId);
    });
}

TEST_F(LocalizationGuiTests, l2b2PresentationFallsBackToOriginalEnglishWithoutExternalCatalogs)
{
    LocalizationService::get().load("resources/locales/nonexistent-test-directory");
    select("ru");
    for (auto const& groupName : {"General", "Location"}) {
        auto const& group = parameterGroup(SimulationParameters::getSpec(), groupName);
        EXPECT_EQ(groupName, SimulationParametersPresentation::label(group));
        for (auto const& parameter : group._parameters) {
            EXPECT_EQ(parameter._name, SimulationParametersPresentation::label(parameter));
            EXPECT_EQ(parameter._description, SimulationParametersPresentation::description(parameter));
        }
    }
    LocalizationService::get().load("resources/locales");
}

TEST_F(LocalizationGuiTests, l2b2ProductionGeneralPreservesAllLocationValues)
{
    auto values = std::make_unique<SimulationParameters>();
    values->numLayers = values->numSources = 1;
    values->layerOrderNumbers[0] = 1;
    values->sourceOrderNumbers[0] = 2;
    values->layerOpacity.layerValues[0] = 0.37f;
    values->sourceRelativeStrength.sourceValues[0].value = 0.42f;
    auto defaults = std::make_unique<SimulationParameters>(*values);
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        for (int order : {0, 1, 2}) {
            frame([&] {
                auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = "General"});
                parametersGroup(parameterGroup(filtered, "General")._parameters, *values, *defaults, order);
            });
            EXPECT_EQ(*defaults, *values);
        }
    }
}

TEST_F(LocalizationGuiTests, l2b3aAllTwentyBindingsRetainSpecsAndDescriptions)
{
    auto const& spec = SimulationParameters::getSpec();
    int colors = 0, booleans = 0, numbers = 0, alternatives = 0;
    for (auto const& groupName : {"Visualization", "Guided energy supply"}) {
        auto const& group = parameterGroup(spec, groupName);
        EXPECT_EQ(std::string(groupName) == "Visualization" ? 13 : 7, group._parameters.size());
        for (auto const& parameter : group._parameters) {
            colors += std::holds_alternative<ColorSpec>(parameter._reference);
            booleans += std::holds_alternative<BoolSpec>(parameter._reference);
            numbers += std::holds_alternative<FloatSpec>(parameter._reference);
            alternatives += std::holds_alternative<AlternativeSpec>(parameter._reference);
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                auto name = SimulationParametersPresentation::label(parameter);
                auto description = SimulationParametersPresentation::description(parameter);
                ASSERT_TRUE(description);
                EXPECT_FALSE(description->empty());
                if (std::string(language) == "ru") {
                    EXPECT_NE(parameter._name, name);
                    EXPECT_NE(parameter._description, description);
                } else {
                    EXPECT_EQ(parameter._name, name);
                    EXPECT_EQ(parameter._description, description);
                }
                auto renamed = parameter;
                renamed._name = "Unrelated canonical caption";
                EXPECT_EQ(name, SimulationParametersPresentation::label(renamed));
                EXPECT_EQ(description, SimulationParametersPresentation::description(renamed));
            }
        }
    }
    EXPECT_EQ(3, colors);
    EXPECT_EQ(6, booleans);
    EXPECT_EQ(10, numbers);
    EXPECT_EQ(1, alternatives);
    select("ru");
    for (auto const& name : {"Cell type: Reconnector", "Cell type: Detonator"}) {
        auto const& group = parameterGroup(spec, name);
        EXPECT_EQ(name, SimulationParametersPresentation::label(group));
        for (auto const& parameter : group._parameters) {
            EXPECT_EQ(parameter._name, SimulationParametersPresentation::label(parameter));
            EXPECT_EQ(parameter._description, SimulationParametersPresentation::description(parameter));
        }
    }
}

TEST_F(LocalizationGuiTests, l2b3aAlternativeLabelsAreBoundToMemberAndOriginalIndex)
{
    auto const& parameter = parameterGroup(SimulationParameters::getSpec(), "Visualization")._parameters.at(2);
    auto const& original = std::get<AlternativeSpec>(parameter._reference);
    std::vector<std::string> english = {"Energy", "Customization", "Lineage + Customization", "Lineage", "Creature"};
    std::vector<std::string> russian = {"Энергия", "Настройка", "Происхождение + Настройка", "Происхождение", "Организм"};
    ASSERT_EQ(5, original._alternatives.size());
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        EXPECT_EQ(std::string(language) == "ru" ? russian : english, SimulationParametersPresentation::alternativeLabels(parameter));
        auto unrelated = ParameterSpec().name("Object coloring").reference(AlternativeSpec().alternatives(original._alternatives));
        EXPECT_EQ(english, SimulationParametersPresentation::alternativeLabels(unrelated));
        for (size_t i = 0; i < english.size(); ++i) {
            EXPECT_EQ(english.at(i), original._alternatives.at(i).first);
            EXPECT_TRUE(original._alternatives.at(i).second.empty());
        }
        auto tooltip = *SimulationParametersPresentation::description(parameter);
        EXPECT_EQ(5, std::ranges::count(tooltip, '\n') / 2 - 1);
        size_t pos = 0;
        for (int i = 0; i < 5; ++i) {
            pos = tooltip.find(ICON_FA_CHEVRON_RIGHT, pos);
            ASSERT_NE(std::string::npos, pos);
            pos += std::string(ICON_FA_CHEVRON_RIGHT).size();
        }
        EXPECT_EQ(std::string::npos, tooltip.find(ICON_FA_CHEVRON_RIGHT, pos));
    }
}

TEST_F(LocalizationGuiTests, l2b3aProductionControlsPreserveEveryValueAndModeAcrossLanguages)
{
    auto values = std::make_unique<SimulationParameters>();
    values->numLayers = values->numSources = 1;
    values->layerOrderNumbers[0] = 1;
    values->sourceOrderNumbers[0] = 2;
    values->glow.value = 0.47f;
    values->fluidIllumination.value = 0.63f;
    values->borderlessRendering.value = true;
    values->gridLines.value = true;
    values->markReferenceDomain.value = false;
    values->backgroundColor.baseValue = {0.23f, 0.42f, 0.73f};
    values->backgroundColor.layerValues[0] = {.value = {0.81f, 0.12f, 0.17f}, .enabled = false};
    values->customizationColors.value[3] = {0.17f, 0.38f, 0.91f};
    values->sourceLightColor.sourceValues[0] = {0.13f, 0.69f, 0.45f};
    values->sourceLightBrightness.sourceValues[0] = 0.36f;
    values->sourceLightRange.sourceValues[0] = 217;
    values->sourceShowRadiationCenter.sourceValues[0] = false;
    values->layerShowForceField.layerValues[0] = false;
    values->externalEnergy.value = 123456;
    values->externalEnergyBackflowLimit.value = Infinity<float>::value;
    for (int color = 0; color < MAX_COLORS; ++color) {
        values->externalEnergyInflowForConstructor.value[color] = color + 0.5f;
        values->externalEnergyInflowThresholdFactor.value[color] = color * 0.01f;
        values->externalEnergyInflowOnlyForFirstOffspring.value[color] = color % 2;
        values->externalEnergyInflowForSources.value[color] = color * 77.0f;
        values->externalEnergyBackflowFactor.value[color] = color * 0.03f;
    }
    auto defaults = std::make_unique<SimulationParameters>();
    defaults->numLayers = defaults->numSources = 1;
    defaults->layerOrderNumbers[0] = 1;
    defaults->sourceOrderNumbers[0] = 2;
    for (int mode = 0; mode < 5; ++mode) {
        values->objectColoring.value = ColorVector<CellColoring>::uniform(static_cast<CellColoring>(mode));
        auto snapshot = std::make_unique<SimulationParameters>(*values);
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            for (int order : {0, 1, 2}) {
                for (auto const& groupName : {"Visualization", "Guided energy supply"}) {
                    frame([&] { parametersGroup(parameterGroup(SimulationParameters::getSpec(), groupName)._parameters, *values, *defaults, order); });
                    EXPECT_EQ(*snapshot, *values) << groupName << ": " << language << ": " << mode << ": " << order;
                }
            }
        }
    }
}

TEST_F(LocalizationGuiTests, l2b3aBilingualSearchRetainsFilterAndOriginalOrder)
{
    auto const& spec = SimulationParameters::getSpec();
    for (auto const& [query, groupName, count] : std::vector<std::tuple<std::string, std::string, int>>{
             {"виЗУАЛизация", "Visualization", 13},
             {"VISUALIZATION", "Visualization", 13},
             {"управляемая", "Guided energy supply", 7},
             {"GUIDED ENERGY", "Guided energy supply", 7},
             {"сВеЧеНие", "Visualization", 1},
             {"GLOW", "Visualization", 1},
             {"ЦВЕТ", "Visualization", 3},
             {"ПЕРЕДАЧА ЭНЕРГИИ", "Guided energy supply", 2},
             {"BACKFLOW", "Guided energy supply", 2}}) {
        ParametersFilter filter{.containedText = query};
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            auto filtered = SimulationParametersPresentation::filter(spec, filter);
            auto const& group = parameterGroup(filtered, groupName);
            bool russianQuery = static_cast<unsigned char>(query.at(0)) >= 128;
            EXPECT_EQ(russianQuery && std::string(language) != "ru" ? 0 : count, visibleParameters(group)) << query;
            EXPECT_EQ(query, filter.containedText);
            auto const& original = parameterGroup(spec, groupName);
            ASSERT_EQ(original._parameters.size(), group._parameters.size());
            for (size_t index = 0; index < group._parameters.size(); ++index) {
                EXPECT_EQ(original._parameters.at(index)._name, group._parameters.at(index)._name);
                EXPECT_EQ(original._parameters.at(index)._reference.index(), group._parameters.at(index)._reference.index());
                EXPECT_EQ(original._parameters.at(index)._description, group._parameters.at(index)._description);
            }
        }
        filter.containedText = "";
        auto full = SimulationParametersPresentation::filter(spec, filter);
        EXPECT_EQ(13, visibleParameters(parameterGroup(full, "Visualization")));
        EXPECT_EQ(7, visibleParameters(parameterGroup(full, "Guided energy supply")));
    }
    select("ru");
    auto result = Utf8Search::find("Передача энергии конструкторам", "ЭНЕРГИИ");
    ASSERT_TRUE(result);
    EXPECT_EQ("энергии", std::string("Передача энергии конструкторам").substr(result->begin, result->end - result->begin));
    auto none = SimulationParametersPresentation::filter(spec, {.containedText = "absent parameter"});
    for (auto const& group : none._groups) {
        EXPECT_EQ(0, visibleParameters(group));
    }
}

TEST_F(LocalizationGuiTests, l2b3aMissingCatalogFallsBackForBothGroupsAndModes)
{
    LocalizationService::get().load("resources/locales/nonexistent-l2b3a");
    select("ru");
    for (auto const& groupName : {"Visualization", "Guided energy supply"}) {
        auto const& group = parameterGroup(SimulationParameters::getSpec(), groupName);
        EXPECT_EQ(groupName, SimulationParametersPresentation::label(group));
        for (auto const& parameter : group._parameters) {
            EXPECT_EQ(parameter._name, SimulationParametersPresentation::label(parameter));
            EXPECT_EQ(parameter._description, SimulationParametersPresentation::description(parameter));
            if (auto alternative = std::get_if<AlternativeSpec>(&parameter._reference)) {
                auto labels = SimulationParametersPresentation::alternativeLabels(parameter);
                for (size_t index = 0; index < labels.size(); ++index) {
                    EXPECT_EQ(alternative->_alternatives.at(index).first, labels.at(index));
                }
            }
        }
    }
    LocalizationService::get().load("resources/locales");
}

TEST_F(LocalizationGuiTests, l2b3aSwitcherKeepsCanonicalIdsCyclesAllModesAndReverts)
{
    auto const& spec = parameterGroup(SimulationParameters::getSpec(), "Visualization")._parameters.at(2);
    std::vector<std::string> canonical = {"Energy", "Customization", "Lineage + Customization", "Lineage", "Creature"};
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    int mode = 1;
    ImVec2 next, revert;
    ImGuiID inputId = 0;
    auto draw = [&] {
        frame([&] {
            auto start = ImGui::GetCursorScreenPos();
            AlienGui::Switcher(
                AlienGui::SwitcherParameters()
                    .name(spec._name)
                    .label(SimulationParametersPresentation::label(spec))
                    .textWidth(360)
                    .values(canonical)
                    .displayValues(SimulationParametersPresentation::alternativeLabels(spec))
                    .defaultValue(1),
                &mode);
            auto labelStart = GImGui->LastItemData.Rect.Min;
            auto spacing = ImGui::GetStyle().ItemSpacing.x;
            auto revertWidth = ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2;
            revert = {labelStart.x - spacing - revertWidth / 2, labelStart.y + ImGui::GetFrameHeight() / 2};
            next = {labelStart.x - 2 * spacing - revertWidth - scale(11), start.y + ImGui::GetFrameHeight() / 2};
            ImGui::PushID(spec._name.c_str());
            ImGui::PushID(0);
            auto current = ImGui::GetID("##switcher");
            if (!inputId) {
                inputId = current;
            }
            EXPECT_EQ(inputId, current);
            ImGui::PopID();
            ImGui::PopID();
        });
    };
    auto click = [&](ImVec2 target) {
        io.AddMousePosEvent(target.x, target.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        draw();
    };
    draw();
    draw();
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        draw();
        for (int i = 0; i < 5; ++i) {
            auto expected = (mode + 1) % 5;
            click(next);
            EXPECT_EQ(expected, mode);
        }
        mode = 4;
        draw();
        click(revert);
        EXPECT_EQ(1, mode);
    }
    // An incomplete display vector must fall back without changing mode or canonical values.
    frame([&] { AlienGui::Switcher(AlienGui::SwitcherParameters().name("Legacy").values(canonical).displayValues({"Incomplete"}), &mode); });
    EXPECT_EQ(1, mode);
}

TEST_F(LocalizationGuiTests, l2b3aColorWidgetsPreservePopupIdsDefaultsAndPalette)
{
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    auto color = FloatColorRGB{0.2f, 0.4f, 0.6f};
    auto defaultColor = FloatColorRGB{0.1f, 0.3f, 0.5f};
    bool enabled = true;
    ImVec2 open, revert;
    ImGuiID originalPopup = 0;
    auto draw = [&] {
        frame([&] {
            auto start = ImGui::GetCursorScreenPos();
            open = {start.x + 100, start.y + ImGui::GetFrameHeight() / 2};
            AlienGui::ColorButton(
                AlienGui::ColorButtonParameters()
                    .name("Background color")
                    .label(tr("parameters.visualization.backgroundcolor.name"))
                    .textWidth(360)
                    .defaultValue(defaultColor)
                    .defaultEnabledValue(false),
                color,
                &enabled);
            auto labelStart = GImGui->LastItemData.Rect.Min;
            auto revertWidth = ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2;
            revert = {labelStart.x - ImGui::GetStyle().ItemSpacing.x - revertWidth / 2, start.y + ImGui::GetFrameHeight() / 2};
            auto id = ImGui::GetID("colorpicker");
            if (!originalPopup) {
                originalPopup = id;
            }
            EXPECT_EQ(originalPopup, id);
        });
    };
    auto click = [&](ImVec2 target) {
        io.AddMousePosEvent(target.x, target.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        draw();
    };
    draw();
    draw();
    click(open);
    ASSERT_FALSE(GImGui->OpenPopupStack.empty());
    auto popup = GImGui->OpenPopupStack.back().Window;
    ASSERT_NE(nullptr, popup);
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        draw();
        ASSERT_FALSE(GImGui->OpenPopupStack.empty());
        EXPECT_EQ(originalPopup, GImGui->OpenPopupStack.back().PopupId);
        EXPECT_EQ(popup, GImGui->OpenPopupStack.back().Window);
        EXPECT_EQ((FloatColorRGB{0.2f, 0.4f, 0.6f}), color);
    }
    // The palette is in the right-hand group; its last row is aligned to the final group line.
    auto paletteTop = popup->DC.CursorPosPrevLine.y + 3 * (ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y)
        + 2 * (40 + ImGui::GetStyle().ItemSpacing.y) + 1 + ImGui::GetStyle().ItemSpacing.y;
    auto palette = ImVec2(popup->DC.CursorMaxPos.x - 10, paletteTop + 3 * (20 + ImGui::GetStyle().ItemSpacing.y) + 10);
    click(palette);
    EXPECT_NE((FloatColorRGB{0.2f, 0.4f, 0.6f}), color);
    auto selectedColor = color;
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        draw();
        EXPECT_EQ(selectedColor, color);
    }
    auto previous = ImVec2(
        popup->DC.CursorMaxPos.x - 188 + 30,
        paletteTop - (ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y) - (1 + ImGui::GetStyle().ItemSpacing.y)
            - (40 + ImGui::GetStyle().ItemSpacing.y) + 20);
    click(previous);
    EXPECT_EQ((FloatColorRGB{0.2f, 0.4f, 0.6f}), color);
    // Close only the disposable picker before testing the reset button.
    frame([&] { ImGui::ClosePopupToLevel(0, true); });
    draw();
    click(revert);
    EXPECT_EQ(defaultColor, color);
    EXPECT_FALSE(enabled);

    auto colors = getDefaultCustomizationColorVector();
    auto defaults = colors;
    colors[0] = {0.12f, 0.23f, 0.34f};
    ImRect reset;
    ImGuiID resetId = 0;
    auto vectorDraw = [&] {
        frame([&] {
            AlienGui::ColorVectorButtons(
                AlienGui::ColorVectorButtonsParameters()
                    .name("Customization colors")
                    .label(tr("parameters.visualization.customizationcolors.name"))
                    .textWidth(360)
                    .defaultValue(defaults)
                    .builtinDefaultValue(defaults),
                colors.values);
            reset = GImGui->LastItemData.Rect;
            auto current = GImGui->LastItemData.ID;
            ImGui::PushID("Customization colors");
            EXPECT_EQ(ImGui::GetID("Default##Customization colors"), current);
            ImGui::PopID();
            if (!resetId) {
                resetId = current;
            }
            EXPECT_EQ(resetId, current);
        });
    };
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        vectorDraw();
    }
    auto center = reset.GetCenter();
    io.AddMousePosEvent(center.x, center.y);
    vectorDraw();
    io.AddMouseButtonEvent(0, true);
    vectorDraw();
    io.AddMouseButtonEvent(0, false);
    vectorDraw();
    EXPECT_EQ(defaults, colors);
}

TEST_F(LocalizationGuiTests, l2b3aRussianLabelsAndModesFitAtRequestedScales)
{
    select("ru");
    auto font = StyleService::get().getDefaultFont();
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        for (auto const& groupName : {"Visualization", "Guided energy supply"}) {
            auto const& group = parameterGroup(SimulationParameters::getSpec(), groupName);
            for (auto const& parameter : group._parameters) {
                auto label = SimulationParametersPresentation::label(parameter);
                auto width = font->CalcTextSizeA(font->FontSize * factor, FLT_MAX, 0, label.c_str()).x;
                auto extra = parameter._name == "Customization colors" ? 150.0f : 65.0f;
                EXPECT_LE(width + extra * factor, 360.0f * factor) << label << ": " << factor;
                if (std::holds_alternative<AlternativeSpec>(parameter._reference)) {
                    for (auto const& option : SimulationParametersPresentation::alternativeLabels(parameter)) {
                        auto optionWidth = font->CalcTextSizeA(font->FontSize * factor, FLT_MAX, 0, option.c_str()).x;
                        EXPECT_LE(optionWidth, (650.0f - 360.0f - 100.0f) * factor) << option << ": " << factor;
                    }
                }
            }
        }
    }
}

TEST_F(LocalizationGuiTests, l2b3aExternalEnergyRetainsActiveNumericInputInfinityAndReset)
{
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    float value = 12345;
    float original = 17;
    ImVec2 slider, revert;
    ImGuiID sliderId = 0;
    bool capture = false;
    std::string logged;
    auto draw = [&] {
        frame([&] {
            auto start = ImGui::GetCursorScreenPos();
            slider = {start.x + 120, start.y + ImGui::GetFrameHeight() / 2};
            if (capture) {
                ImGui::LogToBuffer();
            }
            AlienGui::SliderFloat(
                AlienGui::SliderFloatParameters()
                    .name("External energy amount")
                    .label(tr("parameters.energy.externalenergy.name"))
                    .textWidth(360)
                    .min(0)
                    .max(100000000)
                    .logarithmic(true)
                    .infinity(true)
                    .format("%.0f")
                    .defaultValue(&original),
                &value);
            if (capture) {
                logged = GImGui->LogBuffer.c_str();
                ImGui::LogFinish();
            }
            auto labelStart = GImGui->LastItemData.Rect.Min;
            auto width = ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2;
            revert = {labelStart.x - ImGui::GetStyle().ItemSpacing.x - width / 2, start.y + ImGui::GetFrameHeight() / 2};
            ImGui::PushID("External energy amount");
            ImGui::PushID("");
            ImGui::PushID(0);
            auto current = ImGui::GetID("##slider");
            if (!sliderId) {
                sliderId = current;
            }
            EXPECT_EQ(sliderId, current);
            ImGui::PopID();
            ImGui::PopID();
            ImGui::PopID();
        });
    };
    draw();
    draw();
    io.AddKeyEvent(ImGuiMod_Ctrl, true);
    io.AddMousePosEvent(slider.x, slider.y);
    draw();
    io.AddMouseButtonEvent(0, true);
    draw();
    io.AddMouseButtonEvent(0, false);
    io.AddKeyEvent(ImGuiMod_Ctrl, false);
    draw();
    ASSERT_EQ(sliderId, GImGui->ActiveId);
    io.AddInputCharactersUTF8("43210");
    draw();
    EXPECT_FLOAT_EQ(43210, value);
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        draw();
        EXPECT_EQ(sliderId, GImGui->ActiveId);
        EXPECT_EQ(sliderId, GImGui->InputTextState.ID);
        EXPECT_FLOAT_EQ(43210, value);
    }
    io.AddKeyEvent(ImGuiKey_Enter, true);
    draw();
    io.AddKeyEvent(ImGuiKey_Enter, false);
    draw();
    select("ru");
    value = Infinity<float>::value;
    capture = true;
    draw();
    capture = false;
    EXPECT_FLOAT_EQ(Infinity<float>::value, value);
    EXPECT_NE(std::string::npos, logged.find("Бесконечность"));
    draw();
    io.AddMousePosEvent(revert.x, revert.y);
    draw();
    io.AddMouseButtonEvent(0, true);
    draw();
    io.AddMouseButtonEvent(0, false);
    draw();
    EXPECT_FLOAT_EQ(original, value);
}

TEST_F(LocalizationGuiTests, switcherExpandedRowsUseTheAvailableWidthAtEveryScale)
{
    auto const& spec = parameterGroup(SimulationParameters::getSpec(), "Visualization")._parameters.at(2);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    auto values = ColorVector<int>::uniform(2);
    auto defaults = ColorVector<int>::uniform(1);
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        select("ru");
        ImVec2 start;
        ImRect right;
        float available = 0;
        float hostWidth = 650 * factor;
        auto draw = [&] {
            switcherFrame(hostWidth, [&] {
                ImGui::PushID(static_cast<int>(factor * 10));
                start = ImGui::GetCursorScreenPos();
                available = ImGui::GetContentRegionAvail().x;
                AlienGui::Switcher(
                    AlienGui::SwitcherParameters()
                        .name(spec._name)
                        .label(SimulationParametersPresentation::label(spec))
                        .textWidth(360)
                        .colorDependence(true)
                        .defaultValues(defaults.values)
                        .values({"Energy", "Customization", "Lineage + Customization", "Lineage", "Creature"})
                        .displayValues(SimulationParametersPresentation::alternativeLabels(spec))
                        .tooltip("Help"),
                    values.values);
                right = GImGui->LastItemData.Rect;
                ImGui::PopID();
            });
        };
        draw();
        draw();
        io.AddMousePosEvent(start.x + scale(10), start.y + ImGui::GetFrameHeight() / 2);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        draw();
        for (float width : {220.0f, 400.0f, 650.0f}) {
            hostWidth = width * factor;
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                draw();
                draw();
                auto const& style = ImGui::GetStyle();
                auto toggleWidth = ImGui::CalcTextSize(ICON_FA_PLUS_SQUARE).x + style.FramePadding.x * 2;
                auto inputX = std::floor(start.x + toggleWidth + style.ItemSpacing.x) + scale(30) + style.ItemSpacing.x;
                auto actualInputWidth = right.Max.x - inputX - scale(44) - style.ItemSpacing.x * 2 + style.FramePadding.x * 2;
                auto expectedInputWidth = available - (inputX - start.x) - scale(44) - style.FramePadding.x * 2;
                EXPECT_NEAR(expectedInputWidth, actualInputWidth, 1.1f) << factor << ": " << width;
                EXPECT_GT(actualInputWidth, 1);
                EXPECT_LE(right.Max.x, start.x + available + 1);
                EXPECT_GT(right.Min.y, start.y + 8 * ImGui::GetFrameHeight());
                EXPECT_EQ(ColorVector<int>::uniform(2), values);
                if (width >= 400) {
                    EXPECT_GE(
                        actualInputWidth - style.FramePadding.x * 2,
                        ImGui::CalcTextSize(SimulationParametersPresentation::alternativeLabels(spec).at(2).c_str()).x);
                }
            }
        }
    }
}

TEST_F(LocalizationGuiTests, switcherShowsCompleteClippedValuesAndMixedListsOnHover)
{
    auto const& spec = parameterGroup(SimulationParameters::getSpec(), "Visualization")._parameters.at(2);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    auto values = ColorVector<int>::uniform(2);
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (bool mixed : {false, true}) {
            for (int color = 0; color < MAX_COLORS; ++color) {
                values[color] = mixed ? color % 5 : 2;
            }
            auto original = values;
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                auto labels = SimulationParametersPresentation::alternativeLabels(spec);
                auto fullText = labels.at(2);
                if (mixed) {
                    fullText.clear();
                    for (auto const& label : labels) {
                        fullText += (fullText.empty() ? "" : ", ") + label;
                    }
                }
                for (float width : {200.0f, 420.0f, 650.0f}) {
                    std::string logged;
                    ImVec2 start;
                    ImRect labelRect;
                    float available = 0;
                    bool tooltip = false;
                    auto draw = [&] {
                        switcherFrame(width * factor, [&] {
                            ImGui::PushID("Hover check");
                            start = ImGui::GetCursorScreenPos();
                            available = ImGui::GetContentRegionAvail().x;
                            ImGui::LogToBuffer();
                            AlienGui::Switcher(
                                AlienGui::SwitcherParameters()
                                    .name(spec._name)
                                    .label(SimulationParametersPresentation::label(spec))
                                    .textWidth(360)
                                    .readOnly(width == 200)
                                    .colorDependence(mixed)
                                    .values({"Energy", "Customization", "Lineage + Customization", "Lineage", "Creature"})
                                    .displayValues(labels),
                                values.values);
                            labelRect = GImGui->LastItemData.Rect;
                            logged = GImGui->LogBuffer.c_str();
                            ImGui::LogFinish();
                            tooltip = false;
                            for (auto window : GImGui->Windows) {
                                tooltip |= (window->Flags & ImGuiWindowFlags_Tooltip) && window->Active && !window->Hidden;
                            }
                            ImGui::PopID();
                        });
                    };
                    io.AddMousePosEvent(-100, -100);
                    draw();
                    draw();
                    auto const& style = ImGui::GetStyle();
                    auto prefix = mixed ? ImGui::CalcTextSize(ICON_FA_PLUS_SQUARE).x + style.FramePadding.x * 2 + style.ItemSpacing.x : 0.0f;
                    auto inputX = std::floor(start.x + prefix);
                    auto labelWidth = ImGui::CalcTextSize(SimulationParametersPresentation::label(spec).c_str()).x + style.ItemSpacing.x;
                    float widest = 0;
                    for (auto const& label : labels) {
                        widest = std::max(widest, ImGui::CalcTextSize(label.c_str()).x);
                    }
                    bool ownLine = available - prefix < labelWidth + scale(44) + style.FramePadding.x * 4 + widest;
                    EXPECT_EQ(ownLine, labelRect.Min.y > start.y + ImGui::GetFrameHeight());
                    auto expectedWidth = available - prefix - scale(44) - style.FramePadding.x * 2 - (ownLine ? 0 : labelWidth);
                    if (!ownLine) {
                        auto actualWidth = labelRect.Min.x - inputX - scale(44) - 3 * style.ItemSpacing.x + 2 * style.FramePadding.x;
                        EXPECT_NEAR(expectedWidth, actualWidth, 1.1f);
                    }
                    io.AddMousePosEvent(inputX + std::min(20.0f, expectedWidth / 2), start.y + ImGui::GetFrameHeight() / 2);
                    draw();
                    draw();
                    auto clipped = ImGui::CalcTextSize(fullText.c_str()).x > expectedWidth - style.FramePadding.x * 2;
                    EXPECT_EQ(clipped, tooltip) << factor << ": " << width << ": " << mixed << ": " << language;
                    auto first = logged.find(fullText);
                    ASSERT_NE(std::string::npos, first);
                    if (clipped) {
                        EXPECT_NE(std::string::npos, logged.find(fullText, first + fullText.size()));
                    }
                    EXPECT_EQ(original, values);
                }
            }
        }
    }
}

TEST_F(LocalizationGuiTests, legacySwitchersRetainWidthsIndicesAndIds)
{
    struct Example
    {
        std::string name;
        float width;
        float textWidth;
        std::vector<std::string> values;
    };
    std::vector<Example> examples = {
        {"Sort by", 340, 55, {"Most reactions", "Newest", "Most downloads"}},
        {"Material", 0, 180, {"Solid", "Fluid", "Free cells", "Energy particles"}},
        {"Mode", 260, 45, {"Entire history", "Last steps"}},
        {"Mode", 0, 200, {"Limited save files", "Unlimited save files"}},
        {"Number of branches", 0, 180, {"1", "2", "3", "4", "5", "6"}},
        {"Edit signal", 0, 200, {"0", "1", "2"}},
        {"Display mode", 0, 120, {"Node index", "Cell type"}},
        {"Mode", 0, 180, {"Number of energy particles", "Number of cells"}}};
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto const& example : examples) {
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                int value = 1;
                ImVec2 input;
                ImGuiID inputId = 0;
                auto draw = [&] {
                    switcherFrame(800 * factor, [&] {
                        auto start = ImGui::GetCursorScreenPos();
                        input = {start.x + scale(10), start.y + ImGui::GetFrameHeight() / 2};
                        auto available = ImGui::GetContentRegionAvail().x;
                        AlienGui::Switcher(
                            AlienGui::SwitcherParameters().name(example.name).width(example.width).textWidth(example.textWidth).values(example.values), &value);
                        auto const& style = ImGui::GetStyle();
                        auto expected = (example.width ? scale(example.width) : available) - scale(example.textWidth + 44) - style.FramePadding.x * 2;
                        auto actual = GImGui->LastItemData.Rect.Min.x - start.x - scale(44) - 3 * style.ItemSpacing.x + 2 * style.FramePadding.x;
                        EXPECT_NEAR(expected, actual, 1.1f) << example.name;
                        ImGui::PushID(example.name.c_str());
                        ImGui::PushID(0);
                        inputId = ImGui::GetID("##switcher");
                        ImGui::PopID();
                        ImGui::PopID();
                    });
                };
                draw();
                draw();
                ImGui::GetIO().AddMousePosEvent(input.x, input.y);
                draw();
                EXPECT_EQ(inputId, GImGui->HoveredId) << example.name;
                EXPECT_EQ(1, value);
            }
        }
    }
}

TEST_F(LocalizationGuiTests, unicodeSimulationPathsReachTheOverlayAsUtf8)
{
    std::vector<std::u8string> names = {u8"start.sim", u8"Старт.sim", u8"Start - мир.sim", u8"Тестовый мир №1.sim"};
    for (auto const& directory : {std::filesystem::path("test"), std::filesystem::path(u8"Тестовый каталог")}) {
        for (auto const& name : names) {
            auto path = directory / std::filesystem::path(name);
            auto request = ReadSimulationRequestData{.filename = path, .initSimulation = false};
            EXPECT_EQ(path.native(), request.filename.native());
            auto result = std::make_unique<ReadSimulationResultData>();
            result->filename = request.filename.filename();
            auto expected = std::string(reinterpret_cast<char const*>(name.data()), name.size());
            auto message = PathHelper::toUtf8(result->filename);
            EXPECT_EQ(expected, message);
            EXPECT_EQ(std::string::npos, message.find('?'));
            EXPECT_EQ(path, std::filesystem::path(path.u8string()));
            EXPECT_EQ(path, PathHelper::fromUtf8(PathHelper::toUtf8(path)));
            auto remembered = PathHelper::toUtf8(path.parent_path());
            GlobalSettings::get().setValue("tests.paths.reference", remembered);
            EXPECT_EQ(path.parent_path(), PathHelper::fromUtf8(GlobalSettings::get().getValue("tests.paths.reference", std::string())));
            if (auto legacy = PathHelper::toLegacyString(path.parent_path())) {
                EXPECT_EQ(path.parent_path(), std::filesystem::path(*legacy));
            }
            printOverlayMessage(message);
            EXPECT_EQ(expected, overlayMessage());
            std::vector<unsigned int> decoded;
            auto font = StyleService::get().getMonospaceLargeFont();
            for (auto cursor = message.c_str(); *cursor;) {
                unsigned int codepoint;
                auto count = ImTextCharFromUtf8(&codepoint, cursor, nullptr);
                ASSERT_GT(count, 0);
                EXPECT_NE(nullptr, font->FindGlyphNoFallback(static_cast<ImWchar>(codepoint)));
                decoded.push_back(codepoint);
                cursor += count;
            }
            if (name == u8"Start - мир.sim") {
                EXPECT_EQ(0x043c, decoded.at(8));
                EXPECT_EQ(0x0438, decoded.at(9));
                EXPECT_EQ(0x0440, decoded.at(10));
            }
            frame([&] {
                auto draw = ImGui::GetForegroundDrawList();
                auto before = draw->VtxBuffer.Size;
                renderOverlay();
                EXPECT_GT(draw->VtxBuffer.Size, before);
            });
        }
    }
    auto unsupported = PathHelper::toLegacyString(std::filesystem::path(u8"Каталог 漢"));
    if (unsupported) {
        EXPECT_EQ(std::filesystem::path(u8"Каталог 漢"), std::filesystem::path(*unsupported));
    }
}

namespace
{
    std::vector<ParameterSpec const*> geometryParameters()
    {
        std::vector<ParameterSpec const*> result;
        std::function<void(ParameterSpec const&)> visit = [&](auto const& parameter) {
            result.emplace_back(&parameter);
            if (auto alternative = std::get_if<AlternativeSpec>(&parameter._reference)) {
                for (auto const& [name, children] : alternative->_alternatives) {
                    for (auto const& child : children) {
                        visit(child);
                    }
                }
            }
        };
        for (auto const& name : {"Shape", "Force field"}) {
            for (auto const& parameter : parameterGroup(SimulationParameters::getSpec(), name)._parameters) {
                visit(parameter);
            }
        }
        return result;
    }

    class GeometryWorldScope
    {
        class World : public _SimulationFacadeImpl
        {
            IntVector2D getWorldSize() const override { return {1024, 768}; }
        };

    public:
        GeometryWorldScope()
            : _original(_SimulationFacade::get())
        {
            _SimulationFacadeImpl::set(std::make_shared<World>());
        }
        ~GeometryWorldScope() { _SimulationFacadeImpl::set(_original); }

    private:
        SimulationFacade _original;
    };
}

TEST_F(LocalizationGuiTests, l2b3bAllSeventeenBindingsDescriptionsOptionsAndFallback)
{
    auto all = geometryParameters();
    ASSERT_EQ(17, all.size());
    std::set<std::string> shapeDescriptions, strengthDescriptions;
    std::vector<std::vector<std::string>> russianOptions = {
        {"Круглая", "Прямоугольная"},
        {"Круглая", "Прямоугольная"},
        {"Нет", "Радиальное", "Центральное", "Линейное", "Шум Перлина"},
        {"По часовой стрелке", "Против часовой стрелки"}};
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        size_t alternativeIndex = 0;
        for (auto parameter : all) {
            auto name = SimulationParametersPresentation::label(*parameter);
            auto description = SimulationParametersPresentation::description(*parameter);
            ASSERT_TRUE(description);
            EXPECT_FALSE(description->empty());
            if (std::string(language) == "ru") {
                EXPECT_NE(parameter->_name, name);
                EXPECT_NE(parameter->_description, description);
                if (parameter->_name == "Shape") {
                    shapeDescriptions.insert(*description);
                }
                if (parameter->_name == "Strength") {
                    EXPECT_EQ("Сила", name);
                    strengthDescriptions.insert(*description);
                }
            } else {
                EXPECT_EQ(parameter->_name, name);
                EXPECT_EQ(parameter->_description, description);
            }
            auto renamed = *parameter;
            renamed._name = "Unrelated caption";
            EXPECT_EQ(name, SimulationParametersPresentation::label(renamed));
            EXPECT_EQ(description, SimulationParametersPresentation::description(renamed));
            if (auto alternative = std::get_if<AlternativeSpec>(&parameter->_reference)) {
                auto labels = SimulationParametersPresentation::alternativeLabels(*parameter);
                ASSERT_EQ(alternative->_alternatives.size(), labels.size());
                EXPECT_EQ(labels, SimulationParametersPresentation::alternativeLabels(renamed));
                for (size_t i = 0; i < labels.size(); ++i) {
                    EXPECT_EQ(std::string(language) == "ru" ? russianOptions.at(alternativeIndex).at(i) : alternative->_alternatives.at(i).first, labels.at(i));
                }
                auto unbound = ParameterSpec().name(parameter->_name).reference(AlternativeSpec().alternatives(alternative->_alternatives));
                auto originalLabels = SimulationParametersPresentation::alternativeLabels(unbound);
                for (size_t i = 0; i < originalLabels.size(); ++i) {
                    EXPECT_EQ(alternative->_alternatives.at(i).first, originalLabels.at(i));
                }
                ++alternativeIndex;
            }
        }
        EXPECT_EQ(4, alternativeIndex);
    }
    EXPECT_EQ(2, shapeDescriptions.size());
    EXPECT_EQ(4, strengthDescriptions.size());
    LocalizationService::get().load("resources/locales/nonexistent-l2b3b");
    select("ru");
    for (auto parameter : all) {
        EXPECT_EQ(parameter->_name, SimulationParametersPresentation::label(*parameter));
        EXPECT_EQ(parameter->_description, SimulationParametersPresentation::description(*parameter));
        if (auto alternative = std::get_if<AlternativeSpec>(&parameter->_reference)) {
            auto labels = SimulationParametersPresentation::alternativeLabels(*parameter);
            for (size_t i = 0; i < labels.size(); ++i) {
                EXPECT_EQ(alternative->_alternatives.at(i).first, labels.at(i));
            }
        }
    }
    LocalizationService::get().load("resources/locales");
}

TEST_F(LocalizationGuiTests, l2b3bProductionModesKeepAllValuesAndInactiveSearchDoesNotSelectModes)
{
    GeometryWorldScope world;
    auto values = std::make_unique<SimulationParameters>();
    values->numLayers = values->numSources = 1;
    values->layerOrderNumbers[0] = 1;
    values->sourceOrderNumbers[0] = 2;
    values->layerForceFieldType.layerValues[0].enabled = true;
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& evaluation = SpecificationEvaluationService::get();
    for (auto parameter : geometryParameters()) {
        for (int order : {1, 2}) {
            if (!evaluation.isVisible(*parameter, order == 1 ? LocationType::Layer : LocationType::Source)) {
                continue;
            }
            if (auto number = std::get_if<FloatSpec>(&parameter->_reference)) {
                auto ref = evaluation.getRef(number->_member, *values, order);
                *ref.value = std::get<float>(number->_min) + (std::holds_alternative<float>(number->_max) ? std::get<float>(number->_max) : 768) * 0.13f;
            }
            if (auto pair = std::get_if<Float2Spec>(&parameter->_reference)) {
                auto ref = evaluation.getRef(pair->_member, *values, order);
                *ref.value = {73.5f, 112.25f};
            }
        }
    }
    for (int shape = 0; shape < 2; ++shape) {
        values->layerShape.layerValues[0] = static_cast<LayerShapeType>(shape);
        values->sourceShapeType.sourceValues[0] = static_cast<SourceShapeType>(shape);
        for (int mode = 0; mode < 5; ++mode) {
            values->layerForceFieldType.layerValues[0].value = static_cast<ForceField>(mode);
            for (int orientation = 0; orientation < 2; ++orientation) {
                values->layerRadialForceFieldOrientation.layerValues[0] = static_cast<Orientation>(orientation);
                auto snapshot = std::make_unique<SimulationParameters>(*values);
                for (auto const& language : {"en", "ru", "en"}) {
                    select(language);
                    for (auto const& query : {"", "Shape", "Форма", "Force field", "Силовое поле", "Strength", "Сила", "Temporal structure", "Временной"}) {
                        auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = query});
                        for (int order : {1, 2}) {
                            for (auto const& groupName : {"Shape", "Force field"}) {
                                frame([&] { parametersGroup(parameterGroup(filtered, groupName)._parameters, *values, *defaults, order); });
                                EXPECT_EQ(*snapshot, *values) << language << ": " << query << ": " << mode;
                            }
                        }
                    }
                }
            }
        }
    }
    select("ru");
    values->layerForceFieldType.layerValues[0].value = ForceField_Radial;
    auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = "Временной"});
    auto const& field = parameterGroup(filtered, "Force field")._parameters.at(0);
    EXPECT_TRUE(field._visible);
    auto const& alternatives = std::get<AlternativeSpec>(field._reference)._alternatives;
    EXPECT_FALSE(alternatives.at(1).second.at(1)._visible);
    EXPECT_TRUE(alternatives.at(4).second.at(2)._visible);
    std::string logged;
    frame([&] {
        ImGui::LogToBuffer();
        parametersGroup(parameterGroup(filtered, "Force field")._parameters, *values, *defaults, 1);
        logged = GImGui->LogBuffer.c_str();
        ImGui::LogFinish();
    });
    EXPECT_EQ(ForceField_Radial, values->layerForceFieldType.layerValues[0].value);
    EXPECT_EQ(std::string::npos, logged.find("Временной размер структур"));
    EXPECT_EQ(std::string::npos, logged.find("Пространственный размер структур"));
    for (auto const& query : {"Strength", "Сила"}) {
        auto result = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = query});
        auto const& modes = std::get<AlternativeSpec>(parameterGroup(result, "Force field")._parameters.at(0)._reference)._alternatives;
        int count = 0;
        for (auto const& [name, children] : modes) {
            for (auto const& child : children) {
                count += child._name == "Strength" && child._visible;
            }
        }
        EXPECT_EQ(4, count);
    }
    auto full = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = ""});
    EXPECT_EQ(3, visibleParameters(parameterGroup(full, "Shape")));
    EXPECT_EQ(1, visibleParameters(parameterGroup(full, "Force field")));
    auto match = Utf8Search::find("Временной размер структур", "РАЗМЕР");
    ASSERT_TRUE(match);
    EXPECT_EQ("размер", std::string("Временной размер структур").substr(match->begin, match->end - match->begin));
}

TEST_F(LocalizationGuiTests, l2b3bAllFourSwitchersCycleOriginalIndicesAndRevert)
{
    for (auto parameter : geometryParameters()) {
        auto alternative = std::get_if<AlternativeSpec>(&parameter->_reference);
        if (!alternative) {
            continue;
        }
        auto const& spec = *parameter;
        std::vector<std::string> canonical;
        for (auto const& [name, children] : alternative->_alternatives) {
            canonical.emplace_back(name);
        }
        auto numModes = static_cast<int>(canonical.size());
        auto& io = ImGui::GetIO();
        io.ConfigInputTrickleEventQueue = false;
        int mode = 1;
        ImVec2 next, revert;
        ImGuiID inputId = 0;
        auto draw = [&] {
            frame([&] {
                auto start = ImGui::GetCursorScreenPos();
                AlienGui::Switcher(
                    AlienGui::SwitcherParameters()
                        .name(spec._name)
                        .label(SimulationParametersPresentation::label(spec))
                        .textWidth(360)
                        .values(canonical)
                        .displayValues(SimulationParametersPresentation::alternativeLabels(spec))
                        .defaultValue(1),
                    &mode);
                auto labelStart = GImGui->LastItemData.Rect.Min;
                auto spacing = ImGui::GetStyle().ItemSpacing.x;
                auto revertWidth = ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2;
                revert = {labelStart.x - spacing - revertWidth / 2, labelStart.y + ImGui::GetFrameHeight() / 2};
                next = {labelStart.x - 2 * spacing - revertWidth - scale(11), start.y + ImGui::GetFrameHeight() / 2};
                ImGui::PushID(spec._name.c_str());
                ImGui::PushID(0);
                auto current = ImGui::GetID("##switcher");
                if (!inputId) {
                    inputId = current;
                }
                EXPECT_EQ(inputId, current);
                ImGui::PopID();
                ImGui::PopID();
            });
        };
        auto click = [&](ImVec2 target) {
            io.AddMousePosEvent(target.x, target.y);
            draw();
            io.AddMouseButtonEvent(0, true);
            draw();
            io.AddMouseButtonEvent(0, false);
            draw();
        };
        draw();
        draw();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            draw();
            for (int i = 0; i < numModes; ++i) {
                auto expected = (mode + 1) % numModes;
                click(next);
                EXPECT_EQ(expected, mode);
            }
            mode = numModes - 1;
            draw();
            click(revert);
            EXPECT_EQ(1, mode);
        }
        // An incomplete display vector must fall back without changing mode or canonical values.
        frame([&] { AlienGui::Switcher(AlienGui::SwitcherParameters().name("Legacy").values(canonical).displayValues({"Incomplete"}), &mode); });
        EXPECT_EQ(1, mode);
    }
}

TEST_F(LocalizationGuiTests, l2b3bRectanglesEditBothAxesKeepFocusAndResetUsingProductionWidgets)
{
    GeometryWorldScope world;
    auto values = std::make_unique<SimulationParameters>();
    values->numLayers = values->numSources = 1;
    values->layerOrderNumbers[0] = 1;
    values->sourceOrderNumbers[0] = 2;
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (auto parameter : geometryParameters()) {
        auto pair = std::get_if<Float2Spec>(&parameter->_reference);
        if (!pair) {
            continue;
        }
        auto order = parameter->_name.starts_with("Core") ? 1 : 2;
        auto ref = SpecificationEvaluationService::get().getRef(pair->_member, *values, order);
        auto original = SpecificationEvaluationService::get().getRef(pair->_member, *defaults, order);
        *ref.value = {71, 83};
        ImVec2 slider, revert;
        ImGuiID active = 0;
        for (int axis : {0, 1}) {
            auto draw = [&] {
                frame([&] {
                    auto start = ImGui::GetCursorScreenPos();
                    slider = {start.x + 100, start.y + ImGui::GetFrameHeight() / 2 + axis * ImGui::GetFrameHeightWithSpacing()};
                    auto width = ImGui::GetContentRegionAvail().x - scale(360);
                    revert = {
                        start.x + width + ImGui::GetStyle().ItemSpacing.x + (ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2) / 2,
                        start.y + ImGui::GetFrameHeight() / 2};
                    parametersGroup({*parameter}, *values, *defaults, order);
                    ImGui::PushID(0);
                    ImGui::PushID(parameter->_name.c_str());
                    auto id = ImGui::GetID(axis == 0 ? "##sliderX" : "##sliderY");
                    if (!active) {
                        active = id;
                    }
                    EXPECT_EQ(active, id);
                    ImGui::PopID();
                    ImGui::PopID();
                });
            };
            active = 0;
            draw();
            draw();
            io.AddKeyEvent(ImGuiMod_Ctrl, true);
            io.AddMousePosEvent(slider.x, slider.y);
            draw();
            io.AddMouseButtonEvent(0, true);
            draw();
            io.AddMouseButtonEvent(0, false);
            io.AddKeyEvent(ImGuiMod_Ctrl, false);
            draw();
            ASSERT_EQ(active, GImGui->ActiveId);
            io.AddInputCharactersUTF8(axis == 0 ? "137.25" : "219.5");
            draw();
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                draw();
                EXPECT_EQ(active, GImGui->ActiveId);
                EXPECT_FLOAT_EQ(axis == 0 ? 137.25f : 219.5f, axis == 0 ? ref.value->x : ref.value->y);
            }
            io.AddKeyEvent(ImGuiKey_Enter, true);
            draw();
            io.AddKeyEvent(ImGuiKey_Enter, false);
            draw();
            if (axis == 1) {
                io.AddMousePosEvent(revert.x, revert.y);
                draw();
                io.AddMouseButtonEvent(0, true);
                draw();
                io.AddMouseButtonEvent(0, false);
                draw();
                EXPECT_EQ(*original.value, *ref.value);
            }
        }
    }
}

TEST_F(LocalizationGuiTests, l2b3bAllElevenNumbersEditAndResetThroughProductionDispatch)
{
    GeometryWorldScope world;
    auto values = std::make_unique<SimulationParameters>();
    values->numLayers = values->numSources = 1;
    values->layerOrderNumbers[0] = 1;
    values->sourceOrderNumbers[0] = 2;
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    int count = 0;
    for (auto parameter : geometryParameters()) {
        auto number = std::get_if<FloatSpec>(&parameter->_reference);
        if (!number) {
            continue;
        }
        ++count;
        auto order = parameter->_name == "Radius" ? 2 : 1;
        auto ref = SpecificationEvaluationService::get().getRef(number->_member, *values, order);
        auto original = SpecificationEvaluationService::get().getRef(number->_member, *defaults, order);
        bool isSmallRange = std::holds_alternative<float>(number->_max) && std::get<float>(number->_max) < 1;
        float edited = isSmallRange ? 0.123f : 17.0f;
        ImVec2 slider, revert;
        ImGuiID active = 0;
        auto draw = [&] {
            frame([&] {
                auto start = ImGui::GetCursorScreenPos();
                slider = {start.x + 100, start.y + ImGui::GetFrameHeight() / 2};
                auto width = ImGui::GetContentRegionAvail().x - scale(360);
                revert = {
                    start.x + width + ImGui::GetStyle().ItemSpacing.x + (ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2) / 2,
                    start.y + ImGui::GetFrameHeight() / 2};
                parametersGroup({*parameter}, *values, *defaults, order);
                ImGui::PushID(0);
                ImGui::PushID(parameter->_name.c_str());
                ImGui::PushID("");
                ImGui::PushID(0);
                auto id = ImGui::GetID("##slider");
                if (!active) {
                    active = id;
                }
                EXPECT_EQ(active, id);
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
            });
        };
        draw();
        draw();
        io.AddKeyEvent(ImGuiMod_Ctrl, true);
        io.AddMousePosEvent(slider.x, slider.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        io.AddKeyEvent(ImGuiMod_Ctrl, false);
        draw();
        ASSERT_EQ(active, GImGui->ActiveId) << parameter->_name;
        io.AddInputCharactersUTF8(isSmallRange ? "0.123" : "17");
        draw();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_EQ(active, GImGui->ActiveId);
            EXPECT_FLOAT_EQ(edited, *ref.value);
        }
        io.AddKeyEvent(ImGuiKey_Enter, true);
        draw();
        io.AddKeyEvent(ImGuiKey_Enter, false);
        draw();
        io.AddMousePosEvent(revert.x, revert.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        draw();
        EXPECT_FLOAT_EQ(*original.value, *ref.value) << parameter->_name;
    }
    EXPECT_EQ(11, count);
}

TEST_F(LocalizationGuiTests, l2b3bGroupSearchAndCyrillicHighlightRetainWholeCharacters)
{
    select("ru");
    for (auto const& [query, name, count] : std::vector<std::tuple<std::string, std::string, int>>{
             {"фОРМа", "Shape", 3}, {"SHAPE", "Shape", 3}, {"сИЛОВОЕ ПОЛЕ", "Force field", 1}, {"FORCE FIELD", "Force field", 1}}) {
        auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = query});
        EXPECT_EQ(count, visibleParameters(parameterGroup(filtered, name)));
    }
    auto const& parameter = *geometryParameters().back();
    auto label = SimulationParametersPresentation::label(parameter);
    auto found = Utf8Search::find(label, "РАЗМЕР");
    ASSERT_TRUE(found);
    EXPECT_EQ("размер", label.substr(found->begin, found->end - found->begin));
    int normal = 0, highlighted = 0;
    frame([&] {
        auto list = ImGui::GetWindowDrawList();
        auto before = list->VtxBuffer.Size;
        AlienGui::Text(AlienGui::TextParameters().text(label));
        normal = list->VtxBuffer.Size - before;
    });
    frame([&] {
        auto list = ImGui::GetWindowDrawList();
        auto before = list->VtxBuffer.Size;
        AlienGui::Text(AlienGui::TextParameters().text(label).highlightedSubString("РАЗМЕР"));
        highlighted = list->VtxBuffer.Size - before;
    });
    EXPECT_GT(normal, 0);
    EXPECT_GT(highlighted, normal);
}

TEST_F(LocalizationGuiTests, l2b3bNewModeCaptionsFitOrShowCompleteHoverAtEveryScale)
{
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto parameter : geometryParameters()) {
            auto alternative = std::get_if<AlternativeSpec>(&parameter->_reference);
            if (!alternative) {
                continue;
            }
            std::vector<std::string> canonical;
            for (auto const& [name, children] : alternative->_alternatives) {
                canonical.emplace_back(name);
            }
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                auto labels = SimulationParametersPresentation::alternativeLabels(*parameter);
                for (int mode = 0; mode < static_cast<int>(labels.size()); ++mode) {
                    for (float width : {150.0f, 1400.0f}) {
                        std::string logged;
                        ImVec2 start;
                        bool tooltip = false;
                        auto draw = [&] {
                            switcherFrame(width * factor, [&] {
                                start = ImGui::GetCursorScreenPos();
                                ImGui::LogToBuffer();
                                AlienGui::Switcher(
                                    AlienGui::SwitcherParameters()
                                        .name(parameter->_name)
                                        .label(SimulationParametersPresentation::label(*parameter))
                                        .textWidth(360)
                                        .readOnly(true)
                                        .values(canonical)
                                        .displayValues(labels),
                                    &mode);
                                logged = GImGui->LogBuffer.c_str();
                                ImGui::LogFinish();
                                tooltip = false;
                                for (auto window : GImGui->Windows) {
                                    tooltip |= (window->Flags & ImGuiWindowFlags_Tooltip) && window->Active && !window->Hidden;
                                }
                            });
                        };
                        io.AddMousePosEvent(-100, -100);
                        draw();
                        draw();
                        io.AddMousePosEvent(start.x + scale(10), start.y + ImGui::GetFrameHeight() / 2);
                        draw();
                        draw();
                        auto text = labels.at(mode);
                        auto first = logged.find(text);
                        ASSERT_NE(std::string::npos, first);
                        if (width == 1400) {
                            EXPECT_FALSE(tooltip);
                        }
                        if (tooltip) {
                            EXPECT_NE(std::string::npos, logged.find(text, first + text.size()));
                        }
                        if (width == 150 && std::string(language) == "ru" && text == "Против часовой стрелки") {
                            EXPECT_TRUE(tooltip);
                        }
                    }
                }
            }
        }
    }
}

namespace
{
    std::vector<ParameterSpec const*> physicsParameters()
    {
        std::vector<ParameterSpec const*> result;
        for (auto const& group : SimulationParameters::getSpec()._groups) {
            if (group._name == "Physics: Motion" || group._name == "Physics: Thresholds" || group._name == "Radiation") {
                for (auto const& parameter : group._parameters) {
                    result.emplace_back(&parameter);
                }
            }
        }
        return result;
    }
}

TEST_F(LocalizationGuiTests, l2b3cOrdinaryFloatsEditAndResetThroughProductionDispatch)
{
    GeometryWorldScope world;
    auto values = std::make_unique<SimulationParameters>();
    values->numLayers = values->numSources = 1;
    values->layerOrderNumbers[0] = 1;
    values->sourceOrderNumbers[0] = 2;
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    int count = 0;
    for (auto parameter : physicsParameters()) {
        auto number = std::get_if<FloatSpec>(&parameter->_reference);
        if (!number || number->_getterSetter || !parameter->_visible) {
            continue;
        }
        ++count;
        auto order = parameter->_name == "Radiation angle" ? 2 : 0;
        if (parameter->_name == "Radiation angle") {
            values->sourceRadiationAngle.sourceValues[0].enabled = true;
        }
        auto ref = SpecificationEvaluationService::get().getRef(number->_member, *values, order);
        auto original = SpecificationEvaluationService::get().getRef(number->_member, *defaults, order);
        bool isSmallRange = std::holds_alternative<float>(number->_max) && std::get<float>(number->_max) < 1;
        float edited = std::get<float>(number->_min) + (isSmallRange ? 0.003f : 0.7f);
        ImVec2 slider, revert;
        ImGuiID active = 0;
        auto draw = [&] {
            frame([&] {
                auto start = ImGui::GetCursorScreenPos();
                slider = {start.x + 100, start.y + ImGui::GetFrameHeight() / 2};
                auto width = ImGui::GetContentRegionAvail().x - scale(360);
                revert = {
                    start.x + width + ImGui::GetStyle().ItemSpacing.x + (ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2) / 2,
                    start.y + ImGui::GetFrameHeight() / 2};
                parametersGroup({*parameter}, *values, *defaults, order);
                ImGui::PushID(0);
                ImGui::PushID(parameter->_name.c_str());
                ImGui::PushID("");
                ImGui::PushID(0);
                auto id = ImGui::GetID("##slider");
                if (!active) {
                    active = id;
                }
                EXPECT_EQ(active, id);
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
            });
        };
        draw();
        draw();
        io.AddKeyEvent(ImGuiMod_Ctrl, true);
        io.AddMousePosEvent(slider.x, slider.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        io.AddKeyEvent(ImGuiMod_Ctrl, false);
        draw();
        ASSERT_EQ(active, GImGui->ActiveId) << parameter->_name;
        io.AddInputCharactersUTF8(std::to_string(edited).c_str());
        draw();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_EQ(active, GImGui->ActiveId);
            EXPECT_FLOAT_EQ(edited, *ref.value);
        }
        io.AddKeyEvent(ImGuiKey_Enter, true);
        draw();
        io.AddKeyEvent(ImGuiKey_Enter, false);
        draw();
        io.AddMousePosEvent(revert.x, revert.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        draw();
        EXPECT_FLOAT_EQ(*original.value, *ref.value) << parameter->_name;
    }
    EXPECT_EQ(17, count);
}


TEST_F(LocalizationGuiTests, l2b3cAllBindingsFallbackSearchVisibilityAndLanguageRoundTrip)
{
    auto all = physicsParameters();
    ASSERT_EQ(21, all.size());
    auto values = std::make_unique<SimulationParameters>();
    values->numLayers = values->numSources = 1;
    values->layerOrderNumbers[0] = 1;
    values->sourceOrderNumbers[0] = 2;
    values->sourceRelativeStrength.sourceValues[0].value = 0.3f;
    values->relativeStrengthBasePin.pinned = true;
    values->sourceRadiationAngle.sourceValues[0] = {.value = 27.0f, .enabled = true};
    for (auto parameter : all) {
        if (auto number = std::get_if<FloatSpec>(&parameter->_reference)) {
            for (int order : {0, 1, 2}) {
                auto location = order == 0 ? LocationType::Base : order == 1 ? LocationType::Layer : LocationType::Source;
                if (SpecificationEvaluationService::get().isVisible(*parameter, location)) {
                    auto ref = SpecificationEvaluationService::get().getRef(number->_member, *values, order);
                    if (ref.enabled) {
                        *ref.enabled = true;
                    }
                }
            }
        }
    }
    for (int color = 0; color < MAX_COLORS; ++color) {
        values->maxForce.baseValue[color] = 0.2f + color * 0.1f;
        values->maxBindingDistance.value[color] = 2.0f + color * 0.1f;
        values->radiationType1_minimumAge.value[color] = 100 + color * 10;
    }
    auto defaults = std::make_unique<SimulationParameters>(*values);
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        for (auto parameter : all) {
            auto label = SimulationParametersPresentation::label(*parameter);
            auto description = SimulationParametersPresentation::description(*parameter);
            ASSERT_TRUE(description);
            EXPECT_FALSE(description->empty());
            if (std::string(language) == "ru") {
                EXPECT_NE(parameter->_name, label);
                EXPECT_NE(parameter->_description, description);
            } else {
                EXPECT_EQ(parameter->_name, label);
                EXPECT_EQ(parameter->_description, description);
            }
            auto renamed = *parameter;
            renamed._name = "Unrelated caption";
            EXPECT_EQ(label, SimulationParametersPresentation::label(renamed));
            EXPECT_EQ(description, SimulationParametersPresentation::description(renamed));
        }
        for (auto const& query :
             {"",
              "Inner friction",
              "ВНУТРЕННЕЕ ТРЕНИЕ",
              "Physics: Motion",
              "Физика: движение",
              "Physics: Thresholds",
              "Физика: пороговые значения",
              "Radiation",
              "Излучение",
              "СКОРОСТЬ",
              "velocity"}) {
            ParametersFilter filter{.containedText = query};
            auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), filter);
            for (auto const& name : {"Physics: Motion", "Physics: Thresholds", "Radiation"}) {
                auto const& original = parameterGroup(SimulationParameters::getSpec(), name);
                auto const& group = parameterGroup(filtered, name);
                ASSERT_EQ(original._parameters.size(), group._parameters.size());
                for (size_t i = 0; i < group._parameters.size(); ++i) {
                    EXPECT_EQ(original._parameters.at(i)._name, group._parameters.at(i)._name);
                    if (group._parameters.at(i)._name == "Inner friction") {
                        EXPECT_FALSE(group._parameters.at(i)._visible);
                    }
                }
                for (int order : {0, 1, 2}) {
                    frame([&] { parametersGroup(group._parameters, *values, *defaults, order); });
                    EXPECT_EQ(*defaults, *values) << language << ": " << query << ": " << name;
                }
            }
            EXPECT_EQ(query, filter.containedText);
        }
    }
    select("ru");
    for (auto const& [query, groupName, count] : std::vector<std::tuple<std::string, std::string, int>>{
             {"фИЗИКА: ДВИЖЕНИЕ", "Physics: Motion", 6},
             {"PHYSICS: MOTION", "Physics: Motion", 6},
             {"фИЗИКА: ПОРОГОВЫЕ ЗНАЧЕНИЯ", "Physics: Thresholds", 5},
             {"PHYSICS: THRESHOLDS", "Physics: Thresholds", 5},
             {"иЗЛУЧЕНИЕ", "Radiation", 9},
             {"RADIATION", "Radiation", 9}}) {
        auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = query});
        EXPECT_EQ(count, visibleParameters(parameterGroup(filtered, groupName)));
    }
    auto match = Utf8Search::find("Максимальная скорость", "СКОРОСТЬ");
    ASSERT_TRUE(match);
    EXPECT_EQ("скорость", std::string("Максимальная скорость").substr(match->begin, match->end - match->begin));
    LocalizationService::get().load("resources/locales/missing-l2b3c");
    select("ru");
    for (auto parameter : all) {
        EXPECT_EQ(parameter->_name, SimulationParametersPresentation::label(*parameter));
        EXPECT_EQ(parameter->_description, SimulationParametersPresentation::description(*parameter));
    }
    auto unbound = ParameterSpec().name("Canonical fallback").description("Original help").reference(FloatSpec());
    EXPECT_EQ("Canonical fallback", SimulationParametersPresentation::label(unbound));
    EXPECT_EQ("Original help", SimulationParametersPresentation::description(unbound));
    LocalizationService::get().load("resources/locales");
}

TEST_F(LocalizationGuiTests, l2b3cIntegerColorsLogarithmInfinityEditResetAndStableFocus)
{
    auto all = physicsParameters();
    auto parameter = *std::ranges::find_if(all, [](auto p) { return std::holds_alternative<IntSpec>(p->_reference); });
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    auto const& number = std::get<IntSpec>(parameter->_reference);
    EXPECT_TRUE(number._logarithmic);
    EXPECT_TRUE(number._infinity);
    ImVec2 slider, revert, expand;
    ImGuiID active = 0;
    int row = 0;
    std::string logged;
    auto draw = [&] {
        frame([&] {
            auto start = ImGui::GetCursorScreenPos();
            ImGui::PushID(0);
            ImGui::PushID(parameter->_name.c_str());
            ImGui::PushID("");
            expand = {start.x + 8, start.y + ImGui::GetFrameHeight() / 2};
            ImGui::PushID(row);
            auto current = ImGui::GetID("##slider");
            if (!active) {
                active = current;
            }
            EXPECT_EQ(active, current);
            ImGui::PopID();
            ImGui::PopID();
            ImGui::PopID();
            ImGui::PopID();
            slider = {start.x + 100, start.y + ImGui::GetFrameHeight() / 2 + row * (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y)};
            auto width = ImGui::GetContentRegionAvail().x - scale(360);
            revert = {
                start.x + width + ImGui::GetStyle().ItemSpacing.x + (ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2) / 2,
                start.y + ImGui::GetFrameHeight() / 2};
            ImGui::LogToBuffer();
            parametersGroup({*parameter}, *values, *defaults, 0);
            logged = GImGui->LogBuffer.c_str();
            ImGui::LogFinish();
        });
    };
    auto click = [&](ImVec2 position) {
        io.AddMousePosEvent(position.x, position.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        draw();
    };
    draw();
    draw();
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        values->radiationType1_minimumAge.value = ColorVector<int>::uniform(Infinity<int>::value);
        draw();
        EXPECT_NE(std::string::npos, logged.find(std::string(language) == "ru" ? "Бесконечность" : "Infinity"));
        EXPECT_EQ(Infinity<int>::value, values->radiationType1_minimumAge.value[0]);
    }
    values->radiationType1_minimumAge.value = ColorVector<int>::uniform(0);
    auto enter = [&](int value) {
        io.AddKeyEvent(ImGuiMod_Ctrl, true);
        click(slider);
        io.AddKeyEvent(ImGuiMod_Ctrl, false);
        draw();
        ASSERT_EQ(active, GImGui->ActiveId);
        io.AddInputCharactersUTF8(std::to_string(value).c_str());
        draw();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_EQ(active, GImGui->ActiveId);
            EXPECT_EQ(value, values->radiationType1_minimumAge.value[row]);
        }
        io.AddKeyEvent(ImGuiKey_Enter, true);
        draw();
        io.AddKeyEvent(ImGuiKey_Enter, false);
        draw();
    };
    enter(1234);
    for (int color = 0; color < MAX_COLORS; ++color) {
        EXPECT_EQ(1234, values->radiationType1_minimumAge.value[color]);
    }
    click(revert);
    EXPECT_EQ(defaults->radiationType1_minimumAge, values->radiationType1_minimumAge);
    click(expand);
    row = 2;
    active = 0;
    draw();
    enter(2345);
    EXPECT_EQ(0, values->radiationType1_minimumAge.value[1]);
    click(revert);
    EXPECT_EQ(defaults->radiationType1_minimumAge, values->radiationType1_minimumAge);
    click(expand);
}

TEST_F(LocalizationGuiTests, l2b3cRadiationGetterSetterPinAndSourceShareUseRealControls)
{
    auto values = std::make_unique<SimulationParameters>();
    values->numSources = 2;
    values->sourceOrderNumbers[0] = 1;
    values->sourceOrderNumbers[1] = 2;
    values->sourceRelativeStrength.sourceValues[0].value = 0.3f;
    values->sourceRelativeStrength.sourceValues[1].value = 0.2f;
    values->sourceRelativeStrength.sourceValues[1].pinned = true;
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto const& parameter = parameterGroup(SimulationParameters::getSpec(), "Radiation")._parameters.at(0);
    auto const& number = std::get<FloatSpec>(parameter._reference);
    auto [getter, setter] = *number._getterSetter;
    auto sourceParameter = parameterGroup(SimulationParameters::getSpec(), "General")._parameters.at(4);
    // Find the existing source share by its getter/setter, independently of display text.
    for (auto const& p : parameterGroup(SimulationParameters::getSpec(), "General")._parameters) {
        if (auto f = std::get_if<FloatSpec>(&p._reference); f && f->_getterSetter) {
            sourceParameter = p;
        }
    }
    auto const& sourceNumber = std::get<FloatSpec>(sourceParameter._reference);
    auto [sourceGetter, sourceSetter] = *sourceNumber._getterSetter;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImVec2 slider, pin, revert;
    auto draw = [&] {
        frame([&] {
            auto start = ImGui::GetCursorScreenPos();
            auto width = ImGui::GetContentRegionAvail().x - scale(360) - scale(22) - ImGui::GetStyle().FramePadding.x;
            slider = {start.x + 100, start.y + ImGui::GetFrameHeight() / 2};
            pin = {start.x + width + ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().FramePadding.x + scale(11), slider.y};
            revert = {
                pin.x + scale(11) + ImGui::GetStyle().ItemSpacing.x + (ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2) / 2,
                slider.y};
            parametersGroup({parameter}, *values, *defaults, 0);
        });
    };
    auto click = [&](ImVec2 position) {
        io.AddMousePosEvent(position.x, position.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        draw();
    };
    draw();
    draw();
    click(pin);
    ASSERT_TRUE(values->relativeStrengthBasePin.pinned);
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        draw();
        EXPECT_TRUE(values->relativeStrengthBasePin.pinned);
        sourceSetter(0.9f, *values, 1);
        EXPECT_FLOAT_EQ(0.5f, getter(*values, 0));
        EXPECT_FLOAT_EQ(0.3f, sourceGetter(*values, 1));
    }
    click(pin);
    ASSERT_FALSE(values->relativeStrengthBasePin.pinned);
    io.AddKeyEvent(ImGuiMod_Ctrl, true);
    click(slider);
    io.AddKeyEvent(ImGuiMod_Ctrl, false);
    draw();
    io.AddInputCharactersUTF8("0.6");
    draw();
    io.AddKeyEvent(ImGuiKey_Enter, true);
    draw();
    io.AddKeyEvent(ImGuiKey_Enter, false);
    draw();
    EXPECT_NEAR(0.6f, getter(*values, 0), 0.0001f);
    EXPECT_NEAR(0.2f, sourceGetter(*values, 1), 0.0001f);
    EXPECT_FLOAT_EQ(0.2f, sourceGetter(*values, 2));
    EXPECT_TRUE(values->sourceRelativeStrength.sourceValues[1].pinned);
    click(revert);
    EXPECT_NEAR(0.5f, getter(*values, 0), 0.0001f);
    EXPECT_NEAR(0.3f, sourceGetter(*values, 1), 0.0001f);
    EXPECT_FLOAT_EQ(0.2f, sourceGetter(*values, 2));
    EXPECT_NEAR(1.0f, getter(*values, 0) + sourceGetter(*values, 1) + sourceGetter(*values, 2), 0.00001f);
}

TEST_F(LocalizationGuiTests, l2b3cThresholdColorArraysExpandEditResetAndRetainState)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    auto const& group = parameterGroup(SimulationParameters::getSpec(), "Physics: Thresholds");
    for (size_t index : {1u, 3u}) {
        auto const& parameter = group._parameters.at(index);
        auto const& number = std::get<FloatSpec>(parameter._reference);
        auto ref = SpecificationEvaluationService::get().getRef(number._member, *values, 0);
        auto original = SpecificationEvaluationService::get().getRef(number._member, *defaults, 0);
        ASSERT_EQ(ColorDependence::ColorVector, ref.colorDependence);
        for (int color = 0; color < MAX_COLORS; ++color) {
            ref.value[color] = 0.4f + color * 0.1f;
        }
        ImVec2 expand, slider, revert;
        ImGuiID active = 0;
        auto draw = [&] {
            frame([&] {
                auto start = ImGui::GetCursorScreenPos();
                expand = {start.x + 8, start.y + ImGui::GetFrameHeight() / 2};
                slider = {start.x + 100, start.y + ImGui::GetFrameHeight() / 2 + 2 * (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y)};
                auto width = ImGui::GetContentRegionAvail().x - scale(360);
                revert = {
                    start.x + width + ImGui::GetStyle().ItemSpacing.x + (ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2) / 2,
                    expand.y};
                parametersGroup({parameter}, *values, *defaults, 0);
                ImGui::PushID(0);
                ImGui::PushID(parameter._name.c_str());
                ImGui::PushID("");
                ImGui::PushID(2);
                auto current = ImGui::GetID("##slider");
                if (!active) {
                    active = current;
                }
                EXPECT_EQ(active, current);
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
            });
        };
        auto click = [&](ImVec2 position) {
            io.AddMousePosEvent(position.x, position.y);
            draw();
            io.AddMouseButtonEvent(0, true);
            draw();
            io.AddMouseButtonEvent(0, false);
            draw();
        };
        draw();
        draw();
        click(expand);
        io.AddKeyEvent(ImGuiMod_Ctrl, true);
        click(slider);
        io.AddKeyEvent(ImGuiMod_Ctrl, false);
        draw();
        ASSERT_EQ(active, GImGui->ActiveId);
        io.AddInputCharactersUTF8("1.2");
        draw();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_EQ(active, GImGui->ActiveId);
            for (int color = 0; color < MAX_COLORS; ++color) {
                EXPECT_FLOAT_EQ(color == 2 ? 1.2f : 0.4f + color * 0.1f, ref.value[color]);
            }
        }
        io.AddKeyEvent(ImGuiKey_Enter, true);
        draw();
        io.AddKeyEvent(ImGuiKey_Enter, false);
        draw();
        click(revert);
        for (int color = 0; color < MAX_COLORS; ++color) {
            EXPECT_FLOAT_EQ(original.value[color], ref.value[color]);
        }
        click(expand);
    }
}

TEST_F(LocalizationGuiTests, l2b3cDirectionEnableAndDisableSourcesKeepIndependentFlags)
{
    auto values = std::make_unique<SimulationParameters>();
    values->numLayers = values->numSources = 1;
    values->layerOrderNumbers[0] = 1;
    values->sourceOrderNumbers[0] = 2;
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto const& radiation = parameterGroup(SimulationParameters::getSpec(), "Radiation");
    auto const& angle = radiation._parameters.at(8);
    auto const& sources = radiation._parameters.at(1);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (bool isAngle : {true, false}) {
        auto const& parameter = isAngle ? angle : sources;
        bool* flag = isAngle ? &values->sourceRadiationAngle.sourceValues[0].enabled : &values->disableRadiationSources.layerValues[0];
        *flag = false;
        if (isAngle) {
            values->sourceRadiationAngle.sourceValues[0].value = 0;
        }
        ImVec2 checkbox;
        auto draw = [&] {
            frame([&] {
                auto start = ImGui::GetCursorScreenPos();
                checkbox = {
                    start.x
                        + (isAngle ? 8
                                   : ImGui::GetContentRegionAvail().x - scale(386) + ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().FramePadding.x + 8),
                    start.y + ImGui::GetFrameHeight() / 2};
                parametersGroup({parameter}, *values, *defaults, isAngle ? 2 : 1);
            });
        };
        auto click = [&] {
            io.AddMousePosEvent(checkbox.x, checkbox.y);
            draw();
            io.AddMouseButtonEvent(0, true);
            draw();
            io.AddMouseButtonEvent(0, false);
            draw();
        };
        draw();
        draw();
        click();
        ASSERT_TRUE(*flag);
        if (isAngle) {
            values->sourceRadiationAngle.sourceValues[0].value = -37.5f;
        }
        auto snapshot = std::make_unique<SimulationParameters>(*values);
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_EQ(*snapshot, *values);
        }
        click();
        EXPECT_FALSE(*flag);
        if (isAngle) {
            EXPECT_FLOAT_EQ(-37.5f, values->sourceRadiationAngle.sourceValues[0].value);
        }
    }
}

TEST_F(LocalizationGuiTests, l2b3cCaptionsAndDescriptionsRemainAccessibleAtAllScales)
{
    auto& io = ImGui::GetIO();
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        select("ru");
        for (auto parameter : physicsParameters()) {
            for (auto const& text : {SimulationParametersPresentation::label(*parameter), *SimulationParametersPresentation::description(*parameter)}) {
                ImVec2 start;
                bool tooltip = false;
                std::string logged;
                auto draw = [&] {
                    switcherFrame(230 * factor, [&] {
                        start = ImGui::GetCursorScreenPos();
                        ImGui::LogToBuffer();
                        AlienGui::Text(AlienGui::TextParameters().text(text).truncate(true).highlightedSubString("излучен"));
                        logged = GImGui->LogBuffer.c_str();
                        ImGui::LogFinish();
                        tooltip = false;
                        for (auto window : GImGui->Windows) {
                            tooltip |= (window->Flags & ImGuiWindowFlags_Tooltip) && window->Active && !window->Hidden;
                        }
                    });
                };
                io.AddMousePosEvent(-100, -100);
                draw();
                draw();
                io.AddMousePosEvent(start.x + 4, start.y + ImGui::GetFontSize() / 2);
                draw();
                draw();
                if (ImGui::CalcTextSize(text.c_str()).x > 230 * factor) {
                    EXPECT_TRUE(tooltip) << text;
                    EXPECT_NE(std::string::npos, logged.find(text));
                }
            }
        }
    }
}

TEST_F(LocalizationGuiTests, l2b3cFloatInfinityAndColorArraysSurviveDisplayAndPersistence)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto all = physicsParameters();
    for (auto parameter : all) {
        auto number = std::get_if<FloatSpec>(&parameter->_reference);
        if (!number || !number->_infinity) {
            continue;
        }
        auto ref = SpecificationEvaluationService::get().getRef(number->_member, *values, 0);
        auto count = SpecificationEvaluationService::get().getArraySize(ref.colorDependence);
        for (int color = 0; color < count; ++color) {
            ref.value[color] = Infinity<float>::value;
        }
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            std::string logged;
            frame([&] {
                ImGui::LogToBuffer();
                parametersGroup({*parameter}, *values, *defaults, 0);
                logged = GImGui->LogBuffer.c_str();
                ImGui::LogFinish();
            });
            EXPECT_NE(std::string::npos, logged.find(std::string(language) == "ru" ? "Бесконечность" : "Infinity"));
            for (int color = 0; color < count; ++color) {
                EXPECT_EQ(Infinity<float>::value, ref.value[color]);
            }
        }
    }
    for (int color = 0; color < MAX_COLORS; ++color) {
        values->maxForce.baseValue[color] = 0.2f + color * 0.1f;
        values->maxBindingDistance.value[color] = 2.0f + color * 0.1f;
        values->radiationType1_minimumAge.value[color] = color == 0 ? Infinity<int>::value : 17 * color;
        values->radiationAbsorption.baseValue[color] = color * 0.01f;
        values->radiationType1_strength.baseValue[color] = color * 0.00001f;
        values->radiationType2_strength.value[color] = color * 0.00002f;
    }
    values->relativeStrengthBasePin.pinned = true;
    values->numSources = 1;
    values->sourceOrderNumbers[0] = 1;
    values->sourceRelativeStrength.sourceValues[0] = {.value = 0.3f, .pinned = true};
    values->sourceRadiationAngle.sourceValues[0] = {.value = -37.5f, .enabled = true};
    auto file = std::filesystem::temp_directory_path()
        / ("alien-l2b3c-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".settings.json");
    auto& serializer = SerializerService::get();
    ASSERT_TRUE(serializer.serializeSimulationParametersToFile(file, *values));
    auto restored = std::make_unique<SimulationParameters>();
    ASSERT_TRUE(serializer.deserializeSimulationParametersFromFile(*restored, file));
    std::filesystem::remove(file);
    EXPECT_EQ(values->maxForce, restored->maxForce);
    EXPECT_EQ(values->maxBindingDistance, restored->maxBindingDistance);
    EXPECT_EQ(values->radiationType1_minimumAge, restored->radiationType1_minimumAge);
    for (int color = 0; color < MAX_COLORS; ++color) {
        EXPECT_FLOAT_EQ(values->radiationAbsorption.baseValue[color], restored->radiationAbsorption.baseValue[color]);
        EXPECT_FLOAT_EQ(values->radiationType1_strength.baseValue[color], restored->radiationType1_strength.baseValue[color]);
        EXPECT_FLOAT_EQ(values->radiationType2_strength.value[color], restored->radiationType2_strength.value[color]);
    }
    EXPECT_EQ(values->radiationType2_energyThreshold, restored->radiationType2_energyThreshold);
    EXPECT_EQ(values->particleSplitEnergy, restored->particleSplitEnergy);
    EXPECT_EQ(values->relativeStrengthBasePin, restored->relativeStrengthBasePin);
    EXPECT_EQ(values->sourceRelativeStrength, restored->sourceRelativeStrength);
    EXPECT_EQ(values->sourceRadiationAngle, restored->sourceRadiationAngle);
}

namespace
{
    std::vector<ParameterSpec const*> lifeCycleParameters()
    {
        std::vector<ParameterSpec const*> result;
        for (auto const& name : {"Cell life cycle", "Cell construction", "Mutations"}) {
            for (auto const& parameter : parameterGroup(SimulationParameters::getSpec(), name)._parameters) {
                result.emplace_back(&parameter);
            }
        }
        return result;
    }
}
TEST_F(LocalizationGuiTests, l2b4aOrdinaryFloatsEditAndResetThroughProductionDispatch)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    int count = 0;
    for (auto parameter : lifeCycleParameters()) {
        auto number = std::get_if<FloatSpec>(&parameter->_reference);
        if (!number || number->_getterSetter || !parameter->_visible) {
            continue;
        }
        ++count;
        auto constexpr order = 0;
        auto ref = SpecificationEvaluationService::get().getRef(number->_member, *values, order);
        auto original = SpecificationEvaluationService::get().getRef(number->_member, *defaults, order);
        bool isSmallRange = std::holds_alternative<float>(number->_max) && std::get<float>(number->_max) < 1;
        float edited = std::get<float>(number->_min) + (isSmallRange ? 0.003f : 0.7f);
        ImVec2 slider, revert;
        ImGuiID active = 0;
        auto draw = [&] {
            frame([&] {
                auto start = ImGui::GetCursorScreenPos();
                slider = {start.x + 100, start.y + ImGui::GetFrameHeight() / 2};
                auto width = ImGui::GetContentRegionAvail().x - scale(360);
                revert = {
                    start.x + width + ImGui::GetStyle().ItemSpacing.x + (ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2) / 2,
                    start.y + ImGui::GetFrameHeight() / 2};
                parametersGroup({*parameter}, *values, *defaults, order);
                ImGui::PushID(0);
                ImGui::PushID(parameter->_name.c_str());
                ImGui::PushID("");
                ImGui::PushID(0);
                auto id = ImGui::GetID("##slider");
                if (!active) {
                    active = id;
                }
                EXPECT_EQ(active, id);
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
            });
        };
        draw();
        draw();
        io.AddKeyEvent(ImGuiMod_Ctrl, true);
        io.AddMousePosEvent(slider.x, slider.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        io.AddKeyEvent(ImGuiMod_Ctrl, false);
        draw();
        ASSERT_EQ(active, GImGui->ActiveId) << parameter->_name;
        io.AddInputCharactersUTF8(std::to_string(edited).c_str());
        draw();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_EQ(active, GImGui->ActiveId);
            EXPECT_FLOAT_EQ(edited, *ref.value);
        }
        io.AddKeyEvent(ImGuiKey_Enter, true);
        draw();
        io.AddKeyEvent(ImGuiKey_Enter, false);
        draw();
        io.AddMousePosEvent(revert.x, revert.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        draw();
        EXPECT_FLOAT_EQ(*original.value, *ref.value) << parameter->_name;
    }
    EXPECT_EQ(5, count);
}

TEST_F(LocalizationGuiTests, l2b4aIntegerColorsLogarithmInfinityEditResetAndStableFocus)
{
    for (size_t index : {0u, 1u}) {
        auto parameter = lifeCycleParameters().at(index);
        auto values = std::make_unique<SimulationParameters>();
        auto defaults = std::make_unique<SimulationParameters>(*values);
        auto& io = ImGui::GetIO();
        io.ConfigInputTrickleEventQueue = false;
        auto const& number = std::get<IntSpec>(parameter->_reference);
        auto ref = SpecificationEvaluationService::get().getRef(number._member, *values, 0);
        auto original = SpecificationEvaluationService::get().getRef(number._member, *defaults, 0);
        ASSERT_EQ(ColorDependence::ColorVector, ref.colorDependence);
        EXPECT_TRUE(number._logarithmic);
        EXPECT_TRUE(number._infinity);
        ImVec2 slider, revert, expand;
        ImGuiID active = 0;
        int row = 0;
        std::string logged;
        auto draw = [&] {
            frame([&] {
                auto start = ImGui::GetCursorScreenPos();
                ImGui::PushID(0);
                ImGui::PushID(parameter->_name.c_str());
                ImGui::PushID("");
                expand = {start.x + 8, start.y + ImGui::GetFrameHeight() / 2};
                ImGui::PushID(row);
                auto current = ImGui::GetID("##slider");
                if (!active) {
                    active = current;
                }
                EXPECT_EQ(active, current);
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                slider = {start.x + 100, start.y + ImGui::GetFrameHeight() / 2 + row * (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y)};
                auto width = ImGui::GetContentRegionAvail().x - scale(360);
                revert = {
                    start.x + width + ImGui::GetStyle().ItemSpacing.x + (ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2) / 2,
                    start.y + ImGui::GetFrameHeight() / 2};
                ImGui::LogToBuffer();
                parametersGroup({*parameter}, *values, *defaults, 0);
                logged = GImGui->LogBuffer.c_str();
                ImGui::LogFinish();
            });
        };
        auto click = [&](ImVec2 position) {
            io.AddMousePosEvent(position.x, position.y);
            draw();
            io.AddMouseButtonEvent(0, true);
            draw();
            io.AddMouseButtonEvent(0, false);
            draw();
        };
        draw();
        draw();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            for (int color = 0; color < MAX_COLORS; ++color) {
                ref.value[color] = Infinity<int>::value;
            }
            draw();
            EXPECT_NE(std::string::npos, logged.find(std::string(language) == "ru" ? "Бесконечность" : "Infinity"));
            EXPECT_EQ(Infinity<int>::value, ref.value[0]);
        }
        for (int color = 0; color < MAX_COLORS; ++color) {
            ref.value[color] = 10;
        }
        auto enter = [&](int value) {
            io.AddKeyEvent(ImGuiMod_Ctrl, true);
            click(slider);
            io.AddKeyEvent(ImGuiMod_Ctrl, false);
            draw();
            ASSERT_EQ(active, GImGui->ActiveId);
            io.AddInputCharactersUTF8(std::to_string(value).c_str());
            draw();
            for (auto const& language : {"en", "ru", "en"}) {
                select(language);
                draw();
                EXPECT_EQ(active, GImGui->ActiveId);
                EXPECT_EQ(value, ref.value[row]);
            }
            io.AddKeyEvent(ImGuiKey_Enter, true);
            draw();
            io.AddKeyEvent(ImGuiKey_Enter, false);
            draw();
        };
        enter(1234);
        for (int color = 0; color < MAX_COLORS; ++color) {
            EXPECT_EQ(1234, ref.value[color]);
        }
        click(revert);
        for (int color = 0; color < MAX_COLORS; ++color) {
            EXPECT_EQ(original.value[color], ref.value[color]);
        }
        click(expand);
        row = 2;
        active = 0;
        draw();
        enter(2345);
        EXPECT_EQ(Infinity<int>::value, ref.value[1]);
        click(revert);
        for (int color = 0; color < MAX_COLORS; ++color) {
            EXPECT_EQ(original.value[color], ref.value[color]);
        }
        click(expand);
    }
}
TEST_F(LocalizationGuiTests, l2b4aEnergyDecayConstructionColorsExpandEditResetAndRetainState)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (size_t index : {2u, 3u, 4u, 6u}) {
        auto const& parameter = *lifeCycleParameters().at(index);
        auto const& number = std::get<FloatSpec>(parameter._reference);
        auto ref = SpecificationEvaluationService::get().getRef(number._member, *values, 0);
        auto original = SpecificationEvaluationService::get().getRef(number._member, *defaults, 0);
        ASSERT_EQ(ColorDependence::ColorVector, ref.colorDependence);
        for (int color = 0; color < MAX_COLORS; ++color) {
            ref.value[color] = index == 4 ? 0.001f + color * 0.001f : index == 6 ? 0.4f + color * 0.1f : 70.0f + color;
        }
        ImVec2 expand, slider, revert;
        ImGuiID active = 0;
        auto draw = [&] {
            frame([&] {
                auto start = ImGui::GetCursorScreenPos();
                expand = {start.x + 8, start.y + ImGui::GetFrameHeight() / 2};
                slider = {start.x + 100, start.y + ImGui::GetFrameHeight() / 2 + 2 * (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y)};
                auto width = ImGui::GetContentRegionAvail().x - scale(360);
                revert = {
                    start.x + width + ImGui::GetStyle().ItemSpacing.x + (ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2) / 2,
                    expand.y};
                parametersGroup({parameter}, *values, *defaults, 0);
                ImGui::PushID(0);
                ImGui::PushID(parameter._name.c_str());
                ImGui::PushID("");
                ImGui::PushID(2);
                auto current = ImGui::GetID("##slider");
                if (!active) {
                    active = current;
                }
                EXPECT_EQ(active, current);
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
            });
        };
        auto click = [&](ImVec2 position) {
            io.AddMousePosEvent(position.x, position.y);
            draw();
            io.AddMouseButtonEvent(0, true);
            draw();
            io.AddMouseButtonEvent(0, false);
            draw();
        };
        draw();
        draw();
        click(expand);
        io.AddKeyEvent(ImGuiMod_Ctrl, true);
        click(slider);
        io.AddKeyEvent(ImGuiMod_Ctrl, false);
        draw();
        ASSERT_EQ(active, GImGui->ActiveId);
        float edited = index == 4 ? 0.012f : index == 6 ? 1.2f : 80.0f;
        io.AddInputCharactersUTF8(std::to_string(edited).c_str());
        draw();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_EQ(active, GImGui->ActiveId);
            for (int color = 0; color < MAX_COLORS; ++color) {
                EXPECT_FLOAT_EQ(
                    color == 2       ? edited
                        : index == 4 ? 0.001f + color * 0.001f
                        : index == 6 ? 0.4f + color * 0.1f
                                     : 70.0f + color,
                    ref.value[color]);
            }
        }
        io.AddKeyEvent(ImGuiKey_Enter, true);
        draw();
        io.AddKeyEvent(ImGuiKey_Enter, false);
        draw();
        click(revert);
        for (int color = 0; color < MAX_COLORS; ++color) {
            EXPECT_FLOAT_EQ(original.value[color], ref.value[color]);
        }
        click(expand);
    }
}
TEST_F(LocalizationGuiTests, l2b4aCaptionsAndDescriptionsRemainAccessibleAtAllScales)
{
    auto& io = ImGui::GetIO();
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        select("ru");
        for (auto parameter : lifeCycleParameters()) {
            for (auto const& text : {SimulationParametersPresentation::label(*parameter), *SimulationParametersPresentation::description(*parameter)}) {
                ImVec2 start;
                bool tooltip = false;
                std::string logged;
                auto draw = [&] {
                    switcherFrame(230 * factor, [&] {
                        start = ImGui::GetCursorScreenPos();
                        ImGui::LogToBuffer();
                        AlienGui::Text(AlienGui::TextParameters().text(text).truncate(true).highlightedSubString("возраст"));
                        logged = GImGui->LogBuffer.c_str();
                        ImGui::LogFinish();
                        tooltip = false;
                        for (auto window : GImGui->Windows) {
                            tooltip |= (window->Flags & ImGuiWindowFlags_Tooltip) && window->Active && !window->Hidden;
                        }
                    });
                };
                io.AddMousePosEvent(-100, -100);
                draw();
                draw();
                io.AddMousePosEvent(start.x + 4, start.y + ImGui::GetFontSize() / 2);
                draw();
                draw();
                if (ImGui::CalcTextSize(text.c_str()).x > 230 * factor) {
                    EXPECT_TRUE(tooltip) << text;
                    size_t begin = 0;
                    while (begin < text.size()) {
                        auto end = text.find('\n', begin);
                        auto line = text.substr(begin, end == std::string::npos ? end : end - begin);
                        if (!line.empty()) {
                            EXPECT_NE(std::string::npos, logged.find(line)) << line;
                        }
                        if (end == std::string::npos) {
                            break;
                        }
                        begin = end + 1;
                    }
                }
            }
        }
    }
}

TEST_F(LocalizationGuiTests, l2b4aBindingsFullDescriptionsFallbackAndBilingualSearch)
{
    auto all = lifeCycleParameters();
    ASSERT_EQ(8, all.size());
    std::vector<std::string> prefixes = {
        "lifecycle.maxcellage",
        "lifecycle.freecellmaxage",
        "lifecycle.mincellenergy",
        "lifecycle.normalcellenergy",
        "lifecycle.celldeathprobability",
        "lifecycle.particletransformationallowed",
        "construction.constructorconnectingcelldistance",
        "mutations.newlineagethreshold"};
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        for (size_t i = 0; i < all.size(); ++i) {
            auto const& parameter = *all.at(i);
            auto prefix = "parameters." + prefixes.at(i);
            EXPECT_EQ(tr(prefix + ".name"), SimulationParametersPresentation::label(parameter));
            EXPECT_EQ(tr(prefix + ".description"), SimulationParametersPresentation::description(parameter));
            if (std::string(language) == "en") {
                EXPECT_EQ(parameter._name, SimulationParametersPresentation::label(parameter));
                EXPECT_EQ(parameter._description, SimulationParametersPresentation::description(parameter));
            } else {
                EXPECT_NE(parameter._name, SimulationParametersPresentation::label(parameter));
                EXPECT_NE(parameter._description, SimulationParametersPresentation::description(parameter));
            }
            auto renamed = parameter;
            renamed._name = "Unrelated caption";
            EXPECT_EQ(SimulationParametersPresentation::label(parameter), SimulationParametersPresentation::label(renamed));
            EXPECT_EQ(SimulationParametersPresentation::description(parameter), SimulationParametersPresentation::description(renamed));
        }
        for (auto const& [query, groupName, count] : std::vector<std::tuple<std::string, std::string, int>>{
                 {"cELL LIFE CYCLE", "Cell life cycle", 6},
                 {"ЖИЗНЕННЫЙ ЦИКЛ", "Cell life cycle", 6},
                 {"MAXIMUM AGE", "Cell life cycle", 1},
                 {"Максимальный возраст", "Cell life cycle", 2},
                 {"NORMAL ENERGY", "Cell life cycle", 1},
                 {"НОРМАЛЬНЫЙ", "Cell life cycle", 1},
                 {"CELL CONSTRUCTION", "Cell construction", 1},
                 {"СТРОИТЕЛЬСТВО", "Cell construction", 1},
                 {"New lineage", "Mutations", 1},
                 {"ПРОИСХОЖДЕНИЕ", "Mutations", 1},
                 {"", "Cell life cycle", 6},
                 {"", "Cell construction", 1},
                 {"", "Mutations", 1}}) {
            ParametersFilter filter{.containedText = query};
            auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), filter);
            auto const& group = parameterGroup(filtered, groupName);
            bool russian = std::ranges::any_of(query, [](unsigned char c) { return c >= 128; });
            EXPECT_EQ(russian && std::string(language) == "en" ? 0 : count, visibleParameters(group)) << query;
            EXPECT_EQ(query, filter.containedText);
            auto const& original = parameterGroup(SimulationParameters::getSpec(), groupName);
            ASSERT_EQ(original._parameters.size(), group._parameters.size());
            for (size_t i = 0; i < group._parameters.size(); ++i) {
                EXPECT_EQ(original._parameters.at(i)._name, group._parameters.at(i)._name);
                EXPECT_EQ(original._parameters.at(i)._reference.index(), group._parameters.at(i)._reference.index());
            }
        }
    }
    auto match = Utf8Search::find("Максимальный возраст", "ВОЗРАСТ");
    ASSERT_TRUE(match);
    EXPECT_EQ("возраст", std::string("Максимальный возраст").substr(match->begin, match->end - match->begin));
    LocalizationService::get().load("resources/locales/missing-l2b4a");
    select("ru");
    for (auto parameter : all) {
        EXPECT_EQ(parameter->_name, SimulationParametersPresentation::label(*parameter));
        EXPECT_EQ(parameter->_description, SimulationParametersPresentation::description(*parameter));
    }
    for (auto const& name : {"Cell life cycle", "Cell construction", "Mutations"}) {
        EXPECT_EQ(name, SimulationParametersPresentation::label(parameterGroup(SimulationParameters::getSpec(), name)));
    }
    LocalizationService::get().load("resources/locales");
}

TEST_F(LocalizationGuiTests, l2b4aEnergyDependencyRetainsExistingNormalizationAcrossLanguages)
{
    GeometryWorldScope world;
    auto values = std::make_unique<SimulationParameters>();
    auto const& number = std::get<FloatSpec>(lifeCycleParameters().at(3)->_reference);
    auto dependency = std::get_if<ColorVectorFloatBaseLayerMember>(&number._greaterThan);
    ASSERT_NE(nullptr, dependency);
    ASSERT_TRUE(*dependency);
    EXPECT_EQ(&SimulationParameters::minCellEnergy, **dependency);
    for (int color = 0; color < MAX_COLORS; ++color) {
        values->minCellEnergy.baseValue[color] = 110.0f + color;
        values->normalCellEnergy.value[color] = 90.0f;
    }
    ParametersValidationService::get().validateAndCorrect({{1024, 768}}, *values);
    for (int color = 0; color < MAX_COLORS; ++color) {
        EXPECT_FLOAT_EQ(110.0f + color, values->normalCellEnergy.value[color]);
    }
    auto defaults = std::make_unique<SimulationParameters>(*values);
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        frame([&] { parametersGroup(parameterGroup(SimulationParameters::getSpec(), "Cell life cycle")._parameters, *values, *defaults, 0); });
        ParametersValidationService::get().validateAndCorrect({{1024, 768}}, *values);
        EXPECT_EQ(*defaults, *values);
    }
}

TEST_F(LocalizationGuiTests, l2b4aParticleTransformationToggleAndRevertUseRealCheckbox)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto const& parameter = *lifeCycleParameters().at(5);
    ASSERT_TRUE(std::holds_alternative<BoolSpec>(parameter._reference));
    ImVec2 checkbox, revert;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    auto draw = [&] {
        frame([&] {
            auto start = ImGui::GetCursorScreenPos();
            auto x = start.x + ImGui::GetContentRegionAvail().x - scale(386) + ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().FramePadding.x;
            checkbox = {x + 8, start.y + ImGui::GetFrameHeight() / 2};
            revert = {x + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x + 8, checkbox.y};
            parametersGroup({parameter}, *values, *defaults, 0);
        });
    };
    auto click = [&](ImVec2 position) {
        io.AddMousePosEvent(position.x, position.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        draw();
    };
    draw();
    draw();
    click(checkbox);
    ASSERT_TRUE(values->particleTransformationAllowed.value);
    auto snapshot = std::make_unique<SimulationParameters>(*values);
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        draw();
        EXPECT_EQ(*snapshot, *values);
    }
    click(revert);
    EXPECT_EQ(*defaults, *values);
    click(checkbox);
    ASSERT_TRUE(values->particleTransformationAllowed.value);
    click(checkbox);
    EXPECT_EQ(*defaults, *values);
}

TEST_F(LocalizationGuiTests, l2b4aLineageInfinityAndSyntheticSerializationPreserveAllValues)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto const& parameter = *lifeCycleParameters().at(7);
    auto const& number = std::get<FloatSpec>(parameter._reference);
    EXPECT_TRUE(number._infinity);
    EXPECT_TRUE(number._logarithmic);
    EXPECT_EQ("%.5f", number._format);
    values->newLineageThreshold.value = Infinity<float>::value;
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        std::string logged;
        frame([&] {
            ImGui::LogToBuffer();
            parametersGroup({parameter}, *values, *defaults, 0);
            logged = GImGui->LogBuffer.c_str();
            ImGui::LogFinish();
        });
        EXPECT_NE(std::string::npos, logged.find(std::string(language) == "ru" ? "Бесконечность" : "Infinity"));
        EXPECT_EQ(Infinity<float>::value, values->newLineageThreshold.value);
    }
    values->numLayers = defaults->numLayers = 1;
    values->layerOrderNumbers[0] = defaults->layerOrderNumbers[0] = 1;
    for (int color = 0; color < MAX_COLORS; ++color) {
        values->maxCellAge.value[color] = color == 0 ? Infinity<int>::value : 1200 + color;
        values->freeCellMaxAge.value[color] = color == 0 ? Infinity<int>::value : 2300 + color;
        values->minCellEnergy.baseValue[color] = 50.0f + color;
        values->minCellEnergy.layerValues[0].value[color] = 60.0f + color;
        values->minCellEnergy.layerValues[0].enabled = true;
        values->normalCellEnergy.value[color] = 100.0f + color;
        values->cellDeathProbability.baseValue[color] = 0.001f + color * 0.001f;
        values->cellDeathProbability.layerValues[0].value[color] = 0.002f + color * 0.001f;
        values->cellDeathProbability.layerValues[0].enabled = true;
        values->constructorConnectingCellDistance.value[color] = 1.0f + color * 0.25f;
    }
    values->particleTransformationAllowed.value = true;
    auto snapshot = std::make_unique<SimulationParameters>(*values);
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        for (auto const& name : {"Cell life cycle", "Cell construction", "Mutations"}) {
            for (int order : {0, 1}) {
                frame([&] { parametersGroup(parameterGroup(SimulationParameters::getSpec(), name)._parameters, *values, *defaults, order); });
                EXPECT_EQ(*snapshot, *values);
            }
        }
    }
    auto file = std::filesystem::temp_directory_path()
        / ("alien-l2b4a-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".settings.json");
    auto& serializer = SerializerService::get();
    ASSERT_TRUE(serializer.serializeSimulationParametersToFile(file, *values));
    auto restored = std::make_unique<SimulationParameters>();
    ASSERT_TRUE(serializer.deserializeSimulationParametersFromFile(*restored, file));
    std::filesystem::remove(file);
    EXPECT_EQ(values->maxCellAge, restored->maxCellAge);
    EXPECT_EQ(values->freeCellMaxAge, restored->freeCellMaxAge);
    EXPECT_EQ(values->minCellEnergy, restored->minCellEnergy);
    EXPECT_EQ(values->normalCellEnergy, restored->normalCellEnergy);
    EXPECT_EQ(values->constructorConnectingCellDistance, restored->constructorConnectingCellDistance);
    EXPECT_EQ(values->particleTransformationAllowed, restored->particleTransformationAllowed);
    EXPECT_EQ(values->newLineageThreshold, restored->newLineageThreshold);
    for (int color = 0; color < MAX_COLORS; ++color) {
        EXPECT_FLOAT_EQ(values->cellDeathProbability.baseValue[color], restored->cellDeathProbability.baseValue[color]);
        EXPECT_FLOAT_EQ(values->cellDeathProbability.layerValues[0].value[color], restored->cellDeathProbability.layerValues[0].value[color]);
    }
    EXPECT_TRUE(restored->cellDeathProbability.layerValues[0].enabled);
}

TEST_F(LocalizationGuiTests, l2b4aProductionAgeCaptionsKeepControlsClearAndFullTextOnHover)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    select("ru");
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (size_t index : {0u, 1u}) {
            auto const& parameter = *lifeCycleParameters().at(index);
            auto text = SimulationParametersPresentation::label(parameter);
            ImVec2 labelPosition;
            float available = 0;
            bool tooltip = false;
            std::string logged;
            auto draw = [&] {
                switcherFrame(650 * factor, [&] {
                    auto start = ImGui::GetCursorScreenPos();
                    auto sliderRight = start.x + ImGui::GetContentRegionAvail().x - scale(360);
                    auto undoWidth = ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2;
                    labelPosition = {sliderRight + ImGui::GetStyle().ItemSpacing.x * 2 + undoWidth, start.y};
                    available = start.x + ImGui::GetContentRegionAvail().x - labelPosition.x;
                    EXPECT_GT(labelPosition.x, sliderRight + undoWidth);
                    ImGui::LogToBuffer();
                    parametersGroup({parameter}, *values, *defaults, 0);
                    logged = GImGui->LogBuffer.c_str();
                    ImGui::LogFinish();
                    tooltip = false;
                    for (auto window : GImGui->Windows) {
                        tooltip |= (window->Flags & ImGuiWindowFlags_Tooltip) && window->Active && !window->Hidden;
                    }
                });
            };
            io.AddMousePosEvent(-100, -100);
            draw();
            draw();
            io.AddMousePosEvent(labelPosition.x + 4, labelPosition.y + ImGui::GetFontSize() / 2);
            draw();
            draw();
            if (ImGui::CalcTextSize(text.c_str()).x > available) {
                EXPECT_TRUE(tooltip) << text << ": " << factor;
                EXPECT_NE(std::string::npos, logged.find(text));
            }
            EXPECT_EQ(*defaults, *values);
        }
    }
}
