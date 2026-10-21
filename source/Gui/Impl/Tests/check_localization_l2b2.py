"""L2-B2 source contracts; runtime behavior is covered by GuiLocalizationTests."""

import hashlib
import json
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[4]
GUI = ROOT / "source/Gui/Impl"


def read(path):
    return path.read_text(encoding="utf-8", errors="strict")


def catalog(language):
    def unique(pairs):
        result = dict(pairs)
        if len(result) != len(pairs):
            raise ValueError("Duplicate catalog key")
        return result
    return json.loads(read(ROOT / f"resources/locales/{language}.json"), object_pairs_hook=unique)


class L2b2Contracts(unittest.TestCase):
    def test_nine_declarations_have_separate_complete_catalog_entries(self):
        en, ru = catalog("en"), catalog("ru")
        self.assertEqual(set(en), set(ru))
        self.assertEqual(261, len(en))
        slugs = ["general.project", "general.layer", "general.source", "general.opacity", "general.share",
                 "location.layerposition", "location.layervelocity", "location.sourceposition", "location.sourcevelocity"]
        for slug in slugs:
            for suffix in ("name", "description"):
                key = f"parameters.{slug}.{suffix}"
                self.assertTrue(en[key])
                self.assertTrue(ru[key])
                self.assertNotEqual(en[key], ru[key])
                self.assertNotIn("##", ru[key])
        self.assertEqual("Общие", ru["parameters.general.name"])
        self.assertEqual("Расположение", ru["parameters.location.name"])
        self.assertEqual("Доля", ru["parameters.general.share.name"])
        self.assertEqual(4, len({ru[f"parameters.{slug}.description"] for slug in slugs[-4:]}))
        self.assertIn("базовым уровнем и источниками", ru["parameters.general.share.description"])
        self.assertIn("равна 1", ru["parameters.general.share.description"])

    def test_english_catalog_descriptions_match_original_specification(self):
        source = read(ROOT / "source/Data/SimulationParameters.cpp")
        sections = [source.split('ParameterGroupSpec().name("General")', 1)[1].split('.name("Visualization")')[0],
                    source.split('.name("Location")', 1)[1].split('ParameterGroupSpec().name("Shape")')[0]]
        declarations = []
        for section in sections:
            for match in re.finditer(r'\.name\("([^"\n]+)"\)(?:(?!ParameterSpec\(\)).)*?\.description\((.*?)\),', section, re.S):
                description = ''.join(json.loads(s) for s in re.findall(r'"(?:[^"\\]|\\.)*"', match[2]))
                declarations.append((match[1], description))
        en = catalog("en")
        keys = [k[:-5] for k in en if k.startswith(("parameters.general.", "parameters.location."))
                and k.endswith(".name") and k not in ("parameters.general.name", "parameters.location.name")]
        self.assertEqual(9, len(keys))
        self.assertEqual(declarations, [(en[k + ".name"], en[k + ".description"]) for k in keys])

    def test_engine_specs_filter_and_interaction_controller_are_unchanged(self):
        hashes = {
            'source/Data/SimulationParameters.cpp': 'abe7eedd490a403389f661e23438c97baee2cc5aad06e01f76b8d3449a44f301',
            'source/Data/SimulationParametersSpecification.h': 'a1631c9edf1325dab5612a71841d06252862b4d424b1fbd272ad628fa667553f',
            'source/Data/SimulationParameters.h': '2bf18e69f8adb46cbe7571fba6c0f62438d2a69eb16b42846f25db4ac8604d3a',
            'source/Data/SpecificationFilterService.cpp': '3dc5c2b129334f6ea6517d046309503e13c520fe1d8a61a82b385a7c70f52a42',
            'source/Base/StringHelper.cpp': '50c7d7c7a474bb1882fa117c9eafacf65b7d39d75567e58434cc4bfca590f7b8',
            'source/Gui/Impl/SimulationInteractionController.cpp': '7ab49432a79db6acc059230feca1ce143625da90b6a9b052779a1e4258d2d3a8',
        }
        for file, expected in hashes.items():
            self.assertEqual(expected, hashlib.sha256((ROOT / file).read_bytes().replace(b'\r\n', b'\n')).hexdigest(), file)

    def test_widgets_keep_canonical_identity_and_picker_callbacks(self):
        source = read(GUI / "SpecificationGuiService.cpp")
        self.assertNotIn("_specCache", source + read(GUI / "SpecificationGuiService.h"))
        self.assertIn("SimulationParametersPresentation::filter(parametersSpecs, filter)", source)
        for widget in ("SliderFloatParameters", "SliderFloat2Parameters", "InputTextParameters"):
            self.assertRegex(source, widget + r'\(\)\s*\.name\(parameterSpec\._name\)\s*\.label\(SimulationParametersPresentation::label\(parameterSpec\)\)')
        self.assertIn('sizeof(Char64) / sizeof(char)', source)
        self.assertIn('getPositionSelectionData()', source)
        self.assertIn('InteractionMode_PositionSelection : InteractionMode_Normal', source)
        highlighter = read(GUI / "AlienGui.cpp").split('void hightlightSubstring(', 1)[1].split('\nnamespace', 1)[0]
        self.assertIn('Utf8Search::find(label, substring)', highlighter)
        self.assertNotIn('StringHelper::', highlighter)
        self.assertIn('.id("Filter (case insensitive)").hint(tr("parameters.filter"))', read(GUI / "SimulationParametersMainWindow.cpp"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
