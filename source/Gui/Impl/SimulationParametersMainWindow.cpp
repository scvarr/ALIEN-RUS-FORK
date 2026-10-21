#include "SimulationParametersMainWindow.h"

#include <functional>

#include <Fonts/IconsFontAwesome5.h>

#include <Base/LocalizationService.h>
#include <Base/StringHelper.h>

#include <Data/LocationAccessService.h>
#include <Data/LocationEditService.h>
#include <Data/RadiationStrengthService.h>

#include <Engine/Interface/SimulationFacade.h>

#include <Persister/Interface/SerializerService.h>

#include "AlienGui.h"
#include "GenericFileDialog.h"
#include "GenericMessageDialog.h"
#include "LocationController.h"
#include "OverlayController.h"
#include "SimulationParametersGuiText.h"
#include "SimulationParametersLayerWidget.h"
#include "SimulationParametersSourceWidget.h"
#include "SpecificationGuiService.h"
#include "Viewport.h"

#include <Engine/Interface/SimulationFacade.h>
#include <ImFileDialog.h>

namespace
{
    auto constexpr MasterHeight = 130.0f;
    auto constexpr MasterMinHeight = 50.0f;
    auto constexpr MasterRowHeight = 25.0f;

    auto constexpr DetailWidgetMinHeight = 0.0f;

    auto constexpr ExpertWidgetHeight = 130.0f;
    auto constexpr ExpertWidgetMinHeight = 60.0f;
}

SimulationParametersMainWindow::SimulationParametersMainWindow()
    : AlienWindow("Simulation parameters", "windows.simulation parameters", false, true, {386.0f, 68.0f}, {680.0f, 780.0f})
{
    setTitleKey("menu.parameters");
}

void SimulationParametersMainWindow::initIntern()
{

    _masterWidgetOpen = GlobalSettings::get().getValue("windows.simulation parameters.master widget.open", _masterWidgetOpen);
    _detailWidgetOpen = GlobalSettings::get().getValue("windows.simulation parameters.detail widget.open", _detailWidgetOpen);
    _expertWidgetOpen = GlobalSettings::get().getValue("windows.simulation parameters.expert widget.open", _expertWidgetOpen);
    auto correction = WindowController::get().getContentScaleCorrection();
    _masterWidgetHeight = GlobalSettings::get().getValue("windows.simulation parameters.master widget.height", scale(MasterHeight)) * correction;
    _expertWidgetHeight = GlobalSettings::get().getValue("windows.simulation parameters.expert widget height", scale(ExpertWidgetHeight)) * correction;

    auto baseWidgets = std::make_shared<_SimulationParametersBaseWidget>();
    baseWidgets->init();
    _baseWidgets = baseWidgets;

    auto layerWidgets = std::make_shared<_SimulationParameterLayerWidget>();
    layerWidgets->init(0);
    _layerWidgets = layerWidgets;


    auto sourceWidgets = std::make_shared<_SimulationParametersSourceWidgets>();
    sourceWidgets->init(0);
    _sourceWidgets = sourceWidgets;
}

void SimulationParametersMainWindow::processIntern()
{
    if (!_sessionId.has_value() || _sessionId.value() != _SimulationFacade::get()->getSessionId()) {
        _selectedLocationId = 0;
    }
    auto parameters = _SimulationFacade::get()->getSimulationParameters();
    _selectedOrderNumber = LocationAccessService::get()
                               .findOrderNumber(parameters, _selectedLocationId)
                               .value_or(std::min(_selectedOrderNumber, parameters.numLayers + parameters.numSources));

    processToolbar();

    if (ImGui::BeginChild("##content", {0, -scale(50.0f)})) {

        updateLocations();

        auto origMasterHeight = _masterWidgetHeight;
        auto origExpertWidgetHeight = _expertWidgetHeight;

        processMasterWidget();
        processDetailWidget();
        processExpertWidget();

        correctLayout(origMasterHeight, origExpertWidgetHeight);
    }
    ImGui::EndChild();

    processStatusBar();

    _sessionId = _SimulationFacade::get()->getSessionId();
    _selectedLocationId = LocationAccessService::get().getLocationId(_SimulationFacade::get()->getSimulationParameters(), _selectedOrderNumber);
}

