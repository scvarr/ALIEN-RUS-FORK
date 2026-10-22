"""L2-B3c catalog and immutable simulation contracts."""
import json
import re
import subprocess
import unittest
from check_localization_l2b2 import ROOT, GUI, catalog, read
from check_localization_l2b3b import declarations

MEMBERS = [
    ('FloatSpec', 'timestepSize', 'parameters.motion.timestepsize'),
    ('FloatSpec', 'smoothingLength', 'parameters.motion.smoothinglength'),
    ('FloatSpec', 'pressureStrength', 'parameters.motion.pressurestrength'),
    ('FloatSpec', 'viscosityStrength', 'parameters.motion.viscositystrength'),
    ('FloatSpec', 'friction', 'parameters.motion.friction'),
    ('FloatSpec', 'innerFriction', 'parameters.motion.innerfriction'),
    ('FloatSpec', 'rigidity', 'parameters.motion.rigidity'),
    ('FloatSpec', 'maxVelocity', 'parameters.thresholds.maxvelocity'),
    ('FloatSpec', 'maxForce', 'parameters.thresholds.maxforce'),
    ('FloatSpec', 'minObjectDistance', 'parameters.thresholds.minobjectdistance'),
    ('FloatSpec', 'maxBindingDistance', 'parameters.thresholds.maxbindingdistance'),
    ('FloatSpec', 'objectFusionVelocity', 'parameters.thresholds.objectfusionvelocity'),
    ('FloatSpec', 'relativeStrengthBasePin', 'parameters.radiation.relativestrengthbasepin'),
    ('BoolSpec', 'disableRadiationSources', 'parameters.radiation.disableradiationsources'),
    ('FloatSpec', 'radiationAbsorption', 'parameters.radiation.radiationabsorption'),
    ('FloatSpec', 'radiationType1_strength', 'parameters.radiation.radiationtype1strength'),
    ('IntSpec', 'radiationType1_minimumAge', 'parameters.radiation.radiationtype1minimumage'),
    ('FloatSpec', 'radiationType2_strength', 'parameters.radiation.radiationtype2strength'),
    ('FloatSpec', 'radiationType2_energyThreshold', 'parameters.radiation.radiationtype2energythreshold'),
    ('FloatSpec', 'particleSplitEnergy', 'parameters.radiation.particlesplitenergy'),
    ('FloatSpec', 'sourceRadiationAngle', 'parameters.radiation.sourceradiationangle'),
]


class L2b3cContracts(unittest.TestCase):
    def test_all_twenty_one_typed_bindings_and_exact_english(self):
        en, ru = catalog("en"), catalog("ru")
        self.assertEqual(set(en), set(ru))
        self.assertEqual(433, sum(not key.startswith(("genome.", "neural.", "node.", "mutation.", "genomeissues.", "genegraph.", "preview.", "signals.")) for key in en))
        blocks = declarations()
        adapter = read(GUI / "SimulationParametersPresentation.cpp")
        self.assertEqual(21, len(MEMBERS))
        for typ, member, prefix in MEMBERS:
            self.assertIn(f"references<{typ}>(spec, &SimulationParameters::{member})", adapter)
            self.assertIn(f'return "{prefix}";', adapter)
            block = blocks[member]
            self.assertEqual(re.search(r'\.name\("([^"\n]+)"\)', block)[1], en[prefix + ".name"])
            description = ''.join(json.loads(t) for t in re.findall(r'"(?:[^"\\]|\\.)*"', block.rsplit(".description(", 1)[1]))
            self.assertEqual(description, en[prefix + ".description"])
            for suffix in ("name", "description"):
                self.assertTrue(ru[prefix + "." + suffix])
                self.assertNotEqual(en[prefix + "." + suffix], ru[prefix + "." + suffix])

    def test_original_visibility_getter_setter_ranges_and_data_are_unchanged(self):
        for file in ("source/Data/SimulationParameters.cpp", "source/Data/SimulationParameters.h",
                     "source/Data/SpecificationEvaluationService.cpp", "source/Data/RadiationStrengthService.cpp",
                     "source/Gui/Impl/AlienGui.cpp", "source/Gui/Impl/AlienGui.h"):
            original = subprocess.check_output(["git", "show", "1605ded80f85ea2b3496f1cf2e0d6e65bf7a8306:" + file], cwd=ROOT)
            from check_localization_l2b4b import legacy_widget_text
            actual = legacy_widget_text(file, read(ROOT / file)).encode("utf8")
            if file.endswith("AlienGui.cpp"):
                self.assertEqual(2, actual.count(b".truncate(parameters._label.has_value())"))
                actual = actual.replace(b".truncate(parameters._label.has_value())", b"")
            self.assertEqual(original.replace(b"\r\n", b"\n"), actual, file)
        blocks = declarations()
        self.assertIn(".visible(false)", blocks["innerFriction"])
        self.assertIn(".getterSetter(FloatGetterSetter{radiationStrengthGetter, radiationStrengthSetter})", blocks["relativeStrengthBasePin"])
        self.assertIn('.min(-180.0f).max(180.0f).format("%.1f")', blocks["sourceRadiationAngle"])
        self.assertIn(".logarithmic(true).infinity(true)", blocks["radiationType1_minimumAge"])

    def test_int_dispatch_keeps_ids_matrix_and_optional_label_compatibility(self):
        source = read(GUI / "SpecificationGuiService.cpp").split("void SpecificationGuiService::createWidgetsForIntSpec",1)[1].split("void SpecificationGuiService::createWidgetsForFloatSpec",1)[0]
        matrix, slider = source.split("        AlienGui::SliderInt(",1)
        self.assertIn(".tooltip(parameterSpec._description)", matrix)
        self.assertIn(".name(parameterSpec._name)", slider)
        self.assertIn(".label(SimulationParametersPresentation::label(parameterSpec))", slider)
        self.assertIn(".tooltip(SimulationParametersPresentation::description(parameterSpec))", slider)
        for flag in ("logarithmic", "infinity"):
            self.assertIn(f".{flag}(intSpec._{flag})", slider)

    def test_translation_preserves_thresholds_fractions_delay_and_direction(self):
        ru = catalog("ru")
        required = {
            "thresholds.maxforce": ["с определённой вероятностью"],
            "thresholds.objectfusionvelocity": ["Минимальная относительная скорость", "липким", "у обоих"],
            "radiation.relativestrengthbasepin": ["возле клетки", "равна 1"],
            "radiation.radiationtype1minimumage": ["в шагах симуляции"],
            "radiation.radiationtype1strength": ["Доля", "за один шаг", "не абсолютную мощность"],
            "radiation.radiationtype2strength": ["Доля", "за один шаг", "не абсолютную мощность"],
            "radiation.radiationtype2energythreshold": ["Доступная и сырая энергия", "отдельно"],
            "radiation.particlesplitenergy": ["может разделиться", "не сразу"],
            "radiation.sourceradiationangle": ["Если параметр включён", "Иначе", "от центра"],
        }
        for key, phrases in required.items():
            for phrase in phrases:
                self.assertIn(phrase, ru["parameters." + key + ".description"])

if __name__ == "__main__":
    unittest.main(verbosity=2)
