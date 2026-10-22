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
#include <Gui/Impl/ChangeColorDialog.h>
#include <Gui/Impl/CreaturePreviewWidget.h>
#include <Gui/Impl/DelayedExecutionController.h>
#include <Gui/Impl/EntityAttributeHelp.h>
#include <Gui/Impl/ExitDialog.h>
#include <Gui/Impl/GeneEditorWidget.h>
#include <Gui/Impl/GeneGraphWidget.h>
#include <Gui/Impl/GenericMessageDialog.h>
#include <Gui/Impl/GenomeEditorPresentation.h>
#include <Gui/Impl/GenomeEditorWidget.h>
#include <Gui/Impl/GenomeEditorWindow.h>
#include <Gui/Impl/GenomeIssueDescription.h>
#include <Gui/Impl/GenomeTabEditData.h>
#include <Gui/Impl/GenomeTabLayoutData.h>
#include <Gui/Impl/GenomeTabWidget.h>
#include <Gui/Impl/GenomeWindowEditData.h>
#include <Gui/Impl/LocalizationGui.h>
#include <Gui/Impl/ModalWindow.h>
#include <Gui/Impl/MutationRatesDialog.h>
#include <Gui/Impl/MutationRatesPresentation.h>
#include <Gui/Impl/MutationRatesWidget.h>
#include <Gui/Impl/NeuralNetEditorPresentation.h>
#include <Gui/Impl/NeuralNetEditorWidget.h>
#include <Gui/Impl/NewSimulationDialog.h>
#include <Gui/Impl/NodeEditorWidget.h>
#include <Gui/Impl/OverlayController.h>
#include <Gui/Impl/PreviewSettingsDialog.h>
#include <Gui/Impl/SavePictureDialog.h>
#include <Gui/Impl/SignalsBufferDialog.h>
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

    void initializeGenomeShell()
    {
        auto& window = GenomeEditorWindow::get();
        window._tabs.clear();
        window._tabsToAdd.clear();
        window._selectedTabIndex = 0;
        window._tabIndexToSelect.reset();
        window.initIntern();
    }

    void drawGenomeShell()
    {
        auto& window = GenomeEditorWindow::get();
        for (auto const& tabs : {window._tabs, window._tabsToAdd}) {
            for (auto const& tab : tabs) {
                tab->getEditData()->run = false;
                tab->getEditData()->selectGene(0);
            }
        }
        frame([&] {
            window.processToolbar();
            window.processTabWidget();
        });
    }

    auto& genomeTabs() { return GenomeEditorWindow::get()._tabs; }
    void selectGenomeTab(int index) { GenomeEditorWindow::get()._tabIndexToSelect = index; }
    void copyGenome() { GenomeEditorWindow::get().onCopyGenome(); }
    void pasteGenome() { GenomeEditorWindow::get().onPasteGenome(); }
    void cloneGenome() { GenomeEditorWindow::get().onCloneGenome(); }
    void closeOtherGenomeTabs() { GenomeEditorWindow::get().onCloseOtherTabs(); }

    void removeGene(GenomeEditorWidget const& editor) { editor->onRemoveGene(); }
    void swapRootGene(GenomeEditorWidget const& editor) { editor->onMoveGeneDownward(); }

    void drawGenomeProperties(GenomeEditorWidget const& editor) { editor->processHeaderData(); }
    void drawGeneProperties(GeneEditorWidget const& editor) { editor->processHeaderData(); }
    void drawGenomeTree(GenomeEditorWidget const& editor) { editor->processStructureTree(); }

    void drawGenomeStatus(GenomeTabWidget const& tab) { tab->processStatusBar(); }

    void addGenomeGene(GenomeEditorWidget const& editor) { editor->onAddGene(); }
    void moveGenomeGene(GenomeEditorWidget const& editor) { editor->onMoveGeneDownward(); }

    void drawGenomeGeneRow(GenomeEditorWidget const& editor, int geneIndex, ImVec2& position)
    {
        if (ImGui::BeginTable("Structure list", 4)) {
            auto start = ImGui::GetCursorScreenPos();
            ImGui::SetNextItemOpen(false);
            editor->processGeneNode(geneIndex, editor->_editData->genome._genes.at(geneIndex), {}, false, getDefaultCustomizationColorVector());
            position = {start.x + ImGui::GetFontSize() * 3, start.y + ImGui::GetTextLineHeight() / 2 + ImGui::GetStyle().CellPadding.y};
            ImGui::EndTable();
        }
    }

    void processOpenDialog(AlienDialog& dialog, std::function<void()> const& before = {})
    {
        dialog._modalWindow.process([&] {
            if (before) {
                before();
            }
            dialog.processIntern();
        });
    }

    void neuralActions(NeuralNetEditorWidget const& editor, NeuralNetGenomeDesc& net, bool dialog = false)
    {
        editor->processActionButtons(
            net._weights,
            net._biases,
            net._activationFunctions,
            dialog ? _NeuralNetEditorWidget::EditorMode::Dialog : _NeuralNetEditorWidget::EditorMode::Embedded);
    }

    void neuralEditor(NeuralNetEditorWidget const& editor, NeuralNetGenomeDesc& net, bool dialog = false)
    {
        auto& selection = editor->getValueRef(editor->_dataById);
        auto modules = CellFunctionChannels::getModules(NodeDesc().cellType(SensorGenomeDesc()).constructor(ConstructorGenomeDesc()));
        _NeuralNetEditorWidget::LiveData live;
        live.memory = {0.1f, -0.2f, 0.3f, -0.4f};
        live.energy = 123.4f;
        live.attacked = true;
        live.age = 321;
        live.speed = 0.25f;
        editor->processEditor(
            net._weights,
            net._biases,
            net._activationFunctions,
            net._connectionWeights,
            modules,
            live,
            selection,
            dialog ? _NeuralNetEditorWidget::EditorMode::Dialog : _NeuralNetEditorWidget::EditorMode::Embedded,
            _NeuralNetEditorWidget::HeightMode::Natural);
    }

    void neuralDialog(NeuralNetEditorWidget const& editor, NeuralNetGenomeDesc& net)
    {
        auto& selection = editor->getValueRef(editor->_dataById);
        editor->processDialog(net._weights, net._biases, net._activationFunctions, net._connectionWeights, {}, std::nullopt, selection);
    }

    void editNeuralDialog(NeuralNetEditorWidget const& editor)
    {
        editor->_dialogNet.biases.at(2) = 0.375f;
        editor->_dialogConnectionWeights.at(1) = -0.625f;
    }

    auto neuralSelection(NeuralNetEditorWidget const& editor)
    {
        auto& selection = editor->getValueRef(editor->_dataById);
        return std::pair(selection.inputIndex, selection.outputIndex);
    }

    void selectNeuralConnection(NeuralNetEditorWidget const& editor, int input, int output)
    {
        auto& selection = editor->getValueRef(editor->_dataById);
        selection.inputIndex = input;
        selection.outputIndex = output;
    }

    float neuralCardWidth(NeuralNetEditorWidget const& editor) { return editor->calcInspectorCardWidth(); }
    float neuralInputMargin(NeuralNetEditorWidget const& editor) { return editor->calcInputNodeMargin(); }
    float neuralActionHeight(NeuralNetEditorWidget const& editor, bool dialog)
    {
        return editor->calcActionButtonsHeight(dialog ? _NeuralNetEditorWidget::EditorMode::Dialog : _NeuralNetEditorWidget::EditorMode::Embedded);
    }

    void neuralNodes(NeuralNetEditorWidget const& editor, NeuralNetGenomeDesc& net)
    {
        editor->_labelFont = StyleService::get().getDefaultFont();
        _NeuralNetEditorWidget::LayoutData layout;
        auto start = ImGui::GetCursorScreenPos();
        for (int index = 0; index < NEURAL_NET_INPUTS; ++index) {
            layout.inputNodePos[index] = {start.x + 160, start.y + 20 + index * 24.0f};
        }
        for (int index = 0; index < NEURAL_NET_OUTPUTS; ++index) {
            layout.outputNodePos[index] = {start.x + 350, start.y + 20 + index * 24.0f};
        }
        auto& selection = editor->getValueRef(editor->_dataById);
        editor->drawInputNodes(selection, ImGui::GetWindowDrawList(), layout, std::nullopt);
        editor->drawOutputNodes(net._biases, net._activationFunctions, selection, ImGui::GetWindowDrawList(), layout);
    }

    void neuralCard(NeuralNetEditorWidget const& editor, NeuralNetGenomeDesc& net)
    {
        editor->_labelFont = StyleService::get().getDefaultFont();
        auto& selection = editor->getValueRef(editor->_dataById);
        editor->processInspectorCardContent(
            net._weights, net._biases, net._activationFunctions, selection, editor->calcInspectorCardWidth(), editor->calcInspectorCardHeight());
    }

    void prepareActivity(CreaturePreviewWidget const& editor)
    {
        editor->_selectedCellIdFromPreview = 123;
        editor->_previewDesc._cells = {CellPreviewDesc().id(123).memory({0.1f, 0.2f, -0.3f, 0.4f})};
    }

    void drawActivity(CreaturePreviewWidget const& editor, bool& changed, ContentDesc& phenotype) { editor->processNeuralActivityEditor(changed, phenotype); }

    MutationRatesDesc& mutationDraft(MutationRatesDialog& dialog) { return dialog._mutation; }
    bool mutationOpen(MutationRatesDialog& dialog) { return dialog._modalWindow.isOpen(); }
    void mutationContent(MutationRatesDialog& dialog) { dialog.processContent(); }
    void mutationButtons(MutationRatesDialog& dialog) { dialog.processButtons(); }
    void clearMutationClipboard() { MutationRatesDialog::_clipboard.reset(); }

    SignalsBufferDialog& signalDialog() { return SignalsBufferDialog::get(); }
    void signalContent() { signalDialog().processIntern(); }
    auto& signalDraft() { return signalDialog()._channelsBuffer; }
    int& signalSelection() { return signalDialog()._selectedEntry; }
    void previewTitle(CreaturePreviewWidget const& editor, int count, bool trimmed, int start)
    {
        editor->_subGenome.trimmed = trimmed;
        editor->_subGenome.startIndex = start;
        editor->_previewDesc._cells.resize(count);
        editor->processTitle();
    }

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
    auto other = ParameterGroupSpec().name("Untranslated extension").parameters({ParameterSpec().name("Radius").description("Untranslated description")});
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
    for (auto const& name : {"Untranslated extension", "Another extension"}) {
        auto group = ParameterGroupSpec().name(name).parameters({ParameterSpec().name("Radius").description("Untranslated description")});
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

namespace
{
    std::vector<ParameterSpec const*> metaParameters()
    {
        std::vector<ParameterSpec const*> result;
        for (auto name : {"Meta-mutations: Properties", "Meta-mutations: Cell identity"}) {
            for (auto const& parameter : parameterGroup(SimulationParameters::getSpec(), name)._parameters) {
                result.push_back(&parameter);
            }
        }
        return result;
    }
}
TEST_F(LocalizationGuiTests, l2b4bSigmaFloatsEditAndResetThroughProductionDispatch)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    int count = 0;
    for (auto parameter : metaParameters()) {
        auto number = std::get_if<FloatSpec>(&parameter->_reference);
        if (!number || number->_getterSetter || !parameter->_visible) {
            continue;
        }
        ++count;
        auto constexpr order = 0;
        auto ref = SpecificationEvaluationService::get().getRef(number->_member, *values, order);
        auto original = SpecificationEvaluationService::get().getRef(number->_member, *defaults, order);
        EXPECT_EQ(0.0f, std::get<float>(number->_min));
        EXPECT_EQ(1.0f, std::get<float>(number->_max));
        EXPECT_TRUE(number->_logarithmic);
        EXPECT_EQ("%.5f", number->_format);
        EXPECT_EQ(0.0f, *original.value);
        for (float edited : {0.0f, 0.00001f, 0.99999f}) {
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
    }
    EXPECT_EQ(9, count);
}

TEST_F(LocalizationGuiTests, l2b4bBindingsDescriptionsSearchFallbackAndSerialization)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto all = metaParameters();
    ASSERT_EQ(10, all.size());
    for (int row = 0; row < MAX_COLORS; ++row) {
        for (int col = 0; col < MAX_COLORS; ++col) {
            values->customizationTransitionMatrix.value[row][col] = (row * 7 + col * 3) % 4 == 0;
        }
    }
    for (size_t i = 0; i < 9; ++i) {
        auto const& spec = std::get<FloatSpec>(all.at(i)->_reference);
        *SpecificationEvaluationService::get().getRef(spec._member, *values, 0).value = 0.03125f * (i + 1);
    }
    auto snapshot = std::make_unique<SimulationParameters>(*values);
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        for (auto parameter : all) {
            if (std::string(language) == "en") {
                EXPECT_EQ(parameter->_name, SimulationParametersPresentation::label(*parameter));
                EXPECT_EQ(parameter->_description, SimulationParametersPresentation::description(*parameter));
            } else {
                EXPECT_NE(parameter->_name, SimulationParametersPresentation::label(*parameter));
                EXPECT_NE(parameter->_description, SimulationParametersPresentation::description(*parameter));
            }
            auto renamed = *parameter;
            renamed._name = "Not an identity";
            EXPECT_EQ(SimulationParametersPresentation::label(*parameter), SimulationParametersPresentation::label(renamed));
            EXPECT_EQ(SimulationParametersPresentation::description(*parameter), SimulationParametersPresentation::description(renamed));
            frame([&] { parametersGroup({*parameter}, *values, *defaults, 0); });
            EXPECT_EQ(*snapshot, *values);
            for (auto query : {parameter->_name, SimulationParametersPresentation::label(*parameter)}) {
                auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = query});
                auto found = false;
                for (auto const& group : filtered._groups) {
                    for (auto const& candidate : group._parameters) {
                        found = found || (candidate._visible && candidate._name == parameter->_name);
                    }
                }
                EXPECT_TRUE(found) << query;
            }
        }
        for (auto const& [query, group, count] : std::vector<std::tuple<std::string, std::string, int>>{
                 {"Meta-mutations", "Meta-mutations: Properties", 5},
                 {"Метамутации", "Meta-mutations: Properties", 5},
                 {"Neuron sigma", "Meta-mutations: Properties", 1},
                 {"Нейроны", "Meta-mutations: Properties", 1},
                 {"Cell type", "Meta-mutations: Cell identity", 2},
                 {"Тип клетки", "Meta-mutations: Cell identity", 1},
                 {"Customization", "Meta-mutations: Cell identity", 2},
                 {"Настройка", "Meta-mutations: Cell identity", 1},
                 {"Matrix", "Meta-mutations: Cell identity", 1},
                 {"Матрица", "Meta-mutations: Cell identity", 1}}) {
            auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = query});
            bool russian = std::ranges::any_of(query, [](unsigned char c) { return c >= 128; });
            EXPECT_EQ(russian && std::string(language) == "en" ? 0 : count, visibleParameters(parameterGroup(filtered, group))) << query;
            auto const& actual = parameterGroup(filtered, group)._parameters;
            auto const& original = parameterGroup(SimulationParameters::getSpec(), group)._parameters;
            for (size_t i = 0; i < actual.size(); ++i) {
                EXPECT_EQ(original.at(i)._name, actual.at(i)._name);
            }
        }
    }
    auto file = std::filesystem::temp_directory_path() / "alien-l2b4b-synthetic.settings.json";
    ASSERT_TRUE(SerializerService::get().serializeSimulationParametersToFile(file, *values));
    auto restored = std::make_unique<SimulationParameters>();
    ASSERT_TRUE(SerializerService::get().deserializeSimulationParametersFromFile(*restored, file));
    std::filesystem::remove(file);
    EXPECT_EQ(values->customizationTransitionMatrix, restored->customizationTransitionMatrix);
    for (size_t i = 0; i < 9; ++i) {
        auto const& spec = std::get<FloatSpec>(all.at(i)->_reference);
        EXPECT_EQ(
            *SpecificationEvaluationService::get().getRef(spec._member, *values, 0).value,
            *SpecificationEvaluationService::get().getRef(spec._member, *restored, 0).value);
    }
    LocalizationService::get().load("resources/locales/missing-l2b4b");
    select("ru");
    for (auto parameter : all) {
        EXPECT_EQ(parameter->_name, SimulationParametersPresentation::label(*parameter));
        EXPECT_EQ(parameter->_description, SimulationParametersPresentation::description(*parameter));
    }
    LocalizationService::get().load("resources/locales");
}

TEST_F(LocalizationGuiTests, l2b4bMatrixDialogUsesLegacyIdsEditsDirectedCellsAdoptsCancelsAndResets)
{
    static_assert(MAX_COLORS == 10);
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto& matrix = values->customizationTransitionMatrix.value.values;
    auto const& spec = *metaParameters().back();
    ColorMatrixDialog<bool> dialog;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImGuiID activate = 0, editId = 0, resetId = 0, toggleId = 0;
    auto draw = [&] {
        frame([&] {
            ImGui::PushID(spec._name.c_str());
            editId = ImGui::GetID("Edit matrix");
            toggleId = ImGui::GetID(ICON_FA_PLUS_SQUARE "##toggle");
            resetId = ImGui::GetID((std::string(ICON_FA_UNDO) + "##" + spec._name).c_str());
            ImGui::PopID();
            if (activate) {
                GImGui->NavActivateId = activate;
                GImGui->NavActivateDownId = activate;
                GImGui->NavActivatePressedId = activate;
            }
            std::vector<std::vector<bool>> original;
            for (auto const& row : defaults->customizationTransitionMatrix.value.values) {
                original.emplace_back(std::begin(row), std::end(row));
            }
            AlienGui::CheckboxColorMatrix(
                AlienGui::CheckboxColorMatrixParameters()
                    .name(spec._name)
                    .label(SimulationParametersPresentation::label(spec))
                    .localizationKey("parameters.metamidentity.customizationtransitionmatrix")
                    .rowLabel(tr("parameters.matrix.row"))
                    .columnLabel(tr("parameters.matrix.column"))
                    .textWidth(360)
                    .disableDiagonal(true)
                    .defaultValue(original),
                matrix,
                dialog);
        });
        activate = 0;
    };
    auto press = [&](ImGuiID id) {
        activate = id;
        draw();
        draw();
    };
    draw();
    draw();
    auto originalEdit = editId;
    auto expandedTable = [&] {
        for (int i = 0; i < GImGui->Tables.GetBufSize(); ++i) {
            auto table = GImGui->Tables.GetByIndex(i);
            if (table->OuterWindow == ImGui::FindWindowByName("Host") && table->LastFrameActive == GImGui->FrameCount) {
                return true;
            }
        }
        return false;
    };
    EXPECT_FALSE(expandedTable());
    press(toggleId);
    EXPECT_TRUE(expandedTable());
    press(editId);
    auto popup = ImGui::FindWindowByName(spec._name.c_str());
    ASSERT_NE(nullptr, popup);
    EXPECT_EQ(ImHashStr(spec._name.c_str()), popup->ID);
    auto buttonId = [&](char const* name) {
        auto seed = ImHashStr(spec._name.c_str(), 0, popup->ID);
        return ImHashStr(name, 0, seed);
    };
    auto cellId = [&](int row, int col) {
        for (int i = 0; i < GImGui->Tables.GetBufSize(); ++i) {
            auto table = GImGui->Tables.GetByIndex(i);
            if (table->OuterWindow && table->OuterWindow->ParentWindow == popup) {
                EXPECT_EQ(11, table->ColumnsCount);
                ++row;
                ++col;
                auto seed = ImHashData(&row, sizeof(row), table->ID);
                seed = ImHashData(&col, sizeof(col), seed);
                return ImHashStr(("##" + spec._name).c_str(), 0, seed);
            }
        }
        ADD_FAILURE() << "Missing production matrix table";
        return ImGuiID(0);
    };
    auto snapshot = std::make_unique<SimulationParameters>(*values);
    auto directedId = cellId(2, 7);
    press(directedId);
    EXPECT_EQ(*snapshot, *values);
    press(cellId(4, 4));
    for (auto language : {"ru", "en", "ru"}) {
        select(language);
        draw();
        EXPECT_EQ(originalEdit, editId);
        EXPECT_TRUE(expandedTable());
        EXPECT_EQ(popup, ImGui::FindWindowByName(spec._name.c_str()));
        EXPECT_EQ(directedId, cellId(2, 7));
        EXPECT_EQ(*snapshot, *values);
    }
    press(buttonId("Adopt"));
    for (int row = 0; row < MAX_COLORS; ++row) {
        for (int col = 0; col < MAX_COLORS; ++col) {
            auto expected = snapshot->customizationTransitionMatrix.value[row][col];
            if (row == 2 && col == 7) {
                expected = !expected;
            }
            EXPECT_EQ(expected, matrix[row][col]) << row << "," << col;
        }
    }
    auto adopted = std::make_unique<SimulationParameters>(*values);
    press(editId);
    press(cellId(7, 2));
    press(buttonId("Cancel"));
    EXPECT_EQ(*adopted, *values);
    press(resetId);
    EXPECT_EQ(defaults->customizationTransitionMatrix, values->customizationTransitionMatrix);
}