void SimulationParametersMainWindow::shutdownIntern()
{
    GlobalSettings::get().setValue("windows.simulation parameters.master widget.open", _masterWidgetOpen);
    GlobalSettings::get().setValue("windows.simulation parameters.detail widget.open", _detailWidgetOpen);
    GlobalSettings::get().setValue("windows.simulation parameters.expert widget.open", _expertWidgetOpen);
    GlobalSettings::get().setValue("windows.simulation parameters.master widget.height", _masterWidgetHeight);
    GlobalSettings::get().setValue("windows.simulation parameters.expert widget height", _expertWidgetHeight);
}

void SimulationParametersMainWindow::processToolbar()
{
    AlienGui::Toolbar(
        AlienGui::ToolbarParameters().id("SimulationParameters").overflowTooltip(tr("parameters.overflow")),
        {AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_FOLDER_OPEN).name(tr("parameters.open")).tooltip(tr("parameters.open.help")).action([&] {
                 onOpenParameters();
             })),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_SAVE).name(tr("parameters.save")).tooltip(tr("parameters.save.help")).action([&] {
                 onSaveParameters();
             })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_COPY).name(tr("parameters.copy")).tooltip(tr("parameters.copy.help")).action([&] {
                 _copiedParameters = _SimulationFacade::get()->getSimulationParameters();
                 printOverlayMessage(tr("parameters.copied"));
             })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_PASTE)
                                                 .name(tr("parameters.paste"))
                                                 .tooltip(tr("parameters.paste.help"))
                                                 .disabled(!_copiedParameters)
                                                 .action([&] {
                                                     _SimulationFacade::get()->setSimulationParameters(*_copiedParameters);
                                                     _SimulationFacade::get()->setOriginalSimulationParameters(*_copiedParameters);
                                                     printOverlayMessage(tr("parameters.pasted"));
                                                 })),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters()
                 .icon(ICON_FA_PASTE)
                 .secondIcon(ICON_FA_UNDO)
                 .secondIconOffset(RealVector2D{32.0f, 28.0f})
                 .secondIconScale(0.3f)
                 .name(tr("parameters.reference"))
                 .tooltip(tr("parameters.reference.help"))
                 .disabled(!_copiedParameters)
                 .action([&] {
                     auto parameters = _SimulationFacade::get()->getSimulationParameters();
                     if (_copiedParameters->numLayers == parameters.numLayers && _copiedParameters->numSources == parameters.numSources) {
                         _SimulationFacade::get()->setOriginalSimulationParameters(*_copiedParameters);
                         printOverlayMessage(tr("parameters.replaced"));
                     } else {
                         GenericMessageDialog::get().localizedInformation("Error", "common.error", "parameters.reference.error");
                     }
                 })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_PLUS).secondIcon(ICON_FA_LAYER_GROUP).name(tr("parameters.addlayer")).action([&] {
                 onInsertDefaultLayer();
             })),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_PLUS).secondIcon(ICON_FA_SUN).name(tr("parameters.addsource")).action([&] {
                 onInsertDefaultSource();
             })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_PLUS)
                                                 .secondIcon(ICON_FA_CLONE)
                                                 .name(tr("parameters.clone"))
                                                 .disabled(_selectedOrderNumber == 0)
                                                 .action([&] { onCloneLocation(); })),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_MINUS).name(tr("parameters.delete")).disabled(_selectedOrderNumber == 0).action([&] {
                 onDeleteLocation();
             })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(
             AlienGui::ToolbarItemParameters().icon(ICON_FA_CHEVRON_UP).name(tr("parameters.up")).disabled(_selectedOrderNumber <= 1).action([&] {
                 onDecreaseOrderNumber();
             })),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_CHEVRON_DOWN)
                                                 .name(tr("parameters.down"))
                                                 .disabled(_selectedOrderNumber >= _locations.size() - 1 || _selectedOrderNumber == 0)
                                                 .action([&] { onIncreaseOrderNumber(); })),
         AlienGui::ToolbarItem::createSeparator(),
         AlienGui::ToolbarItem::createButton(AlienGui::ToolbarItemParameters()
                                                 .icon(ICON_FA_EXTERNAL_LINK_SQUARE_ALT)
                                                 .name(tr("parameters.detach"))
                                                 .tooltip(tr("parameters.detach.help"))
                                                 .action([&] { onOpenInLocationWindow(); }))});
}

