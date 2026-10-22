#include "GenomeEditorWindow.h"

#include <boost/range/adaptor/indexed.hpp>

#include <imgui.h>
#include <ImFileDialog.h>

#include <Fonts/AlienIconFont.h>
#include <Fonts/IconsFontAwesome5.h>

#include <Base/GlobalSettings.h>
#include <Base/LocalizationService.h>
#include <Base/NameGeneratorService.h>
#include <Base/StringHelper.h>

#include <Base/NumberGenerator.h>

#include <Data/GenomeDescAccessService.h>
#include <Data/GenomeDescEditService.h>
#include <Data/ObjectColoring.h>

#include <Engine/Interface/SimulationFacade.h>

#include <Persister/Interface/SerializerService.h>

#include "AlienGui.h"
#include "ChangeColorDialog.h"
#include "EditorController.h"
#include "EditorModel.h"
#include "FileTransferController.h"
#include "GenericFileDialog.h"
#include "GenericMessageDialog.h"
#include "GenomeEditorPresentation.h"
#include "GenomeTabLayoutData.h"
#include "GenomeTabWidget.h"
#include "GenomeWindowEditData.h"
#include "OverlayController.h"
#include "UploadSimulationDialog.h"

namespace
{
    auto constexpr UnsavedChangesChipPadding = 34.0f;

    std::optional<ImColor> getLineageMarkerColor(std::optional<int> const& lineageId)
    {
        if (!lineageId.has_value()) {
            return std::nullopt;
        }
        auto rgb = ObjectColoring::getColorFromId(toUInt32(lineageId.value()));
        return ImColor(toInt((rgb >> 16) & 0xff), toInt((rgb >> 8) & 0xff), toInt(rgb & 0xff));
    }
}

void GenomeEditorWindow::openTab(GenomeDesc const& genome, bool forceNewTab, bool openEditorIfClosed, std::optional<int> lineageId)
{
    if (openEditorIfClosed) {
        setOn(false);
        delayedExecution([this] { setOn(true); });
    }
    if (_tabs.size() == 1 && _tabs.front()->isEmpty()) {
        _tabs.clear();
    }
    auto normalizedGenome = _GenomeTabWidget::normalizeForEditor(genome);
    std::optional<int> tabIndex;
    if (!forceNewTab) {
        for (auto const& [index, tab] : _tabs | boost::adaptors::indexed(0)) {
            auto tabGenome = tab->getGenomeDesc();
            if (normalizedGenome.equalWithoutId(tabGenome)) {
                tabIndex = toInt(index);
            }
        }
    }
    if (tabIndex) {
        _tabIndexToSelect = *tabIndex;
        _tabs.at(*tabIndex)->resetOriginal();
        _tabs.at(*tabIndex)->setLineageId(lineageId);
    } else {
        onScheduleAddTab(normalizedGenome, lineageId);
    }
}

GenomeDesc GenomeEditorWindow::getCurrentGenome() const
{
    return _tabs.at(_selectedTabIndex)->getGenomeDesc();
}

std::vector<PreviewDesc> GenomeEditorWindow::getCurrentPreviewDescs() const
{
    return _tabs.at(_selectedTabIndex)->getPreviewDescs();
}

GenomeEditorWindow::GenomeEditorWindow()
    : AlienWindow("Genome editor", "windows.genome editor", false, true, {345.0f, 192.0f}, {1100.0f, 732.0f}, {500.0f, 300.0f})
{
    setTitleKey("genome.title");
}

void GenomeEditorWindow::initIntern()
{
    ChangeColorDialog::get().setup();

    _genomeEditData = std::make_shared<_GenomeWindowEditData>();
    _genomeEditData->defaultShowNodeIndex = GlobalSettings::get().getValue(_settingsNode + ".show node index", true);
    _genomeEditData->defaultDetailSimulation = GlobalSettings::get().getValue(_settingsNode + ".detail simulation", false);
    _genomeEditData->defaultShowNeuralActivityEditor = GlobalSettings::get().getValue(_settingsNode + ".show neural activity editor", true);

    // Initialize the first tab with default genome
    _tabs.emplace_back(_GenomeTabWidget::create(_genomeEditData, getDefaultGenome()));
}

void GenomeEditorWindow::shutdownIntern()
{
    GlobalSettings::get().setValue(_settingsNode + ".show node index", _genomeEditData->defaultShowNodeIndex);
    GlobalSettings::get().setValue(_settingsNode + ".detail simulation", _genomeEditData->defaultDetailSimulation);
    GlobalSettings::get().setValue(_settingsNode + ".show neural activity editor", _genomeEditData->defaultShowNeuralActivityEditor);
}

void GenomeEditorWindow::processIntern()
{
    processToolbar();
    processTabWidget();
}

bool GenomeEditorWindow::isShown()
{
    return _on;
}

