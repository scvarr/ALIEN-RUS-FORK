#include "NewSimulationDialog.h"

#include <imgui.h>

#include <Base/GlobalSettings.h>
#include <Base/LocalizationService.h>
#include <Base/NameGeneratorService.h>
#include <Base/StringHelper.h>

#include <Engine/Interface/SimulationFacade.h>

#include "AlienGui.h"
#include "NewSimulationService.h"
#include "StyleService.h"
#include <Engine/Interface/SimulationFacade.h>

namespace
{
    auto const ContentTextInputWidth = 290.0f;
    auto const ProjectNameSize = sizeof(Char64) / sizeof(char);
}

void NewSimulationDialog::initIntern()
{

    _adoptSimulationParameters = GlobalSettings::get().getValue("dialogs.new simulation.adopt simulation parameters", true);
}

void NewSimulationDialog::shutdownIntern()
{
    GlobalSettings::get().setValue("dialogs.new simulation.adopt simulation parameters", _adoptSimulationParameters);
}

NewSimulationDialog::NewSimulationDialog()
    : AlienDialog("New simulation", {620.0f, 330.0f})
{
    setTitleKey("new.title");
}

void NewSimulationDialog::processIntern()
{
    AlienGui::InputText(
        AlienGui::InputTextParameters()
            .name("Project name")
            .label(tr("new.project"))
            .textWidth(ContentTextInputWidth)
            .generateValueTooltip(tr("new.generate"))
            .generateValueFunc([] { return NameGeneratorService::get().createSimulationName(); }),
        _projectName,
        ProjectNameSize);
    AlienGui::InputInt(AlienGui::InputIntParameters().name("Width").label(tr("common.width")).textWidth(ContentTextInputWidth), _width);
    AlienGui::InputInt(AlienGui::InputIntParameters().name("Height").label(tr("common.height")).textWidth(ContentTextInputWidth), _height);
    AlienGui::InputFloat(
        AlienGui::InputFloatParameters().name("Energy").label(tr("new.energy")).textWidth(ContentTextInputWidth).format("%.0f").step(1000.0f),
        _externalEnergy);
    AlienGui::Checkbox(
        AlienGui::CheckboxParameters().name("Adopt parameters").label(tr("new.adopt")).textWidth(ContentTextInputWidth), &_adoptSimulationParameters);

    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - scale(50.0f)});
    AlienGui::Separator();
    if (AlienGui::Button(tr("common.ok") + "###OK")) {
        ImGui::CloseCurrentPopup();
        onNewSimulation();
        close();
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienGui::Button(tr("common.cancel") + "###Cancel")) {
        ImGui::CloseCurrentPopup();
        close();
    }

    _width = std::max(1, _width);
    _height = std::max(1, _height);
    _externalEnergy = std::max(0.0f, _externalEnergy);
}

void NewSimulationDialog::openIntern()
{
    StringHelper::copy(_projectName, ProjectNameSize, NameGeneratorService::get().createSimulationName());

    auto worldSize = _SimulationFacade::get()->getWorldSize();
    _width = worldSize.x;
    _height = worldSize.y;
    _externalEnergy = _SimulationFacade::get()->getSimulationParameters().externalEnergy.value;
}

void NewSimulationDialog::onNewSimulation()
{
    NewSimulationService::get().createSimulation(NewSimulationService::Parameters()
                                                     .projectName(_projectName)
                                                     .worldSize({_width, _height})
                                                     .externalEnergy(_externalEnergy)
                                                     .adoptSimulationParameters(_adoptSimulationParameters));
}