void SimulationParametersMainWindow::processMasterWidget()
{
    if (ImGui::BeginChild("##master", {0, getMasterWidgetHeight()})) {

        if (_masterWidgetOpen = AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters()
                                                            .name("Overview")
                                                            .label(tr("parameters.overview"))
                                                            .rank(AlienGui::TreeNodeRank::High)
                                                            .defaultOpen(_masterWidgetOpen))) {
            ImGui::Spacing();
            if (ImGui::BeginChild("##master2", {0, -ImGui::GetStyle().FramePadding.y})) {
                processLocationTable();
            }
            ImGui::EndChild();
        }
        AlienGui::EndTreeNode();
    }
    ImGui::EndChild();

    if (_masterWidgetOpen && (_detailWidgetOpen || _expertWidgetOpen)) {
        ImGui::PushID("master");
        AlienGui::MovableHorizontalSeparator(AlienGui::MovableHorizontalSeparatorParameters(), _masterWidgetHeight);
        ImGui::PopID();
    }
}

void SimulationParametersMainWindow::processDetailWidget()
{
    auto height = getDetailWidgetHeight();
    if (ImGui::BeginChild("##detail", {0, height})) {
        auto const& location = _locations.at(_selectedOrderNumber);
        auto title = SimulationParametersGuiText::section(location.name, location.type == LocationType::Base, !_filter.empty());
        if (_detailWidgetOpen = AlienGui::BeginTreeNode(
                AlienGui::TreeNodeParameters().name("###parameters").label(title).rank(AlienGui::TreeNodeRank::High).defaultOpen(_detailWidgetOpen))) {
            ImGui::Spacing();
            //AlienGui::SetFilterText(_filter);
            if (ImGui::BeginChild(
                    "##detail2", {0, -ImGui::GetStyle().FramePadding.y - scale(33.0f)}, ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar)) {
                auto type = _locations.at(_selectedOrderNumber).type;
                ParametersFilter filter{.containedText = _filter};
                if (type == LocationType::Base) {
                    _baseWidgets->process(filter);
                } else if (type == LocationType::Layer) {
                    _layerWidgets->setOrderNumber(_selectedOrderNumber);
                    _layerWidgets->process(filter);
                } else if (type == LocationType::Source) {
                    _sourceWidgets->setOrderNumber(_selectedOrderNumber);
                    _sourceWidgets->process(filter);
                }
            }
            ImGui::EndChild();
            //AlienGui::ResetFilterText();

            ImGui::Spacing();
            startFilterTypingIfNeeded();
            AlienGui::InputFilter(AlienGui::InputFilterParameters().hint(tr("parameters.filter")), _filter);
        }
        AlienGui::EndTreeNode();
    }
    ImGui::EndChild();

    if (_detailWidgetOpen && _expertWidgetOpen) {
        ImGui::PushID("detail");
        AlienGui::MovableHorizontalSeparator(AlienGui::MovableHorizontalSeparatorParameters().additive(false), _expertWidgetHeight);
        ImGui::PopID();
    }
}

