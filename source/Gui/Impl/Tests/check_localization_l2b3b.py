"""L2-B3b contracts supplement production headless widget tests."""
import hashlib
import json
from pathlib import Path
import re
import unittest

from check_localization_l2b2 import ROOT, GUI, catalog, read

MEMBERS = [
    ('AlternativeSpec', 'layerShape', 'parameters.shape.layershape'),
    ('FloatSpec', 'layerCoreRadius', 'parameters.shape.layercoreradius'),
    ('Float2Spec', 'layerCoreRect', 'parameters.shape.layercorerect'),
    ('FloatSpec', 'layerFadeoutRadius', 'parameters.shape.layerfadeoutradius'),
    ('AlternativeSpec', 'sourceShapeType', 'parameters.shape.sourceshapetype'),
    ('FloatSpec', 'sourceCircularRadius', 'parameters.shape.sourcecircularradius'),
    ('Float2Spec', 'sourceRectangularRect', 'parameters.shape.sourcerectangularrect'),
    ('AlternativeSpec', 'layerForceFieldType', 'parameters.forcefield.layerforcefieldtype'),
    ('AlternativeSpec', 'layerRadialForceFieldOrientation', 'parameters.forcefield.layerradialforcefieldorientation'),
    ('FloatSpec', 'layerRadialForceFieldStrength', 'parameters.forcefield.layerradialforcefieldstrength'),
    ('FloatSpec', 'layerRadialForceFieldDriftAngle', 'parameters.forcefield.layerradialforcefielddriftangle'),
    ('FloatSpec', 'layerCentralForceFieldStrength', 'parameters.forcefield.layercentralforcefieldstrength'),
    ('FloatSpec', 'layerLinearForceFieldAngle', 'parameters.forcefield.layerlinearforcefieldangle'),
    ('FloatSpec', 'layerLinearForceFieldStrength', 'parameters.forcefield.layerlinearforcefieldstrength'),
    ('FloatSpec', 'layerPerlinNoiseForceFieldStrength', 'parameters.forcefield.layerperlinnoiseforcefieldstrength'),
    ('FloatSpec', 'layerPerlinNoiseForceFieldSpatialSize', 'parameters.forcefield.layerperlinnoiseforcefieldspatialsize'),
    ('FloatSpec', 'layerPerlinNoiseForceFieldTemporalSize', 'parameters.forcefield.layerperlinnoiseforcefieldtemporalsize'),
]


def declarations():
    source = read(ROOT / "source/Data/SimulationParameters.cpp")
    result = {}
    for match in re.finditer(r"ParameterSpec\(\)", source):
        depth, quoted, escaped = 0, False, False
        for end in range(match.start(), len(source)):
            char = source[end]
            if quoted:
                if escaped:
                    escaped = False
                elif char == "\\":
                    escaped = True
                elif char == '"':
                    quoted = False
            elif char == '"':
                quoted = True
            elif char in "({[":
                depth += 1
            elif char in ")}]":
                if depth == 0:
                    break
                depth -= 1
            elif char == "," and depth == 0:
                break
        block = source[match.start():end]
        member = re.search(r"\.member\(&SimulationParameters::(\w+)\)", block)
        if member:
            result[member[1]] = block
    return result


def baseline_bytes(file):
    text = read(ROOT / file)
    if file.endswith("SpecificationGuiService.cpp"):
        start = text.index("        AlienGui::SliderInt(")
        end = text.index("\nvoid SpecificationGuiService::createWidgetsForFloatSpec", start)
        block = text[start:end]
        block = block.replace("\n                .label(SimulationParametersPresentation::label(parameterSpec))", "")
        block = block.replace(".tooltip(SimulationParametersPresentation::description(parameterSpec))", ".tooltip(parameterSpec._description)")
        text = text[:start] + block + text[end:]
    if file.endswith("AlienGui.cpp"):
        text = text.replace(".truncate(parameters._label.has_value())", "")
    return text.encode()


