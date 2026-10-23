#pragma once

#include <algorithm>
#include <map>
#include <string>

#include <imgui.h>

#include <Base/LocalizationService.h>

#include "StyleService.h"

namespace ControlStatisticsPresentation
{
    inline std::string text(std::string const& name)
    {
        static std::map<std::string, std::string> const keys = {
            {"Time steps per second", "simulationui.timestepspersecond"},
            {"Total time steps", "simulationui.totaltimesteps"},
            {"Real-time", "simulationui.realtime"},
            {"Slow down", "simulationui.slowdown"},
            {"Sync with rendering", "simulationui.syncwithrendering"},
            {"Run", "menu.run"},
            {"Pause", "menu.pause"},
            {"Load previous time step", "simulationui.loadprevioustimestep"},
            {"Loading previous time step ...", "simulationui.loadingprevioustimestep"},
            {"Process single time step", "simulationui.processsingletimestep"},
            {"Create flashback", "simulationui.createflashback"},
            {"Creating in-memory flashback: It saves the content of the current world to the memory.", "simulationui.flashback.createhelp"},
            {"Creating flashback ...", "simulationui.creatingflashback"},
            {"Load flashback", "simulationui.loadflashback"},
            {"Loading in-memory flashback: It loads the saved world from the memory. Static simulation parameters will not be changed. Non-static parameters "
             "(such as the position of moving layers) will be restored as well.",
             "simulationui.flashback.loadhelp"},
            {"Loading flashback ...", "simulationui.loadingflashback"},
            {"World size", "simulationui.worldsize"},
            {"Zoom factor", "simulationui.zoomfactor"},
            {"Center position", "simulationui.centerposition"},
            {"Autotracking on selection", "simulationui.autotrackingonselection"},
            {"Zoom sensitivity", "simulationui.zoomsensitivity"},
            {"Zoom in", "simulationui.zoomin"},
            {"Zoom out", "simulationui.zoomout"},
            {"Center", "simulationui.center"},
            {"Resize", "simulationui.resize"},
            {"Creatures", "simulationui.creatures"},
            {"Avg cells", "simulationui.avgcells"},
            {"Avg nodes", "simulationui.avgnodes"},
            {"Avg generation", "simulationui.avggeneration"},
            {"Avg mut. rate", "simulationui.avgmutrate"},
            {"Avg mutation rate", "simulationui.avgmutationrate"},
            {"Internal energy", "simulationui.internalenergy"},
            {"External energy", "simulationui.externalenergy"},
            {"Created /1K", "simulationui.created1k"},
            {"Mutations /1K", "simulationui.mutations1k"},
            {"Avg attacks /1K", "simulationui.avgattacks1k"},
            {"Avg muscles /1K", "simulationui.avgmuscles1k"},
            {"Entities", "simulationui.entities"},
            {"Solids", "simulationui.solids"},
            {"Fluids", "simulationui.fluids"},
            {"Cells", "simulationui.cells"},
            {"Free cells", "worldtools.freecells"},
            {"Energy particles", "worldtools.energyparticles"},
            {"Lineages", "simulationui.lineages"},
            {"> 1% creatures", "simulationui.aboveonepercent"},
            {"> 5% creatures", "simulationui.abovefivepercent"},
            {"Total energy", "simulationui.totalenergy"},
            {"Filter by customizations", "simulationui.filterbycustomizations"},
            {"Lineage", "parameters.visualization.objectcoloring.option3"},
            {"Customizations", "simulationui.customizations"},
            {"Open the genome of a representative creature (highest generation) in the genome editor", "simulationui.representative.help"},
            {"The creature no longer exists", "simulationui.thecreaturenolongerexists"},
            {"Last time steps", "simulationui.lasttimesteps"},
            {"Entire history", "simulationui.entirehistory"},
            {"Mode", "node.label.mode"},
            {"Steps", "simulationui.steps"},
            {"Plot height", "simulationui.plotheight"},
            {"infinity", "simulationui.infinity"},
            {"No data", "simulationui.nodata"},
        };
        return tr(keys.at(name));
    }

    inline float textWidth(float minimum, std::string const& name)
    {
        return std::max(minimum, (ImGui::CalcTextSize(text(name).c_str()).x + ImGui::GetStyle().ItemSpacing.x) / StyleService::get().scale(1.0f) + 8.0f);
    }
}
