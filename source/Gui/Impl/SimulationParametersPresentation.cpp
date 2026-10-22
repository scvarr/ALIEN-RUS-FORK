#include "SimulationParametersPresentation.h"

#include <Base/LocalizationService.h>
#include <Data/SimulationParameters.h>

#include "Utf8Search.h"

namespace
{
    template <typename Spec, typename Member>
    bool references(ParameterSpec const& parameter, Member member)
    {
        auto spec = std::get_if<Spec>(&parameter._reference);
        if (!spec) {
            return false;
        }
        auto reference = std::get_if<std::shared_ptr<Member>>(&spec->_member);
        return reference && *reference && **reference == member;
    }

    std::string key(ParameterSpec const& spec)
    {
        if (references<Char64Spec>(spec, &SimulationParameters::projectName)) {
            return "parameters.general.project";
        }
        if (references<Char64Spec>(spec, &SimulationParameters::layerName)) {
            return "parameters.general.layer";
        }
        if (references<Char64Spec>(spec, &SimulationParameters::sourceName)) {
            return "parameters.general.source";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerOpacity)) {
            return "parameters.general.opacity";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::sourceRelativeStrength)) {
            return "parameters.general.share";
        }
        if (references<Float2Spec>(spec, &SimulationParameters::layerPosition)) {
            return "parameters.location.layerposition";
        }
        if (references<Float2Spec>(spec, &SimulationParameters::layerVelocity)) {
            return "parameters.location.layervelocity";
        }
        if (references<Float2Spec>(spec, &SimulationParameters::sourcePosition)) {
            return "parameters.location.sourceposition";
        }
        if (references<Float2Spec>(spec, &SimulationParameters::sourceVelocity)) {
            return "parameters.location.sourcevelocity";
        }
        if (references<ColorSpec>(spec, &SimulationParameters::backgroundColor)) {
            return "parameters.visualization.backgroundcolor";
        }
        if (references<ColorSpec>(spec, &SimulationParameters::customizationColors)) {
            return "parameters.visualization.customizationcolors";
        }
        if (references<AlternativeSpec>(spec, &SimulationParameters::objectColoring)) {
            return "parameters.visualization.objectcoloring";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::glow)) {
            return "parameters.visualization.glow";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::fluidIllumination)) {
            return "parameters.visualization.fluidillumination";
        }
        if (references<BoolSpec>(spec, &SimulationParameters::borderlessRendering)) {
            return "parameters.visualization.borderlessrendering";
        }
        if (references<BoolSpec>(spec, &SimulationParameters::gridLines)) {
            return "parameters.visualization.gridlines";
        }
        if (references<BoolSpec>(spec, &SimulationParameters::markReferenceDomain)) {
            return "parameters.visualization.markreferencedomain";
        }
        if (references<BoolSpec>(spec, &SimulationParameters::sourceShowRadiationCenter)) {
            return "parameters.visualization.sourceshowradiationcenter";
        }
        if (references<ColorSpec>(spec, &SimulationParameters::sourceLightColor)) {
            return "parameters.visualization.sourcelightcolor";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::sourceLightBrightness)) {
            return "parameters.visualization.sourcelightbrightness";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::sourceLightRange)) {
            return "parameters.visualization.sourcelightrange";
        }
        if (references<BoolSpec>(spec, &SimulationParameters::layerShowForceField)) {
            return "parameters.visualization.layershowforcefield";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::externalEnergy)) {
            return "parameters.energy.externalenergy";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::externalEnergyInflowForConstructor)) {
            return "parameters.energy.externalenergyinflowforconstructor";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::externalEnergyInflowThresholdFactor)) {
            return "parameters.energy.externalenergyinflowthresholdfactor";
        }
        if (references<BoolSpec>(spec, &SimulationParameters::externalEnergyInflowOnlyForFirstOffspring)) {
            return "parameters.energy.externalenergyinflowonlyforfirstoffspring";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::externalEnergyInflowForSources)) {
            return "parameters.energy.externalenergyinflowforsources";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::externalEnergyBackflowFactor)) {
            return "parameters.energy.externalenergybackflowfactor";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::externalEnergyBackflowLimit)) {
            return "parameters.energy.externalenergybackflowlimit";
        }
        if (references<AlternativeSpec>(spec, &SimulationParameters::layerShape)) {
            return "parameters.shape.layershape";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerCoreRadius)) {
            return "parameters.shape.layercoreradius";
        }
        if (references<Float2Spec>(spec, &SimulationParameters::layerCoreRect)) {
            return "parameters.shape.layercorerect";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerFadeoutRadius)) {
            return "parameters.shape.layerfadeoutradius";
        }
        if (references<AlternativeSpec>(spec, &SimulationParameters::sourceShapeType)) {
            return "parameters.shape.sourceshapetype";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::sourceCircularRadius)) {
            return "parameters.shape.sourcecircularradius";
        }
        if (references<Float2Spec>(spec, &SimulationParameters::sourceRectangularRect)) {
            return "parameters.shape.sourcerectangularrect";
        }
        if (references<AlternativeSpec>(spec, &SimulationParameters::layerForceFieldType)) {
            return "parameters.forcefield.layerforcefieldtype";
        }
        if (references<AlternativeSpec>(spec, &SimulationParameters::layerRadialForceFieldOrientation)) {
            return "parameters.forcefield.layerradialforcefieldorientation";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerRadialForceFieldStrength)) {
            return "parameters.forcefield.layerradialforcefieldstrength";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerRadialForceFieldDriftAngle)) {
            return "parameters.forcefield.layerradialforcefielddriftangle";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerCentralForceFieldStrength)) {
            return "parameters.forcefield.layercentralforcefieldstrength";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerLinearForceFieldAngle)) {
            return "parameters.forcefield.layerlinearforcefieldangle";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerLinearForceFieldStrength)) {
            return "parameters.forcefield.layerlinearforcefieldstrength";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerPerlinNoiseForceFieldStrength)) {
            return "parameters.forcefield.layerperlinnoiseforcefieldstrength";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerPerlinNoiseForceFieldSpatialSize)) {
            return "parameters.forcefield.layerperlinnoiseforcefieldspatialsize";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::layerPerlinNoiseForceFieldTemporalSize)) {
            return "parameters.forcefield.layerperlinnoiseforcefieldtemporalsize";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::timestepSize)) {
            return "parameters.motion.timestepsize";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::smoothingLength)) {
            return "parameters.motion.smoothinglength";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::pressureStrength)) {
            return "parameters.motion.pressurestrength";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::viscosityStrength)) {
            return "parameters.motion.viscositystrength";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::friction)) {
            return "parameters.motion.friction";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::innerFriction)) {
            return "parameters.motion.innerfriction";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::rigidity)) {
            return "parameters.motion.rigidity";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::maxVelocity)) {
            return "parameters.thresholds.maxvelocity";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::maxForce)) {
            return "parameters.thresholds.maxforce";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::minObjectDistance)) {
            return "parameters.thresholds.minobjectdistance";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::maxBindingDistance)) {
            return "parameters.thresholds.maxbindingdistance";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::objectFusionVelocity)) {
            return "parameters.thresholds.objectfusionvelocity";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::relativeStrengthBasePin)) {
            return "parameters.radiation.relativestrengthbasepin";
        }
        if (references<BoolSpec>(spec, &SimulationParameters::disableRadiationSources)) {
            return "parameters.radiation.disableradiationsources";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::radiationAbsorption)) {
            return "parameters.radiation.radiationabsorption";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::radiationType1_strength)) {
            return "parameters.radiation.radiationtype1strength";
        }
        if (references<IntSpec>(spec, &SimulationParameters::radiationType1_minimumAge)) {
            return "parameters.radiation.radiationtype1minimumage";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::radiationType2_strength)) {
            return "parameters.radiation.radiationtype2strength";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::radiationType2_energyThreshold)) {
            return "parameters.radiation.radiationtype2energythreshold";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::particleSplitEnergy)) {
            return "parameters.radiation.particlesplitenergy";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::sourceRadiationAngle)) {
            return "parameters.radiation.sourceradiationangle";
        }
        if (references<IntSpec>(spec, &SimulationParameters::maxCellAge)) {
            return "parameters.lifecycle.maxcellage";
        }
        if (references<IntSpec>(spec, &SimulationParameters::freeCellMaxAge)) {
            return "parameters.lifecycle.freecellmaxage";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::minCellEnergy)) {
            return "parameters.lifecycle.mincellenergy";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::normalCellEnergy)) {
            return "parameters.lifecycle.normalcellenergy";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::cellDeathProbability)) {
            return "parameters.lifecycle.celldeathprobability";
        }
        if (references<BoolSpec>(spec, &SimulationParameters::particleTransformationAllowed)) {
            return "parameters.lifecycle.particletransformationallowed";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::constructorConnectingCellDistance)) {
            return "parameters.construction.constructorconnectingcelldistance";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::newLineageThreshold)) {
            return "parameters.mutations.newlineagethreshold";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::neuronsMetaMutationsSigma)) {
            return "parameters.metamproperties.neuronsmetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::connectionsMetaMutationsSigma)) {
            return "parameters.metamproperties.connectionsmetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::cellTypePropertiesMetaMutationsSigma)) {
            return "parameters.metamproperties.celltypepropertiesmetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::constructorMetaMutationsSigma)) {
            return "parameters.metamproperties.constructormetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::geometryMetaMutationsSigma)) {
            return "parameters.metamproperties.geometrymetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::cellTypeMetaMutationsSigma)) {
            return "parameters.metamidentity.celltypemetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::cellTypeModeMetaMutationsSigma)) {
            return "parameters.metamidentity.celltypemodemetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::voidMetaMutationsSigma)) {
            return "parameters.metamidentity.voidmetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::customizationMetaMutationsSigma)) {
            return "parameters.metamidentity.customizationmetamutationssigma";
        }
        if (references<BoolSpec>(spec, &SimulationParameters::customizationTransitionMatrix)) {
            return "parameters.metamidentity.customizationtransitionmatrix";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::addNodeMetaMutationsSigma)) {
            return "parameters.metamgene.addnodemetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::deleteNodeMetaMutationsSigma)) {
            return "parameters.metamgene.deletenodemetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::extendGeneMetaMutationsSigma)) {
            return "parameters.metamgene.extendgenemetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::trimGeneMetaMutationsSigma)) {
            return "parameters.metamgene.trimgenemetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::copyNodeSectionMetaMutationsSigma)) {
            return "parameters.metamgene.copynodesectionmetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::moveNodeSectionMetaMutationsSigma)) {
            return "parameters.metamgene.movenodesectionmetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::addGeneMetaMutationsSigma)) {
            return "parameters.metamgenome.addgenemetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::duplicateGeneMetaMutationsSigma)) {
            return "parameters.metamgenome.duplicategenemetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::deleteGeneMetaMutationsSigma)) {
            return "parameters.metamgenome.deletegenemetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::swapGeneMetaMutationsSigma)) {
            return "parameters.metamgenome.swapgenemetamutationssigma";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::attackerEnergyCost)) {
            return "parameters.attacker.attackerenergycost";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::attackerFoodChainColorMatrix)) {
            return "parameters.attacker.attackerfoodchaincolormatrix";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::attackerStrength)) {
            return "parameters.attacker.attackerstrength";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::attackerRelatedLineageProtection)) {
            return "parameters.attacker.attackerrelatedlineageprotection";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::attackerSizeProtection)) {
            return "parameters.attacker.attackersizeprotection";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::attackerRadius)) {
            return "parameters.attacker.attackerradius";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::maxRawEnergyConductivity)) {
            return "parameters.digestor.maxrawenergyconductivity";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::maxRawEnergyConversion)) {
            return "parameters.digestor.maxrawenergyconversion";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::defenderAntiAttackerStrength)) {
            return "parameters.defender.defenderantiattackerstrength";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::defenderAntiInjectorStrength)) {
            return "parameters.defender.defenderantiinjectorstrength";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::injectorEnergyCost)) {
            return "parameters.injector.injectorenergycost";
        }
        if (references<FloatSpec>(spec, &SimulationParameters::injectorRadius)) {
            return "parameters.injector.injectorradius";
        }
        return {};
    }

    std::string translated(std::string const& key, std::string const& fallback)
    {
        if (key.empty()) {
            return fallback;
        }
        auto text = tr(key);
        return text == key ? fallback : text;
    }

    bool matches(std::string const& canonical, std::string const& display, std::string const& query)
    {
        return Utf8Search::find(canonical, query).has_value() || Utf8Search::find(display, query).has_value();
    }

    bool filterParameter(ParameterSpec& spec, std::string const& query)
    {
        auto visible = matches(spec._name, SimulationParametersPresentation::label(spec), query);
        if (auto alternative = std::get_if<AlternativeSpec>(&spec._reference)) {
            for (auto& [name, parameters] : alternative->_alternatives) {
                for (auto& parameter : parameters) {
                    visible = filterParameter(parameter, query) || visible;
                }
            }
        }
        spec._visible = spec._visible && visible;
        return spec._visible;
    }
}

