#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>

#include <Data/CellTypeConstants.h>

#include "GenomeEditorPresentation.h"

namespace InspectionPresentation
{
    using LocalizationGuiText::format;

    inline constexpr auto labelKeys = std::to_array<char const*>({
        "node.label.geneindex",
        "inspection.label.energyparticle",
        "inspection.label.position",
        "neural.velocity",
        "new.energy",
        "parameters.color.color",
        "inspection.label.object",
        "genome.field.gene.stiffness",
        "inspection.label.static",
        "inspection.label.sticky",
        "inspection.label.objecttype",
        "inspection.label.connections",
        "inspection.label.distance",
        "inspection.label.refangle",
        "inspection.label.solid",
        "inspection.label.fluid",
        "parameters.visualization.glow.name",
        "inspection.label.freecell",
        "neural.age",
        "inspection.label.cell",
        "inspection.label.usableenergy",
        "inspection.label.rawenergy",
        "genome.field.frontangle",
        "inspection.label.cellstate",
        "neural.preview.node",
        "inspection.label.concatenationindex",
        "inspection.label.branchindex",
        "inspection.label.constructionid",
        "inspection.label.activationtime",
        "inspection.label.headcell",
        "neural.preview.cell",
        "inspection.label.hasconstructor",
        "mutation.constructorsummary",
        "node.label.autotriggerinterval",
        "node.label.constructionangle",
        "node.label.provideenergy",
        "inspection.label.reservedenergy",
        "node.label.separation",
        "node.label.numberofbranches",
        "node.label.concatenations",
        "inspection.label.lineageid",
        "inspection.label.editgenome",
        "inspection.label.creature",
        "inspection.label.neuralactivity",
        "neural.title",
        "node.label.channelmaskbit03",
        "node.label.channelmaskbit47",
        "inspection.label.storagelimit",
        "inspection.label.storedusableenergy",
        "node.label.autotrigger",
        "node.label.tagforattackers",
        "node.label.mode",
        "node.label.mindensity",
        "node.label.restricttocolors",
        "inspection.label.minnumcells",
        "inspection.label.maxnumcells",
        "node.label.restricttolineage",
        "node.label.minrange",
        "node.label.maxrange",
        "node.label.additive",
        "node.label.minvalue",
        "node.label.maxvalue",
        "node.label.timeoffset",
        "node.label.period",
        "node.label.maxangledeviation",
        "node.label.maxdistancedeviation",
        "node.label.forwardbackwardratio",
        "node.label.attractionrepulsionratio",
        "inspection.label.state",
        "node.label.countdown",
        "node.label.energyconductivity",
        "node.label.energyconversion",
        "node.label.delay",
        "node.label.readonly",
        "node.label.newsignalweight",
        "node.label.signalbuffer",
        "node.label.range",
        "node.label.oneway",
        "inspection.label.particleid",
        "inspection.label.objectid",
        "inspection.label.connectedid",
        "inspection.label.creatureid",
        "inspection.label.generation",
        "inspection.label.numcells",
        "inspection.label.mutationsinlineage",
        "inspection.label.mutationstotal",
        "genome.field.name",
        "genome.field.resistance",
        "genome.field.metamutations",
        "inspection.label.properties",
    });

    inline std::string label(std::string const& name)
    {
        static std::map<std::string, std::string> const keys = {
            {"Particle id", "inspection.label.particleid"},
            {"Object id", "inspection.label.objectid"},
            {"Connected id", "inspection.label.connectedid"},
            {"Creature id", "inspection.label.creatureid"},
            {"Generation", "inspection.label.generation"},
            {"Num cells", "inspection.label.numcells"},
            {"Mutations (in lineage)", "inspection.label.mutationsinlineage"},
            {"Mutations (total)", "inspection.label.mutationstotal"},
            {"Genome name", "genome.field.name"},
            {"Resistance to injection", "genome.field.resistance"},
            {"Apply meta-mutations", "genome.field.metamutations"},
        };
        return tr(keys.at(name));
    }