TEST_F(LocalizationGuiTests, l2b4bMatrixCaptionsAxesAndButtonsRenderAtAllScales)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto spec = *metaParameters().back();
    select("ru");
    for (auto factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (float width : {650.0f, 950.0f}) {
            std::string logged;
            auto render = [&] {
                switcherFrame(scale(width), [&] {
                    ImGui::LogToBuffer();
                    parametersGroup({spec}, *values, *defaults, 0);
                    ColorMatrixDialog<bool> dialog;
                    auto parameters = AlienGui::ExpandedColorMatrixParameters<bool>()
                                          .name(spec._name)
                                          .label(SimulationParametersPresentation::label(spec))
                                          .localizationKey("parameters.metamidentity.customizationtransitionmatrix")
                                          .rowLabel(tr("parameters.matrix.row"))
                                          .columnLabel(tr("parameters.matrix.column"))
                                          .disableDiagonal(true);
                    dialog.open(parameters, values->customizationTransitionMatrix.value.values, [](auto const&) {});
                    if (auto popup = ImGui::FindWindowByName(spec._name.c_str())) {
                        ImGui::SetWindowSize(popup, {scale(width == 650.0f ? 350.0f : 700.0f), scale(500.0f)});
                    }
                    dialog.process();
                    auto popup = ImGui::FindWindowByName(spec._name.c_str());
                    ASSERT_NE(nullptr, popup);
                    EXPECT_LE(GImGui->LastItemData.Rect.Max.x, popup->Pos.x + popup->Size.x);
                    EXPECT_LE(GImGui->LastItemData.Rect.Max.y, popup->Pos.y + popup->Size.y);
                    logged = GImGui->LogBuffer.c_str();
                    ImGui::LogFinish();
                });
            };
            render();
            render();
            for (auto text : {"Новая настройка", "Применить", "Отмена", "Очистить", "Выбрать всё", "Инвертировать", "Случайно"}) {
                EXPECT_NE(std::string::npos, logged.find(text)) << text << ": " << factor << ", " << width;
            }
            EXPECT_EQ(defaults->customizationTransitionMatrix, values->customizationTransitionMatrix);
        }
    }
}

namespace
{
    std::vector<ParameterSpec const*> structureMetaParameters()
    {
        std::vector<ParameterSpec const*> result;
        for (auto name : {"Meta-mutations: Gene structure", "Meta-mutations: Genome structure"}) {
            for (auto const& parameter : parameterGroup(SimulationParameters::getSpec(), name)._parameters) {
                result.push_back(&parameter);
            }
        }
        return result;
    }
}
TEST_F(LocalizationGuiTests, l2b4cSigmaFloatsEditAndResetThroughProductionDispatch)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    int count = 0;
    for (auto parameter : structureMetaParameters()) {
        auto number = std::get_if<FloatSpec>(&parameter->_reference);
        if (!number || number->_getterSetter || !parameter->_visible) {
            continue;
        }
        ++count;
        auto constexpr order = 0;
        auto ref = SpecificationEvaluationService::get().getRef(number->_member, *values, order);
        auto original = SpecificationEvaluationService::get().getRef(number->_member, *defaults, order);
        EXPECT_EQ(0.0f, std::get<float>(number->_min));
        EXPECT_EQ(1.0f, std::get<float>(number->_max));
        EXPECT_TRUE(number->_logarithmic);
        EXPECT_EQ("%.5f", number->_format);
        EXPECT_EQ(0.0f, *original.value);
        for (float edited : {0.0f, 0.00001f, 0.99999f}) {
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
    }
    EXPECT_EQ(10, count);
}


TEST_F(LocalizationGuiTests, l2b4cProductionStructureCaptionsKeepControlsClearAndFullTextOnHover)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    select("ru");
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto spec : structureMetaParameters()) {
            auto const& parameter = *spec;
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

TEST_F(LocalizationGuiTests, l2b4cTypedBindingsDescriptionsSearchFallbackAndPersistence)
{
    std::vector<_FloatMember> members = {
        &SimulationParameters::addNodeMetaMutationsSigma,
        &SimulationParameters::deleteNodeMetaMutationsSigma,
        &SimulationParameters::extendGeneMetaMutationsSigma,
        &SimulationParameters::trimGeneMetaMutationsSigma,
        &SimulationParameters::copyNodeSectionMetaMutationsSigma,
        &SimulationParameters::moveNodeSectionMetaMutationsSigma,
        &SimulationParameters::addGeneMetaMutationsSigma,
        &SimulationParameters::duplicateGeneMetaMutationsSigma,
        &SimulationParameters::deleteGeneMetaMutationsSigma,
        &SimulationParameters::swapGeneMetaMutationsSigma};
    auto all = structureMetaParameters();
    ASSERT_EQ(members.size(), all.size());
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    for (size_t i = 0; i < all.size(); ++i) {
        auto const& number = std::get<FloatSpec>(all.at(i)->_reference);
        EXPECT_EQ(members.at(i), *std::get<FloatMember>(number._member));
        (values.get()->*members.at(i)).value = 0.03125f * (i + 1);
    }
    auto snapshot = std::make_unique<SimulationParameters>(*values);
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        for (auto parameter : all) {
            auto label = SimulationParametersPresentation::label(*parameter);
            auto description = SimulationParametersPresentation::description(*parameter);
            ASSERT_TRUE(description);
            EXPECT_FALSE(description->empty());
            if (std::string(language) == "en") {
                EXPECT_EQ(parameter->_name, label);
                EXPECT_EQ(parameter->_description, description);
            } else {
                EXPECT_NE(parameter->_name, label);
                for (auto phrase :
                     {"Стандартное отклонение", "гауссова изменения", "сохранённой в геноме вероятности", "Apply meta-mutations", "один раз на потомка"}) {
                    EXPECT_NE(std::string::npos, description->find(phrase));
                }
            }
            auto renamed = *parameter;
            renamed._name = "Not an identity";
            EXPECT_EQ(label, SimulationParametersPresentation::label(renamed));
            EXPECT_EQ(description, SimulationParametersPresentation::description(renamed));
            frame([&] { parametersGroup({*parameter}, *values, *defaults, 0); });
            EXPECT_EQ(*snapshot, *values);
            for (auto query : {parameter->_name, label}) {
                auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = query});
                bool found = false;
                for (auto const& group : filtered._groups) {
                    for (auto const& candidate : group._parameters) {
                        found |= candidate._visible && candidate._name == parameter->_name;
                    }
                }
                EXPECT_TRUE(found) << query;
            }
        }
    }
    auto file = std::filesystem::temp_directory_path()
        / ("alien-l2b4c-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".settings.json");
    ASSERT_TRUE(SerializerService::get().serializeSimulationParametersToFile(file, *values));
    auto restored = std::make_unique<SimulationParameters>();
    ASSERT_TRUE(SerializerService::get().deserializeSimulationParametersFromFile(*restored, file));
    std::filesystem::remove(file);
    for (auto member : members) {
        EXPECT_EQ(values.get()->*member, restored.get()->*member);
    }
    LocalizationService::get().load("resources/locales/missing-l2b4c");
    select("ru");
    for (auto parameter : all) {
        EXPECT_EQ(parameter->_name, SimulationParametersPresentation::label(*parameter));
        EXPECT_EQ(parameter->_description, SimulationParametersPresentation::description(*parameter));
    }
    for (auto name : {"Meta-mutations: Gene structure", "Meta-mutations: Genome structure"}) {
        EXPECT_EQ(name, SimulationParametersPresentation::label(parameterGroup(SimulationParameters::getSpec(), name)));
    }
    LocalizationService::get().load("resources/locales");
}

TEST_F(LocalizationGuiTests, l2b4cGroupSearchOrderAndUtf8Highlight)
{
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        for (auto const& [query, groupName, count] : std::vector<std::tuple<std::string, std::string, int>>{
                 {"Meta-mutations: Gene structure", "Meta-mutations: Gene structure", 6},
                 {"мЕТАМУТАЦИИ: сТРУКТУРА гЕНА", "Meta-mutations: Gene structure", 6},
                 {"Meta-mutations: Genome structure", "Meta-mutations: Genome structure", 4},
                 {"мЕТАМУТАЦИИ: сТРУКТУРА гЕНОМА", "Meta-mutations: Genome structure", 4},
                 {"ADD NODE", "Meta-mutations: Gene structure", 1},
                 {"дОБАВЛЕНИЕ уЗЛА", "Meta-mutations: Gene structure", 1},
                 {"EXTEND GENE", "Meta-mutations: Gene structure", 1},
                 {"рАСШИРЕНИЕ гЕНА", "Meta-mutations: Gene structure", 1},
                 {"DELETE NODE", "Meta-mutations: Gene structure", 1},
                 {"уДАЛЕНИЕ уЗЛА", "Meta-mutations: Gene structure", 1},
                 {"TRIM GENE", "Meta-mutations: Gene structure", 1},
                 {"уКОРОЧЕНИЕ гЕНА", "Meta-mutations: Gene structure", 1},
                 {"COPY NODE SECTION", "Meta-mutations: Gene structure", 1},
                 {"кОПИРОВАНИЕ уЧАСТКА", "Meta-mutations: Gene structure", 1},
                 {"MOVE NODE SECTION", "Meta-mutations: Gene structure", 1},
                 {"пЕРЕМЕЩЕНИЕ уЧАСТКА", "Meta-mutations: Gene structure", 1},
                 {"SWAP GENE", "Meta-mutations: Genome structure", 1},
                 {"пЕРЕСТАНОВКА гЕНОВ", "Meta-mutations: Genome structure", 1}}) {
            auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = query});
            bool russian = std::ranges::any_of(query, [](unsigned char c) { return c >= 128; });
            EXPECT_EQ(russian && std::string(language) == "en" ? 0 : count, visibleParameters(parameterGroup(filtered, groupName))) << query;
            auto cleared = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = ""});
            auto const& original = parameterGroup(SimulationParameters::getSpec(), groupName);
            auto const& actual = parameterGroup(cleared, groupName);
            EXPECT_EQ(original._parameters.size(), visibleParameters(actual));
            for (size_t i = 0; i < original._parameters.size(); ++i) {
                EXPECT_EQ(original._parameters.at(i)._name, actual._parameters.at(i)._name);
                EXPECT_EQ(original._parameters.at(i)._name, parameterGroup(filtered, groupName)._parameters.at(i)._name);
            }
        }
    }
    select("ru");
    auto all = structureMetaParameters();
    EXPECT_NE(SimulationParametersPresentation::label(*all.at(0)), SimulationParametersPresentation::label(*all.at(2)));
    EXPECT_NE(SimulationParametersPresentation::label(*all.at(1)), SimulationParametersPresentation::label(*all.at(3)));
    auto label = SimulationParametersPresentation::label(*all.at(4));
    auto match = Utf8Search::find(label, "УЧАСТКА");
    ASSERT_TRUE(match);
    EXPECT_EQ("участка", label.substr(match->begin, match->end - match->begin));
    int normal = 0, highlighted = 0;
    frame([&] {
        auto list = ImGui::GetWindowDrawList();
        auto before = list->VtxBuffer.Size;
        AlienGui::Text(AlienGui::TextParameters().text(label));
        normal = list->VtxBuffer.Size - before;
        before = list->VtxBuffer.Size;
        AlienGui::Text(AlienGui::TextParameters().text(label).highlightedSubString("УЧАСТКА"));
        highlighted = list->VtxBuffer.Size - before;
    });
    EXPECT_GT(highlighted, normal);
}

TEST_F(LocalizationGuiTests, l2b4cGroupExpansionAndCanonicalIdsSurviveLanguageChanges)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (auto name : {"Meta-mutations: Gene structure", "Meta-mutations: Genome structure"}) {
        auto const& group = parameterGroup(SimulationParameters::getSpec(), name);
        ImGuiID originalId = 0;
        ImVec2 header;
        bool open = false;
        auto draw = [&] {
            frame([&] {
                ImGui::PushID(name);
                open = AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(SimulationParametersPresentation::label(group)));
                auto id = ImGui::GetItemID();
                if (!originalId) {
                    originalId = id;
                }
                EXPECT_EQ(originalId, id);
                header = GImGui->LastItemData.Rect.GetCenter();
                if (open) {
                    parametersGroup(group._parameters, *values, *defaults, 0);
                }
                AlienGui::EndTreeNode();
                ImGui::PopID();
            });
        };
        auto click = [&] {
            io.AddMousePosEvent(header.x, header.y);
            draw();
            io.AddMouseButtonEvent(0, true);
            draw();
            io.AddMouseButtonEvent(0, false);
            draw();
        };
        select("en");
        draw();
        draw();
        ASSERT_TRUE(open);
        click();
        ASSERT_FALSE(open);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_FALSE(open);
            EXPECT_EQ(*defaults, *values);
        }
        click();
        ASSERT_TRUE(open);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_TRUE(open);
            EXPECT_EQ(*defaults, *values);
        }
    }
}

namespace
{
    std::vector<ParameterSpec const*> combatParameters()
    {
        std::vector<ParameterSpec const*> result;
        for (auto name : {"Cell type: Attacker", "Cell type: Digestor", "Cell type: Defender", "Cell type: Injector"}) {
            for (auto const& parameter : parameterGroup(SimulationParameters::getSpec(), name)._parameters) {
                result.push_back(&parameter);
            }
        }
        return result;
    }
}

TEST_F(LocalizationGuiTests, l2b4dScalarAndColorFloatsEditResetAndRetainFocus)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    int count = 0;
    for (auto parameter : combatParameters()) {
        auto number = std::get_if<FloatSpec>(&parameter->_reference);
        if (!number || number->_getterSetter || !parameter->_visible || parameter == combatParameters().at(1)) {
            continue;
        }
        ++count;
        auto constexpr order = 0;
        auto ref = SpecificationEvaluationService::get().getRef(number->_member, *values, order);
        auto original = SpecificationEvaluationService::get().getRef(number->_member, *defaults, order);
        for (float edited : {std::get<float>(number->_min), (std::get<float>(number->_min) + std::get<float>(number->_max)) / 2}) {
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
    }
    EXPECT_EQ(11, count);
}


TEST_F(LocalizationGuiTests, l2b4dNumericMatrixEditsAllCellsRetainsDraftIdsAppliesCancelsAndResets)
{
    static_assert(MAX_COLORS == 10);
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto& matrix = values->attackerFoodChainColorMatrix.baseValue.values;
    auto const& spec = *combatParameters().at(1);
    ColorMatrixDialog<float> dialog;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImGuiID activate = 0, editId = 0, resetId = 0, toggleId = 0;
    auto draw = [&] {
        frame([&] {
            ImGui::PushID(spec._name.c_str());
            editId = ImGui::GetID("Edit matrix");
            toggleId = ImGui::GetID(ICON_FA_PLUS_SQUARE "##toggle");
            resetId = ImGui::GetID((std::string(ICON_FA_UNDO) + "##" + spec._name).c_str());
            ImGui::PopID();
            if (activate) {
                GImGui->NavActivateId = activate;
                GImGui->NavActivateDownId = activate;
                GImGui->NavActivatePressedId = activate;
            }
            std::vector<std::vector<float>> original;
            for (auto const& row : defaults->attackerFoodChainColorMatrix.baseValue.values) {
                original.emplace_back(std::begin(row), std::end(row));
            }
            AlienGui::InputFloatColorMatrix(
                AlienGui::InputFloatColorMatrixParameters()
                    .name(spec._name)
                    .label(SimulationParametersPresentation::label(spec))
                    .localizationKey("parameters.attacker.attackerfoodchaincolormatrix")
                    .rowLabel(tr("parameters.matrix.attackerrow"))
                    .columnLabel(tr("parameters.matrix.targetcolumn"))
                    .textWidth(360)
                    .min(0)
                    .max(1)
                    .format("%.2f")
                    .defaultValue(original),
                matrix,
                dialog);
        });
        activate = 0;
    };
    auto press = [&](ImGuiID id) {
        activate = id;
        draw();
        draw();
    };
    draw();
    draw();
    auto originalEdit = editId;
    auto expandedTable = [&] {
        for (int i = 0; i < GImGui->Tables.GetBufSize(); ++i) {
            auto table = GImGui->Tables.GetByIndex(i);
            if (table->OuterWindow == ImGui::FindWindowByName("Host") && table->LastFrameActive == GImGui->FrameCount) {
                return true;
            }
        }
        return false;
    };
    EXPECT_FALSE(expandedTable());
    press(toggleId);
    EXPECT_TRUE(expandedTable());
    press(editId);
    auto popup = ImGui::FindWindowByName(spec._name.c_str());
    ImGui::SetWindowSize(popup, {900, 1100});
    draw();
    draw();
    ASSERT_NE(nullptr, popup);
    EXPECT_EQ(ImHashStr(spec._name.c_str()), popup->ID);
    auto buttonId = [&](char const* name) {
        auto seed = ImHashStr(spec._name.c_str(), 0, popup->ID);
        return ImHashStr(name, 0, seed);
    };
    auto cellId = [&](int row, int col) {
        for (int i = 0; i < GImGui->Tables.GetBufSize(); ++i) {
            auto table = GImGui->Tables.GetByIndex(i);
            if (table->OuterWindow && table->OuterWindow->ParentWindow == popup) {
                EXPECT_EQ(11, table->ColumnsCount);
                ++row;
                ++col;
                auto seed = ImHashData(&row, sizeof(row), table->ID);
                seed = ImHashData(&col, sizeof(col), seed);
                seed = ImHashStr("", 0, seed);
                seed = ImHashStr("", 0, seed);
                int color = 0;
                seed = ImHashData(&color, sizeof(color), seed);
                return ImHashStr("##slider", 0, seed);
            }
        }
        ADD_FAILURE() << "Missing production matrix table";
        return ImGuiID(0);
    };
    auto snapshot = std::make_unique<SimulationParameters>(*values);
    auto directedId = cellId(2, 7);
    auto input = [&](int row, int col, float value) {
        auto id = cellId(row, col);
        ImVec2 pos;
        for (int i = 0; i < GImGui->Tables.GetBufSize(); ++i) {
            auto table = GImGui->Tables.GetByIndex(i);
            if (table->OuterWindow && table->OuterWindow->ParentWindow == popup) {
                auto child = table->OuterWindow;
                auto height = ImGui::GetFrameHeight() + ImGui::GetStyle().CellPadding.y * 2;
                pos = {table->Columns[col + 1].MinX + 8, table->OuterRect.Min.y + height * (row + 1) + height / 2};
                if (pos.y > child->InnerRect.Max.y) {
                    ImGui::SetScrollY(child, child->Scroll.y + pos.y - child->InnerRect.Max.y + height);
                    draw();
                    draw();
                    pos.y -= child->Scroll.y;
                }
            }
        }
        io.AddKeyEvent(ImGuiMod_Ctrl, true);
        io.AddMousePosEvent(pos.x, pos.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        io.AddKeyEvent(ImGuiMod_Ctrl, false);
        draw();
        EXPECT_EQ(id, GImGui->ActiveId) << row << "," << col;
        io.AddInputCharactersUTF8(std::to_string(value).c_str());
        draw();
        io.AddKeyEvent(ImGuiKey_Enter, true);
        draw();
        io.AddKeyEvent(ImGuiKey_Enter, false);
        draw();
    };
    for (int row = 0; row < MAX_COLORS; ++row) {
        for (int col = 0; col < MAX_COLORS; ++col) {
            auto value = row == 9 && col == 9 ? 1.0f : row == 2 && col == 7 ? 0.25f : row == 7 && col == 2 ? 0.75f : float((row * 7 + col * 3) % 101) / 100;
            input(row, col, value);
        }
    }
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        draw();
        EXPECT_EQ(originalEdit, editId);
        EXPECT_TRUE(expandedTable());
        EXPECT_EQ(popup, ImGui::FindWindowByName(spec._name.c_str()));
        EXPECT_EQ(directedId, cellId(2, 7));
        EXPECT_EQ(*snapshot, *values);
    }
    press(buttonId("Adopt"));
    for (int row = 0; row < MAX_COLORS; ++row) {
        for (int col = 0; col < MAX_COLORS; ++col) {
            auto expected = row == 9 && col == 9 ? 1.0f : row == 2 && col == 7 ? 0.25f : row == 7 && col == 2 ? 0.75f : float((row * 7 + col * 3) % 101) / 100;
            EXPECT_FLOAT_EQ(expected, matrix[row][col]) << row << "," << col;
        }
    }
    auto adopted = std::make_unique<SimulationParameters>(*values);
    press(editId);
    input(7, 2, 0.5f);
    press(buttonId("Cancel"));
    EXPECT_EQ(*adopted, *values);
    press(resetId);
    EXPECT_EQ(defaults->customizationTransitionMatrix, values->customizationTransitionMatrix);
}