namespace
{
    float calcUnsavedChangesChipWidth()
    {
        return scaleInverse(ImGui::CalcTextSize(tr("genome.unsaved").c_str()).x) + UnsavedChangesChipPadding;
    }

    void processUnsavedChangesChip()
    {
        AlienGui::Chip(AlienGui::ChipParameters()
                           .text(tr("genome.unsaved"))
                           .textColor(Const::UnsavedChangesColor)
                           .backgroundColor(Const::UnsavedChangesBackgroundColor)
                           .dotColor(Const::UnsavedChangesColor));
    }
}

void GenomeEditorWindow::processToolbar()
{
    auto hasGenomeChanged = _tabs.at(_selectedTabIndex)->hasGenomeChanged();
    auto creaturesSelected = EditorModel::get().getSelectionShallowData().numCreatures > 0;

    auto toolbarParameters = AlienGui::ToolbarParameters().id("GenomeEditor").overflowTooltip(tr("genome.toolbar.more"));
    if (hasGenomeChanged) {
        toolbarParameters.trailing([] { processUnsavedChangesChip(); }).trailingWidth(calcUnsavedChangesChipWidth());
    }

    AlienGui::Toolbar(
        toolbarParameters,
        {AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_FOLDER_OPEN).name(tr("genome.action.open")).tooltip(tr("genome.action.open.help")).action([&] {
                 onOpenGenome();
             })),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_SAVE).name(tr("genome.action.save")).tooltip(tr("genome.action.save.help")).action([&] {
                 onSaveGenome();
             })),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_UPLOAD).name(tr("genome.action.share")).tooltip(tr("genome.action.share.help")).action([&] {
                 onShareGenome();
             })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_CLONE).name(tr("genome.action.clone")).tooltip(tr("genome.action.clone.help")).action([&] {
                 onCloneGenome();
             })),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_COPY).name(tr("genome.action.copy")).tooltip(tr("genome.action.copy.help")).action([&] {
                 onCopyGenome();
             })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_PASTE)
                                                 .name(tr("genome.action.paste"))
                                                 .tooltip(tr("genome.action.paste.help"))
                                                 .disabled(!_copiedGenome.has_value())
                                                 .action([&] { onPasteGenome(); })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_WINDOW_CLOSE)
                                                 .name(tr("genome.action.closeothers"))
                                                 .tooltip(tr("genome.action.closeothers.help"))
                                                 .disabled(_tabs.size() <= 1)
                                                 .action([&] { onCloseOtherTabs(); })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_CAMERA)
                                                 .name(tr("genome.action.savepoint"))
                                                 .tooltip(tr("genome.action.savepoint.help"))
                                                 .disabled(!hasGenomeChanged)
                                                 .action([&] { onSavepointGenome(); })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_UNDO)
                                                 .name(tr("genome.action.revert"))
                                                 .tooltip(tr("genome.action.revert.help"))
                                                 .disabled(!hasGenomeChanged)
                                                 .action([&] { _tabs.at(_selectedTabIndex)->revertChanges(); })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_COLOR).name(tr("genome.action.colors")).tooltip(tr("genome.action.colors.help")).action([&] {
                 ChangeColorDialog::get().open(_tabs.at(_selectedTabIndex)->getEditData());
             })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_SYRINGE)
                                                 .name(tr("genome.action.inject"))
                                                 .tooltip(tr("genome.action.inject.help"))
                                                 .disabled(!creaturesSelected)
                                                 .action([&] { onInjectGenome(); })),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_SEEDLING).name(tr("genome.action.seed")).tooltip(tr("genome.action.seed.help")).action([&] {
                 onCreateSeed(false);
             })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_SEEDLING)
                                                 .secondIcon(ICON_FA_BOLT)
                                                 .secondIconOffset({30.0f, 25.0f})
                                                 .name(tr("genome.action.seedenergy"))
                                                 .tooltip(tr("genome.action.seedenergy.help"))
                                                 .action([&] { onCreateSeed(true); }))});
}

