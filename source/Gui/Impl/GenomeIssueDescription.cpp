#include "GenomeIssueDescription.h"

#include <algorithm>
#include <ranges>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <Fonts/IconsFontAwesome5.h>

#include <Base/Definitions.h>
#include <Base/StringHelper.h>

#include "GenomeEditorPresentation.h"
#include "StyleService.h"

char const* GenomeIssueDescription::getIcon(GenomeIssue const& issue)
{
    return issue.isFollowUp() ? ICON_FA_LONG_ARROW_ALT_RIGHT : ICON_FA_EXCLAMATION_TRIANGLE;
}

ImColor GenomeIssueDescription::getColor(GenomeIssue const& issue)
{
    return issue.isFollowUp() ? Const::TextDecentColor : Const::WarningColor;
}

std::optional<GenomeIssue> GenomeIssueDescription::findMostRelevantIssue(std::vector<GenomeIssue> const& issues)
{
    if (issues.empty()) {
        return std::nullopt;
    }
    return *std::ranges::max_element(issues, {}, [](GenomeIssue const& issue) { return !issue.isFollowUp(); });
}

namespace
{
    struct IssueText
    {
        std::string title;
        std::string problem;
        std::string explanation;
        std::string consequence;
    };

    using LocalizationGuiText::format;

    std::string formatCount(std::string const& key, int count, std::map<std::string, std::string> arguments)
    {
        auto suffix = count == 1 ? ".one" : ".many";
        if (LocalizationService::get().getLanguage() == "ru") {
            auto lastTwo = count % 100;
            auto last = count % 10;
            suffix = lastTwo >= 11 && lastTwo <= 14 ? ".many" : last == 1 ? ".one" : last >= 2 && last <= 4 ? ".few" : ".many";
        }
        arguments.emplace("count", std::to_string(count));
        return LocalizationGuiText::format(key + suffix, arguments);
    }

    std::vector<int> getVoidNodeIndices(GeneDesc const& gene)
    {
        std::vector<int> result;
        for (auto const& [nodeIndex, node] : gene._nodes | boost::adaptors::indexed(0)) {
            if (node.getCellType() == CellType_Void) {
                result.emplace_back(toInt(nodeIndex));
            }
        }
        return result;
    }

    std::vector<int> getNodeIndicesBecomingVoid(GenomeIssue const& issue, GeneDesc const& gene)
    {
        auto result = issue.cutOffNodeIndices | std::views::filter([&](int nodeIndex) { return gene._nodes.at(nodeIndex).getCellType() != CellType_Void; });
        return std::vector(result.begin(), result.end());
    }

    std::string getVoidBoundaryNodeProblem(GenomeIssue const& issue)
    {
        return tr(issue.nodeIndex == 0 ? "genomeissues.first" : "genomeissues.last");
    }

    std::string getMetaMutationNote(GenomeDesc const& genome)
    {
        return genome._applyMetaMutations ? " " + tr("genomeissues.meta") : "";
    }

    IssueText getNodesCutOffText(GenomeIssue const& issue, GenomeDesc const& genome)
    {
        auto const& gene = genome._genes.at(issue.geneIndex);
        auto nodeIndicesBecomingVoid = getNodeIndicesBecomingVoid(issue, gene);
        IssueText result{.title = tr("genomeissues.cut.title"), .explanation = tr("genomeissues.cut.explanation")};
        if (issue.nodeIndex.has_value()) {
            result.problem = getVoidBoundaryNodeProblem(issue);
            result.explanation += getMetaMutationNote(genome);
        } else if (nodeIndicesBecomingVoid.empty()) {
            result.problem = issue.cutOffNodeIndices.empty()
                ? tr("genomeissues.cut.empty")
                : formatCount("genomeissues.cut.void", toInt(issue.cutOffNodeIndices.size()), {{"nodes", StringHelper::formatRanges(issue.cutOffNodeIndices)}});
        } else {
            auto voidNodeIndices = getVoidNodeIndices(gene);
            auto key = std::string("genomeissues.cut.disconnect") + (voidNodeIndices.size() == 1 ? ".one" : ".many")
                + (nodeIndicesBecomingVoid.size() == 1 ? ".one" : ".many");
            result.problem =
                format(key, {{"voids", StringHelper::formatRanges(voidNodeIndices)}, {"nodes", StringHelper::formatRanges(nodeIndicesBecomingVoid)}});
        }
        if (issue.removesGene) {
            result.consequence = tr("genomeissues.removed");
        } else if (nodeIndicesBecomingVoid.empty()) {
            result.consequence = tr("genomeissues.constructorsoff");
        } else {
            result.consequence =
                formatCount("genomeissues.cut.become", toInt(nodeIndicesBecomingVoid.size()), {{"nodes", StringHelper::formatRanges(nodeIndicesBecomingVoid)}});
        }
        return result;
    }