TEST_F(LocalizationGuiTests, l2b4dTypedBindingsCompleteDescriptionsGroupsSearchFallbackAndLayerPersistence)
{
    auto all = combatParameters();
    ASSERT_EQ(12, all.size());
    std::vector<std::string> prefixes = {
        "parameters.attacker.attackerenergycost",
        "parameters.attacker.attackerfoodchaincolormatrix",
        "parameters.attacker.attackerstrength",
        "parameters.attacker.attackerrelatedlineageprotection",
        "parameters.attacker.attackersizeprotection",
        "parameters.attacker.attackerradius",
        "parameters.digestor.maxrawenergyconductivity",
        "parameters.digestor.maxrawenergyconversion",
        "parameters.defender.defenderantiattackerstrength",
        "parameters.defender.defenderantiinjectorstrength",
        "parameters.injector.injectorenergycost",
        "parameters.injector.injectorradius"};
    auto checkMember = [&]<typename T>(size_t index, T member) {
        auto pointer = std::get_if<std::shared_ptr<T>>(&std::get<FloatSpec>(all.at(index)->_reference)._member);
        ASSERT_NE(nullptr, pointer);
        ASSERT_TRUE(*pointer);
        EXPECT_EQ(member, **pointer);
    };
    checkMember(0, &SimulationParameters::attackerEnergyCost);
    checkMember(1, &SimulationParameters::attackerFoodChainColorMatrix);
    checkMember(2, &SimulationParameters::attackerStrength);
    checkMember(3, &SimulationParameters::attackerRelatedLineageProtection);
    checkMember(4, &SimulationParameters::attackerSizeProtection);
    checkMember(5, &SimulationParameters::attackerRadius);
    checkMember(6, &SimulationParameters::maxRawEnergyConductivity);
    checkMember(7, &SimulationParameters::maxRawEnergyConversion);
    checkMember(8, &SimulationParameters::defenderAntiAttackerStrength);
    checkMember(9, &SimulationParameters::defenderAntiInjectorStrength);
    checkMember(10, &SimulationParameters::injectorEnergyCost);
    checkMember(11, &SimulationParameters::injectorRadius);

    auto values = std::make_unique<SimulationParameters>();
    values->numLayers = 1;
    values->layerOrderNumbers[0] = 1;
    values->attackerEnergyCost.layerValues[0].enabled = true;
    values->attackerFoodChainColorMatrix.layerValues[0].enabled = true;
    values->attackerSizeProtection.layerValues[0].enabled = true;
    for (size_t i = 0; i < all.size(); ++i) {
        auto const& number = std::get<FloatSpec>(all.at(i)->_reference);
        for (int order : {0, 1}) {
            if (order == 1 && !std::holds_alternative<FloatBaseLayerMember>(number._member)
                && !std::holds_alternative<ColorVectorFloatBaseLayerMember>(number._member)
                && !std::holds_alternative<ColorMatrixFloatBaseLayerMember>(number._member)) {
                continue;
            }
            auto ref = SpecificationEvaluationService::get().getRef(number._member, *values, order);
            if (!ref.value) {
                continue;
            }
            auto count = ref.colorDependence == ColorDependence::ColorMatrix ? 100 : ref.colorDependence == ColorDependence::ColorVector ? 10 : 1;
            for (int k = 0; k < count; ++k) {
                ref.value[k] = std::get<float>(number._min) + (std::get<float>(number._max) - std::get<float>(number._min)) * ((k + order) % 9) / 10;
            }
        }
    }
    values->attackerFoodChainColorMatrix.baseValue[2][7] = 0.25f;
    values->attackerFoodChainColorMatrix.baseValue[7][2] = 0.75f;
    auto defaults = std::make_unique<SimulationParameters>(*values);
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        for (size_t i = 0; i < all.size(); ++i) {
            auto parameter = all.at(i);
            auto prefix = prefixes.at(i);
            EXPECT_EQ(tr(prefix + ".name"), SimulationParametersPresentation::label(*parameter));
            EXPECT_EQ(tr(prefix + ".description"), SimulationParametersPresentation::description(*parameter));
            if (std::string(language) == "en") {
                EXPECT_EQ(parameter->_name, SimulationParametersPresentation::label(*parameter));
                EXPECT_EQ(parameter->_description, SimulationParametersPresentation::description(*parameter));
            }
            auto renamed = *parameter;
            renamed._name = "Not a typed identity";
            EXPECT_EQ(SimulationParametersPresentation::label(*parameter), SimulationParametersPresentation::label(renamed));
            EXPECT_EQ(SimulationParametersPresentation::description(*parameter), SimulationParametersPresentation::description(renamed));
            auto const& number = std::get<FloatSpec>(parameter->_reference);
            for (int order : {0, 1}) {
                if (order == 1 && !std::holds_alternative<FloatBaseLayerMember>(number._member)
                    && !std::holds_alternative<ColorVectorFloatBaseLayerMember>(number._member)
                    && !std::holds_alternative<ColorMatrixFloatBaseLayerMember>(number._member)) {
                    continue;
                }
                frame([&] { parametersGroup({*parameter}, *values, *defaults, order); });
                EXPECT_EQ(*defaults, *values);
            }
        }
        for (auto const& name : {"Attacker", "Digestor", "Defender", "Injector"}) {
            auto group = parameterGroup(SimulationParameters::getSpec(), std::string("Cell type: ") + name);
            auto key = std::string(name);
            std::ranges::transform(key, key.begin(), [](unsigned char c) { return char(std::tolower(c)); });
            EXPECT_EQ(tr("parameters." + key + ".name"), SimulationParametersPresentation::label(group));
        }
    }
    select("ru");
    for (auto const& [query, group, count] : std::vector<std::tuple<std::string, std::string, int>>{
             {"Attacker", "Attacker", 6},      {"аТАКУЮЩАЯ", "Attacker", 6},     {"Digestor", "Digestor", 2},         {"Конвертер", "Digestor", 2},
             {"Defender", "Defender", 2},      {"Защитник", "Defender", 2},      {"Injector", "Injector", 2},         {"Инжектор", "Injector", 2},
             {"Пищевая цепь", "Attacker", 0},  {"Сырая энергия", "Digestor", 0}, {"Food chain", "Attacker", 1},       {"пищев", "Attacker", 1},
             {"Raw energy", "Digestor", 2},    {"сырой энергии", "Digestor", 2}, {"Energy cost", "Attacker", 1},      {"Расход энергии", "Injector", 1},
             {"Attack radius", "Attacker", 1}, {"Радиус атаки", "Attacker", 1},  {"Injection radius", "Injector", 1}, {"Радиус инъекции", "Injector", 1}}) {
        auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = query});
        EXPECT_EQ(count, visibleParameters(parameterGroup(filtered, "Cell type: " + group))) << query;
    }
    auto cleared = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = ""});
    for (auto name : {"Attacker", "Digestor", "Defender", "Injector"}) {
        auto canonical = std::string("Cell type: ") + name;
        auto const& group = parameterGroup(cleared, canonical);
        auto const& original = parameterGroup(SimulationParameters::getSpec(), canonical);
        ASSERT_EQ(original._parameters.size(), group._parameters.size());
        for (size_t i = 0; i < group._parameters.size(); ++i) {
            EXPECT_TRUE(group._parameters.at(i)._visible);
            EXPECT_EQ(original._parameters.at(i)._name, group._parameters.at(i)._name);
        }
    }
    auto file = std::filesystem::temp_directory_path()
        / ("alien-l2b4d-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".settings.json");
    ASSERT_TRUE(SerializerService::get().serializeSimulationParametersToFile(file, *values));
    auto restored = std::make_unique<SimulationParameters>();
    ASSERT_TRUE(SerializerService::get().deserializeSimulationParametersFromFile(*restored, file));
    std::filesystem::remove(file);
    for (auto parameter : all) {
        auto const& number = std::get<FloatSpec>(parameter->_reference);
        for (int order : {0, 1}) {
            if (order == 1 && !std::holds_alternative<FloatBaseLayerMember>(number._member)
                && !std::holds_alternative<ColorVectorFloatBaseLayerMember>(number._member)
                && !std::holds_alternative<ColorMatrixFloatBaseLayerMember>(number._member)) {
                continue;
            }
            auto before = SpecificationEvaluationService::get().getRef(number._member, *values, order);
            auto after = SpecificationEvaluationService::get().getRef(number._member, *restored, order);
            if (!before.value) {
                continue;
            }
            ASSERT_NE(nullptr, after.value);
            auto count = before.colorDependence == ColorDependence::ColorMatrix ? 100 : before.colorDependence == ColorDependence::ColorVector ? 10 : 1;
            for (int i = 0; i < count; ++i) {
                EXPECT_NEAR(before.value[i], after.value[i], 0.0001f);
            }
            if (before.enabled) {
                EXPECT_EQ(*before.enabled, *after.enabled);
            }
        }
    }
    LocalizationService::get().load("resources/locales/missing-l2b4d");
    select("ru");
    for (auto parameter : all) {
        EXPECT_EQ(parameter->_name, SimulationParametersPresentation::label(*parameter));
        EXPECT_EQ(parameter->_description, SimulationParametersPresentation::description(*parameter));
    }
    for (auto name : {"Attacker", "Digestor", "Defender", "Injector"}) {
        auto canonical = std::string("Cell type: ") + name;
        EXPECT_EQ(canonical, SimulationParametersPresentation::label(parameterGroup(SimulationParameters::getSpec(), canonical)));
    }
    LocalizationService::get().load("resources/locales");
}

TEST_F(LocalizationGuiTests, l2b4dCombatCaptionsKeepControlsClearAndFullTextOnHover)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    select("ru");
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto spec : combatParameters()) {
            auto const& parameter = *spec;
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

TEST_F(LocalizationGuiTests, l2b4dNumericMatrixCaptionsAxesButtonsAndPrecisionAtAllScales)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto spec = *combatParameters().at(1);
    select("ru");
    for (auto factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (float width : {650.0f, 950.0f}) {
            std::string logged;
            auto render = [&] {
                switcherFrame(scale(width), [&] {
                    ImGui::LogToBuffer();
                    parametersGroup({spec}, *values, *defaults, 0);
                    ColorMatrixDialog<float> dialog;
                    auto parameters = AlienGui::ExpandedColorMatrixParameters<float>()
                                          .name(spec._name)
                                          .label(SimulationParametersPresentation::label(spec))
                                          .localizationKey("parameters.attacker.attackerfoodchaincolormatrix")
                                          .rowLabel(tr("parameters.matrix.attackerrow"))
                                          .columnLabel(tr("parameters.matrix.targetcolumn"))
                                          .min(0)
                                          .max(1)
                                          .format("%.2f");
                    dialog.open(parameters, values->attackerFoodChainColorMatrix.baseValue.values, [](auto const&) {});
                    if (auto popup = ImGui::FindWindowByName(spec._name.c_str())) {
                        ImGui::SetWindowSize(popup, {scale(width == 650.0f ? 350.0f : 700.0f), scale(500.0f)});
                    }
                    dialog.process();
                    auto popup = ImGui::FindWindowByName(spec._name.c_str());
                    ASSERT_NE(nullptr, popup);
                    EXPECT_LE(GImGui->LastItemData.Rect.Max.x, popup->Pos.x + popup->Size.x);
                    EXPECT_LE(GImGui->LastItemData.Rect.Max.y, popup->Pos.y + popup->Size.y);
                    logged = GImGui->LogBuffer.c_str();
                    ImGui::LogFinish();
                });
            };
            render();
            render();
            for (auto text : {"Цвет цели", "Применить", "Отмена", "1.00"}) {
                EXPECT_NE(std::string::npos, logged.find(text)) << text << ": " << factor << ", " << width;
            }
            EXPECT_EQ(defaults->customizationTransitionMatrix, values->customizationTransitionMatrix);
        }
    }
}

TEST_F(LocalizationGuiTests, l2b4dCombatColorVectorsExpandEditResetAndRetainState)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (size_t index : {0u, 3u, 5u, 6u, 7u, 8u, 9u, 10u, 11u}) {
        auto const& parameter = *combatParameters().at(index);
        auto const& number = std::get<FloatSpec>(parameter._reference);
        auto ref = SpecificationEvaluationService::get().getRef(number._member, *values, 0);
        auto original = SpecificationEvaluationService::get().getRef(number._member, *defaults, 0);
        ASSERT_EQ(ColorDependence::ColorVector, ref.colorDependence);
        for (int color = 0; color < MAX_COLORS; ++color) {
            ref.value[color] = std::get<float>(number._min) + (std::get<float>(number._max) - std::get<float>(number._min)) * color / 11;
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
        float edited = (std::get<float>(number._min) + std::get<float>(number._max)) / 2;
        io.AddInputCharactersUTF8(std::to_string(edited).c_str());
        draw();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_EQ(active, GImGui->ActiveId);
            for (int color = 0; color < MAX_COLORS; ++color) {
                EXPECT_FLOAT_EQ(
                    color == 2 ? edited : std::get<float>(number._min) + (std::get<float>(number._max) - std::get<float>(number._min)) * color / 11,
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
TEST_F(LocalizationGuiTests, l2b4dCombatGroupsRetainExpansionAndCanonicalIds)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (auto name : {"Cell type: Attacker", "Cell type: Digestor", "Cell type: Defender", "Cell type: Injector"}) {
        auto const& group = parameterGroup(SimulationParameters::getSpec(), name);
        ImGuiID originalId = 0;
        ImVec2 header;
        bool open = false;
        auto draw = [&] {
            frame([&] {
                ImGui::PushID(name);
                open = AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(SimulationParametersPresentation::label(group)));
                auto id = ImGui::GetItemID();
                if (!originalId) {
                    originalId = id;
                }
                EXPECT_EQ(originalId, id);
                header = GImGui->LastItemData.Rect.GetCenter();
                if (open) {
                    parametersGroup(group._parameters, *values, *defaults, 0);
                }
                AlienGui::EndTreeNode();
                ImGui::PopID();
            });
        };
        auto click = [&] {
            io.AddMousePosEvent(header.x, header.y);
            draw();
            io.AddMouseButtonEvent(0, true);
            draw();
            io.AddMouseButtonEvent(0, false);
            draw();
        };
        select("en");
        draw();
        draw();
        ASSERT_TRUE(open);
        click();
        ASSERT_FALSE(open);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_FALSE(open);
            EXPECT_EQ(*defaults, *values);
        }
        click();
        ASSERT_TRUE(open);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_TRUE(open);
            EXPECT_EQ(*defaults, *values);
        }
    }
}

TEST_F(LocalizationGuiTests, l2b4dLayerMatrixUsesProductionDispatcherAndRetainsBaseValues)
{
    static_assert(MAX_COLORS == 10);
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    values->numLayers = defaults->numLayers = 1;
    values->layerOrderNumbers[0] = defaults->layerOrderNumbers[0] = 1;
    values->attackerFoodChainColorMatrix.layerValues[0].enabled = true;
    defaults->attackerFoodChainColorMatrix.layerValues[0].enabled = true;
    auto& matrix = values->attackerFoodChainColorMatrix.layerValues[0].value.values;
    auto const& spec = *combatParameters().at(1);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImGuiID activate = 0, editId = 0, resetId = 0, toggleId = 0;
    auto draw = [&] {
        frame([&] {
            ImGui::PushID(0);
            ImGui::PushID(spec._name.c_str());
            editId = ImGui::GetID("Edit matrix");
            toggleId = ImGui::GetID(ICON_FA_PLUS_SQUARE "##toggle");
            resetId = ImGui::GetID((std::string(ICON_FA_UNDO) + "##" + spec._name).c_str());
            ImGui::PopID();
            ImGui::PopID();
            if (activate) {
                GImGui->NavActivateId = activate;
                GImGui->NavActivateDownId = activate;
                GImGui->NavActivatePressedId = activate;
            }
            parametersGroup({spec}, *values, *defaults, 1);
        });
        activate = 0;
    };
    auto press = [&](ImGuiID id) {
        activate = id;
        draw();
        draw();
    };
    draw();
    draw();
    auto originalEdit = editId;
    auto expandedTable = [&] {
        for (int i = 0; i < GImGui->Tables.GetBufSize(); ++i) {
            auto table = GImGui->Tables.GetByIndex(i);
            if (table->OuterWindow == ImGui::FindWindowByName("Host") && table->LastFrameActive == GImGui->FrameCount) {
                return true;
            }
        }
        return false;
    };
    EXPECT_FALSE(expandedTable());
    press(toggleId);
    EXPECT_TRUE(expandedTable());
    press(editId);
    auto popup = ImGui::FindWindowByName(spec._name.c_str());
    ImGui::SetWindowSize(popup, {900, 1100});
    draw();
    draw();
    ASSERT_NE(nullptr, popup);
    EXPECT_EQ(ImHashStr(spec._name.c_str()), popup->ID);
    auto buttonId = [&](char const* name) {
        auto seed = ImHashStr(spec._name.c_str(), 0, popup->ID);
        return ImHashStr(name, 0, seed);
    };
    auto cellId = [&](int row, int col) {
        for (int i = 0; i < GImGui->Tables.GetBufSize(); ++i) {
            auto table = GImGui->Tables.GetByIndex(i);
            if (table->OuterWindow && table->OuterWindow->ParentWindow == popup) {
                EXPECT_EQ(11, table->ColumnsCount);
                ++row;
                ++col;
                auto seed = ImHashData(&row, sizeof(row), table->ID);
                seed = ImHashData(&col, sizeof(col), seed);
                seed = ImHashStr("", 0, seed);
                seed = ImHashStr("", 0, seed);
                int color = 0;
                seed = ImHashData(&color, sizeof(color), seed);
                return ImHashStr("##slider", 0, seed);
            }
        }
        ADD_FAILURE() << "Missing production matrix table";
        return ImGuiID(0);
    };
    auto snapshot = std::make_unique<SimulationParameters>(*values);
    auto directedId = cellId(2, 7);
    auto input = [&](int row, int col, float value) {
        auto id = cellId(row, col);
        ImVec2 pos;
        for (int i = 0; i < GImGui->Tables.GetBufSize(); ++i) {
            auto table = GImGui->Tables.GetByIndex(i);
            if (table->OuterWindow && table->OuterWindow->ParentWindow == popup) {
                auto child = table->OuterWindow;
                auto height = ImGui::GetFrameHeight() + ImGui::GetStyle().CellPadding.y * 2;
                pos = {table->Columns[col + 1].MinX + 8, table->OuterRect.Min.y + height * (row + 1) + height / 2};
                if (pos.y > child->InnerRect.Max.y) {
                    ImGui::SetScrollY(child, child->Scroll.y + pos.y - child->InnerRect.Max.y + height);
                    draw();
                    draw();
                    pos.y -= child->Scroll.y;
                }
            }
        }
        io.AddKeyEvent(ImGuiMod_Ctrl, true);
        io.AddMousePosEvent(pos.x, pos.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        io.AddKeyEvent(ImGuiMod_Ctrl, false);
        draw();
        EXPECT_EQ(id, GImGui->ActiveId) << row << "," << col;
        io.AddInputCharactersUTF8(std::to_string(value).c_str());
        draw();
        io.AddKeyEvent(ImGuiKey_Enter, true);
        draw();
        io.AddKeyEvent(ImGuiKey_Enter, false);
        draw();
    };
    for (int row = 0; row < MAX_COLORS; ++row) {
        for (int col = 0; col < MAX_COLORS; ++col) {
            auto value = row == 9 && col == 9 ? 1.0f : row == 2 && col == 7 ? 0.25f : row == 7 && col == 2 ? 0.75f : float((row * 7 + col * 3) % 101) / 100;
            input(row, col, value);
        }
    }
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        draw();
        EXPECT_EQ(originalEdit, editId);
        EXPECT_TRUE(expandedTable());
        EXPECT_EQ(popup, ImGui::FindWindowByName(spec._name.c_str()));
        EXPECT_EQ(directedId, cellId(2, 7));
        EXPECT_EQ(*snapshot, *values);
    }
    press(buttonId("Adopt"));
    for (int row = 0; row < MAX_COLORS; ++row) {
        for (int col = 0; col < MAX_COLORS; ++col) {
            auto expected = row == 9 && col == 9 ? 1.0f : row == 2 && col == 7 ? 0.25f : row == 7 && col == 2 ? 0.75f : float((row * 7 + col * 3) % 101) / 100;
            EXPECT_FLOAT_EQ(expected, matrix[row][col]) << row << "," << col;
        }
    }
    auto adopted = std::make_unique<SimulationParameters>(*values);
    press(editId);
    input(7, 2, 0.5f);
    press(buttonId("Cancel"));
    EXPECT_EQ(*adopted, *values);
    press(resetId);
    EXPECT_EQ(defaults->customizationTransitionMatrix, values->customizationTransitionMatrix);
}
TEST_F(LocalizationGuiTests, l2b4dProductionHelpMarkersShowCompleteDescriptionsInBothLanguages)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto& io = ImGui::GetIO();
    for (auto factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto language : {"en", "ru"}) {
            select(language);
            for (float width : {650.0f, 950.0f}) {
                for (auto parameter : combatParameters()) {
                    ImVec2 marker;
                    std::string logged;
                    auto draw = [&] {
                        switcherFrame(width * factor, [&] {
                            ImGui::LogToBuffer();
                            parametersGroup({*parameter}, *values, *defaults, 0);
                            marker = GImGui->LastItemData.Rect.GetCenter();
                            logged = GImGui->LogBuffer.c_str();
                            ImGui::LogFinish();
                        });
                    };
                    io.AddMousePosEvent(-100, -100);
                    draw();
                    draw();
                    io.AddMousePosEvent(marker.x, marker.y);
                    draw();
                    draw();
                    auto description = *SimulationParametersPresentation::description(*parameter);
                    size_t start = 0;
                    while (start < description.size()) {
                        auto end = description.find('\n', start);
                        auto line = description.substr(start, end == std::string::npos ? end : end - start);
                        if (!line.empty()) {
                            EXPECT_NE(std::string::npos, logged.find(line)) << parameter->_name << ": " << language << ": " << width;
                        }
                        if (end == std::string::npos) {
                            break;
                        }
                        start = end + 1;
                    }
                }
            }
        }
    }
    EXPECT_EQ(*defaults, *values);
}