    inline std::string attributeDescription(EntityAttribute attribute)
    {
        switch (attribute) {
        case EntityAttribute::ObjectId:
            return tr("inspection.attribute.objectid");
        case EntityAttribute::Position:
            return tr("inspection.attribute.position");
        case EntityAttribute::Velocity:
            return tr("inspection.attribute.velocity");
        case EntityAttribute::Stiffness:
            return tr("inspection.attribute.stiffness");
        case EntityAttribute::Color:
            return tr("inspection.attribute.color");
        case EntityAttribute::Static:
            return tr("inspection.attribute.static");
        case EntityAttribute::Sticky:
            return tr("inspection.attribute.sticky");
        case EntityAttribute::ObjectType:
            return tr("inspection.attribute.objecttype");
        case EntityAttribute::ConnectedId:
            return tr("inspection.attribute.connectedid");
        case EntityAttribute::ConnectionDistance:
            return tr("inspection.attribute.connectiondistance");
        case EntityAttribute::ConnectionRefAngle:
            return tr("inspection.attribute.connectionrefangle");
        case EntityAttribute::ParticleId:
            return tr("inspection.attribute.particleid");
        case EntityAttribute::ParticleEnergy:
            return tr("inspection.attribute.particleenergy");
        case EntityAttribute::SolidEnergy:
            return tr("inspection.attribute.solidenergy");
        case EntityAttribute::FluidEnergy:
            return tr("inspection.attribute.fluidenergy");
        case EntityAttribute::FluidGlow:
            return tr("inspection.attribute.fluidglow");
        case EntityAttribute::FreeCellEnergy:
            return tr("inspection.attribute.freecellenergy");
        case EntityAttribute::FreeCellAge:
            return tr("inspection.attribute.freecellage");
        case EntityAttribute::CellUsableEnergy:
            return tr("inspection.attribute.cellusableenergy");
        case EntityAttribute::CellRawEnergy:
            return tr("inspection.attribute.cellrawenergy");
        case EntityAttribute::CellFrontAngle:
            return tr("inspection.attribute.cellfrontangle");
        case EntityAttribute::CellAge:
            return tr("inspection.attribute.cellage");
        case EntityAttribute::CellState:
            return tr("inspection.attribute.cellstate");
        case EntityAttribute::CellNodeIndex:
            return tr("inspection.attribute.cellnodeindex");
        case EntityAttribute::CellGeneIndex:
            return tr("inspection.attribute.cellgeneindex");
        case EntityAttribute::CellConcatenationIndex:
            return tr("inspection.attribute.cellconcatenationindex");
        case EntityAttribute::CellBranchIndex:
            return tr("inspection.attribute.cellbranchindex");
        case EntityAttribute::CellConstructionId:
            return tr("inspection.attribute.cellconstructionid");
        case EntityAttribute::CellActivationTime:
            return tr("inspection.attribute.cellactivationtime");
        case EntityAttribute::CellHeadCell:
            return tr("inspection.attribute.cellheadcell");
        case EntityAttribute::CellHasConstructor:
            return tr("inspection.attribute.cellhasconstructor");
        case EntityAttribute::ConstructorReservedEnergy:
            return tr("inspection.attribute.constructorreservedenergy");
        case EntityAttribute::ConstructorGeneIndex:
            return tr("inspection.attribute.constructorgeneindex");
        case EntityAttribute::DepotStoredEnergy:
            return tr("inspection.attribute.depotstoredenergy");
        case EntityAttribute::DetonatorState:
            return tr("inspection.attribute.detonatorstate");
        case EntityAttribute::NeuralSignal:
            return tr("inspection.attribute.neuralsignal");
        case EntityAttribute::NeuralMemory:
            return tr("inspection.attribute.neuralmemory");
        case EntityAttribute::CreatureId:
            return tr("inspection.attribute.creatureid");
        case EntityAttribute::CreatureGeneration:
            return tr("inspection.attribute.creaturegeneration");
        case EntityAttribute::CreatureNumCells:
            return tr("inspection.attribute.creaturenumcells");
        case EntityAttribute::CreatureLineageId:
            return tr("inspection.attribute.creaturelineageid");
        case EntityAttribute::CreatureMutationsInLineage:
            return tr("inspection.attribute.creaturemutationsinlineage");
        case EntityAttribute::CreatureMutationsTotal:
            return tr("inspection.attribute.creaturemutationstotal");
        case EntityAttribute::CreatureGenomeName:
            return tr("inspection.attribute.creaturegenomename");
        case EntityAttribute::CreatureResistanceToInjection:
            return tr("inspection.attribute.creatureresistancetoinjection");
        case EntityAttribute::CreatureApplyMetaMutations:
            return tr("inspection.attribute.creatureapplymetamutations");
        case EntityAttribute::CreatureEditGenome:
            return tr("inspection.attribute.creatureeditgenome");
        default:
            return GenomeEditorPresentation::attributeDescription(attribute);
        }
    }

    inline std::vector<std::string> objectTypes()
    {
        static_assert(ObjectType_Count == 4);
        return {tr("inspection.label.solid"), tr("inspection.label.fluid"), tr("inspection.label.freecell"), tr("inspection.label.cell")};
    }

    inline std::vector<std::string> provideEnergy()
    {
        static_assert(ProvideEnergy_Count == 3);
        auto result = GenomeEditorPresentation::provideEnergyGenomes();
        result.emplace_back(Const::ProvideEnergyStrings.at(ProvideEnergy_Free));
        return result;
    }

    inline std::vector<std::string> cellStates()
    {
        static_assert(CellState_Count == 5);
        return {
            tr("inspection.state.cell.ready"),
            tr("inspection.state.cell.construction"),
            tr("inspection.state.cell.activation"),
            tr("inspection.state.cell.dying"),
            tr("inspection.state.cell.instant")};
    }

    inline std::vector<std::string> detonatorStates()
    {
        static_assert(DetonatorState_Ready == 0 && DetonatorState_Activated == 1 && DetonatorState_Exploded == 2);
        return {tr("inspection.state.detonator.ready"), tr("inspection.state.detonator.activated"), tr("inspection.state.detonator.exploded")};
    }
}