    IssueText getIssueText(GenomeIssue const& issue, GenomeDesc const& genome)
    {
        switch (issue.type) {
        case GenomeIssueType::VoidBoundaryNode:
            return IssueText{
                .title = tr("genomeissues.boundary.title"),
                .problem = getVoidBoundaryNodeProblem(issue),
                .explanation = tr("genomeissues.boundary.explanation") + getMetaMutationNote(genome),
                .consequence = tr("genomeissues.boundary.consequence")};
        case GenomeIssueType::NodesCutOffFromLastNode:
            return getNodesCutOffText(issue, genome);
        case GenomeIssueType::CycleAvoidingRootGene: {
            auto genes = issue.cycleGeneIndices
                | std::views::transform([](int geneIndex) { return format("genomeissues.gene", {{"gene", std::to_string(geneIndex)}}); });
            return IssueText{
                .title = tr("genomeissues.cycle.title"),
                .problem = format(
                    "genomeissues.cycle.problem",
                    {{"cycle", boost::algorithm::join(std::vector(genes.begin(), genes.end()), " " ICON_FA_LONG_ARROW_ALT_RIGHT " ")}}),
                .explanation = tr("genomeissues.cycle.explanation"),
                .consequence = tr("genomeissues.constructoroff")};
        }
        case GenomeIssueType::GeneUnreachableFromRoot:
            return IssueText{
                .title = tr("genomeissues.unreachable.title"),
                .problem = tr(issue.isFollowUp() ? "genomeissues.unreachable.followup" : "genomeissues.unreachable.problem"),
                .explanation = tr(issue.isFollowUp() ? "genomeissues.unreachable.cause" : "genomeissues.unreachable.explanation"),
                .consequence = tr("genomeissues.removed")};
        case GenomeIssueType::TooManyGenesWithSeparation: {
            auto const& constructor = genome._genes.at(issue.geneIndex)._nodes.at(issue.nodeIndex.value())._constructor;
            auto targetGene = constructor.has_value() ? std::to_string(constructor->_geneIndex) : std::string("?");
            return IssueText{
                .title = tr("genomeissues.separation.title"),
                .problem = format(
                    "genomeissues.separation.problem", {{"target", targetGene}, {"genes", StringHelper::formatRanges(issue.geneIndicesKeepingSeparation)}}),
                .explanation = formatCount("genomeissues.separation.limit", Const::MaxGenesWithSeparation, {}),
                .consequence = format("genomeissues.separation.consequence", {{"target", targetGene}})};
        }
        case GenomeIssueType::ConstructsRemovedGene:
            return IssueText{
                .title = tr("genomeissues.constructs.title"),
                .problem = format("genomeissues.constructs.problem", {{"gene", std::to_string(issue.referencedGeneIndex.value_or(0))}}),
                .explanation = tr("genomeissues.constructs.explanation"),
                .consequence = tr("genomeissues.constructoroff")};
        case GenomeIssueType::InjectsRemovedGene:
            return IssueText{
                .title = tr("genomeissues.injects.title"),
                .problem = format("genomeissues.injects.problem", {{"gene", std::to_string(issue.referencedGeneIndex.value_or(0))}}),
                .explanation = tr("genomeissues.injects.explanation"),
                .consequence = tr("genomeissues.injector")};
        default:
            return IssueText();
        }
    }

    std::string getLocation(GenomeIssue const& issue)
    {
        if (issue.nodeIndex.has_value()) {
            return format("genomeissues.location.node", {{"gene", std::to_string(issue.geneIndex)}, {"node", std::to_string(issue.nodeIndex.value())}});
        }
        return format("genomeissues.gene", {{"gene", std::to_string(issue.geneIndex)}});
    }

    std::string getIssueTooltip(GenomeIssue const& issue, GenomeDesc const& genome)
    {
        auto text = getIssueText(issue, genome);
        return format(
            "genomeissues.tooltip",
            {{"icon", GenomeIssueDescription::getIcon(issue)},
             {"title", text.title},
             {"location", getLocation(issue)},
             {"problem", text.problem},
             {"explanation", text.explanation},
             {"consequence", text.consequence}});
    }

}

std::string GenomeIssueDescription::getTooltip(std::vector<GenomeIssue> const& issues, GenomeDesc const& genome)
{
    auto tooltips = issues | std::views::transform([&](auto const& issue) { return getIssueTooltip(issue, genome); });
    return boost::algorithm::join(std::vector(tooltips.begin(), tooltips.end()), "\n\n");
}