namespace
{
    std::vector<ParameterSpec const*> finalCellParameters()
    {
        std::vector<ParameterSpec const*> result;
        for (auto name : {"Cell type: Muscle", "Cell type: Sensor", "Cell type: Reconnector", "Cell type: Detonator"}) {
            for (auto const& parameter : parameterGroup(SimulationParameters::getSpec(), name)._parameters) {
                result.push_back(&parameter);
            }
        }
        return result;
    }
}

TEST_F(LocalizationGuiTests, l2b4eFinalCellColorVectorsExpandEditResetAndRetainState)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (size_t index : {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u}) {
        auto const& parameter = *finalCellParameters().at(index);
        auto const& number = std::get<FloatSpec>(parameter._reference);
        auto ref = SpecificationEvaluationService::get().getRef(number._member, *values, 0);
        auto original = SpecificationEvaluationService::get().getRef(number._member, *defaults, 0);
        ASSERT_EQ(ColorDependence::ColorVector, ref.colorDependence);
        for (int color = 0; color < MAX_COLORS; ++color) {
            ref.value[color] = std::get<float>(number._min) + (std::get<float>(number._max) - std::get<float>(number._min)) * color / 11;
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
        float edited = (std::get<float>(number._min) + std::get<float>(number._max)) / 2;
        io.AddInputCharactersUTF8(std::to_string(edited).c_str());
        draw();
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_EQ(active, GImGui->ActiveId);
            for (int color = 0; color < MAX_COLORS; ++color) {
                EXPECT_FLOAT_EQ(
                    color == 2 ? edited : std::get<float>(number._min) + (std::get<float>(number._max) - std::get<float>(number._min)) * color / 11,
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
TEST_F(LocalizationGuiTests, l2b4eFinalCellGroupsRetainExpansionAndCanonicalIds)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (auto name : {"Cell type: Muscle", "Cell type: Sensor", "Cell type: Reconnector", "Cell type: Detonator"}) {
        auto const& group = parameterGroup(SimulationParameters::getSpec(), name);
        ImGuiID originalId = 0;
        ImVec2 header;
        bool open = false;
        auto draw = [&] {
            frame([&] {
                ImGui::PushID(name);
                open = AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters().name(name).label(SimulationParametersPresentation::label(group)));
                auto id = ImGui::GetItemID();
                if (!originalId) {
                    originalId = id;
                }
                EXPECT_EQ(originalId, id);
                header = GImGui->LastItemData.Rect.GetCenter();
                if (open) {
                    parametersGroup(group._parameters, *values, *defaults, 0);
                }
                AlienGui::EndTreeNode();
                ImGui::PopID();
            });
        };
        auto click = [&] {
            io.AddMousePosEvent(header.x, header.y);
            draw();
            io.AddMouseButtonEvent(0, true);
            draw();
            io.AddMouseButtonEvent(0, false);
            draw();
        };
        select("en");
        draw();
        draw();
        ASSERT_TRUE(open);
        click();
        ASSERT_FALSE(open);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_FALSE(open);
            EXPECT_EQ(*defaults, *values);
        }
        click();
        ASSERT_TRUE(open);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_TRUE(open);
            EXPECT_EQ(*defaults, *values);
        }
    }
}

TEST_F(LocalizationGuiTests, l2b4eFinalCellCaptionsKeepControlsClearAndFullTextOnHover)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>(*values);
    auto& io = ImGui::GetIO();
    select("ru");
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto spec : finalCellParameters()) {
            auto const& parameter = *spec;
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

TEST_F(LocalizationGuiTests, l2b4eProductionHelpMarkersShowCompleteDescriptionsInBothLanguages)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto& io = ImGui::GetIO();
    for (auto factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto language : {"en", "ru"}) {
            select(language);
            for (float width : {650.0f, 950.0f}) {
                for (auto parameter : finalCellParameters()) {
                    ImVec2 marker;
                    std::string logged;
                    auto draw = [&] {
                        switcherFrame(width * factor, [&] {
                            ImGui::LogToBuffer();
                            parametersGroup({*parameter}, *values, *defaults, 0);
                            marker = GImGui->LastItemData.Rect.GetCenter();
                            logged = GImGui->LogBuffer.c_str();
                            ImGui::LogFinish();
                        });
                    };
                    io.AddMousePosEvent(-100, -100);
                    draw();
                    draw();
                    io.AddMousePosEvent(marker.x, marker.y);
                    draw();
                    draw();
                    auto description = *SimulationParametersPresentation::description(*parameter);
                    size_t start = 0;
                    while (start < description.size()) {
                        auto end = description.find('\n', start);
                        auto line = description.substr(start, end == std::string::npos ? end : end - start);
                        if (!line.empty()) {
                            EXPECT_NE(std::string::npos, logged.find(line)) << parameter->_name << ": " << language << ": " << width;
                        }
                        if (end == std::string::npos) {
                            break;
                        }
                        start = end + 1;
                    }
                }
            }
        }
    }
    EXPECT_EQ(*defaults, *values);
}
TEST_F(LocalizationGuiTests, l2b4eTypedBindingsRangesDefaultsSearchFallbackAndPersistence)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto all = finalCellParameters();
    std::vector<std::string> prefixes{
        "muscle.muscleenergycost",
        "muscle.musclemovementacceleration",
        "muscle.musclecrawlingacceleration",
        "muscle.musclebendingacceleration",
        "sensor.sensorradius",
        "reconnector.reconnectorradius",
        "detonator.detonatorradius",
        "detonator.detonatorchainexplosionprobability"};
    std::vector<float> minima{0, 0, 0, 0, 10, 0, 0, 0}, maxima{5, 10, 10, 10, 800, 3, 10, 1}, initial{0, 1, 1, 1, 255, 2, 10, 0.2f};
    std::vector<decltype(&SimulationParameters::muscleEnergyCost)> members{
        &SimulationParameters::muscleEnergyCost,
        &SimulationParameters::muscleMovementAcceleration,
        &SimulationParameters::muscleCrawlingAcceleration,
        &SimulationParameters::muscleBendingAcceleration,
        &SimulationParameters::sensorRadius,
        &SimulationParameters::reconnectorRadius,
        &SimulationParameters::detonatorRadius,
        &SimulationParameters::detonatorChainExplosionProbability};
    for (size_t i = 0; i < all.size(); ++i) {
        auto const& number = std::get<FloatSpec>(all.at(i)->_reference);
        auto pointer = std::get<ColorVectorFloatMember>(number._member);
        ASSERT_TRUE(pointer);
        EXPECT_EQ(members.at(i), *pointer);
        EXPECT_FLOAT_EQ(minima.at(i), std::get<float>(number._min));
        EXPECT_FLOAT_EQ(maxima.at(i), std::get<float>(number._max));
        EXPECT_EQ(i < 4, number._logarithmic);
        EXPECT_EQ(i == 0 ? "%.5f" : "%.3f", number._format);
        auto ref = SpecificationEvaluationService::get().getRef(number._member, *values, 0);
        ASSERT_EQ(ColorDependence::ColorVector, ref.colorDependence);
        for (int color = 0; color < MAX_COLORS; ++color) {
            EXPECT_FLOAT_EQ(initial.at(i), ref.value[color]);
            ref.value[color] = minima.at(i) + (maxima.at(i) - minima.at(i)) * color / 11;
        }
    }
    auto const& transitionGroup = parameterGroup(SimulationParameters::getSpec(), "Object color transition rules");
    auto transition = transitionGroup._parameters.at(0);
    ASSERT_EQ(
        &SimulationParameters::colorTransitionRules,
        *std::get<ColorTransitionRulesBaseLayerMember>(std::get<ColorTransitionRulesSpec>(transition._reference)._member));
    values->numLayers = 1;
    values->layerOrderNumbers[0] = 1;
    values->colorTransitionRulesToggle.value = true;
    values->colorTransitionRules.layerValues[0].enabled = true;
    for (int color = 0; color < MAX_COLORS; ++color) {
        values->colorTransitionRules.baseValue[color] = {.duration = color == 9 ? Infinity<int>::value : 100 + color, .targetColor = (color + 2) % MAX_COLORS};
        values->colorTransitionRules.layerValues[0].value[color] = {.duration = 500 + color, .targetColor = (color + 4) % MAX_COLORS};
    }
    auto snapshot = std::make_unique<SimulationParameters>(*values);
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        for (size_t i = 0; i < all.size(); ++i) {
            auto parameter = *all.at(i);
            auto prefix = "parameters." + prefixes.at(i);
            EXPECT_EQ(tr(prefix + ".name"), SimulationParametersPresentation::label(parameter));
            EXPECT_EQ(tr(prefix + ".description"), SimulationParametersPresentation::description(parameter));
            if (std::string(language) == "en") {
                EXPECT_EQ(parameter._name, SimulationParametersPresentation::label(parameter));
                EXPECT_EQ(parameter._description, SimulationParametersPresentation::description(parameter));
            }
            parameter._name = "Radius and Energy cost are not identities";
            EXPECT_EQ(tr(prefix + ".name"), SimulationParametersPresentation::label(parameter));
            frame([&] { parametersGroup({*all.at(i)}, *values, *defaults, 0); });
            EXPECT_EQ(*snapshot, *values);
        }
        for (int order : {0, 1}) {
            frame([&] { parametersGroup({transition}, *values, *snapshot, order); });
            EXPECT_EQ(*snapshot, *values);
        }
        transition._name = "Unrelated name";
        EXPECT_EQ(tr("parameters.colortransition.colortransitionrules.name"), SimulationParametersPresentation::label(transition));
    }
    select("ru");
    for (auto const& [query, group, count] : std::vector<std::tuple<std::string, std::string, int>>{
             {"Muscle", "Cell type: Muscle", 4},
             {"Мышца", "Cell type: Muscle", 4},
             {"Sensor", "Cell type: Sensor", 1},
             {"Сенсор", "Cell type: Sensor", 1},
             {"Reconnector", "Cell type: Reconnector", 1},
             {"Соединитель", "Cell type: Reconnector", 1},
             {"Detonator", "Cell type: Detonator", 2},
             {"Детонатор", "Cell type: Detonator", 2},
             {"Radius", "Cell type: Sensor", 1},
             {"Radius", "Cell type: Reconnector", 1},
             {"Радиус сканирования", "Cell type: Sensor", 1},
             {"Радиус соединения", "Cell type: Reconnector", 1},
             {"Радиус сканирования", "Cell type: Reconnector", 0},
             {"Energy cost", "Cell type: Muscle", 1},
             {"Расход энергии", "Cell type: Attacker", 1},
             {"Расход энергии", "Cell type: Injector", 1},
             {"Object color transition rules", "Object color transition rules", 1},
             {"Правила переходов цветов объектов", "Object color transition rules", 1},
             {"Expert settings: Object color transition rules", "Object color transition rules", 1},
             {"Экспертные настройки: Правила переходов цветов объектов", "Object color transition rules", 1},
             {"Target color and duration", "Object color transition rules", 1},
             {"цЕЛЕВОЙ ЦВЕТ И ДЛИТЕЛЬНОСТЬ", "Object color transition rules", 1}}) {
        auto filtered = SimulationParametersPresentation::filter(SimulationParameters::getSpec(), {.containedText = query});
        EXPECT_EQ(count, visibleParameters(parameterGroup(filtered, group))) << query;
        EXPECT_EQ(*snapshot, *values);
    }
    auto file = std::filesystem::temp_directory_path()
        / ("alien-l2b4e-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".settings.json");
    ASSERT_TRUE(SerializerService::get().serializeSimulationParametersToFile(file, *values));
    auto restored = std::make_unique<SimulationParameters>();
    ASSERT_TRUE(SerializerService::get().deserializeSimulationParametersFromFile(*restored, file));
    std::filesystem::remove(file);
    EXPECT_EQ(values->colorTransitionRules, restored->colorTransitionRules);
    EXPECT_EQ(values->colorTransitionRulesToggle, restored->colorTransitionRulesToggle);
    for (auto member : members) {
        for (int color = 0; color < MAX_COLORS; ++color) {
            EXPECT_NEAR(((*values).*member).value[color], ((*restored).*member).value[color], 0.0001f);
        }
    }
    LocalizationService::get().load("resources/locales/missing-l2b4e");
    select("ru");
    all.push_back(&transitionGroup._parameters.at(0));
    for (auto parameter : all) {
        EXPECT_EQ(parameter->_name, SimulationParametersPresentation::label(*parameter));
        EXPECT_EQ(parameter->_description, SimulationParametersPresentation::description(*parameter));
    }
    EXPECT_EQ(transitionGroup._name, SimulationParametersPresentation::label(transitionGroup));
    EXPECT_EQ(transitionGroup._description, SimulationParametersPresentation::description(transitionGroup));
    EXPECT_EQ("Expert settings: " + transitionGroup._name, SimulationParametersPresentation::heading(transitionGroup));
    LocalizationService::get().load("resources/locales");
}

TEST_F(LocalizationGuiTests, l2b4eExpertToggleAndExpandedGroupUseCanonicalIdsAndRetainRules)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto const& group = parameterGroup(SimulationParameters::getSpec(), "Object color transition rules");
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImVec2 toggle, marker;
    ImGuiID expected = 0;
    std::string logged;
    auto drawToggle = [&] {
        frame([&] {
            auto start = ImGui::GetCursorScreenPos();
            toggle = {start.x + ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().FramePadding.x + 8, start.y + ImGui::GetFrameHeight() / 2};
            ImGui::PushID(group._name.c_str());
            ImGui::PushID(0);
            expected = ImGui::GetID("##checkbox");
            ImGui::PopID();
            ImGui::PopID();
            ImGui::LogToBuffer();
            SpecificationGuiService::get().createWidgetsForExpertToggles(*values, *defaults);
            marker = GImGui->LastItemData.Rect.GetCenter();
            logged = GImGui->LogBuffer.c_str();
            ImGui::LogFinish();
        });
    };
    auto clickToggle = [&] {
        io.AddMousePosEvent(toggle.x, toggle.y);
        drawToggle();
        io.AddMouseButtonEvent(0, true);
        drawToggle();
        EXPECT_EQ(expected, GImGui->ActiveId);
        io.AddMouseButtonEvent(0, false);
        drawToggle();
    };
    drawToggle();
    drawToggle();
    clickToggle();
    ASSERT_TRUE(values->colorTransitionRulesToggle.value);
    values->colorTransitionRules.baseValue[2] = {.duration = 123, .targetColor = 7};
    values->colorTransitionRules.baseValue[7] = {.duration = 987, .targetColor = 2};
    auto snapshot = std::make_unique<SimulationParameters>(*values);
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        drawToggle();
        EXPECT_NE(std::string::npos, logged.find(SimulationParametersPresentation::label(group)));
        io.AddMousePosEvent(marker.x, marker.y);
        drawToggle();
        drawToggle();
        EXPECT_NE(std::string::npos, logged.find(*SimulationParametersPresentation::description(group)));
        frame([&] {
            ImGui::LogToBuffer();
            SpecificationGuiService::get().createWidgetsForParameters(*values, *defaults, 0, {.containedText = group._name});
            auto window = ImGui::GetCurrentWindow();
            ImGui::PushOverrideID(ImGui::GetID("##"));
            ImGui::PushID(("Expert settings: " + group._name).c_str());
            auto scope = ImGui::GetID(("Expert settings: " + group._name).c_str());
            ImGui::PushID(scope);
            auto id = ImGui::GetID(("Expert settings: " + group._name).c_str());
            EXPECT_EQ(1, window->DC.StateStorage->GetInt(id, -1));
            ImGui::PopID();
            ImGui::PopID();
            ImGui::PopID();
            auto output = std::string(GImGui->LogBuffer.c_str());
            EXPECT_NE(std::string::npos, output.find(SimulationParametersPresentation::heading(group)));
            EXPECT_NE(std::string::npos, output.find(SimulationParametersPresentation::label(group._parameters.at(0))));
            ImGui::LogFinish();
        });
        EXPECT_EQ(*snapshot, *values);
    }
    drawToggle();
    drawToggle();
    clickToggle();
    EXPECT_FALSE(values->colorTransitionRulesToggle.value);
    EXPECT_EQ(snapshot->colorTransitionRules, values->colorTransitionRules);
    clickToggle();
    EXPECT_EQ(*snapshot, *values);
}

