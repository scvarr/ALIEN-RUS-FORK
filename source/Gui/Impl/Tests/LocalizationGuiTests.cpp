#include <gtest/gtest.h>

#include <imgui_internal.h>

#include <Fonts/IconsFontAwesome5.h>

#include <Base/GlobalSettings.h>
#include <Base/LocalizationService.h>
#include <Base/Tests/SettingsTestScope.h>

#include <Gui/Impl/AlienDialog.h>
#include <Gui/Impl/AlienGui.h>
#include <Gui/Impl/AlienWindow.h>
#include <Gui/Impl/DelayedExecutionController.h>
#include <Gui/Impl/ExitDialog.h>
#include <Gui/Impl/GenericMessageDialog.h>
#include <Gui/Impl/LocalizationGui.h>
#include <Gui/Impl/ModalWindow.h>
#include <Gui/Impl/NewSimulationDialog.h>
#include <Gui/Impl/SavePictureDialog.h>
#include <Gui/Impl/SimulationParametersBaseWidget.h>
#include <Gui/Impl/SimulationParametersGuiText.h>
#include <Gui/Impl/SimulationParametersMainWindow.h>
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