void SimulationParametersMainWindow::startFilterTypingIfNeeded()
{
    // SetKeyboardFocusHere() takes effect only on the next frame. Characters typed on the
    // triggering frame are queued here and replayed into ImGui's input queue on the frame the
    // filter field actually becomes active, so ImGui's own selection/insertion logic handles
    // them (e.g. correctly replacing a pre-existing filter instead of us splicing text in and
    // fighting the auto-select-on-focus behavior).
    if (!_pendingFilterChars.empty()) {
        for (auto c : _pendingFilterChars) {
            ImGui::GetIO().AddInputCharacter(c);
        }
        _pendingFilterChars.clear();
        return;
    }

    if (ImGui::IsAnyItemActive() || !ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        return;
    }

    for (auto const& c : ImGui::GetIO().InputQueueCharacters) {
        // A leading space must not start the filter typing, it is reserved for the run/pause hotkey
        if (c == ' ' && _pendingFilterChars.empty()) {
            continue;
        }
        if (c >= 32 && c != 127) {
            _pendingFilterChars.push_back(c);
        }
    }
    if (!_pendingFilterChars.empty()) {
        ImGui::SetKeyboardFocusHere();

        // The filter field becomes active only on the next frame, so global hotkeys (e.g. run/pause on space) need to be suppressed there explicitly
        ImGui::SetNextFrameWantCaptureKeyboard(true);
    }
}

void SimulationParametersMainWindow::processExpertWidget()
{
    if (ImGui::BeginChild("##expert", {0, 0})) {
        if (_expertWidgetOpen = AlienGui::BeginTreeNode(AlienGui::TreeNodeParameters()
                                                            .name("Expert settings")
                                                            .label(tr("parameters.expert"))
                                                            .rank(AlienGui::TreeNodeRank::High)
                                                            .defaultOpen(_expertWidgetOpen))) {
            if (ImGui::BeginChild("##expert2", {0, 0}, ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar)) {
                processExpertSettings();
            }
            ImGui::EndChild();
        }
        AlienGui::EndTreeNode();
    }
    ImGui::EndChild();
}

void SimulationParametersMainWindow::processStatusBar()
{
    std::vector<std::string> statusItems;
    statusItems.emplace_back(tr("parameters.precise"));

    AlienGui::StatusBar(AlienGui::StatusBarParameters().infoItems(statusItems));
}

void SimulationParametersMainWindow::processLocationTable()
{
    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX;

    if (ImGui::BeginTable("Locations", 5, flags, ImVec2(-1, -1), 0)) {

        ImGui::TableSetupColumn(
            (tr("parameters.column.name") + "###Name").c_str(), ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, scale(140.0f));
        ImGui::TableSetupColumn(
            (tr("parameters.column.type") + "###Type").c_str(), ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, scale(140.0f));
        ImGui::TableSetupColumn(
            (tr("parameters.column.position") + "###Position").c_str(), ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, scale(115.0f));
        ImGui::TableSetupColumn(
            (tr("parameters.column.strength") + "###Strength").c_str(), ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, scale(90.0f));
        ImGui::TableSetupColumn(
            (tr("parameters.column.opacity") + "###Opacity").c_str(), ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, scale(90.0f));
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Const::TableHeaderColor);

        ImGuiListClipper clipper;
        clipper.Begin(toInt(_locations.size()));
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                auto const& entry = _locations.at(row);

                ImGui::PushID(row);
                ImGui::TableNextRow(0, scale(MasterRowHeight));

                // Column: Name
                ImGui::TableNextColumn();
                auto selected = _selectedOrderNumber == row;
                if (AlienGui::TableRowSelectable(
                        "", selected, ImGuiSelectableFlags_None, RealVector2D(0, scale(MasterRowHeight) - ImGui::GetStyle().FramePadding.y))) {
                    _selectedOrderNumber = row;
                }
                ImGui::SameLine();
                ImGui::AlignTextToFramePadding();
                std::string icon;
                if (entry.type == LocationType::Base) {
                    icon = ICON_FA_SQUARE_FULL " ";
                } else if (entry.type == LocationType::Layer) {
                    icon = ICON_FA_LAYER_GROUP " ";
                } else if (entry.type == LocationType::Source) {
                    icon = ICON_FA_SUN " ";
                }
                AlienGui::Text(icon + (entry.type == LocationType::Base ? tr("parameters.base") : entry.name));


                // Column: Type
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                if (entry.type == LocationType::Base) {
                    AlienGui::Text(tr("parameters.type.base"));
                } else if (entry.type == LocationType::Layer) {
                    AlienGui::Text(tr("parameters.type.layer"));
                } else if (entry.type == LocationType::Source) {
                    AlienGui::Text(tr("parameters.type.source"));
                }

                // Column: Position
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                if (row > 0) {
                    if (AlienGui::ActionButton(AlienGui::ActionButtonParameters().buttonText(ICON_FA_SEARCH))) {
                        onCenterLocation(row);
                        _selectedOrderNumber = row;
                    }
                    ImGui::SameLine();
                }
                AlienGui::Text(entry.position);

                // Column: Strength
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                if (entry.type == LocationType::Base || entry.type == LocationType::Source) {
                    AlienGui::Text(entry.strength);
                } else {
                    AlienGui::Text("-");
                }

                // Column: Opacity
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                if (entry.type == LocationType::Layer) {
                    AlienGui::Text(entry.strength);
                } else {
                    AlienGui::Text("-");
                }

                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}