TEST_F(LocalizationGuiTests, l2b4eTenTransitionRowsEditTargetsDurationsInfinityAndResetIndependently)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto const& parameter = parameterGroup(SimulationParameters::getSpec(), "Object color transition rules")._parameters.at(0);
    values->numLayers = defaults->numLayers = 1;
    values->layerOrderNumbers[0] = defaults->layerOrderNumbers[0] = 1;
    values->colorTransitionRules.layerValues[0].enabled = true;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (int order : {0, 1}) {
        auto rules = order == 0 ? &values->colorTransitionRules.baseValue : &values->colorTransitionRules.layerValues[0].value;
        auto otherBefore = order == 0 ? values->colorTransitionRules.layerValues[0].value : values->colorTransitionRules.baseValue;
        ImVec2 start;
        float rowHeight = 0, right = 0;
        std::string logged;
        std::vector<ImGuiID> ids;
        auto draw = [&] {
            frame([&] {
                start = ImGui::GetCursorScreenPos();
                right = start.x + ImGui::GetContentRegionAvail().x - scale(360);
                rowHeight = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
                ImGui::LogToBuffer();
                parametersGroup({parameter}, *values, *defaults, order);
                logged = GImGui->LogBuffer.c_str();
                ImGui::LogFinish();
                EXPECT_NEAR(MAX_COLORS * rowHeight, ImGui::GetCursorScreenPos().y - start.y, 1);
                std::set<ImGuiID> unique;
                for (int row = 0; row < MAX_COLORS; ++row) {
                    ImGui::PushID(0);
                    ImGui::PushID(row);
                    ImGui::PushID(row);
                    ImGui::PushID(2);
                    auto id = ImGui::GetID(row == 0 ? "##Target color and duration" : "##");
                    unique.insert(id);
                    if (ids.size() < MAX_COLORS) {
                        ids.push_back(id);
                    }
                    EXPECT_EQ(ids.at(row), id);
                    ImGui::PopID();
                    ImGui::PopID();
                    ImGui::PopID();
                    ImGui::PopID();
                }
                EXPECT_EQ(MAX_COLORS, unique.size());
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
        EXPECT_NE(std::string::npos, logged.find("Infinity"));
        for (int row : {0, 2, 7}) {
            auto comboX = start.x + scale(30) + 2 * ImGui::GetStyle().ItemSpacing.x + ImGui::CalcTextSize(ICON_FA_LONG_ARROW_ALT_RIGHT).x + scale(35);
            click({comboX, start.y + row * rowHeight + ImGui::GetFrameHeight() / 2});
            ASSERT_GT(GImGui->OpenPopupStack.Size, 0);
            auto popup = GImGui->OpenPopupStack.back().Window;
            ASSERT_NE(nullptr, popup);
            auto target = row == 2 ? 7 : 2;
            auto position = ImVec2(
                popup->DC.CursorStartPos.x + 8,
                popup->DC.CursorStartPos.y + (target + 0.5f) * (popup->DC.CursorPos.y - popup->DC.CursorStartPos.y) / MAX_COLORS);
            click(position);
            EXPECT_EQ(target, (*rules)[row].targetColor);
            io.AddKeyEvent(ImGuiMod_Ctrl, true);
            click({start.x + 250, start.y + row * rowHeight + ImGui::GetFrameHeight() / 2});
            io.AddKeyEvent(ImGuiMod_Ctrl, false);
            draw();
            ASSERT_EQ(ids.at(row), GImGui->ActiveId);
            auto duration = row == 2 ? 123 : 987;
            io.AddInputCharactersUTF8(std::to_string(duration).c_str());
            draw();
            EXPECT_EQ(duration, (*rules)[row].duration);
            auto snapshot = std::make_unique<SimulationParameters>(*values);
            for (auto language : {"en", "ru", "en"}) {
                select(language);
                draw();
                EXPECT_EQ(ids.at(row), GImGui->ActiveId);
                EXPECT_EQ(*snapshot, *values);
                EXPECT_NE(std::string::npos, logged.find(SimulationParametersPresentation::label(parameter)));
            }
            io.AddKeyEvent(ImGuiKey_Enter, true);
            draw();
            io.AddKeyEvent(ImGuiKey_Enter, false);
            draw();
            if (row == 0) {
                auto undoWidth = ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2;
                click({right + ImGui::GetStyle().ItemSpacing.x + undoWidth / 2, start.y + ImGui::GetFrameHeight() / 2});
                EXPECT_EQ(defaults->colorTransitionRules.baseValue[0], (*rules)[0]);
            }
        }
        auto row7 = (*rules)[7];
        auto undoWidth = ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2;
        click({right + ImGui::GetStyle().ItemSpacing.x + undoWidth / 2, start.y + 2 * rowHeight + ImGui::GetFrameHeight() / 2});
        EXPECT_EQ(defaults->colorTransitionRules.baseValue[2], (*rules)[2]);
        EXPECT_EQ(row7, (*rules)[7]);
        EXPECT_EQ(Infinity<int>::value, (*rules)[9].duration);
        for (int row = 0; row < MAX_COLORS; ++row) {
            if (row != 7) {
                EXPECT_EQ(defaults->colorTransitionRules.baseValue[row], (*rules)[row]);
            }
        }
        EXPECT_EQ(otherBefore, order == 0 ? values->colorTransitionRules.layerValues[0].value : values->colorTransitionRules.baseValue);
    }
}


TEST_F(LocalizationGuiTests, l2b4eExpertHeaderCollapseUsesLegacyIdAcrossLanguages)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    values->colorTransitionRulesToggle.value = true;
    auto const& group = parameterGroup(SimulationParameters::getSpec(), "Object color transition rules");
    auto const canonical = "Expert settings: " + group._name;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImGuiID expected = 0;
    ImVec2 header;
    std::string logged;
    auto draw = [&] {
        frame([&] {
            auto start = ImGui::GetCursorScreenPos();
            header = {start.x + 60, start.y + ImGui::GetStyle().CellPadding.y + ImGui::GetFrameHeight() / 2};
            ImGui::PushOverrideID(ImGui::GetID("##"));
            ImGui::PushID(canonical.c_str());
            auto nodeScope = ImGui::GetID(canonical.c_str());
            ImGui::PushID(nodeScope);
            expected = ImGui::GetID(canonical.c_str());
            ImGui::PopID();
            ImGui::PopID();
            ImGui::PopID();
            ImGui::LogToBuffer(0);
            SpecificationGuiService::get().createWidgetsForParameters(*values, *defaults, 0, {.containedText = group._name});
            logged = GImGui->LogBuffer.c_str();
            ImGui::LogFinish();
        });
    };
    auto click = [&] {
        io.AddMousePosEvent(header.x, header.y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        EXPECT_EQ(expected, GImGui->ActiveId);
        io.AddMouseButtonEvent(0, false);
        draw();
    };
    draw();
    draw();
    EXPECT_NE(std::string::npos, logged.find("Target color and duration"));
    click();
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        draw();
        EXPECT_EQ(std::string::npos, logged.find(SimulationParametersPresentation::label(group._parameters.at(0))));
        EXPECT_EQ(0, ImGui::FindWindowByName("Host")->StateStorage.GetInt(expected, -1));
    }
    click();
    draw();
    EXPECT_EQ(1, ImGui::FindWindowByName("Host")->StateStorage.GetInt(expected, -1));
    EXPECT_NE(std::string::npos, logged.find("Target color and duration"));
    EXPECT_EQ(defaults->colorTransitionRules, values->colorTransitionRules);
}

TEST_F(LocalizationGuiTests, l2b4eTransitionCaptionAndFullDescriptionRemainAccessibleInNarrowEditors)
{
    auto values = std::make_unique<SimulationParameters>();
    auto defaults = std::make_unique<SimulationParameters>();
    auto const& parameter = parameterGroup(SimulationParameters::getSpec(), "Object color transition rules")._parameters.at(0);
    auto& io = ImGui::GetIO();
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto language : {"en", "ru"}) {
            select(language);
            for (float width : {650.0f, 950.0f}) {
                ImVec2 label, marker;
                std::string logged;
                auto draw = [&] {
                    switcherFrame(width * factor, [&] {
                        auto start = ImGui::GetCursorScreenPos();
                        auto undoWidth = ImGui::CalcTextSize(ICON_FA_UNDO).x + ImGui::GetStyle().FramePadding.x * 2;
                        label = {start.x + ImGui::GetContentRegionAvail().x - scale(360) + ImGui::GetStyle().ItemSpacing.x * 2 + undoWidth, start.y};
                        auto labelWidth = ImGui::CalcTextSize(SimulationParametersPresentation::label(parameter).c_str()).x;
                        auto helpWidth = ImGui::CalcTextSize(ICON_FA_QUESTION_CIRCLE).x;
                        auto available = start.x + ImGui::GetContentRegionAvail().x - label.x;
                        marker = {
                            label.x + std::min(labelWidth, available - helpWidth - ImGui::GetStyle().ItemSpacing.x) + ImGui::GetStyle().ItemSpacing.x
                                + helpWidth / 2,
                            label.y + ImGui::GetFrameHeight() / 2};
                        ImGui::LogToBuffer();
                        parametersGroup({parameter}, *values, *defaults, 0);
                        logged = GImGui->LogBuffer.c_str();
                        ImGui::LogFinish();
                    });
                };
                io.AddMousePosEvent(-100, -100);
                draw();
                draw();
                io.AddMousePosEvent(label.x + 4, label.y + ImGui::GetFrameHeight() / 2);
                draw();
                draw();
                EXPECT_NE(std::string::npos, logged.find(SimulationParametersPresentation::label(parameter))) << language << ": " << factor << ": " << width;
                io.AddMousePosEvent(marker.x, marker.y);
                draw();
                draw();
                EXPECT_NE(std::string::npos, logged.find(*SimulationParametersPresentation::description(parameter)))
                    << language << ": " << factor << ": " << width;
                EXPECT_EQ(*defaults, *values);
            }
        }
    }
}

namespace
{
    class GenomeShellWorldScope
    {
        class World : public _SimulationFacadeImpl
        {
        public:
            int getSessionId() const override { return 1; }
            IntVector2D getWorldSize() const override { return {1024, 768}; }
            SimulationParameters getSimulationParameters() const override { return *_parameters; }
            ContentDesc getPreviewData() override { return {}; }
            uint64_t getCurrentTimestepForPreview() override { return 0; }

        private:
            std::unique_ptr<SimulationParameters> _parameters = std::make_unique<SimulationParameters>();
        };

    public:
        GenomeShellWorldScope()
            : _original(_SimulationFacade::get())
        {
            _SimulationFacadeImpl::set(std::make_shared<World>());
        }
        ~GenomeShellWorldScope() { _SimulationFacadeImpl::set(_original); }

    private:
        SimulationFacade _original;
    };

    GenomeDesc syntheticEditorGenome()
    {
        return GenomeDesc()
            .name("Synthetic / Геном {index}")
            .frontAngle(-180)
            .genes(
                {GeneDesc().name("User gene / Пользовательский ген").shape(ConstructorShape_Segment).nodes({NodeDesc(), NodeDesc()}),
                 GeneDesc().name("Second / Второй").shape(ConstructorShape_Triangle).nodes({NodeDesc()})});
    }
}

TEST_F(LocalizationGuiTests, l4a1PresentationRetainsCanonicalOrderAndAttributeFallback)
{
    std::vector<EntityAttribute> attributes = {
        EntityAttribute::GenomeName,
        EntityAttribute::GenomeFrontAngle,
        EntityAttribute::GenomeResistanceToInjection,
        EntityAttribute::GenomeApplyMetaMutations,
        EntityAttribute::GeneName,
        EntityAttribute::GeneShape,
        EntityAttribute::GeneConnectionDistance,
        EntityAttribute::GeneStiffness,
        EntityAttribute::GeneHomogeneousCellType};
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        auto shapes = GenomeEditorPresentation::shapes();
        auto cells = GenomeEditorPresentation::cellTypes();
        ASSERT_EQ(ConstructorShape_Count, shapes.size());
        ASSERT_EQ(CellType_Count, cells.size());
        if (std::string(language) == "en") {
            EXPECT_EQ(Const::ConstructorShapeStrings, shapes);
            EXPECT_EQ(Const::CellTypeStrings, cells);
        } else {
            EXPECT_EQ("Большой леденец", shapes.at(ConstructorShape_LargeLolli));
            EXPECT_EQ("Малый леденец", shapes.at(ConstructorShape_SmallLolli));
            EXPECT_EQ("Донор", cells.at(CellType_Void));
            EXPECT_EQ("Конвертер", cells.at(CellType_Digestor));
        }
        for (auto attribute : attributes) {
            auto canonical = EntityAttributeHelp::get(attribute);
            auto displayed = GenomeEditorPresentation::attributeDescription(attribute);
            EXPECT_FALSE(displayed.empty());
            EXPECT_EQ(std::string(language) == "en", canonical == displayed);
        }
        EXPECT_EQ(EntityAttributeHelp::get(EntityAttribute::ObjectType), GenomeEditorPresentation::attributeDescription(EntityAttribute::ObjectType));
    }
    LocalizationService::get().load("missing-l4a1-test-catalogs");
    select("ru");
    EXPECT_EQ(Const::ConstructorShapeStrings, GenomeEditorPresentation::shapes());
    EXPECT_EQ(EntityAttributeHelp::get(EntityAttribute::GeneShape), GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneShape));
    LocalizationService::get().load("resources/locales");
}

TEST_F(LocalizationGuiTests, l4a1WholeMessagesUseRussianCountFormsAndDoNotInterpretArguments)
{
    select("ru");
    for (auto const& [count, expected] : std::vector<std::pair<int, std::string>>{
             {0, "0 генов"},
             {1, "1 ген"},
             {2, "2 гена"},
             {5, "5 генов"},
             {11, "11 генов"},
             {12, "12 генов"},
             {14, "14 генов"},
             {21, "21 ген"},
             {22, "22 гена"},
             {25, "25 генов"},
             {101, "101 ген"}}) {
        EXPECT_EQ(expected, GenomeEditorPresentation::count("genome.count.genes", count));
    }
    EXPECT_EQ("Геном внедрён в 21 организм", GenomeEditorPresentation::count("genome.injected", 21));
    EXPECT_EQ("Геном внедрён в 12 организмов", GenomeEditorPresentation::count("genome.injected", 12));
    EXPECT_EQ("Ген {index}", GenomeEditorPresentation::format("genome.tree.gene", {{"index", "{index}"}}));
    select("en");
    EXPECT_EQ("1 gene", GenomeEditorPresentation::count("genome.count.genes", 1));
    EXPECT_EQ("21 genes", GenomeEditorPresentation::count("genome.count.genes", 21));
}

TEST_F(LocalizationGuiTests, l4a1ProductionTabsCloneCopyPasteAndSyntheticFileRoundTripSurviveLanguageSwitch)
{
    GenomeShellWorldScope world;
    initializeGenomeShell();
    auto& window = GenomeEditorWindow::get();
    auto genome = syntheticEditorGenome();
    window.openTab(genome, true, false, 42);
    drawGenomeShell();
    drawGenomeShell();
    ASSERT_EQ(2, genomeTabs().size());
    selectGenomeTab(1);
    drawGenomeShell();
    drawGenomeShell();
    EXPECT_EQ(genome._name, window.getCurrentGenome()._name);
    auto tab = genomeTabs().at(1);
    auto tabId = tab->getTabId();
    auto normalized = tab->getGenomeDesc();
    tab->getEditData()->genome._frontAngle = 37;
    tab->getEditData()->changesMade = true;
    copyGenome();
    cloneGenome();
    drawGenomeShell();
    drawGenomeShell();
    ASSERT_EQ(3, genomeTabs().size());
    auto cloned = genomeTabs().back();
    EXPECT_NE(tabId, cloned->getTabId());
    EXPECT_EQ(tab->getGenomeDesc(), cloned->getGenomeDesc());
    for (auto const& language : {"ru", "en", "ru"}) {
        select(language);
        drawGenomeShell();
        EXPECT_EQ(tabId, tab->getTabId());
        EXPECT_TRUE(tab->hasGenomeChanged());
        EXPECT_EQ(42, tab->getLineageId());
        EXPECT_EQ(genome._name, tab->getGenomeDesc()._name);
        EXPECT_EQ(genome._genes.at(0)._name, tab->getGenomeDesc()._genes.at(0)._name);
        EXPECT_EQ(0, tab->getEditData()->selectedGeneIndex);
    }
    selectGenomeTab(2);
    drawGenomeShell();
    drawGenomeShell();
    cloned->getEditData()->genome._name = "Temporary";
    pasteGenome();
    EXPECT_EQ(tab->getGenomeDesc(), cloned->getGenomeDesc());
    EXPECT_FALSE(cloned->hasGenomeChanged());

    auto directory = std::filesystem::temp_directory_path() / ("alien-l4a1-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    ASSERT_TRUE(std::filesystem::create_directory(directory));
    auto filename = directory / "synthetic.genome";
    ASSERT_TRUE(SerializerService::get().serializeGenomeToFile(filename, cloned->getGenomeDesc()));
    GenomeDesc loaded;
    ASSERT_TRUE(SerializerService::get().deserializeGenomeFromFile(loaded, filename));
    EXPECT_TRUE(loaded.equalWithoutId(cloned->getGenomeDesc()));
    window.openTab(loaded, true, false);
    drawGenomeShell();
    drawGenomeShell();
    ASSERT_EQ(4, genomeTabs().size());
    tab->revertChanges();
    EXPECT_EQ(normalized, tab->getGenomeDesc());
    closeOtherGenomeTabs();
    EXPECT_EQ(1, genomeTabs().size());
    std::filesystem::remove(filename);
    std::filesystem::remove(directory);
}

TEST_F(LocalizationGuiTests, l4a1GeneInputKeepsActiveIdAndCyrillicEditsAcrossLanguageSwitch)
{
    auto data = std::make_shared<_GenomeTabEditData>();
    data->genome = syntheticEditorGenome();
    data->selectGene(1);
    auto layout = std::make_shared<_GenomeTabLayoutData>();
    auto editor = _GeneEditorWidget::create(data, layout);
    ImGuiID activeId = 0;
    ImGuiWindow* header = nullptr;
    auto draw = [&](bool activate) {
        frame([&] {
            if (activate) {
                GImGui->NavActivateId = activeId;
                GImGui->NavActivateFlags = ImGuiActivateFlags_PreferInput;
            }
            drawGeneProperties(editor);
        });
        for (auto candidate : GImGui->Windows) {
            if (std::string(candidate->Name).find("/GeneHeader_") != std::string::npos) {
                header = candidate;
                activeId = ImHashStr("##Gene name", 0, ImHashStr("##", 0, header->ID));
            }
        }
    };
    draw(false);
    ASSERT_NE(nullptr, header);
    ImGui::FocusWindow(header);
    draw(true);
    draw(false);
    ASSERT_EQ(activeId, ImGui::GetActiveID());
    select("ru");
    ImGui::GetIO().AddInputCharactersUTF8("ИмяЁ");
    draw(false);
    EXPECT_EQ(activeId, ImGui::GetActiveID());
    EXPECT_NE(std::string::npos, data->getSelectedGeneRef()._name.find("ИмяЁ"));
    auto name = data->getSelectedGeneRef()._name;
    select("en");
    draw(false);
    EXPECT_EQ(activeId, ImGui::GetActiveID());
    EXPECT_EQ(name, data->getSelectedGeneRef()._name);
    EXPECT_EQ(1, data->selectedGeneIndex);
}

TEST_F(LocalizationGuiTests, l4a1TreeMouseSelectionKeepsGeneIndicesAndExpansionStorage)
{
    auto data = std::make_shared<_GenomeTabEditData>();
    data->genome = syntheticEditorGenome();
    data->selectGene(0);
    data->setSelectedNodeIndex(1);
    auto editor = _GenomeEditorWidget::create(data, std::make_shared<_GenomeTabLayoutData>());
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImVec2 position;
    auto draw = [&] { frame([&] { drawGenomeGeneRow(editor, 1, position); }); };
    draw();
    draw();
    io.AddMousePosEvent(position.x, position.y);
    draw();
    io.AddMouseButtonEvent(0, true);
    draw();
    io.AddMouseButtonEvent(0, false);
    draw();
    ASSERT_EQ(1, data->selectedGeneIndex);
    EXPECT_EQ(GenomeSelectionLevel::Gene, data->selectionLevel);
    select("ru");
    draw();
    EXPECT_EQ(1, data->selectedGeneIndex);
    EXPECT_EQ(1, data->selectedNodeByGeneIndex.at(0));
    auto original = data->genome;
    select("en");
    draw();
    EXPECT_EQ(original, data->genome);
}

TEST_F(LocalizationGuiTests, l4a1RootConfirmationAndReferenceErrorsStayOpenAndKeepArguments)
{
    auto data = std::make_shared<_GenomeTabEditData>();
    data->genome = syntheticEditorGenome();
    data->selectGene(0);
    auto editor = _GenomeEditorWidget::create(data, std::make_shared<_GenomeTabLayoutData>());
    auto& dialog = GenericMessageDialog::get();
    removeGene(editor);
    auto original = data->genome;
    ImGuiWindow* popup = nullptr;
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        frame([&] {
            processOpenDialog(dialog, [&] {
                if (popup) {
                    EXPECT_EQ(popup, ImGui::GetCurrentWindow());
                }
                popup = ImGui::GetCurrentWindow();
            });
        });
        EXPECT_TRUE(dialog.isOpen());
        EXPECT_EQ(original, data->genome);
    }
    closeImmediately(dialog);
    select("en");
    dialog.localizedInformation("Error", "common.error", "genome.error.used.many", {{"genes", "0, 2, 5"}});
    for (auto const& language : {"en", "ru", "en"}) {
        select(language);
        std::string logged;
        frame([&] {
            ImGui::LogToBuffer();
            processOpenDialog(dialog);
            logged = GImGui->LogBuffer.c_str();
            ImGui::LogFinish();
        });
        EXPECT_NE(std::string::npos, logged.find("0, 2, 5"));
        EXPECT_EQ(std::string::npos, logged.find("{genes}"));
    }
    closeImmediately(dialog);
}

