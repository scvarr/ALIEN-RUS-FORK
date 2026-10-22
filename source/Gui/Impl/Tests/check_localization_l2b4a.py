"""L2-B4a presentation, terminology and immutable model contracts."""
import json
import re
import subprocess
import unittest

from check_localization_l2b2 import ROOT, GUI, catalog, read
from check_localization_l2b3b import declarations

BASE = "eff787fc38247f338a7be3e07bde07968c43669f"
MEMBERS = [
    ("IntSpec", "maxCellAge", "lifecycle", "Максимальный возраст"),
    ("IntSpec", "freeCellMaxAge", "lifecycle", "Максимальный возраст свободных клеток"),
    ("FloatSpec", "minCellEnergy", "lifecycle", "Минимальная энергия"),
    ("FloatSpec", "normalCellEnergy", "lifecycle", "Нормальный уровень энергии"),
    ("FloatSpec", "cellDeathProbability", "lifecycle", "Вероятность распада умирающих клеток"),
    ("BoolSpec", "particleTransformationAllowed", "lifecycle", "Образование свободных клеток"),
    ("FloatSpec", "constructorConnectingCellDistance", "construction", "Расстояние проверки соединений"),
    ("FloatSpec", "newLineageThreshold", "mutations", "Новое происхождение: порог"),
]


class L2b4aContracts(unittest.TestCase):
    def test_eight_unique_typed_bindings_and_complete_descriptions(self):
        en, ru = catalog("en"), catalog("ru")
        self.assertEqual(set(en), set(ru))
        self.assertEqual(433, sum(not key.startswith(("genome.", "neural.", "node.", "mutation.")) for key in en))
        adapter = read(GUI / "SimulationParametersPresentation.cpp")
        blocks = declarations()
        for typ, member, group, russian in MEMBERS:
            binding = f"references<{typ}>(spec, &SimulationParameters::{member})"
            self.assertEqual(1, adapter.count(binding))
            prefix = f"parameters.{group}.{member.lower()}"
            self.assertIn(f'return "{prefix}";', adapter)
            block = blocks[member]
            self.assertEqual(re.search(r'\.name\("([^"\n]+)"\)', block)[1], en[prefix + ".name"])
            expression = block.rsplit(".description(", 1)[1].replace("ICON_FA_CHEVRON_RIGHT", '"\\uf054"')
            description = ''.join(json.loads(t) for t in re.findall(r'"(?:[^"\\]|\\.)*"', expression))
            self.assertEqual(description, en[prefix + ".description"])
            self.assertEqual(russian, ru[prefix + ".name"])
            self.assertNotEqual(description, ru[prefix + ".description"])
            self.assertTrue(ru[prefix + ".description"])
            self.assertEqual(description.count("\uf054"), ru[prefix + ".description"].count("\uf054"))
            self.assertEqual(description.count("\n\n"), ru[prefix + ".description"].count("\n\n"))

    def test_group_labels_and_existing_catalog_entries(self):
        for language in ("en", "ru"):
            current = catalog(language)
            old = json.loads(subprocess.check_output(["git", "show", f"{BASE}:resources/locales/{language}.json"], cwd=ROOT))
            self.assertEqual(old, {key: current[key] for key in old})
        ru = catalog("ru")
        for group, label in (("lifecycle", "Жизненный цикл клеток"), ("construction", "Строительство клеток"), ("mutations", "Мутации")):
            self.assertEqual(label, ru[f"parameters.{group}.name"])

    def test_model_widgets_constraints_serialization_and_glossary_are_unchanged(self):
        for file in ("source/Data/SimulationParameters.cpp", "source/Data/SimulationParameters.h",
                     "source/Data/SimulationParametersSpecification.h", "source/Data/ParametersValidationService.cpp",
                     "source/Data/SpecificationEvaluationService.cpp", "source/Gui/Impl/SpecificationGuiService.cpp",
                     "source/Gui/Impl/AlienGui.cpp", "source/Gui/Impl/AlienGui.h", "source/Gui/Impl/Utf8Search.h",
                     "source/Persister/Interface/SerializerService.cpp", "docs/localization-terminology.md"):
            original = subprocess.check_output(["git", "show", f"{BASE}:{file}"], cwd=ROOT).replace(b"\r\n", b"\n")
            from check_localization_l2b4b import legacy_widget_text
            self.assertEqual(original, legacy_widget_text(file, read(ROOT / file)).encode("utf8"), file)
        blocks = declarations()
        self.assertIn(".greaterThan(&SimulationParameters::minCellEnergy)", blocks["normalCellEnergy"])
        for member in ("maxCellAge", "freeCellMaxAge"):
            self.assertIn(".min(1).max(1e7).logarithmic(true).infinity(true)", blocks[member])
        self.assertIn('.min(1e-6f).max(1e-1f).format("%.6f").logarithmic(true)', blocks["cellDeathProbability"])
        self.assertIn(".min(0.1f).max(4.0f)", blocks["constructorConnectingCellDistance"])
        self.assertIn('.format("%.5f")', blocks["newLineageThreshold"])

    def test_russian_meaning_preserves_life_cycle_and_all_four_energy_uses(self):
        ru = catalog("ru")
        required = {
            "lifecycle.maxcellage": ["в шагах симуляции", "«Умирание»", "после чего распадается"],
            "lifecycle.freecellmaxage": ["из частиц энергии", "в шагах симуляции", "на частицы энергии"],
            "lifecycle.mincellenergy": ["доступная энергия", "«Умирание»", "Свободные клетки"],
            "lifecycle.normalcellenergy": ["Передача энергии", "ещё ведёт строительство", "Накопитель", "выше нормального уровня", "остаточная энергия конструктора", "не опускается ниже", "частица энергии", "достигает нормального уровня"],
            "lifecycle.celldeathprobability": ["Вероятность за один шаг симуляции", "в состоянии «Умирание»", "недостаточно энергии", "превысила максимальный возраст", "головной клеткой", "Свободные клетки", "с той же вероятностью"],
            "lifecycle.particletransformationallowed": ["Если параметр включён", "частица энергии", "свободную клетку", "нормального уровня"],
            "construction.constructorconnectingcelldistance": ["конструктор", "новую ветвь", "первой новой клеткой", "существующие соединения объектов", "строительство откладывается"],
            "mutations.newlineagethreshold": ["накопленные мутации", "превышают", "идентификатор происхождения", "счётчик накопленных мутаций сбрасывается"],
        }
        for key, phrases in required.items():
            for phrase in phrases:
                self.assertIn(phrase, ru[f"parameters.{key}.description"])
        lineage = ru["parameters.mutations.newlineagethreshold.description"]
        for forbidden in ("новый вид", "вероятность мутации", "изменяет геном", "дополнительные мутации"):
            self.assertNotIn(forbidden, lineage)


if __name__ == "__main__":
    unittest.main(verbosity=2)