void SimulationParametersMainWindow::processExpertSettings()
{
    auto parameters = _SimulationFacade::get()->getSimulationParameters();
    auto origParameters = _SimulationFacade::get()->getOriginalSimulationParameters();
    auto lastParameters = parameters;

    SpecificationGuiService::get().createWidgetsForExpertToggles(parameters, origParameters);

    if (parameters != lastParameters) {
        _SimulationFacade::get()->setSimulationParameters(parameters);
    }
}

void SimulationParametersMainWindow::onOpenParameters()
{
    GenericFileDialog::get().showOpenFileDialog(
        tr("parameters.file.open"),
        tr("parameters.file.filter") + " (*.settings.json){.settings.json,.json},.*",
        _fileDialogPath,
        [&](std::filesystem::path const& path) {
            auto firstFilename = ifd::FileDialog::Instance().GetResult();
            auto firstFilenameCopy = firstFilename;
            _fileDialogPath = firstFilenameCopy.remove_filename().string();

            SimulationParameters parameters;
            if (!SerializerService::get().deserializeSimulationParametersFromFile(parameters, firstFilename.string())) {
                GenericMessageDialog::get().localizedInformation("Open simulation parameters", "parameters.file.open", "parameters.file.openerror");
            } else {
                _SimulationFacade::get()->setSimulationParameters(parameters);
                _SimulationFacade::get()->setOriginalSimulationParameters(parameters);
            }
        });
}

void SimulationParametersMainWindow::onSaveParameters()
{
    GenericFileDialog::get().showSaveFileDialog(
        tr("parameters.file.save"),
        tr("parameters.file.filter") + " (*.settings.json){.settings.json,.json},.*",
        _fileDialogPath,
        [&](std::filesystem::path const& path) {
            auto firstFilename = ifd::FileDialog::Instance().GetResult();
            auto firstFilenameCopy = firstFilename;
            _fileDialogPath = firstFilenameCopy.remove_filename().string();

            auto parameters = _SimulationFacade::get()->getSimulationParameters();
            if (!SerializerService::get().serializeSimulationParametersToFile(firstFilename.string(), parameters)) {
                GenericMessageDialog::get().localizedInformation("Save simulation parameters", "parameters.file.save", "parameters.file.saveerror");
            }
        });
}