TEST_F(LocalizationGuiTests, l4a1PropertiesAndStatusRenderAtAllScalesWithoutChangingData)
{
    GenomeShellWorldScope world;
    auto data = std::make_shared<_GenomeTabEditData>();
    data->genome = _GenomeTabWidget::normalizeForEditor(syntheticEditorGenome());
    data->selectGene(1);
    auto original = data->genome;
    auto layout = std::make_shared<_GenomeTabLayoutData>();
    layout->structureHeight = 200;
    auto genomeEditor = _GenomeEditorWidget::create(data, layout);
    auto geneEditor = _GeneEditorWidget::create(data, layout);
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto const& language : {"en", "ru", "en"}) {
            select(language);
            for (float width : {380.0f, 700.0f}) {
                switcherFrame(width * factor, [&] { drawGeneProperties(geneEditor); });
                EXPECT_EQ(original, data->genome);
                switcherFrame(width * factor, [&] { drawGenomeProperties(genomeEditor); });
                EXPECT_EQ(original, data->genome);
                EXPECT_EQ(1, data->selectedGeneIndex);
            }
        }
    }
}

TEST_F(LocalizationGuiTests, l4a2ActivationAdapterPreservesEnumOrderAndCanonicalNames)
{
    auto const canonical = Const::ActivationFunctionStrings;
    select("en");
    auto english = NeuralNetEditorPresentation::activationFunctions();
    ASSERT_EQ(ActivationFunction_Count, english.size());
    select("ru");
    auto russian = NeuralNetEditorPresentation::activationFunctions();
    ASSERT_EQ(english.size(), russian.size());
    EXPECT_EQ("Тождественная функция", russian.at(ActivationFunction_Identity));
    EXPECT_EQ("Абсолютное значение", russian.at(ActivationFunction_Abs));
    EXPECT_EQ("ступ", NeuralNetEditorPresentation::shortActivationFunctions().at(ActivationFunction_BinaryStep));
    for (int index = 0; index < ActivationFunction_Count; ++index) {
        EXPECT_NE(english.at(index), russian.at(index));
    }
    EXPECT_EQ(canonical, Const::ActivationFunctionStrings);
    select("en");
    EXPECT_EQ(english, NeuralNetEditorPresentation::activationFunctions());
}

TEST_F(LocalizationGuiTests, l4a2ChannelRolesMasksAndOrderSurviveLanguageSwitch)
{
    for (int mode = 0; mode < MemoryMode_Count; ++mode) {
        for (int mask : {0, 1, 85, 255}) {
            std::vector<MemoryModeGenomeDesc> modes = {
                SignalDelayGenomeDesc(), SignalRecorderGenomeDesc(), SignalStorageGenomeDesc(), SignalIntegratorGenomeDesc()};
            auto node = NodeDesc()
                            .cellType(MemoryGenomeDesc().mode(modes.at(mode)).channelBitMask(mask))
                            .constructor(ConstructorGenomeDesc().autoTriggerInterval(std::nullopt));
            select("en");
            auto english = CellFunctionChannels::getModules(node);
            for (auto language : {"ru", "en"}) {
                select(language);
                auto modules = CellFunctionChannels::getModules(node);
                ASSERT_EQ(english.size(), modules.size());
                for (size_t index = 0; index < modules.size(); ++index) {
                    auto const& before = english.at(index).channels;
                    auto const& after = modules.at(index).channels;
                    ASSERT_EQ(before.size(), after.size());
                    for (size_t channel = 0; channel < before.size(); ++channel) {
                        EXPECT_EQ(before.at(channel).channel, after.at(channel).channel);
                        EXPECT_EQ(before.at(channel).readLabel.empty(), after.at(channel).readLabel.empty());
                        EXPECT_EQ(before.at(channel).writeLabel.empty(), after.at(channel).writeLabel.empty());
                    }
                }
            }
            EXPECT_EQ(mask, std::get<MemoryGenomeDesc>(node._cellType)._channelBitMask);
        }
    }
    auto sensor = NodeDesc().cellType(SensorGenomeDesc());
    select("ru");
    auto modules = CellFunctionChannels::getModules(sensor);
    ASSERT_EQ(1, modules.size());
    EXPECT_EQ("Сенсор", modules.front().name);
    EXPECT_EQ("запуск", modules.front().channels.front().readLabel);
    EXPECT_EQ("найдено", modules.front().channels.front().writeLabel);
}

TEST_F(LocalizationGuiTests, l4a2GraphRetainsDataSelectionAndMeasuredLayoutAtAllScales)
{
    auto editor = _NeuralNetEditorWidget::create();
    auto net = NeuralNetGenomeDesc().weight(2, 4, -0.625f).bias(2, 0.375f).connectionWeight(1, -0.75f);
    auto original = net;
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (bool dialog : {false, true}) {
            for (auto language : {"en", "ru", "en"}) {
                select(language);
                for (float width : {380.0f, 1200.0f}) {
                    switcherFrame(width * factor, [&] {
                        selectNeuralConnection(editor, NEURAL_NET_INPUTS - 1, NEURAL_NET_OUTPUTS - 1);
                        neuralEditor(editor, net, dialog);
                        EXPECT_EQ(std::make_pair(NEURAL_NET_INPUTS - 1, NEURAL_NET_OUTPUTS - 1), neuralSelection(editor));
                        auto font = dialog ? StyleService::get().getDefaultFont() : StyleService::get().getTinyFont();
                        auto label = tr("neural.velocity");
                        EXPECT_GE(neuralInputMargin(editor), font->CalcTextSizeA(font->FontSize, FLT_MAX, 0, label.c_str()).x);
                        EXPECT_GE(neuralCardWidth(editor), scale(260));
                        EXPECT_GE(neuralActionHeight(editor, dialog), scale(50));
                    });
                    EXPECT_EQ(original, net);
                    auto drawData = ImGui::GetDrawData();
                    ASSERT_GT(drawData->TotalVtxCount, 0);
                    for (auto drawList : drawData->CmdLists) {
                        for (auto const& vertex : drawList->VtxBuffer) {
                            EXPECT_TRUE(std::isfinite(vertex.pos.x));
                            EXPECT_TRUE(std::isfinite(vertex.pos.y));
                        }
                    }
                }
            }
        }
    }
}

TEST_F(LocalizationGuiTests, l4a2ToolsUseCanonicalIdsAndOperateAcrossLanguageSwitch)
{
    auto editor = _NeuralNetEditorWidget::create();
    auto net = NeuralNetGenomeDesc().weight(2, 4, -0.625f).bias(2, 0.375f).connectionWeight(1, -0.75f);
    auto original = net;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImVec2 target;
    auto draw = [&](std::string const& button) {
        frame([&] {
            auto pos = ImGui::GetCursorScreenPos();
            for (auto name : {"Clear", "Identity", "Randomize", "Copy", "Paste"}) {
                auto key = std::string(name);
                std::ranges::transform(key, key.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                auto label = tr("neural." + key);
                auto width = ImGui::CalcTextSize(label.c_str()).x + 2 * ImGui::GetStyle().FramePadding.x;
                if (button == name) {
                    target = {pos.x + width / 2, pos.y + ImGui::GetFrameHeight() / 2};
                }
                pos.x += width + ImGui::GetStyle().ItemSpacing.x;
            }
            auto expectedPaste = ImGui::GetID("Paste");
            neuralActions(editor, net);
            EXPECT_EQ(expectedPaste, GImGui->LastItemData.ID);
        });
    };
    auto click = [&](std::string const& button) {
        draw(button);
        draw(button);
        io.AddMousePosEvent(target.x, target.y);
        draw(button);
        io.AddMouseButtonEvent(0, true);
        draw(button);
        io.AddMouseButtonEvent(0, false);
        draw(button);
    };
    select("en");
    click("Copy");
    select("ru");
    click("Randomize");
    EXPECT_NE(original, net);
    for (auto const& value : net._weights) {
        EXPECT_LE(std::abs(value.getValue()), 2.0f);
    }
    EXPECT_EQ(original._connectionWeights, net._connectionWeights);
    click("Paste");
    EXPECT_EQ(original, net);
    click("Clear");
    for (auto const& value : net._weights) {
        EXPECT_FLOAT_EQ(0, value.getValue());
    }
    click("Identity");
    for (int row = 0; row < NEURAL_NET_OUTPUTS; ++row) {
        EXPECT_FLOAT_EQ(0, net._biases.at(row));
        EXPECT_EQ(ActivationFunction_Identity, net._activationFunctions.at(row));
        for (int col = 0; col < NEURAL_NET_INPUTS; ++col) {
            EXPECT_FLOAT_EQ(row == col ? 1.0f : 0.0f, net._weights.at(row * NEURAL_NET_INPUTS + col).getValue());
        }
    }
    select("en");
    click("Paste");
    EXPECT_EQ(original, net);
}

TEST_F(LocalizationGuiTests, l4a2OpenModalKeepsIdentityAndAdoptsOrCancelsCopiedData)
{
    auto editor = _NeuralNetEditorWidget::create();
    auto net = NeuralNetGenomeDesc().weight(2, 4, -0.625f).bias(2, 0.25f).connectionWeight(1, -0.75f);
    auto original = net;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    auto draw = [&] { frame([&] { neuralDialog(editor, net); }); };
    for (bool adopt : {false, true}) {
        select("en");
        editor->openDialog();
        draw();
        draw();
        ImGuiWindow* popup = nullptr;
        for (auto window : GImGui->Windows) {
            if (window->Active && (window->Flags & ImGuiWindowFlags_Modal)) {
                popup = window;
            }
        }
        ASSERT_NE(nullptr, popup);
        auto popupId = popup->ID;
        editNeuralDialog(editor);
        select("ru");
        draw();
        EXPECT_EQ(popupId, popup->ID);
        EXPECT_EQ(original, net);
        // The first modal action row follows the content child and separator.
        ImGuiWindow* editorWindow = nullptr;
        for (auto window : GImGui->Windows) {
            if (window->Active && window->ParentWindow == popup && std::string(window->Name).find("NeuralNetEditor_") != std::string::npos) {
                editorWindow = window;
            }
        }
        ASSERT_NE(nullptr, editorWindow);
        auto y = editorWindow->DC.CursorMaxPos.y - ImGui::GetFrameHeightWithSpacing() - ImGui::GetFrameHeight() / 2;
        auto x = editorWindow->Pos.x + ImGui::GetStyle().WindowPadding.x;
        auto adoptWidth = ImGui::CalcTextSize(tr("neural.adopt").c_str()).x + 2 * ImGui::GetStyle().FramePadding.x;
        auto cancelWidth = ImGui::CalcTextSize(tr("neural.cancel").c_str()).x + 2 * ImGui::GetStyle().FramePadding.x;
        x += adopt ? adoptWidth / 2 : adoptWidth + ImGui::GetStyle().ItemSpacing.x + cancelWidth / 2;
        io.AddMousePosEvent(x, y);
        draw();
        io.AddMouseButtonEvent(0, true);
        draw();
        io.AddMouseButtonEvent(0, false);
        draw();
        if (adopt) {
            EXPECT_FLOAT_EQ(0.375f, net._biases.at(2));
            EXPECT_FLOAT_EQ(-0.625f, net._connectionWeights.at(1));
            EXPECT_EQ(original._weights, net._weights);
        } else {
            EXPECT_EQ(original, net);
        }
    }
}

TEST_F(LocalizationGuiTests, l4a2MouseNodeSelectionAndActiveWeightInputKeepTheirIds)
{
    auto editor = _NeuralNetEditorWidget::create();
    NeuralNetGenomeDesc net;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImVec2 input, output;
    auto nodes = [&] {
        frame([&] {
            auto start = ImGui::GetCursorScreenPos();
            input = {start.x + 160, start.y + 20 + 3 * 24.0f};
            output = {start.x + 350, start.y + 20 + 5 * 24.0f};
            neuralNodes(editor, net);
        });
    };
    nodes();
    nodes();
    auto click = [&](ImVec2 target) {
        io.AddMousePosEvent(target.x, target.y);
        nodes();
        io.AddMouseButtonEvent(0, true);
        nodes();
        io.AddMouseButtonEvent(0, false);
        nodes();
    };
    click(input);
    select("ru");
    click(output);
    frame([&] { EXPECT_EQ(std::make_pair(3, 5), neuralSelection(editor)); });
    ImVec2 slider;
    auto card = [&] {
        frame([&] {
            auto start = ImGui::GetWindowPos();
            slider = {start.x + scale(40), start.y + scale(9) + ImGui::GetTextLineHeight() + scale(5) + ImGui::GetFrameHeight() / 2};
            neuralCard(editor, net);
        });
    };
    card();
    card();
    io.AddMousePosEvent(slider.x, slider.y);
    io.AddKeyEvent(ImGuiMod_Ctrl, true);
    card();
    io.AddMouseButtonEvent(0, true);
    card();
    io.AddMouseButtonEvent(0, false);
    card();
    io.AddKeyEvent(ImGuiMod_Ctrl, false);
    card();
    ASSERT_NE(0, ImGui::GetActiveID());
    auto active = ImGui::GetActiveID();
    select("en");
    card();
    EXPECT_EQ(active, ImGui::GetActiveID());
    io.AddKeyEvent(ImGuiKey_Escape, true);
    card();
    io.AddKeyEvent(ImGuiKey_Escape, false);
    card();
    EXPECT_FLOAT_EQ(0, net._weights.at(5 * NEURAL_NET_INPUTS + 3).getValue());
}

TEST_F(LocalizationGuiTests, l4a2ActivityPreviewFitsAndRetainsSignalsAtAllScales)
{
    auto data = std::make_shared<_GenomeTabEditData>();
    data->showNeuralActivityEditor = true;
    data->detailSimulation = true;
    auto editor = _CreaturePreviewWidget::create(data, {}, {});
    prepareActivity(editor);
    auto original = editor->getPreviewDesc();
    ContentDesc phenotype;
    bool changed = false;
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            for (float width : {500.0f, 1200.0f}) {
                switcherFrame(width * factor, [&] { drawActivity(editor, changed, phenotype); });
                switcherFrame(width * factor, [&] { drawActivity(editor, changed, phenotype); });
                EXPECT_FALSE(changed);
                EXPECT_EQ(original, editor->getPreviewDesc());
                EXPECT_TRUE(data->showNeuralActivityEditor);
                for (auto window : GImGui->Windows) {
                    if (window->Active && std::string(window->Name).find("/signalEditor_") != std::string::npos && window->ParentWindow
                        && !window->ParentWindow->ParentWindow) {
                        EXPECT_GE(window->Pos.x, window->ParentWindow->Pos.x);
                        EXPECT_LE(window->Pos.x + window->Size.x, window->ParentWindow->Pos.x + window->ParentWindow->Size.x);
                    }
                }
            }
        }
    }
}

TEST_F(LocalizationGuiTests, l4a2NeuralHeadingKeepsExpandButtonIdentity)
{
    ImGuiID canonical = 0;
    frame([&] {
        AlienGui::Group(AlienGui::GroupParameters().text("Neural network").highlighted(true).expandButton(true));
        canonical = GImGui->LastItemData.ID;
    });
    ASSERT_NE(0, canonical);
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        frame([&] {
            AlienGui::Group(AlienGui::GroupParameters().text("Neural network").label(tr("neural.title")).highlighted(true).expandButton(true));
            EXPECT_EQ(canonical, GImGui->LastItemData.ID);
        });
    }
}