std::string SimulationParametersPresentation::label(ParameterSpec const& spec)
{
    auto prefix = key(spec);
    return translated(prefix.empty() ? prefix : prefix + ".name", spec._name);
}

std::optional<std::string> SimulationParametersPresentation::description(ParameterSpec const& spec)
{
    auto prefix = key(spec);
    if (prefix.empty()) {
        return spec._description;
    }
    return translated(prefix + ".description", spec._description.value_or(""));
}

std::string SimulationParametersPresentation::label(ParameterGroupSpec const& spec)
{
    if (spec._name == "General") {
        return translated("parameters.general.name", spec._name);
    }
    if (spec._name == "Location") {
        return translated("parameters.location.name", spec._name);
    }
    if (spec._name == "Visualization") {
        return translated("parameters.visualization.name", spec._name);
    }
    if (spec._name == "Shape") {
        return translated("parameters.shape.name", spec._name);
    }
    if (spec._name == "Force field") {
        return translated("parameters.forcefield.name", spec._name);
    }
    if (spec._name == "Guided energy supply") {
        return translated("parameters.energy.name", spec._name);
    }
    if (spec._name == "Physics: Motion") {
        return translated("parameters.motion.name", spec._name);
    }
    if (spec._name == "Physics: Thresholds") {
        return translated("parameters.thresholds.name", spec._name);
    }
    if (spec._name == "Radiation") {
        return translated("parameters.radiation.name", spec._name);
    }
    if (spec._name == "Cell life cycle") {
        return translated("parameters.lifecycle.name", spec._name);
    }
    if (spec._name == "Cell construction") {
        return translated("parameters.construction.name", spec._name);
    }
    if (spec._name == "Mutations") {
        return translated("parameters.mutations.name", spec._name);
    }
    if (spec._name == "Meta-mutations: Properties") {
        return translated("parameters.metamproperties.name", spec._name);
    }
    if (spec._name == "Meta-mutations: Cell identity") {
        return translated("parameters.metamidentity.name", spec._name);
    }
    if (spec._name == "Meta-mutations: Gene structure") {
        return translated("parameters.metamgene.name", spec._name);
    }
    if (spec._name == "Meta-mutations: Genome structure") {
        return translated("parameters.metamgenome.name", spec._name);
    }
    if (spec._name == "Cell type: Attacker") {
        return translated("parameters.attacker.name", spec._name);
    }
    if (spec._name == "Cell type: Digestor") {
        return translated("parameters.digestor.name", spec._name);
    }
    if (spec._name == "Cell type: Defender") {
        return translated("parameters.defender.name", spec._name);
    }
    if (spec._name == "Cell type: Injector") {
        return translated("parameters.injector.name", spec._name);
    }
    return spec._name;
}