namespace
{
    void editLocations(std::function<void(SimulationParameters& parameters, SimulationParameters& origParameters)> const& edit)
    {
        auto parameters = _SimulationFacade::get()->getSimulationParameters();
        auto origParameters = _SimulationFacade::get()->getOriginalSimulationParameters();
        edit(parameters, origParameters);
        _SimulationFacade::get()->setSimulationParameters(parameters);
        _SimulationFacade::get()->setOriginalSimulationParameters(origParameters);
    }
}

void SimulationParametersMainWindow::onInsertDefaultLayer()
{
    std::optional<int> orderNumber;
    editLocations([&](auto& parameters, auto& origParameters) {
        orderNumber = LocationEditService::get().insertDefaultLayer(parameters, origParameters, _selectedOrderNumber, _SimulationFacade::get()->getWorldSize());
    });
    if (orderNumber) {
        _selectedOrderNumber = orderNumber.value();
    } else {
        showMaxLocationsReachedMessage(LocationType::Layer);
    }
}

void SimulationParametersMainWindow::onInsertDefaultSource()
{
    std::optional<int> orderNumber;
    editLocations([&](auto& parameters, auto& origParameters) {
        orderNumber =
            LocationEditService::get().insertDefaultSource(parameters, origParameters, _selectedOrderNumber, _SimulationFacade::get()->getWorldSize());
    });
    if (orderNumber) {
        _selectedOrderNumber = orderNumber.value();
    } else {
        showMaxLocationsReachedMessage(LocationType::Source);
    }
}

void SimulationParametersMainWindow::onCloneLocation()
{
    std::optional<int> orderNumber;
    editLocations([&](auto& parameters, auto& origParameters) {
        orderNumber = LocationEditService::get().cloneLocation(parameters, origParameters, _selectedOrderNumber);
    });
    if (orderNumber) {
        _selectedOrderNumber = orderNumber.value();
    } else {
        showMaxLocationsReachedMessage(_locations.at(_selectedOrderNumber).type);
    }
}

void SimulationParametersMainWindow::onDeleteLocation()
{
    editLocations([&](auto& parameters, auto& origParameters) { LocationEditService::get().deleteLocation(parameters, origParameters, _selectedOrderNumber); });

    if (_locations.size() - 1 == _selectedOrderNumber) {
        --_selectedOrderNumber;
    }
}

void SimulationParametersMainWindow::onDecreaseOrderNumber()
{
    editLocations(
        [&](auto& parameters, auto& origParameters) { LocationEditService::get().moveLocationUpwards(parameters, origParameters, _selectedOrderNumber); });
    --_selectedOrderNumber;
}

void SimulationParametersMainWindow::onIncreaseOrderNumber()
{
    editLocations(
        [&](auto& parameters, auto& origParameters) { LocationEditService::get().moveLocationDownwards(parameters, origParameters, _selectedOrderNumber); });
    ++_selectedOrderNumber;
}

void SimulationParametersMainWindow::onOpenInLocationWindow()
{
    auto mousePos = ImGui::GetMousePos();
    auto offset = RealVector2D{50.0f + toFloat(_locationWindowCounter) * 15, toFloat(_locationWindowCounter) * 15};
    LocationController::get().addLocationWindow(_selectedOrderNumber, {mousePos.x + offset.x, mousePos.y + offset.y});
    _locationWindowCounter = (_locationWindowCounter + 1) % 8;
}

void SimulationParametersMainWindow::onCenterLocation(int orderNumber)
{
    auto parameters = _SimulationFacade::get()->getSimulationParameters();
    auto locationType = LocationAccessService::get().getLocationType(orderNumber, parameters);
    auto arrayIndex = LocationAccessService::get().findLocationArrayIndex(parameters, orderNumber);
    RealVector2D pos;
    if (locationType == LocationType::Layer) {
        pos = parameters.layerPosition.layerValues[arrayIndex];
    } else if (locationType == LocationType::Source) {
        pos = parameters.sourcePosition.sourceValues[arrayIndex];
    }
    Viewport::get().setCenterInWorldPos(pos);
}