TEST_F(LocalizationGuiTests, l4a2WeightAndBiasResetsAndActivationChooserEditOnlySelectedNeuron)
{
    auto editor = _NeuralNetEditorWidget::create();
    auto net = NeuralNetGenomeDesc().weight(2, 4, 0.75f).bias(2, -0.5f).bias(1, 0.25f);
    select("ru");
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImVec2 weightReset, biasReset, activation;
    auto draw = [&] {
        frame([&] {
            selectNeuralConnection(editor, 4, 2);
            auto min = ImGui::GetWindowPos();
            auto cardWidth = neuralCardWidth(editor);
            auto labelWidth = std::max(ImGui::CalcTextSize(tr("neural.weight").c_str()).x, ImGui::CalcTextSize(tr("neural.bias").c_str()).x);
            auto resetWidth = StyleService::get().getTinyFont()->CalcTextSizeA(StyleService::get().getTinyFont()->FontSize, FLT_MAX, 0, ICON_FA_TIMES).x
                + 2 * ImGui::GetStyle().FramePadding.x;
            auto x = min.x + cardWidth - scale(9) - ImGui::GetStyle().ItemSpacing.x - labelWidth - resetWidth / 2;
            auto y = min.y + scale(9) + ImGui::GetTextLineHeight() + scale(5) + ImGui::GetFrameHeight() / 2;
            weightReset = {x, y};
            biasReset = {x, y + ImGui::GetFrameHeight() + scale(5)};
            auto contentWidth = cardWidth - scale(18);
            auto buttonWidth = (contentWidth - 5 * ImGui::GetStyle().ItemSpacing.x) / 6;
            activation = {
                min.x + scale(9) + buttonWidth / 2, min.y + scale(9) + ImGui::GetTextLineHeight() + 3 * scale(5) + 2 * ImGui::GetFrameHeight() + scale(9)};
            neuralCard(editor, net);
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
    click(weightReset);
    EXPECT_FLOAT_EQ(0, net._weights.at(2 * NEURAL_NET_INPUTS + 4).getValue());
    click(biasReset);
    EXPECT_FLOAT_EQ(0, net._biases.at(2));
    EXPECT_FLOAT_EQ(0.25f, net._biases.at(1));
    net._activationFunctions.at(2) = ActivationFunction_Identity;
    click(activation);
    EXPECT_EQ(ActivationFunction_Tanh, net._activationFunctions.at(2));
    EXPECT_EQ(ActivationFunction_Identity, net._activationFunctions.at(1));
}

TEST_F(LocalizationGuiTests, l4a3ModeAdaptersKeepCanonicalOrderAndReloadAtRenderTime)
{
    using namespace GenomeEditorPresentation;
    std::vector<std::pair<std::vector<std::string> const*, std::vector<std::string> (*)()>> adapters = {
        {&Const::ProvideEnergyGenomeStrings, provideEnergyGenomes},
        {&Const::SensorModeStrings, sensorModes},
        {&Const::GeneratorModeStrings, generatorModes},
        {&Const::AttackerModeStrings, attackerModes},
        {&Const::MuscleModeStrings, muscleModes},
        {&Const::DefenderModeStrings, defenderModes},
        {&Const::ReconnectorModeStrings, reconnectorModes},
        {&Const::MemoryModeStrings, memoryModes},
        {&Const::CommunicatorModeStrings, communicatorModes}};
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        for (auto const& [canonical, adapter] : adapters) {
            auto display = adapter();
            ASSERT_EQ(canonical->size(), display.size());
            for (size_t index = 0; index < display.size(); ++index) {
                EXPECT_FALSE(display.at(index).empty());
                EXPECT_EQ(std::string(language) == "en", display.at(index) == canonical->at(index));
            }
        }
    }
}

TEST_F(LocalizationGuiTests, l4a3AttributeAdapterMatchesCanonicalEnglishAndEmbeddedFallback)
{
    std::vector<EntityAttribute> attributes = {
        EntityAttribute::NodeAngle,
        EntityAttribute::NodeCustomization,
        EntityAttribute::CellType,
        EntityAttribute::NodeConstructionGene,
        EntityAttribute::ConstructorAutoTriggerInterval,
        EntityAttribute::ConstructorConstructionAngle,
        EntityAttribute::ConstructorProvideEnergy,
        EntityAttribute::ConstructorSeparation,
        EntityAttribute::ConstructorNumBranches,
        EntityAttribute::ConstructorNumConcatenations,
        EntityAttribute::DepotStorageLimit,
        EntityAttribute::SensorAutoTrigger,
        EntityAttribute::SensorTagForAttackers,
        EntityAttribute::SensorMode,
        EntityAttribute::SensorEnergyMinDensity,
        EntityAttribute::SensorFreeCellMinDensity,
        EntityAttribute::SensorRestrictToColors,
        EntityAttribute::SensorMinNumCells,
        EntityAttribute::SensorMaxNumCells,
        EntityAttribute::SensorRestrictToLineage,
        EntityAttribute::SensorMinRange,
        EntityAttribute::SensorMaxRange,
        EntityAttribute::GeneratorAdditive,
        EntityAttribute::GeneratorMinValue,
        EntityAttribute::GeneratorMaxValue,
        EntityAttribute::GeneratorTimeOffset,
        EntityAttribute::GeneratorMode,
        EntityAttribute::GeneratorPeriod,
        EntityAttribute::AttackerMode,
        EntityAttribute::AttackerRestrictToColors,
        EntityAttribute::InjectorGeneIndex,
        EntityAttribute::MuscleMode,
        EntityAttribute::MuscleMaxAngleDeviation,
        EntityAttribute::MuscleForwardBackwardRatio,
        EntityAttribute::MuscleAttractionRepulsionRatio,
        EntityAttribute::MuscleMaxDistanceDeviation,
        EntityAttribute::DefenderMode,
        EntityAttribute::ReconnectorMode,
        EntityAttribute::ReconnectorRestrictToColors,
        EntityAttribute::ReconnectorMinNumCells,
        EntityAttribute::ReconnectorMaxNumCells,
        EntityAttribute::ReconnectorRestrictToLineage,
        EntityAttribute::DetonatorCountdown,
        EntityAttribute::DigestorEnergyConductivity,
        EntityAttribute::DigestorEnergyConversion,
        EntityAttribute::MemoryMode,
        EntityAttribute::MemoryDelay,
        EntityAttribute::MemoryReadOnly,
        EntityAttribute::MemorySignalBuffer,
        EntityAttribute::MemoryNewSignalWeight,
        EntityAttribute::MemoryChannelMask,
        EntityAttribute::CommunicatorMode,
        EntityAttribute::CommunicatorRange,
        EntityAttribute::CommunicatorOneway,
        EntityAttribute::CommunicatorRestrictToColors,
        EntityAttribute::CommunicatorRestrictToLineage};
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        for (auto attribute : attributes) {
            auto display = GenomeEditorPresentation::attributeDescription(attribute);
            EXPECT_FALSE(display.empty());
            EXPECT_EQ(std::string(language) == "en", display == EntityAttributeHelp::get(attribute));
        }
    }
    LocalizationService::get().load("missing-l4a3-catalogs");
    select("ru");
    for (auto attribute : attributes) {
        EXPECT_EQ(EntityAttributeHelp::get(attribute), GenomeEditorPresentation::attributeDescription(attribute));
    }
    EXPECT_EQ(Const::SensorModeStrings, GenomeEditorPresentation::sensorModes());
}

TEST_F(LocalizationGuiTests, l4a3AllNodePanelsModesAndConstructorsRetainGenomeAtAllScales)
{
    GenomeShellWorldScope world;
    std::vector<CellTypeGenomeDesc> types = {
        BaseGenomeDesc(),
        DepotGenomeDesc(),
        InjectorGenomeDesc(),
        DefenderGenomeDesc(),
        DefenderGenomeDesc().mode(DefenderMode_DefendAgainstInjector),
        DetonatorGenomeDesc(),
        DigestorGenomeDesc(),
        VoidGenomeDesc()};
    for (auto mode :
         {SensorModeGenomeDesc(DetectEnergyGenomeDesc()),
          SensorModeGenomeDesc(DetectSolidGenomeDesc()),
          SensorModeGenomeDesc(DetectFreeCellGenomeDesc()),
          SensorModeGenomeDesc(DetectCreatureGenomeDesc())}) {
        types.emplace_back(SensorGenomeDesc().mode(mode));
    }
    for (auto mode : {GeneratorModeGenomeDesc(SquareSignalGenomeDesc()), GeneratorModeGenomeDesc(SawtoothSignalGenomeDesc())}) {
        types.emplace_back(GeneratorGenomeDesc().mode(mode));
    }
    for (auto mode : {AttackerModeGenomeDesc(AttackFreeCellGenomeDesc()), AttackerModeGenomeDesc(AttackCreatureGenomeDesc())}) {
        types.emplace_back(AttackerGenomeDesc().mode(mode));
    }
    for (auto mode :
         {MuscleModeGenomeDesc(AutoBendingGenomeDesc()),
          MuscleModeGenomeDesc(ManualBendingGenomeDesc()),
          MuscleModeGenomeDesc(AngleBendingGenomeDesc()),
          MuscleModeGenomeDesc(AutoCrawlingGenomeDesc()),
          MuscleModeGenomeDesc(ManualCrawlingGenomeDesc()),
          MuscleModeGenomeDesc(DirectMovementGenomeDesc())}) {
        types.emplace_back(MuscleGenomeDesc().mode(mode));
    }
    for (auto mode :
         {ReconnectorModeGenomeDesc(ReconnectSolidGenomeDesc()),
          ReconnectorModeGenomeDesc(ReconnectFreeCellGenomeDesc()),
          ReconnectorModeGenomeDesc(ReconnectCreatureGenomeDesc())}) {
        types.emplace_back(ReconnectorGenomeDesc().mode(mode));
    }
    for (auto mode :
         {MemoryModeGenomeDesc(SignalDelayGenomeDesc()),
          MemoryModeGenomeDesc(SignalRecorderGenomeDesc()),
          MemoryModeGenomeDesc(SignalStorageGenomeDesc()),
          MemoryModeGenomeDesc(SignalIntegratorGenomeDesc())}) {
        types.emplace_back(MemoryGenomeDesc().mode(mode).channelBitMask(85).signalEntries({SignalEntryGenomeDesc()}));
    }
    for (auto mode : {CommunicatorModeGenomeDesc(SenderGenomeDesc()), CommunicatorModeGenomeDesc(ReceiverGenomeDesc())}) {
        types.emplace_back(CommunicatorGenomeDesc().mode(mode));
    }
    auto data = std::make_shared<_GenomeTabEditData>();
    auto layout = std::make_shared<_GenomeTabLayoutData>();
    auto editor = _NodeEditorWidget::create(data, layout);
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto const& type : types) {
            data->genome = syntheticEditorGenome();
            data->selectGene(0);
            data->setSelectedNodeIndex(1);
            data->genome._genes.at(0)._nodes.emplace_back(NodeDesc());
            auto& node = data->getSelectedNodeRef();
            node._cellType = type;
            node._constructor = node.getCellType() == CellType_Void
                ? std::nullopt
                : std::make_optional(ConstructorGenomeDesc().geneIndex(1).provideEnergy(ProvideEnergyGenome_TransitiveCells).separation(false).numBranches(3));
            auto original = data->genome;
            for (auto language : {"en", "ru", "en"}) {
                select(language);
                for (float width : {380.0f, 1400.0f}) {
                    switcherFrame(width * factor, [&] { editor->process(); });
                    EXPECT_EQ(original, data->genome);
                    EXPECT_EQ(1, data->getSelectedNodeIndex());
                    for (auto window : GImGui->Windows) {
                        for (auto const& vertex : window->DrawList->VtxBuffer) {
                            EXPECT_TRUE(std::isfinite(vertex.pos.x));
                            EXPECT_TRUE(std::isfinite(vertex.pos.y));
                        }
                    }
                }
            }
        }
    }
}

TEST_F(LocalizationGuiTests, l4a3ActiveNodeAngleSurvivesLanguageSwitch)
{
    GenomeShellWorldScope world;
    auto data = std::make_shared<_GenomeTabEditData>();
    data->genome = syntheticEditorGenome();
    data->selectGene(0);
    data->setSelectedNodeIndex(0);
    auto editor = _NodeEditorWidget::create(data, std::make_shared<_GenomeTabLayoutData>());
    ImGuiID angleId = 0;
    ImGuiWindow* nodeWindow = nullptr;
    auto draw = [&](bool activate) {
        frame([&] {
            if (activate) {
                GImGui->NavActivateId = angleId;
                GImGui->NavActivateFlags = ImGuiActivateFlags_PreferInput;
            }
            editor->process();
        });
        for (auto window : GImGui->Windows) {
            if (std::string(window->Name).find("/NodeData_") != std::string::npos) {
                nodeWindow = window;
                auto tableSeed = ImHashStr("##", 0, window->ID);
                angleId = ImHashStr("##Angle", 0, ImHashStr("", 0, tableSeed));
            }
        }
    };
    draw(false);
    ASSERT_NE(nullptr, nodeWindow);
    ImGui::FocusWindow(nodeWindow);
    draw(true);
    draw(false);
    ASSERT_EQ(angleId, GImGui->ActiveId);
    ImGui::GetIO().AddInputCharactersUTF8("37.5");
    draw(false);
    EXPECT_FLOAT_EQ(37.5f, data->getSelectedNodeRef()._referenceAngle);
    auto original = data->genome;
    for (auto language : {"ru", "en"}) {
        select(language);
        draw(false);
        EXPECT_EQ(angleId, GImGui->ActiveId);
        EXPECT_EQ(original, data->genome);
    }
}

TEST_F(LocalizationGuiTests, l4a3SharedDisplayExtensionsKeepCanonicalIdsAndFallback)
{
    int mode = 2;
    ImGuiID comboId = 0;
    auto draw = [&](bool localized, bool open) {
        frame([&] {
            auto parameters = AlienGui::ComboParameters().name("Type").values(Const::CellTypeStrings);
            if (localized) {
                parameters.label(tr("node.label.type")).displayValues(GenomeEditorPresentation::cellTypes());
            }
            comboId = ImGui::GetID("##Type");
            if (open) {
                ImGui::OpenPopupEx(ImHashStr("##ComboPopup", 0, comboId));
            }
            AlienGui::Combo(parameters, mode);
        });
    };
    draw(false, true);
    ASSERT_FALSE(GImGui->OpenPopupStack.empty());
    auto popupId = GImGui->OpenPopupStack.back().PopupId;
    for (auto language : {"ru", "en"}) {
        select(language);
        draw(true, false);
        EXPECT_EQ(popupId, GImGui->OpenPopupStack.back().PopupId);
        EXPECT_EQ(2, mode);
    }
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    auto popup = GImGui->OpenPopupStack.back().Window;
    ASSERT_NE(nullptr, popup);
    ImVec2 target(popup->DC.CursorStartPos.x + 20, popup->DC.CursorStartPos.y + ImGui::GetTextLineHeightWithSpacing() * 1.5f);
    io.AddMousePosEvent(target.x, target.y);
    draw(true, false);
    io.AddMouseButtonEvent(0, true);
    draw(true, false);
    select("ru");
    io.AddMouseButtonEvent(0, false);
    draw(true, false);
    EXPECT_EQ(CellType_Depot, mode);
    EXPECT_TRUE(GImGui->OpenPopupStack.empty());

    ImGuiID canonicalButton = 0;
    ImVec2 buttonPosition;
    bool pressed = false;
    auto buttonFrame = [&] {
        frame([&] {
            canonicalButton = ImGui::GetID("Edit");
            buttonPosition = ImGui::GetCursorScreenPos();
            buttonPosition.x += 20;
            buttonPosition.y += ImGui::GetFrameHeight() / 2;
            pressed = AlienGui::Button(
                AlienGui::ButtonParameters().name("Signal buffer").buttonText("Edit").label(tr("node.label.signalbuffer")).buttonLabel(tr("node.edit")));
        });
    };
    buttonFrame();
    io.AddMousePosEvent(buttonPosition.x, buttonPosition.y);
    buttonFrame();
    io.AddMouseButtonEvent(0, true);
    buttonFrame();
    ASSERT_EQ(canonicalButton, GImGui->ActiveId);
    select("en");
    buttonFrame();
    EXPECT_EQ(canonicalButton, GImGui->ActiveId);
    io.AddMouseButtonEvent(0, false);
    buttonFrame();
    EXPECT_TRUE(pressed);
}

TEST_F(LocalizationGuiTests, l4a3ColorAndChannelMasksKeepLegacyIdsAndBitOrder)
{
    int colorMask = 85;
    bool bits[] = {false, true, false, true};
    ImGuiID colorId = 0;
    ImGuiID bitId = 0;
    ImVec2 colorPosition;
    ImVec2 bitPosition;
    auto draw = [&](bool localized) {
        frame([&] {
            auto colors = AlienGui::ColorCheckboxesParameters().name("Restrict to colors").textWidth(300);
            auto channels = AlienGui::MultiCheckboxesParameters().name("Channel mask bit 0-3").textWidth(300);
            if (localized) {
                colors.label(tr("node.label.restricttocolors"));
                channels.label(tr("node.label.channelmaskbit03"));
            }
            colorId = ImHashStr("###0", 0, ImGui::GetID("Restrict to colors"));
            bitId = ImHashStr("###1", 0, ImGui::GetID("Channel mask bit 0-3"));
            auto start = ImGui::GetCursorScreenPos();
            auto width = ImGui::GetContentRegionAvail().x;
            colorPosition = {start.x + width - scale(300) - scale(26.0f * MAX_COLORS / 2) + 5, start.y + ImGui::GetFrameHeight() / 2};
            AlienGui::ColorCheckboxes(colors, colorMask);
            start = ImGui::GetCursorScreenPos();
            bitPosition = {start.x + width - scale(300) - scale(104) + 5, start.y + ImGui::GetFrameHeight() / 2};
            AlienGui::MultiCheckboxes(channels, bits[0], bits[1], bits[2], bits[3]);
        });
    };
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    for (bool color : {true, false}) {
        select("en");
        draw(false);
        auto position = color ? colorPosition : bitPosition;
        auto expectedId = color ? colorId : bitId;
        io.AddMousePosEvent(position.x, position.y);
        draw(false);
        io.AddMouseButtonEvent(0, true);
        draw(false);
        ASSERT_EQ(expectedId, GImGui->ActiveId);
        select("ru");
        draw(true);
        EXPECT_EQ(expectedId, GImGui->ActiveId);
        io.AddMouseButtonEvent(0, false);
        draw(true);
    }
    EXPECT_EQ(84, colorMask);
    EXPECT_TRUE(bits[0]);
    EXPECT_TRUE(bits[1]);
    EXPECT_FALSE(bits[2]);
    EXPECT_TRUE(bits[3]);
}

TEST_F(LocalizationGuiTests, l4a4MutationDescriptionsAndMissingCatalogFallback)
{
    std::vector<EntityAttribute> attributes = {
        EntityAttribute::GenomeMutationRatesEdit,
        EntityAttribute::GenomeMutationRatesSummary,
        EntityAttribute::MutationConnectionProbability,
        EntityAttribute::MutationConnectionValueChangeSigma,
        EntityAttribute::MutationNeuronProbability,
        EntityAttribute::MutationNeuronWeightChangeSigma,
        EntityAttribute::MutationNeuronBiasChangeSigma,
        EntityAttribute::MutationNeuronActfnChangeProbability,
        EntityAttribute::MutationCellTypePropertiesProbability,
        EntityAttribute::MutationCellTypePropertiesValueChangeSigma,
        EntityAttribute::MutationCellTypePropertiesEnumChangeProbability,
        EntityAttribute::MutationGeometryProbability,
        EntityAttribute::MutationGeometryValueChangeSigma,
        EntityAttribute::MutationGeometryEnumChangeProbability,
        EntityAttribute::MutationCellTypeModeProbability,
        EntityAttribute::MutationCellTypeProbability,
        EntityAttribute::MutationCustomizationProbability,
        EntityAttribute::MutationVoidProbability,
        EntityAttribute::MutationExtendGeneProbability,
        EntityAttribute::MutationAddNodeProbability,
        EntityAttribute::MutationTrimGeneProbability,
        EntityAttribute::MutationDeleteNodeProbability,
        EntityAttribute::MutationAddGeneProbability,
        EntityAttribute::MutationDuplicateGeneProbability,
        EntityAttribute::MutationDeleteGeneProbability,
        EntityAttribute::MutationSwapGeneProbability,
        EntityAttribute::MutationCopyNodeSectionProbability,
        EntityAttribute::MutationMoveNodeSectionProbability,
        EntityAttribute::MutationConstructorProbability,
        EntityAttribute::MutationConstructorValueChangeSigma,
        EntityAttribute::MutationConstructorEnumChangeProbability,
        EntityAttribute::MutationConstructorToggleProbability};
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        for (auto attribute : attributes) {
            auto description = GenomeEditorPresentation::attributeDescription(attribute);
            EXPECT_FALSE(description.empty());
            EXPECT_EQ(std::string(language) == "en", description == EntityAttributeHelp::get(attribute));
        }
    }
    LocalizationService::get().load("missing-l4a4-catalogs");
    select("ru");
    for (auto attribute : attributes) {
        EXPECT_EQ(EntityAttributeHelp::get(attribute), GenomeEditorPresentation::attributeDescription(attribute));
    }
}

TEST_F(LocalizationGuiTests, l4a4MutationDialogAndSummariesRetainAllValuesAtEveryScale)
{
    auto genome = syntheticEditorGenome();
    auto& rates = genome._mutationRates;
    rates._connectionMutations.at(0)._nodeProbability = 0.00017f;
    rates._connectionMutations.at(1)._nodeProbability = 0.31f;
    rates._neuronMutations.at(0)._nodeProbability = 0.25f;
    rates._neuronMutations.at(1)._nodeProbability = 0.5f;
    rates._cellTypePropertiesMutations.at(0)._nodeProbability = 0.3f;
    rates._geometryMutations.at(1)._geneProbability = 0.4f;
    rates._constructorMutations.at(1)._nodeProbability = 0.7f;
    rates._cellTypeModeMutation._nodeProbability = 0.03125f;
    rates._cellTypeMutation._nodeProbability = 0.03125f;
    rates._customizationMutation._genomeProbability = 0.03125f;
    rates._voidMutation._nodeProbability = 0.03125f;
    rates._extendGeneMutation._geneProbability = 0.03125f;
    rates._addNodeMutation._nodeProbability = 0.03125f;
    rates._trimGeneMutation._geneProbability = 0.03125f;
    rates._deleteNodeMutation._nodeProbability = 0.03125f;
    rates._addGeneMutation._geneProbability = 0.03125f;
    rates._duplicateGeneMutation._geneProbability = 0.03125f;
    rates._deleteGeneMutation._geneProbability = 0.03125f;
    rates._swapGeneMutation._geneProbability = 0.03125f;
    rates._copyNodeSectionMutation._geneProbability = 0.03125f;
    rates._moveNodeSectionMutation._geneProbability = 0.03125f;
    auto original = genome;
    MutationRatesWidget widget;
    MutationRatesDialog dialog;
    dialog.open(rates, [&](auto const& adopted) { rates = adopted; });
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            for (float width : {700.0f, 1800.0f}) {
                switcherFrame(width * factor, [&] {
                    widget.process(rates, 360, true);
                    widget.processAsSingleRow(rates, 360);
                    ImGui::SetNextWindowSize({width * factor, 900 * factor});
                    dialog.process();
                    for (auto name :
                         {"Node probability",
                          "Gene probability",
                          "Genome probability",
                          "Value change sigma",
                          "Weight change sigma",
                          "Bias change sigma",
                          "ActFn change probability",
                          "Enum change probability",
                          "Constructor toggle probability"}) {
                        auto text = MutationRatesPresentation::label(name);
                        EXPECT_LE(ImGui::CalcTextSize(text.c_str()).x + scale(35), scale(MutationRatesPresentation::parameterLabelWidth()) + 0.01f);
                    }
                });
                EXPECT_TRUE(mutationOpen(dialog));
                EXPECT_EQ(original._mutationRates, mutationDraft(dialog));
                EXPECT_EQ(original, genome);
                EXPECT_GT(MutationRatesPresentation::parameterLabelWidth(), 0);
            }
        }
    }
    auto directory = std::filesystem::temp_directory_path() / ("alien-l4a4-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(directory);
    auto file = directory / "synthetic.genome";
    ASSERT_TRUE(SerializerService::get().serializeGenomeToFile(file, genome));
    GenomeDesc loaded;
    ASSERT_TRUE(SerializerService::get().deserializeGenomeFromFile(loaded, file));
    EXPECT_TRUE(genome.equalWithoutId(loaded));
    EXPECT_EQ(rates, loaded._mutationRates);
    std::filesystem::remove_all(directory);
}

