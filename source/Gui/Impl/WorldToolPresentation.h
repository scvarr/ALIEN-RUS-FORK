#pragma once

#include <algorithm>
#include <initializer_list>
#include <map>
#include <string>

#include <imgui.h>
#include <imgui_internal.h>

#include <Base/LocalizationService.h>
#include <Base/StringHelper.h>

#include "GenomeEditorPresentation.h"
#include "StyleService.h"

namespace WorldToolPresentation
{
    inline bool selectable(std::string const& name, std::string const& label, ImGuiSelectableFlags flags = 0)
    {
        auto pos = ImGui::GetCursorScreenPos();
        auto labelSize = ImGui::CalcTextSize(label.c_str());
        ImGui::PushOverrideID(ImGui::GetID(name.c_str()));
        auto pressed = ImGui::Selectable("", false, flags, {labelSize.x, labelSize.y});
        ImGui::PopID();
        ImGui::BeginDisabled((flags & ImGuiSelectableFlags_Disabled) != 0);
        ImGui::RenderTextClipped(
            pos, {ImGui::GetItemRectMax().x, pos.y + labelSize.y}, label.c_str(), nullptr, &labelSize, ImGui::GetStyle().SelectableTextAlign);
        ImGui::EndDisabled();
        return pressed;
    }

    inline std::string text(std::string const& name)
    {
        static std::map<std::string, std::string> const keys = {
            {"Select and move", "worldtools.selectandmove"},
            {"Apply forces", "worldtools.applyforces"},
            {"Scissors: cut connections", "worldtools.scissorscutconnections"},
            {"Draw freehand", "worldtools.drawfreehand"},
            {"Single object", "worldtools.singleobject"},
            {"Rectangular object network", "worldtools.rectangularobjectnetwork"},
            {"Hexagonal object network", "worldtools.hexagonalobjectnetwork"},
            {"Disc-shaped object network", "worldtools.discshapedobjectnetwork"},
            {"Object network along a line", "worldtools.objectnetworkalongaline"},
            {"Object network along a Bezier curve", "worldtools.objectnetworkalongabeziercurve"},
            {"Polygon-shaped object network", "worldtools.polygonshapedobjectnetwork"},
            {"Grid multiplier", "worldtools.gridmultiplier"},
            {"Random multiplier", "worldtools.randommultiplier"},
            {"Number of copies", "worldtools.numberofcopies"},
            {"Distance", "inspection.label.distance"},
            {"Angle increment", "worldtools.angleincrement"},
            {"Velocity X increment", "worldtools.velocityxincrement"},
            {"Velocity Y increment", "worldtools.velocityyincrement"},
            {"Angular vel. increment", "worldtools.angularvelincrement"},
        };
        return tr(keys.at(name));
    }

    inline float textWidth(float minimum, std::initializer_list<char const*> keys)
    {
        for (auto key : keys) {
            minimum = std::max(minimum, ImGui::CalcTextSize(tr(key).c_str()).x / StyleService::get().scale(1.0f) + 12.0f);
        }
        return minimum;
    }

    inline float creatorTextWidth()
    {
        return textWidth(
            140.0f,
            {"parameters.color.color",
             "worldtools.pencilradius",
             "worldtools.material",
             "new.energy",
             "parameters.visualization.glow.name",
             "genome.field.gene.stiffness",
             "worldtools.horizontalobjects",
             "worldtools.verticalobjects",
             "worldtools.layers",
             "worldtools.outerradius",
             "worldtools.innerradius",
             "worldtools.objectdistance",
             "inspection.label.sticky",
             "inspection.label.static"});
    }

    inline float transformTextWidth()
    {
        return textWidth(
            130.0f,
            {"worldtools.positionx",
             "worldtools.positiony",
             "worldtools.velocityx",
             "worldtools.velocityy",
             "parameters.forcefield.layerlinearforcefieldangle.name",
             "worldtools.angularvelocity"});
    }

    inline float randomTextWidth()
    {
        return textWidth(
            170.0f,
            {"worldtools.numberofcopies",
             "worldtools.minangle",
             "worldtools.maxangle",
             "worldtools.minvelocityx",
             "worldtools.maxvelocityx",
             "worldtools.minvelocityy",
             "worldtools.maxvelocityy",
             "worldtools.minangularvelocity",
             "worldtools.maxangularvelocity",
             "worldtools.overlappingcheck"});
    }

    inline float gridTextWidth()
    {
        return textWidth(
            130.0f,
            {"worldtools.numberofcopies",
             "inspection.label.distance",
             "worldtools.angleincrement",
             "worldtools.velocityxincrement",
             "worldtools.velocityyincrement",
             "worldtools.angularvelincrement"});
    }

    inline float massTextWidth()
    {
        return textWidth(
            140.0f,
            {"parameters.lifecycle.mincellenergy.name",
             "worldtools.maximumenergy",
             "worldtools.minimumage",
             "parameters.lifecycle.maxcellage.name",
             "worldtools.minimumvalue",
             "worldtools.maximumvalue",
             "worldtools.minimumglow",
             "worldtools.maximumglow"});
    }

    inline std::string selectionCount(std::string const& key, int value)
    {
        auto result = GenomeEditorPresentation::count(key, value);
        result.replace(0, std::to_string(value).size(), StringHelper::format(static_cast<uint64_t>(value)));
        return result;
    }
}