void SimulationParametersMainWindow::updateLocations()
{
    auto parameters = _SimulationFacade::get()->getSimulationParameters();

    _locations = std::vector<Location>(1 + parameters.numLayers + parameters.numSources);
    auto radiationStrength = RadiationStrengthService::get().getRadiationStrengths(parameters);
    auto pinnedString = radiationStrength.pinned.contains(0) ? ICON_FA_THUMBTACK " " : " ";
    _locations.at(0) = Location{"Base", LocationType::Base, "-", pinnedString + StringHelper::format(radiationStrength.values.front() * 100, 1) + "%"};
    for (int i = 0; i < parameters.numLayers; ++i) {
        auto position = "(" + StringHelper::format(parameters.layerPosition.layerValues[i].x, 0) + ", "
            + StringHelper::format(parameters.layerPosition.layerValues[i].y, 0) + ")";
        _locations.at(parameters.layerOrderNumbers[i]) = Location{
            .name = parameters.layerName.layerValues[i],
            .type = LocationType::Layer,
            .position = position,
            .strength = " " + StringHelper::format(parameters.layerOpacity.layerValues[i] * 100, 1) + "%"};
    }
    for (int i = 0; i < parameters.numSources; ++i) {
        auto position = "(" + StringHelper::format(parameters.sourcePosition.sourceValues[i].x, 0) + ", "
            + StringHelper::format(parameters.sourcePosition.sourceValues[i].y, 0) + ")";
        auto pinnedString = radiationStrength.pinned.contains(i + 1) ? ICON_FA_THUMBTACK " " : " ";
        _locations.at(parameters.sourceOrderNumbers[i]) = Location{
            parameters.sourceName.sourceValues[i],
            LocationType::Source,
            position,
            pinnedString + StringHelper::format(radiationStrength.values.at(i + 1) * 100, 1) + "%"};
    }
}

void SimulationParametersMainWindow::correctLayout(float origMasterHeight, float origExpertWidgetHeight)
{
    auto detailHeight = ImGui::GetWindowSize().y - getMasterWidgetRefHeight() - getExpertWidgetRefHeight();

    if (detailHeight < scale(DetailWidgetMinHeight) || _masterWidgetHeight < scale(MasterMinHeight) || _expertWidgetHeight < scale(ExpertWidgetMinHeight)) {
        _masterWidgetHeight = origMasterHeight;
        _expertWidgetHeight = origExpertWidgetHeight;
    }
}

void SimulationParametersMainWindow::showMaxLocationsReachedMessage(LocationType locationType) const
{
    if (locationType == LocationType::Layer) {
        GenericMessageDialog::get().localizedInformation("Error", "common.error", "parameters.limit.layers");
    } else {
        GenericMessageDialog::get().localizedInformation("Error", "common.error", "parameters.limit.sources");
    }
}

float SimulationParametersMainWindow::getMasterWidgetRefHeight() const
{
    return _masterWidgetOpen ? _masterWidgetHeight : scale(25.0f);
}

float SimulationParametersMainWindow::getExpertWidgetRefHeight() const
{
    return _expertWidgetOpen ? _expertWidgetHeight : scale(30.0f);
}

float SimulationParametersMainWindow::getMasterWidgetHeight() const
{
    if (_masterWidgetOpen && !_detailWidgetOpen && !_expertWidgetOpen) {
        return std::max(scale(MasterMinHeight), ImGui::GetContentRegionAvail().y - getDetailWidgetHeight() - getExpertWidgetRefHeight());
    }
    return getMasterWidgetRefHeight();
}

float SimulationParametersMainWindow::getDetailWidgetHeight() const
{
    return _detailWidgetOpen ? std::max(scale(MasterMinHeight), ImGui::GetContentRegionAvail().y - getExpertWidgetRefHeight() + scale(4.0f)) : scale(25.0f);
}