TEST_F(LocalizationGuiTests, l4a4ActiveProbabilityInputKeepsItsIdAndIndependentPartner)
{
    MutationRatesDialog dialog;
    MutationRatesDesc rates;
    rates._connectionMutations.at(0)._nodeProbability = 0.125f;
    rates._connectionMutations.at(1)._nodeProbability = 0.875f;
    dialog.open(rates, [](auto const&) {});
    auto draw = [&] { frame([&] { mutationContent(dialog); }); };
    draw();
    draw();
    ImGuiWindow* child = nullptr;
    for (auto window : GImGui->Windows) {
        if (std::string(window->Name).find("MutationRateContent") != std::string::npos) {
            child = window;
            break;
        }
    }
    ASSERT_NE(nullptr, child);
    auto start = child->DC.CursorStartPos;
    auto const& style = ImGui::GetStyle();
    auto headerHeight = StyleService::get().getSmallBoldFont()->FontSize + 2 * style.FramePadding.y + style.ItemSpacing.y;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    io.AddMousePosEvent(start.x + 2 * style.IndentSpacing + 40, start.y + 2 * headerHeight + 2 * style.CellPadding.y + ImGui::GetFrameHeight() / 2);
    draw();
    io.AddKeyEvent(ImGuiMod_Ctrl, true);
    io.AddMouseButtonEvent(0, true);
    draw();
    auto active = GImGui->ActiveId;
    ASSERT_NE(0u, active);
    io.AddMouseButtonEvent(0, false);
    io.AddKeyEvent(ImGuiMod_Ctrl, false);
    draw();
    select("ru");
    draw();
    EXPECT_EQ(active, GImGui->ActiveId);
    io.AddKeyEvent(ImGuiMod_Ctrl, true);
    io.AddKeyEvent(ImGuiKey_A, true);
    draw();
    io.AddKeyEvent(ImGuiKey_A, false);
    io.AddKeyEvent(ImGuiMod_Ctrl, false);
    io.AddInputCharactersUTF8("0.375");
    draw();
    io.AddKeyEvent(ImGuiKey_Enter, true);
    draw();
    io.AddKeyEvent(ImGuiKey_Enter, false);
    draw();
    EXPECT_FLOAT_EQ(0.375f, mutationDraft(dialog)._connectionMutations.at(0)._nodeProbability);
    EXPECT_FLOAT_EQ(0.875f, mutationDraft(dialog)._connectionMutations.at(1)._nodeProbability);
    EXPECT_EQ(rates._neuronMutations, mutationDraft(dialog)._neuronMutations);
    auto genome = syntheticEditorGenome();
    genome._mutationRates = mutationDraft(dialog);
    auto file = std::filesystem::temp_directory_path()
        / ("alien-l4a4-edited-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".genome");
    ASSERT_TRUE(SerializerService::get().serializeGenomeToFile(file, genome));
    GenomeDesc loaded;
    ASSERT_TRUE(SerializerService::get().deserializeGenomeFromFile(loaded, file));
    EXPECT_TRUE(genome.equalWithoutId(loaded));
    EXPECT_FLOAT_EQ(0.375f, loaded._mutationRates._connectionMutations.at(0)._nodeProbability);
    std::filesystem::remove(file);
}

TEST_F(LocalizationGuiTests, l4a4MutationCopyResetPasteAdoptAndCancelUseProductionButtons)
{
    clearMutationClipboard();
    MutationRatesDesc original;
    original._connectionMutations.at(0)._nodeProbability = 0.125f;
    original._connectionMutations.at(1)._nodeProbability = 0.875f;
    original._neuronMutations.at(1)._biasChangeSigma = 1.375f;
    auto adopted = original;
    int adopts = 0;
    MutationRatesDialog dialog;
    dialog.open(original, [&](auto const& value) {
        adopted = value;
        ++adopts;
    });
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImVec2 target;
    ImGuiID expected = 0;
    auto draw = [&](std::string const& button) {
        frame([&] {
            auto start = ImGui::GetCursorScreenPos();
            auto const& style = ImGui::GetStyle();
            float x = start.x;
            for (auto const& [canonical, key] :
                 std::vector<std::pair<std::string, std::string>>{{"Adopt", "mutation.adopt"}, {"Cancel", "mutation.cancel"}, {"Reset", "mutation.reset"}}) {
                auto width = ImGui::CalcTextSize(tr(key).c_str()).x + style.FramePadding.x * 2;
                if (canonical == button) {
                    target = {x + width / 2, start.y + ImGui::GetFrameHeight() / 2};
                }
                x += width + style.ItemSpacing.x;
            }
            auto copyWidth = ImGui::CalcTextSize(tr("mutation.copy").c_str()).x + style.FramePadding.x * 2;
            auto pasteWidth = ImGui::CalcTextSize(tr("mutation.paste").c_str()).x + style.FramePadding.x * 2;
            auto right = start.x + ImGui::GetContentRegionAvail().x;
            if (button == "Copy")
                target = {right - pasteWidth - style.ItemSpacing.x - copyWidth / 2, start.y + ImGui::GetFrameHeight() / 2};
            if (button == "Paste")
                target = {right - pasteWidth / 2, start.y + ImGui::GetFrameHeight() / 2};
            expected = ImGui::GetID(button.c_str());
            mutationButtons(dialog);
        });
    };
    auto click = [&](std::string const& button, bool switchWhileHeld = false) {
        draw(button);
        draw(button);
        io.AddMousePosEvent(target.x, target.y);
        draw(button);
        io.AddMouseButtonEvent(0, true);
        draw(button);
        EXPECT_EQ(expected, GImGui->ActiveId);
        if (switchWhileHeld) {
            select("ru");
            draw(button);
            EXPECT_EQ(expected, GImGui->ActiveId);
            io.AddMousePosEvent(target.x, target.y);
            draw(button);
        }
        io.AddMouseButtonEvent(0, false);
        draw(button);
    };
    click("Copy", true);
    EXPECT_EQ(tr("mutation.copied"), overlayMessage());
    click("Reset");
    EXPECT_EQ(MutationRatesDesc(), mutationDraft(dialog));
    EXPECT_EQ(original, adopted);
    EXPECT_EQ(0, adopts);
    EXPECT_EQ(tr("mutation.resetmessage"), overlayMessage());
    click("Paste");
    EXPECT_EQ(original, mutationDraft(dialog));
    EXPECT_EQ(tr("mutation.pasted"), overlayMessage());
    mutationDraft(dialog)._connectionMutations.at(0)._nodeProbability = 0.625f;
    click("Adopt");
    EXPECT_EQ(1, adopts);
    EXPECT_FLOAT_EQ(0.625f, adopted._connectionMutations.at(0)._nodeProbability);
    EXPECT_FLOAT_EQ(0.875f, adopted._connectionMutations.at(1)._nodeProbability);
    dialog.open(adopted, [&](auto const&) { ++adopts; });
    click("Reset");
    click("Cancel");
    EXPECT_EQ(1, adopts);
    EXPECT_FALSE(mutationOpen(dialog));
}

TEST_F(LocalizationGuiTests, l4a5DiagnosticFamiliesFollowUpsLocationsAndFallback)
{
    auto genome = GenomeDesc().genes({GeneDesc().nodes({NodeDesc().cellType(VoidGenomeDesc()), NodeDesc(), NodeDesc()})});
    auto original = genome;
    std::vector<std::string> english;
    for (auto language : {"en", "ru", "en"}) {
        select(language);
        for (auto type :
             {GenomeIssueType::VoidBoundaryNode,
              GenomeIssueType::NodesCutOffFromLastNode,
              GenomeIssueType::CycleAvoidingRootGene,
              GenomeIssueType::GeneUnreachableFromRoot,
              GenomeIssueType::TooManyGenesWithSeparation,
              GenomeIssueType::ConstructsRemovedGene,
              GenomeIssueType::InjectsRemovedGene}) {
            GenomeIssue issue{
                .type = type,
                .geneIndex = 0,
                .nodeIndex = 0,
                .cutOffNodeIndices = {0, 1},
                .cycleGeneIndices = {1, 2, 1},
                .geneIndicesKeepingSeparation = {0, 1, 2},
                .referencedGeneIndex = 21};
            for (bool meta : {false, true}) {
                genome._applyMetaMutations = meta;
                for (bool followUp : {false, true}) {
                    issue.causeIssueIndex = followUp ? std::optional<int>(0) : std::nullopt;
                    auto text = GenomeIssueDescription::getTooltip({issue}, genome);
                    EXPECT_EQ(std::string::npos, text.find('{')) << text;
                    EXPECT_NE(std::string::npos, text.find(std::string(language) == "ru" ? "ген 0, узел 0" : "gene 0, node 0"));
                    EXPECT_NE(
                        std::string::npos,
                        text.find(std::string(language) == "ru" ? "Исправление в геноме потомства:" : "Correction in the genome of offspring:"));
                    EXPECT_EQ(std::string::npos, text.find(std::string(language) == "ru" ? "gene " : "ген "));
                    EXPECT_EQ(followUp, issue.isFollowUp());
                    if (type == GenomeIssueType::ConstructsRemovedGene || type == GenomeIssueType::InjectsRemovedGene) {
                        EXPECT_NE(std::string::npos, text.find("21"));
                    }
                    if (english.size() < 28 && std::string(language) == "en") {
                        english.push_back(text);
                    }
                }
            }
        }
    }
    genome._applyMetaMutations = original._applyMetaMutations;
    EXPECT_EQ(original, genome);
    EXPECT_TRUE(GenomeIssueDescription::getTooltip({}, genome).empty());
    auto enText = GenomeIssueDescription::getTooltip({GenomeIssue()}, genome);
    LocalizationService::get().load("missing-l4a5-catalogs");
    select("ru");
    EXPECT_EQ(enText, GenomeIssueDescription::getTooltip({GenomeIssue()}, genome));
}

TEST_F(LocalizationGuiTests, l4a5DiagnosticCountFormsAndNodeConsequences)
{
    select("ru");
    for (auto const& [count, form] : std::vector<std::pair<int, std::string>>{
             {0, "узлов-доноров"}, {1, "узел-донор"}, {2, "узла-донора"}, {5, "узлов-доноров"}, {11, "узлов-доноров"}, {21, "узел-донор"}}) {
        GenomeDesc genome;
        genome._genes = {GeneDesc()};
        GenomeIssue issue{.type = GenomeIssueType::NodesCutOffFromLastNode};
        for (int i = 0; i < count; ++i) {
            genome._genes.at(0)._nodes.emplace_back(NodeDesc().cellType(VoidGenomeDesc()));
            issue.cutOffNodeIndices.push_back(i);
        }
        auto text = GenomeIssueDescription::getTooltip({issue}, genome);
        EXPECT_NE(std::string::npos, text.find(std::to_string(count) + " " + form)) << text;
        EXPECT_NE(std::string::npos, text.find("Их конструкторы будут отключены."));
        for (auto& node : genome._genes.at(0)._nodes) {
            node._cellType = BaseGenomeDesc();
        }
        if (count > 0) {
            text = GenomeIssueDescription::getTooltip({issue}, genome);
            EXPECT_NE(std::string::npos, text.find(count % 10 == 1 && count != 11 ? "Донором станет" : "Донорами станут"));
        }
        issue.removesGene = true;
        EXPECT_NE(std::string::npos, GenomeIssueDescription::getTooltip({issue}, genome).find("Ген будет удалён."));
    }
    for (int donors : {1, 2, 5, 21}) {
        for (int affected : {1, 2, 5, 21}) {
            auto genome = GenomeDesc().genes({GeneDesc().nodes(std::vector<NodeDesc>(donors + affected))});
            GenomeIssue issue{.type = GenomeIssueType::NodesCutOffFromLastNode};
            for (int index = 0; index < donors; ++index) {
                genome._genes.at(0)._nodes.at(index).cellType(VoidGenomeDesc());
            }
            for (int index = donors; index < donors + affected; ++index) {
                issue.cutOffNodeIndices.push_back(index);
            }
            auto text = GenomeIssueDescription::getTooltip({issue}, genome);
            EXPECT_NE(std::string::npos, text.find(donors == 1 ? "Узел-донор с индексом" : "Узлы-доноры с индексами"));
            if (donors == 1) {
                EXPECT_NE(std::string::npos, text.find(affected == 1 ? "отсоединяет узел с индексом" : "отсоединяет узлы с индексами"));
            } else {
                EXPECT_NE(std::string::npos, text.find(affected == 1 ? "отсоединяют узел с индексом" : "отсоединяют узлы с индексами"));
            }
        }
    }
}

TEST_F(LocalizationGuiTests, l4a5GraphPreviewHeadersAndDiagnosticsWrapAtEveryScale)
{
    auto data = std::make_shared<_GenomeTabEditData>();
    data->genome = syntheticEditorGenome();
    data->genome._genes.at(0)._name = "Длинное имя гена / {genes} / Ёжик";
    data->selectNode(0, 0);
    auto original = data->genome;
    auto preview = _CreaturePreviewWidget::create(data, {}, {});
    std::vector<int> geneIndices;
    for (int index = 0; index < 21; ++index) {
        geneIndices.push_back(index);
    }
    preview->setGeneIndices(geneIndices);
    GeneGraphWidget graph;
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            for (int count : {0, 1, 2, 5, 21}) {
                for (bool trimmed : {false, true}) {
                    for (int start : {0, 1}) {
                        switcherFrame(600 * factor, [&] {
                            previewTitle(preview, count, trimmed, start);
                            auto rect = GImGui->LastItemData.Rect;
                            EXPECT_LE(rect.Max.x, ImGui::GetWindowPos().x + ImGui::GetWindowWidth());
                            ImGui::Dummy({0, 120 * factor});
                            graph.process(data);
                        });
                    }
                }
            }
            auto text = GenomeIssueDescription::getTooltip({GenomeIssue()}, data->genome);
            frame([&] {
                auto wrapWidth = ImGui::GetFontSize() * 35;
                auto size = ImGui::CalcTextSize(text.c_str(), nullptr, false, wrapWidth);
                EXPECT_LE(size.x, wrapWidth + ImGui::GetFontSize());
                EXPECT_GT(size.y, ImGui::GetFontSize() * 3);
                for (auto character : {L'Д', L'я', L'Ё', L'ё'}) {
                    EXPECT_NE(nullptr, ImGui::GetFont()->FindGlyphNoFallback(character));
                }
            });
            EXPECT_EQ(original, data->genome);
            EXPECT_EQ(0, data->selectedGeneIndex);
            EXPECT_EQ(0, data->getSelectedNodeIndex());
        }
    }
}

TEST_F(LocalizationGuiTests, l4a5SharedSignalsRetainActiveInputAndIndependentValues)
{
    std::vector<SignalEntryGenomeDesc> entries(2);
    entries.at(0)._channels.at(0) = 0.125f;
    entries.at(1)._channels.at(0) = 0.875f;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImVec2 start;
    auto draw = [&] {
        frame([&] {
            start = ImGui::GetCursorScreenPos();
            AlienGui::SignalMemoryEditor(AlienGui::SignalMemoryEditorParameters().textWidth(180), entries);
        });
    };
    draw();
    draw();
    auto row = ImGui::GetFrameHeightWithSpacing();
    io.AddMousePosEvent(start.x + 80, start.y + 2 * row + ImGui::GetFrameHeight() / 2);
    draw();
    io.AddKeyEvent(ImGuiMod_Ctrl, true);
    io.AddMouseButtonEvent(0, true);
    draw();
    auto active = GImGui->ActiveId;
    ASSERT_NE(0u, active);
    io.AddMouseButtonEvent(0, false);
    io.AddKeyEvent(ImGuiMod_Ctrl, false);
    draw();
    select("ru");
    draw();
    EXPECT_EQ(active, GImGui->ActiveId);
    io.AddKeyEvent(ImGuiMod_Ctrl, true);
    io.AddKeyEvent(ImGuiKey_A, true);
    draw();
    io.AddKeyEvent(ImGuiKey_A, false);
    io.AddKeyEvent(ImGuiMod_Ctrl, false);
    io.AddInputCharactersUTF8("-0.625");
    draw();
    io.AddKeyEvent(ImGuiKey_Enter, true);
    draw();
    io.AddKeyEvent(ImGuiKey_Enter, false);
    draw();
    EXPECT_FLOAT_EQ(-0.625f, entries.at(0)._channels.at(0));
    EXPECT_FLOAT_EQ(0.875f, entries.at(1)._channels.at(0));
    auto edited = entries;
    auto genome = GenomeDesc().genes({GeneDesc().nodes({NodeDesc().cellType(MemoryGenomeDesc().signalEntries(entries))})});
    auto file = std::filesystem::temp_directory_path()
        / ("alien-l4a5-signals-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".genome");
    ASSERT_TRUE(SerializerService::get().serializeGenomeToFile(file, genome));
    GenomeDesc loaded;
    ASSERT_TRUE(SerializerService::get().deserializeGenomeFromFile(loaded, file));
    EXPECT_TRUE(genome.equalWithoutId(loaded));
    std::filesystem::remove(file);
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_EQ(edited, entries);
        }
    }
}

TEST_F(LocalizationGuiTests, l4a5SignalsModalDraftSelectionCallbacksAdoptAndCancel)
{
    std::vector<SignalEntryGenomeDesc> entries(2);
    entries.at(0)._channels.at(0) = 0.125f;
    entries.at(1)._channels.at(0) = 0.875f;
    int adopts = 0;
    signalDialog().open(entries, [&](auto const& result) {
        entries = result;
        ++adopts;
    });
    signalSelection() = 1;
    signalDraft().at(1).at(0) = -0.375f;
    auto& io = ImGui::GetIO();
    io.ConfigInputTrickleEventQueue = false;
    ImRect cancel;
    auto draw = [&] {
        frame([&] {
            signalContent();
            cancel = GImGui->LastItemData.Rect;
        });
    };
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            draw();
            EXPECT_EQ(1, signalSelection());
            EXPECT_FLOAT_EQ(-0.375f, signalDraft().at(1).at(0));
            EXPECT_FLOAT_EQ(0.875f, entries.at(1)._channels.at(0));
            EXPECT_EQ(0, adopts);
        }
    }
    setTestScale(1);
    draw();
    draw();
    io.AddMousePosEvent(cancel.GetCenter().x, cancel.GetCenter().y);
    draw();
    io.AddMouseButtonEvent(0, true);
    draw();
    auto active = GImGui->ActiveId;
    ASSERT_NE(0u, active);
    select("ru");
    draw();
    EXPECT_EQ(active, GImGui->ActiveId);
    io.AddMousePosEvent(cancel.GetCenter().x, cancel.GetCenter().y);
    draw();
    io.AddMouseButtonEvent(0, false);
    draw();
    EXPECT_EQ(0, adopts);
    EXPECT_FLOAT_EQ(0.875f, entries.at(1)._channels.at(0));
    signalDialog().open(entries, [&](auto const& result) {
        entries = result;
        ++adopts;
    });
    signalDraft().at(1).at(0) = -0.375f;
    auto adoptCenter = [&] {
        auto width = ImGui::CalcTextSize(tr("neural.adopt").c_str()).x + ImGui::GetStyle().FramePadding.x * 2;
        return ImVec2(cancel.Min.x - ImGui::GetStyle().ItemSpacing.x - width / 2, cancel.GetCenter().y);
    };
    draw();
    draw();
    io.AddMousePosEvent(adoptCenter().x, adoptCenter().y);
    draw();
    io.AddMouseButtonEvent(0, true);
    draw();
    io.AddMouseButtonEvent(0, false);
    draw();
    EXPECT_EQ(1, adopts);
    EXPECT_FLOAT_EQ(-0.375f, entries.at(1)._channels.at(0));
    EXPECT_FLOAT_EQ(0.125f, entries.at(0)._channels.at(0));
    std::vector<SignalEntryDesc> live(1);
    live.at(0)._channels.at(2) = 1.25f;
    signalDialog().open(live, [&](auto const& result) {
        live = result;
        ++adopts;
    });
    signalDraft().at(0).at(2) = -1.25f;
    draw();
    draw();
    io.AddMousePosEvent(adoptCenter().x, adoptCenter().y);
    draw();
    io.AddMouseButtonEvent(0, true);
    draw();
    io.AddMouseButtonEvent(0, false);
    draw();
    EXPECT_EQ(2, adopts);
    EXPECT_FLOAT_EQ(-1.25f, live.at(0)._channels.at(2));
}

TEST_F(LocalizationGuiTests, l4a5PreviewSettingsOpenModalKeepsDraftAcrossLanguagesAndScales)
{
    GenomeShellWorldScope world;
    auto windowData = std::make_shared<_GenomeWindowEditData>();
    auto tabData = std::make_shared<_GenomeTabEditData>();
    tabData->showNodeIndex = false;
    auto& dialog = PreviewSettingsDialog::get();
    dialog.setEditData(windowData, tabData);
    dialog.open();
    ImRect cancel;
    auto draw = [&] { frame([&] { cancel = processDialogButton(dialog); }); };
    for (float factor : {1.0f, 1.5f, 2.0f}) {
        setTestScale(factor);
        for (auto language : {"en", "ru", "en"}) {
            select(language);
            draw();
            draw();
            EXPECT_TRUE(dialog.isOpen());
            EXPECT_FALSE(tabData->showNodeIndex);
            EXPECT_GT(cancel.GetWidth(), 0);
        }
    }
    closeImmediately(dialog);
    EXPECT_FALSE(tabData->showNodeIndex);
}