ParametersSpec SimulationParametersPresentation::filter(ParametersSpec const& spec, ParametersFilter const& filter)
{
    auto result = spec;
    if (filter.containedText && !filter.containedText->empty()) {
        for (auto& group : result._groups) {
            if (!matches(group._name, label(group), *filter.containedText)) {
                for (auto& parameter : group._parameters) {
                    filterParameter(parameter, *filter.containedText);
                }
            }
        }
    }
    return result;
}

std::vector<std::string> SimulationParametersPresentation::alternativeLabels(ParameterSpec const& spec)
{
    auto const& alternative = std::get<AlternativeSpec>(spec._reference);
    std::vector<std::string> result;
    for (size_t index = 0; index < alternative._alternatives.size(); ++index) {
        auto const& canonical = alternative._alternatives.at(index).first;
        auto localized = references<AlternativeSpec>(spec, &SimulationParameters::objectColoring)
            || references<AlternativeSpec>(spec, &SimulationParameters::layerShape) || references<AlternativeSpec>(spec, &SimulationParameters::sourceShapeType)
            || references<AlternativeSpec>(spec, &SimulationParameters::layerForceFieldType)
            || references<AlternativeSpec>(spec, &SimulationParameters::layerRadialForceFieldOrientation);
        result.emplace_back(localized ? translated(key(spec) + ".option" + std::to_string(index), canonical) : canonical);
    }
    return result;
}
