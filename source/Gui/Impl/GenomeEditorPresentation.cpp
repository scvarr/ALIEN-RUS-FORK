#include "GenomeEditorPresentation.h"

#include <array>

#include <Base/LocalizationService.h>

#include <Data/CellTypeConstants.h>

namespace GenomeEditorPresentation
{
    std::string count(std::string const& key, int value)
    {
        auto suffix = value == 1 ? ".one" : ".many";
        if (LocalizationService::get().getLanguage() == "ru") {
            auto lastTwo = value % 100;
            auto last = value % 10;
            suffix = lastTwo >= 11 && lastTwo <= 14 ? ".many" : last == 1 ? ".one" : last >= 2 && last <= 4 ? ".few" : ".many";
        }
        return format(key + suffix, {{"count", std::to_string(value)}});
    }

    std::vector<std::string> shapes()
    {
        static constexpr auto keys = std::to_array<char const*>(
            {"genome.shape.segment",
             "genome.shape.triangle",
             "genome.shape.rectangle",
             "genome.shape.hexagon",
             "genome.shape.tube",
             "genome.shape.largelolli",
             "genome.shape.smalllolli",
             "genome.shape.zigzag"});
        static_assert(keys.size() == ConstructorShape_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    std::vector<std::string> cellTypes()
    {
        static constexpr auto keys = std::to_array<char const*>(
            {"genome.cell.base",
             "genome.cell.depot",
             "genome.cell.sensor",
             "genome.cell.generator",
             "genome.cell.attacker",
             "genome.cell.injector",
             "genome.cell.muscle",
             "genome.cell.defender",
             "genome.cell.reconnector",
             "genome.cell.detonator",
             "genome.cell.digestor",
             "genome.cell.memory",
             "genome.cell.communicator",
             "genome.cell.void"});
        static_assert(keys.size() == CellType_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    std::vector<std::string> provideEnergyGenomes()
    {
        static constexpr auto keys = std::to_array<char const*>({"node.option.provideenergygenome.v0", "node.option.provideenergygenome.v1"});
        static_assert(keys.size() == ProvideEnergyGenome_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    std::vector<std::string> sensorModes()
    {
        static constexpr auto keys =
            std::to_array<char const*>({"node.option.sensormode.v0", "node.option.sensormode.v1", "node.option.sensormode.v2", "node.option.sensormode.v3"});
        static_assert(keys.size() == SensorMode_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    std::vector<std::string> generatorModes()
    {
        static constexpr auto keys = std::to_array<char const*>({"node.option.generatormode.v0", "node.option.generatormode.v1"});
        static_assert(keys.size() == GeneratorMode_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    std::vector<std::string> attackerModes()
    {
        static constexpr auto keys = std::to_array<char const*>({"node.option.attackermode.v0", "node.option.attackermode.v1"});
        static_assert(keys.size() == AttackerMode_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    std::vector<std::string> muscleModes()
    {
        static constexpr auto keys = std::to_array<char const*>(
            {"node.option.musclemode.v0",
             "node.option.musclemode.v1",
             "node.option.musclemode.v2",
             "node.option.musclemode.v3",
             "node.option.musclemode.v4",
             "node.option.musclemode.v5"});
        static_assert(keys.size() == MuscleMode_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    std::vector<std::string> defenderModes()
    {
        static constexpr auto keys = std::to_array<char const*>({"node.option.defendermode.v0", "node.option.defendermode.v1"});
        static_assert(keys.size() == DefenderMode_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    std::vector<std::string> reconnectorModes()
    {
        static constexpr auto keys =
            std::to_array<char const*>({"node.option.reconnectormode.v0", "node.option.reconnectormode.v1", "node.option.reconnectormode.v2"});
        static_assert(keys.size() == ReconnectorMode_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    std::vector<std::string> memoryModes()
    {
        static constexpr auto keys =
            std::to_array<char const*>({"node.option.memorymode.v0", "node.option.memorymode.v1", "node.option.memorymode.v2", "node.option.memorymode.v3"});
        static_assert(keys.size() == MemoryMode_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    std::vector<std::string> communicatorModes()
    {
        static constexpr auto keys = std::to_array<char const*>({"node.option.communicatormode.v0", "node.option.communicatormode.v1"});
        static_assert(keys.size() == CommunicatorMode_Count);
        std::vector<std::string> result;
        for (auto key : keys) {
            result.emplace_back(tr(key));
        }
        return result;
    }

    std::string attributeDescription(EntityAttribute attribute)
    {
        switch (attribute) {
        case EntityAttribute::GenomeName:
            return tr("genome.attribute.genomename");
        case EntityAttribute::GenomeFrontAngle:
            return tr("genome.attribute.genomefrontangle");
        case EntityAttribute::GenomeResistanceToInjection:
            return tr("genome.attribute.genomeresistancetoinjection");
        case EntityAttribute::GenomeApplyMetaMutations:
            return tr("genome.attribute.genomeapplymetamutations");
        case EntityAttribute::GeneName:
            return tr("genome.attribute.genename");
        case EntityAttribute::GeneShape:
            return tr("genome.attribute.geneshape");
        case EntityAttribute::GeneConnectionDistance:
            return tr("genome.attribute.geneconnectiondistance");
        case EntityAttribute::GeneStiffness:
            return tr("genome.attribute.genestiffness");
        case EntityAttribute::GeneHomogeneousCellType:
            return tr("genome.attribute.genehomogeneouscelltype");
        case EntityAttribute::NodeAngle:
            return tr("node.attribute.nodeangle");
        case EntityAttribute::NodeCustomization:
            return tr("node.attribute.nodecustomization");
        case EntityAttribute::CellType:
            return tr("node.attribute.celltype");
        case EntityAttribute::NodeConstructionGene:
            return tr("node.attribute.nodeconstructiongene");
        case EntityAttribute::ConstructorAutoTriggerInterval:
            return tr("node.attribute.constructorautotriggerinterval");
        case EntityAttribute::ConstructorConstructionAngle:
            return tr("node.attribute.constructorconstructionangle");
        case EntityAttribute::ConstructorProvideEnergy:
            return tr("node.attribute.constructorprovideenergy");
        case EntityAttribute::ConstructorSeparation:
            return tr("node.attribute.constructorseparation");
        case EntityAttribute::ConstructorNumBranches:
            return tr("node.attribute.constructornumbranches");
        case EntityAttribute::ConstructorNumConcatenations:
            return tr("node.attribute.constructornumconcatenations");
        case EntityAttribute::DepotStorageLimit:
            return tr("node.attribute.depotstoragelimit");
        case EntityAttribute::SensorAutoTrigger:
            return tr("node.attribute.sensorautotrigger");
        case EntityAttribute::SensorTagForAttackers:
            return tr("node.attribute.sensortagforattackers");
        case EntityAttribute::SensorMode:
            return tr("node.attribute.sensormode");
        case EntityAttribute::SensorEnergyMinDensity:
            return tr("node.attribute.sensorenergymindensity");
        case EntityAttribute::SensorFreeCellMinDensity:
            return tr("node.attribute.sensorfreecellmindensity");
        case EntityAttribute::SensorRestrictToColors:
            return tr("node.attribute.sensorrestricttocolors");
        case EntityAttribute::SensorMinNumCells:
            return tr("node.attribute.sensorminnumcells");
        case EntityAttribute::SensorMaxNumCells:
            return tr("node.attribute.sensormaxnumcells");
        case EntityAttribute::SensorRestrictToLineage:
            return tr("node.attribute.sensorrestricttolineage");
        case EntityAttribute::SensorMinRange:
            return tr("node.attribute.sensorminrange");
        case EntityAttribute::SensorMaxRange:
            return tr("node.attribute.sensormaxrange");
        case EntityAttribute::GeneratorAdditive:
            return tr("node.attribute.generatoradditive");
        case EntityAttribute::GeneratorMinValue:
            return tr("node.attribute.generatorminvalue");
        case EntityAttribute::GeneratorMaxValue:
            return tr("node.attribute.generatormaxvalue");
        case EntityAttribute::GeneratorTimeOffset:
            return tr("node.attribute.generatortimeoffset");
        case EntityAttribute::GeneratorMode:
            return tr("node.attribute.generatormode");
        case EntityAttribute::GeneratorPeriod:
            return tr("node.attribute.generatorperiod");
        case EntityAttribute::AttackerMode:
            return tr("node.attribute.attackermode");
        case EntityAttribute::AttackerRestrictToColors:
            return tr("node.attribute.attackerrestricttocolors");
        case EntityAttribute::InjectorGeneIndex:
            return tr("node.attribute.injectorgeneindex");
        case EntityAttribute::MuscleMode:
            return tr("node.attribute.musclemode");
        case EntityAttribute::MuscleMaxAngleDeviation:
            return tr("node.attribute.musclemaxangledeviation");
        case EntityAttribute::MuscleForwardBackwardRatio:
            return tr("node.attribute.muscleforwardbackwardratio");
        case EntityAttribute::MuscleAttractionRepulsionRatio:
            return tr("node.attribute.muscleattractionrepulsionratio");
        case EntityAttribute::MuscleMaxDistanceDeviation:
            return tr("node.attribute.musclemaxdistancedeviation");
        case EntityAttribute::DefenderMode:
            return tr("node.attribute.defendermode");
        case EntityAttribute::ReconnectorMode:
            return tr("node.attribute.reconnectormode");
        case EntityAttribute::ReconnectorRestrictToColors:
            return tr("node.attribute.reconnectorrestricttocolors");
        case EntityAttribute::ReconnectorMinNumCells:
            return tr("node.attribute.reconnectorminnumcells");
        case EntityAttribute::ReconnectorMaxNumCells:
            return tr("node.attribute.reconnectormaxnumcells");
        case EntityAttribute::ReconnectorRestrictToLineage:
            return tr("node.attribute.reconnectorrestricttolineage");
        case EntityAttribute::DetonatorCountdown:
            return tr("node.attribute.detonatorcountdown");
        case EntityAttribute::DigestorEnergyConductivity:
            return tr("node.attribute.digestorenergyconductivity");
        case EntityAttribute::DigestorEnergyConversion:
            return tr("node.attribute.digestorenergyconversion");
        case EntityAttribute::MemoryMode:
            return tr("node.attribute.memorymode");
        case EntityAttribute::MemoryDelay:
            return tr("node.attribute.memorydelay");
        case EntityAttribute::MemoryReadOnly:
            return tr("node.attribute.memoryreadonly");
        case EntityAttribute::MemorySignalBuffer:
            return tr("node.attribute.memorysignalbuffer");
        case EntityAttribute::MemoryNewSignalWeight:
            return tr("node.attribute.memorynewsignalweight");
        case EntityAttribute::MemoryChannelMask:
            return tr("node.attribute.memorychannelmask");
        case EntityAttribute::CommunicatorMode:
            return tr("node.attribute.communicatormode");
        case EntityAttribute::CommunicatorRange:
            return tr("node.attribute.communicatorrange");
        case EntityAttribute::CommunicatorOneway:
            return tr("node.attribute.communicatoroneway");
        case EntityAttribute::CommunicatorRestrictToColors:
            return tr("node.attribute.communicatorrestricttocolors");
        case EntityAttribute::CommunicatorRestrictToLineage:
            return tr("node.attribute.communicatorrestricttolineage");
        case EntityAttribute::GenomeMutationRatesEdit:
            return tr("mutation.attribute.genomemutationratesedit");
        case EntityAttribute::GenomeMutationRatesSummary:
            return tr("mutation.attribute.genomemutationratessummary");
        case EntityAttribute::MutationConnectionProbability:
            return tr("mutation.attribute.mutationconnectionprobability");
        case EntityAttribute::MutationConnectionValueChangeSigma:
            return tr("mutation.attribute.mutationconnectionvaluechangesigma");
        case EntityAttribute::MutationNeuronProbability:
            return tr("mutation.attribute.mutationneuronprobability");
        case EntityAttribute::MutationNeuronWeightChangeSigma:
            return tr("mutation.attribute.mutationneuronweightchangesigma");
        case EntityAttribute::MutationNeuronBiasChangeSigma:
            return tr("mutation.attribute.mutationneuronbiaschangesigma");
        case EntityAttribute::MutationNeuronActfnChangeProbability:
            return tr("mutation.attribute.mutationneuronactfnchangeprobability");
        case EntityAttribute::MutationCellTypePropertiesProbability:
            return tr("mutation.attribute.mutationcelltypepropertiesprobability");
        case EntityAttribute::MutationCellTypePropertiesValueChangeSigma:
            return tr("mutation.attribute.mutationcelltypepropertiesvaluechangesigma");
        case EntityAttribute::MutationCellTypePropertiesEnumChangeProbability:
            return tr("mutation.attribute.mutationcelltypepropertiesenumchangeprobability");
        case EntityAttribute::MutationGeometryProbability:
            return tr("mutation.attribute.mutationgeometryprobability");
        case EntityAttribute::MutationGeometryValueChangeSigma:
            return tr("mutation.attribute.mutationgeometryvaluechangesigma");
        case EntityAttribute::MutationGeometryEnumChangeProbability:
            return tr("mutation.attribute.mutationgeometryenumchangeprobability");
        case EntityAttribute::MutationCellTypeModeProbability:
            return tr("mutation.attribute.mutationcelltypemodeprobability");
        case EntityAttribute::MutationCellTypeProbability:
            return tr("mutation.attribute.mutationcelltypeprobability");
        case EntityAttribute::MutationCustomizationProbability:
            return tr("mutation.attribute.mutationcustomizationprobability");
        case EntityAttribute::MutationVoidProbability:
            return tr("mutation.attribute.mutationvoidprobability");
        case EntityAttribute::MutationExtendGeneProbability:
            return tr("mutation.attribute.mutationextendgeneprobability");
        case EntityAttribute::MutationAddNodeProbability:
            return tr("mutation.attribute.mutationaddnodeprobability");
        case EntityAttribute::MutationTrimGeneProbability:
            return tr("mutation.attribute.mutationtrimgeneprobability");
        case EntityAttribute::MutationDeleteNodeProbability:
            return tr("mutation.attribute.mutationdeletenodeprobability");
        case EntityAttribute::MutationAddGeneProbability:
            return tr("mutation.attribute.mutationaddgeneprobability");
        case EntityAttribute::MutationDuplicateGeneProbability:
            return tr("mutation.attribute.mutationduplicategeneprobability");
        case EntityAttribute::MutationDeleteGeneProbability:
            return tr("mutation.attribute.mutationdeletegeneprobability");
        case EntityAttribute::MutationSwapGeneProbability:
            return tr("mutation.attribute.mutationswapgeneprobability");
        case EntityAttribute::MutationCopyNodeSectionProbability:
            return tr("mutation.attribute.mutationcopynodesectionprobability");
        case EntityAttribute::MutationMoveNodeSectionProbability:
            return tr("mutation.attribute.mutationmovenodesectionprobability");
        case EntityAttribute::MutationConstructorProbability:
            return tr("mutation.attribute.mutationconstructorprobability");
        case EntityAttribute::MutationConstructorValueChangeSigma:
            return tr("mutation.attribute.mutationconstructorvaluechangesigma");
        case EntityAttribute::MutationConstructorEnumChangeProbability:
            return tr("mutation.attribute.mutationconstructorenumchangeprobability");
        case EntityAttribute::MutationConstructorToggleProbability:
            return tr("mutation.attribute.mutationconstructortoggleprobability");
        default:
            return EntityAttributeHelp::get(attribute);
        }
    }
}
