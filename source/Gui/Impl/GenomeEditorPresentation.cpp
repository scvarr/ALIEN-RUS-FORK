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
        default:
            return EntityAttributeHelp::get(attribute);
        }
    }
}
