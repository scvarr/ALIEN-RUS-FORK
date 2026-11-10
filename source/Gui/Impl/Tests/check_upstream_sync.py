"""Cross-version localization contracts for the 2026-11 upstream integration.

Historical stage snapshots remain available in check_localization_*.py.
This suite checks additive catalogs and actual bindings without freezing source hashes.
"""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
BASE = "f395e3530"


def unique(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise AssertionError("Duplicate localization key: " + key)
        result[key] = value
    return result


def catalog(language):
    return json.loads((ROOT / f"resources/locales/{language}.json").read_text(encoding="utf8"), object_pairs_hook=unique)


def assertBaselineKeys(test, current):
    previous = json.loads(subprocess.check_output(
        ["git", "show", f"{BASE}:resources/locales/en.json"], cwd=ROOT).decode("utf8"))
    test.assertTrue(previous.keys() <= current.keys(), sorted(previous.keys() - current.keys()))


class UpstreamLocalizationContracts(unittest.TestCase):
    def test_utf8_keys_and_placeholders(self):
        en, ru = catalog("en"), catalog("ru")
        self.assertEqual(en.keys(), ru.keys())
        for key in en:
            self.assertNotIn("\ufffd", ru[key], key)
            self.assertEqual(sorted(re.findall(r"\{[a-zA-Z_]+\}", en[key])),
                             sorted(re.findall(r"\{[a-zA-Z_]+\}", ru[key])), key)

    def test_existing_catalog_entries_are_preserved(self):
        for language in ("en", "ru"):
            previous = json.loads(subprocess.check_output(
                ["git", "show", f"{BASE}:resources/locales/{language}.json"], cwd=ROOT).decode("utf8"))
            actual = catalog(language)
            self.assertTrue(previous.keys() <= actual.keys())
            for key in previous:
                if key != "localui.network.activehelp":
                    self.assertEqual(previous[key], actual[key], key)

    def test_every_new_entry_has_a_production_binding(self):
        en = catalog("en")
        previous = json.loads(subprocess.check_output(
            ["git", "show", f"{BASE}:resources/locales/en.json"], cwd=ROOT).decode("utf8"))
        production = "\n".join(p.read_text(encoding="utf8") for p in (ROOT / "source/Gui/Impl").glob("*.cpp"))
        production += "\n".join(p.read_text(encoding="utf8") for p in (ROOT / "source/Gui/Impl").glob("*.h"))
        for key in en.keys() - previous.keys():
            base = key.removesuffix(".description").removesuffix(".name")
            self.assertTrue('"' + base + '"' in production, key)

    def test_recorder_controls_keep_programmatic_names(self):
        source = (ROOT / "source/Gui/Impl/RecorderWindow.cpp").read_text(encoding="utf8")
        for name in ("Settings", "Smoothing", "Frames per second", "Bit rate (MBit/s)",
                     "Time steps per second at 1x", "Show time-lapse and zoom", "Run simulation during flight"):
            self.assertRegex(source, re.escape('.name("' + name + '")') + r'\s*\.label\(tr\("')
        self.assertIn('setTitleKey("localui.sync.recorder")', source)

    def test_new_parameter_bindings_use_typed_references(self):
        source = (ROOT / "source/Gui/Impl/SimulationParametersPresentation.cpp").read_text(encoding="utf8")
        self.assertIn('references<BoolSpec>(spec, &SimulationParameters::hardwareRayTracing)', source)
        self.assertIn('references<FloatSpec>(spec, &SimulationParameters::sourceShadowStrength)', source)
        for language in ("en", "ru"):
            values = catalog(language)
            self.assertIn("24", values["localui.network.activehelp"])
            self.assertNotIn("25", values["localui.network.activehelp"])

    def test_flight_serialization_keys_stay_upstream(self):
        path = "source/Gui/Impl/RecorderController.cpp"
        source = (ROOT / path).read_text(encoding="utf8")
        upstream = subprocess.check_output(["git", "show", f"MERGE_HEAD:{path}"], cwd=ROOT).decode("utf8")
        expressions = r'JsonParser::encodeDecode\([^;]+;'
        self.assertEqual(re.findall(expressions, upstream), re.findall(expressions, source))


if __name__ == "__main__":
    unittest.main(verbosity=2)