void GenomeEditorWindow::processTabWidget()
{
    if (ImGui::BeginChild("TabWidget", ImVec2(0, 0), 0, 0)) {

        if (AlienGui::BeginTabBar(AlienGui::TabBarParameters().id("##GenomeTabWidget").reorderable(true).autoSelectNewTabs(true))) {

            if (AlienGui::TrailingTabButton(AlienGui::TrailingTabButtonParameters().text("+").tooltip(tr("genome.new")))) {
                onScheduleAddTab(getDefaultGenome(), std::nullopt);
            }

            std::optional<int> tabIndexToSelect = _tabIndexToSelect;
            std::optional<int> tabToDelete;
            _tabIndexToSelect.reset();

            // Process tabs
            for (auto const& [index, genomeTab] : _tabs | boost::adaptors::indexed(0)) {

                bool open = true;
                bool* openPtr = nullptr;
                if (_tabs.size() > 1) {
                    openPtr = &open;
                }

                if (AlienGui::BeginTabItem(AlienGui::TabItemParameters()
                                               .name(genomeTab->getName())
                                               .id(std::to_string(genomeTab->getTabId()))
                                               .selected(tabIndexToSelect && *tabIndexToSelect == index)
                                               .open(openPtr)
                                               .markerColor(getLineageMarkerColor(genomeTab->getLineageId())))) {
                    _selectedTabIndex = toInt(index);
                    genomeTab->process();
                    AlienGui::EndTabItem();
                }

                if (openPtr && *openPtr == false) {
                    tabToDelete = toInt(index);
                }
            }

            // Delete tab
            if (tabToDelete.has_value()) {
                _tabs.erase(_tabs.begin() + *tabToDelete);
                if (_selectedTabIndex == _tabs.size()) {
                    _selectedTabIndex = toInt(_tabs.size() - 1);
                }
            }

            // Add tabs
            for (auto& tab : _tabsToAdd) {
                _tabs.emplace_back(std::move(tab));
            }
            _tabsToAdd.clear();

            AlienGui::EndTabBar();
        }
    }
    ImGui::EndChild();
}

void GenomeEditorWindow::onOpenGenome()
{
    FileTransferController::get().onOpenGenomeDialog([this](GenomeDesc const& genome) { openTab(genome, true, false); });
}

void GenomeEditorWindow::onSaveGenome()
{
    auto const& selectedTab = _tabs.at(_selectedTabIndex);
    auto genome = selectedTab->getGenomeDesc();
    FileTransferController::get().onSaveGenomeDialog(genome, [selectedTab]() { selectedTab->resetOriginal(); });
}

void GenomeEditorWindow::onShareGenome()
{
    UploadSimulationDialog::get().open(NetworkResourceType_Genome);
}

void GenomeEditorWindow::onCloneGenome()
{
    openTab(getCurrentGenome(), true, false);
}

void GenomeEditorWindow::onCloseOtherTabs()
{
    auto selectedTab = _tabs.at(_selectedTabIndex);
    _tabs.clear();
    _tabs.emplace_back(selectedTab);
    _selectedTabIndex = 0;
}

void GenomeEditorWindow::onCopyGenome()
{
    _copiedGenome = getCurrentGenome();
}

void GenomeEditorWindow::onPasteGenome()
{
    auto const& selectedTab = _tabs.at(_selectedTabIndex);
    selectedTab->setGenomeDesc(_copiedGenome.value());
    selectedTab->resetOriginal();
}

void GenomeEditorWindow::onSavepointGenome()
{
    auto const& selectedTab = _tabs.at(_selectedTabIndex);
    selectedTab->resetOriginal();
}

void GenomeEditorWindow::onInjectGenome()
{
    auto const& selectedTab = _tabs.at(_selectedTabIndex);
    auto numCreatures = _SimulationFacade::get()->injectGenomeToSelectedCreatures(selectedTab->getGenomeDesc());
    printOverlayMessage(GenomeEditorPresentation::count("genome.injected", numCreatures));
    selectedTab->resetOriginal();
}

void GenomeEditorWindow::onCreateSeed(bool freeEnergy)
{
    auto pos = Viewport::get().getCenterInWorldPos();
    pos.x += (toFloat(std::rand()) / RAND_MAX - 0.5f) * 8;
    pos.y += (toFloat(std::rand()) / RAND_MAX - 0.5f) * 8;

    auto tab = _tabs.at(_selectedTabIndex);
    auto seed = GenomeDescEditService::get().createSeed(tab->getGenomeDesc(), pos, EditorModel::get().getDefaultColorCode(), freeEnergy);

    _SimulationFacade::get()->addAndSelectSimulationData(std::move(seed));
    EditorModel::get().update();

    printOverlayMessage(tr("genome.seed.created"));
}

void GenomeEditorWindow::onScheduleAddTab(GenomeDesc const& genome, std::optional<int> lineageId)
{
    auto const& currentTab = _tabs.at(_selectedTabIndex);
    _tabsToAdd.emplace_back(_GenomeTabWidget::create(_genomeEditData, genome, currentTab->getLayoutData()->clone(), lineageId));
}

GenomeDesc GenomeEditorWindow::getDefaultGenome()
{
    std::unordered_set<std::string> usedNames;
    for (auto const& tab : _tabs) {
        usedNames.insert(tab->getGenomeDesc()._name);
    }

    return GenomeDesc()
        .name(NameGeneratorService::get().createGenomeName(usedNames))
        .frontAngle(-180.0f)
        .genes({
            GeneDesc().name("Gene 0").nodes({NodeDesc()}).shape(ConstructorShape_Segment),
        });
}