class L2b3bContracts(unittest.TestCase):
    def test_complete_catalogs_and_all_seventeen_typed_bindings(self):
        en, ru = catalog("en"), catalog("ru")
        self.assertEqual(set(en), set(ru))
        self.assertEqual(325, len(en))
        adapter = read(GUI / "SimulationParametersPresentation.cpp")
        for spec, member, prefix in MEMBERS:
            self.assertIn(f"references<{spec}>(spec, &SimulationParameters::{member})", adapter)
            self.assertIn(f'return "{prefix}";', adapter)
            for suffix in ("name", "description"):
                self.assertTrue(en[prefix + "." + suffix])
                self.assertTrue(ru[prefix + "." + suffix])
                self.assertNotEqual(en[prefix + "." + suffix], ru[prefix + "." + suffix])
                self.assertNotIn("##", ru[prefix + "." + suffix])
        self.assertEqual(4, sum(t == "AlternativeSpec" for t, _, _ in MEMBERS))
        self.assertEqual(2, sum(t == "Float2Spec" for t, _, _ in MEMBERS))

    def test_english_names_and_full_descriptions_match_original_nested_specs(self):
        en = catalog("en")
        blocks = declarations()
        for _, member, prefix in MEMBERS:
            block = blocks[member]
            name = re.search(r'\.name\("([^"\n]+)"\)', block)[1]
            expression = block.rsplit(".description(", 1)[1].replace("ICON_FA_CHEVRON_RIGHT", '"\\uf054"')
            description = ''.join(json.loads(t) for t in re.findall(r'"(?:[^"\\]|\\.)*"', expression))
            self.assertEqual(name, en[prefix + ".name"])
            self.assertEqual(description, en[prefix + ".description"])

    def test_four_independent_option_lists_retain_original_indices(self):
        en, ru = catalog("en"), catalog("ru")
        options = {
            "shape.layershape": (["Circular", "Rectangular"], ["Круглая", "Прямоугольная"]),
            "shape.sourceshapetype": (["Circular", "Rectangular"], ["Круглая", "Прямоугольная"]),
            "forcefield.layerforcefieldtype": (["None", "Radial", "Central", "Linear", "Perlin noise"], ["Нет", "Радиальное", "Центральное", "Линейное", "Шум Перлина"]),
            "forcefield.layerradialforcefieldorientation": (["Clockwise", "Counter clockwise"], ["По часовой стрелке", "Против часовой стрелки"]),
        }
        for prefix, (english, russian) in options.items():
            for index, pair in enumerate(zip(english, russian)):
                key = f"parameters.{prefix}.option{index}"
                self.assertEqual(pair, (en[key], ru[key]))
        for language in (en, ru):
            self.assertEqual(2, len({language[f"parameters.shape.{m}.description"] for m in ("layershape", "sourceshapetype")}))
            self.assertEqual(4, len({language[f"parameters.forcefield.layer{mode}forcefieldstrength.description"] for mode in ("radial", "central", "linear", "perlinnoise")}))

    def test_translation_preserves_physical_meaning(self):
        ru = catalog("ru")
        self.assertIn("до нуля", ru["parameters.shape.layerfadeoutradius.description"])
        self.assertIn("создаются частицы энергии", ru["parameters.shape.sourceshapetype.description"])
        drift = ru["parameters.forcefield.layerradialforcefielddriftangle.description"]
        for word in ("касательной", "При 0", "внутрь", "наружу"):
            self.assertIn(word, drift)
        self.assertIn("вращаются", ru["parameters.forcefield.layerforcefieldtype.description"])
        self.assertIn("шагов симуляции", ru["parameters.forcefield.layerperlinnoiseforcefieldtemporalsize.description"])
        self.assertNotIn("секунд", ru["parameters.forcefield.layerperlinnoiseforcefieldtemporalsize.description"])
        for mode in ("radial", "central", "linear", "perlinnoise"):
            self.assertEqual("Сила", ru[f"parameters.forcefield.layer{mode}forcefieldstrength.name"])

    def test_production_filter_widgets_and_serialization_remain_unchanged(self):
        hashes = {
            'source/Gui/Impl/SpecificationGuiService.cpp': '2307dd316ac41d197e3a730825b2cc8def259db9e64b802d4893839f636bd108',
            'source/Gui/Impl/AlienGui.cpp': '37d8218a1a591b8fdf4c2dbb4b83c2aaa0e7f7afb3db18d4d40ca34ca74db915',
            'source/Gui/Impl/AlienGui.h': 'e299f29fa19c5169fa91fd16fbb8189229dd57c43f2d757f48912b9147d187f6',
            'source/Gui/Impl/Utf8Search.h': '87af0a1a217cb669d2e3594afcbb1bcb2cd420fdebd6c263a34ba3739aa0c6b4',
            'source/Persister/Interface/SerializerService.cpp': '41e590b5b96d1123c13c011e2be603e14f002f9c48b7ca0a642be673305fdf49',
            'source/Data/SimulationParameters.cpp': 'abe7eedd490a403389f661e23438c97baee2cc5aad06e01f76b8d3449a44f301',
            'source/Data/SimulationParameters.h': '2bf18e69f8adb46cbe7571fba6c0f62438d2a69eb16b42846f25db4ac8604d3a',
        }
        for file, expected in hashes.items():
            self.assertEqual(expected, hashlib.sha256(baseline_bytes(file)).hexdigest(), file)
        source = read(GUI / "SpecificationGuiService.cpp")
        self.assertIn("alternativeSpec._alternatives.at(*value).second", source)
        self.assertIn(".displayValues(SimulationParametersPresentation::alternativeLabels(parameterSpec))", source)


if __name__ == "__main__":
    unittest.main(verbosity=2)
