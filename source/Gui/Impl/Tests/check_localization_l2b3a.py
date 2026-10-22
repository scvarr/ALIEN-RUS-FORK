"""L2-B3a source contracts supplement the production headless GUI tests."""
import json
from pathlib import Path
import re
import unittest

from check_localization_l2b2 import catalog, read, ROOT, GUI

MEMBERS = [
    ("ColorSpec", "backgroundColor"), ("ColorSpec", "customizationColors"),
    ("AlternativeSpec", "objectColoring"), ("FloatSpec", "glow"),
    ("FloatSpec", "fluidIllumination"), ("BoolSpec", "borderlessRendering"),
    ("BoolSpec", "gridLines"), ("BoolSpec", "markReferenceDomain"),
    ("BoolSpec", "sourceShowRadiationCenter"), ("ColorSpec", "sourceLightColor"),
    ("FloatSpec", "sourceLightBrightness"), ("FloatSpec", "sourceLightRange"),
    ("BoolSpec", "layerShowForceField"), ("FloatSpec", "externalEnergy"),
    ("FloatSpec", "externalEnergyInflowForConstructor"), ("FloatSpec", "externalEnergyInflowThresholdFactor"),
    ("BoolSpec", "externalEnergyInflowOnlyForFirstOffspring"), ("FloatSpec", "externalEnergyInflowForSources"),
    ("FloatSpec", "externalEnergyBackflowFactor"), ("FloatSpec", "externalEnergyBackflowLimit"),
]


def strings(expression):
    return ''.join(json.loads(t) for t in re.findall(r'"(?:[^"\\]|\\.)*"', expression))


class L2b3aContracts(unittest.TestCase):
    def test_catalogs_and_all_twenty_typed_member_bindings(self):
        en, ru = catalog("en"), catalog("ru")
        self.assertEqual(set(en), set(ru))
        self.assertEqual(433, sum(not key.startswith(("genome.", "neural.", "node.")) for key in en))
        source = read(GUI / "SimulationParametersPresentation.cpp")
        for index, (spec, member) in enumerate(MEMBERS):
            prefix = "parameters." + ("visualization." if index < 13 else "energy.") + member.lower()
            self.assertIn(f'references<{spec}>(spec, &SimulationParameters::{member})', source)
            self.assertIn(f'return "{prefix}";', source)
            for suffix in ("name", "description"):
                self.assertTrue(en[prefix + "." + suffix])
                self.assertTrue(ru[prefix + "." + suffix])
                self.assertNotEqual(en[prefix + "." + suffix], ru[prefix + "." + suffix])
                self.assertNotIn("##", ru[prefix + "." + suffix])
        self.assertEqual("Визуализация", ru["parameters.visualization.name"])
        self.assertEqual("Управляемая подача энергии", ru["parameters.energy.name"])
        self.assertEqual([3, 6, 10, 1], [sum(t == name for t, _ in MEMBERS) for name in ("ColorSpec", "BoolSpec", "FloatSpec", "AlternativeSpec")])

    def test_all_english_descriptions_and_options_match_immutable_specs(self):
        source = read(ROOT / "source/Data/SimulationParameters.cpp")
        coloring = source.split('std::string const coloringTooltip =', 1)[1].split(';', 1)[0]
        coloring = strings(coloring.replace('ICON_FA_CHEVRON_RIGHT', '"\\uf054"'))
        en, ru = catalog("en"), catalog("ru")
        count = 0
        for group, slug in (("Visualization", "visualization"), ("Guided energy supply", "energy")):
            section = source.split(f'.name("{group}")', 1)[1].split('ParameterGroupSpec()', 1)[0]
            for block in section.split('ParameterSpec()')[1:]:
                name = re.search(r'\.name\("(.*?)"\)', block)[1]
                member = re.search(r'\.member\(&SimulationParameters::(\w+)\)', block)[1]
                expression = block.split('.description(', 1)[1].rsplit(')', 1)[0]
                description = coloring if 'coloringTooltip' in expression else strings(expression)
                prefix = f'parameters.{slug}.{member.lower()}'
                self.assertEqual(name, en[prefix + '.name'])
                self.assertEqual(description, en[prefix + '.description'])
                count += 1
        self.assertEqual(20, count)
        for index, (english, russian) in enumerate(zip(
                ["Energy", "Customization", "Lineage + Customization", "Lineage", "Creature"],
                ["Энергия", "Настройка", "Происхождение + Настройка", "Происхождение", "Организм"])):
            key = f'parameters.visualization.objectcoloring.option{index}'
            self.assertEqual(english, en[key])
            self.assertEqual(russian, ru[key])
        for language in (en, ru):
            tooltip = language['parameters.visualization.objectcoloring.description']
            self.assertEqual(5, tooltip.count('\uf054'))
            self.assertEqual(6, tooltip.count('\n\n'))

    def test_widget_labels_and_options_do_not_replace_canonical_identity(self):
        source = read(GUI / "SpecificationGuiService.cpp")
        for widget in ("CheckboxParameters", "ColorButtonParameters", "ColorVectorButtonsParameters", "SwitcherParameters"):
            self.assertRegex(source, widget + r'\(\)\s*\.name\(parameterSpec\._name\)\s*\.label\(SimulationParametersPresentation::label\(parameterSpec\)\)')
        self.assertIn('.values(values)', source)
        self.assertIn('.displayValues(SimulationParametersPresentation::alternativeLabels(parameterSpec))', source)
        adapter = read(GUI / "SimulationParametersPresentation.cpp").split('SimulationParametersPresentation::alternativeLabels', 1)[1]
        self.assertIn('references<AlternativeSpec>(spec, &SimulationParameters::objectColoring)', adapter)
        self.assertIn('std::to_string(index)', adapter)
        self.assertIn('alternative._alternatives.at(index).first', adapter)
        widgets = read(GUI / "AlienGui.cpp")
        for identity in ('"##picker"', '"colorpicker"', '"##previous"', '"##current"', '"##palette"', '"##color"', '"##switcher"'):
            self.assertIn(identity, widgets)
        self.assertIn('ImGui::PushOverrideID(defaultId)', widgets)
        self.assertIn('"Default##" + parameters._name', widgets)
        self.assertIn('parameters._displayValues.size() == parameters._values.size()', widgets)

    def test_technical_conditions_and_common_color_strings_are_present(self):
        ru = catalog("ru")
        required = {
            'visualization.fluidillumination': ['аппаратной трассировке лучей', 'При 0', 'При 1'],
            'visualization.borderlessrendering': ['периодическим повторением'],
            'visualization.sourcelightbrightness': ['тени', 'мягче', 'При 0'],
            'visualization.layershowforcefield': ['карте высот', 'темнее'],
            'energy.externalenergy': ['аварийному завершению', 'массовое образование клеток'],
            'energy.externalenergyinflowforconstructor': ['пропорционально'],
            'energy.externalenergyinflowthresholdfactor': ['60%', 'собственную энергию'],
            'energy.externalenergyinflowonlyforfirstoffspring': ['собственного организма', 'цвету настройки'],
            'energy.externalenergyinflowforsources': ['каждом шаге времени', 'относительную долю', 'оставшаяся энергия'],
            'energy.externalenergybackflowfactor': ['расходе энергии', 'частицы энергии'],
            'energy.externalenergybackflowlimit': ['меньше этого значения'],
        }
        for slug, phrases in required.items():
            for phrase in phrases:
                self.assertIn(phrase, ru[f'parameters.{slug}.description'])
        for key in ('choose', 'current', 'previous', 'palette', 'default', 'color'):
            self.assertIn(f'parameters.color.{key}', ru)


    def test_switcher_arrow_handlers_and_mixed_order_keep_baseline(self):
        import hashlib
        switcher = read(GUI / "AlienGui.cpp").split('bool AlienGui::Switcher(', 1)[1].split('bool AlienGui::ComboColor(', 1)[0]
        def digest(text):
            return hashlib.sha256(re.sub(r'\s+', '', text).encode()).hexdigest()
        arrows = switcher.split('if (ImGui::Button(ICON_FA_CARET_LEFT', 1)[1].split('ImGui::PopID();', 1)[0]
        collapsed = switcher.split('auto getCollapsedText =', 1)[1].split('auto setAllSwitcherValues =', 1)[0]
        self.assertEqual('a3224eaa4b1d198ed3c7f847525969335c13249c5a457e6f5b90e2014bacbe2f', digest(arrows))
        self.assertEqual('56fb968024144dbb5e4d2cf4f4fa183c74dc1453b16a93aa9f3ed0fc971cebc2', digest(collapsed))
        self.assertIn('auto textWidth = color == 0 ?', switcher)
        self.assertIn('std::max(1.0f, width - textAndButtonWidth)', switcher)
        self.assertIn('Tooltip(TooltipParameters().text(text).delay(false))', switcher)

    def test_simulation_file_boundaries_keep_native_paths_and_use_utf8_for_display(self):
        source = read(GUI / "FileTransferController.cpp").split('void FileTransferController::onOpenGenomeDialog', 1)[0]
        self.assertIn('ReadSimulationRequestData{.filename = filename,', source)
        self.assertIn('SaveSimulationRequestData{firstFilename,', source)
        self.assertIn('printOverlayMessage(PathHelper::toUtf8(data.filename))', source)
        self.assertNotIn('filename.string()', source)
        self.assertNotIn('firstFilename.string()', source)
        serializer = read(ROOT / "source/Persister/Interface/SerializerService.cpp")
        io = serializer.split('bool SerializerService::serializeSimulationToFiles', 1)[1].split('bool SerializerService::deleteSimulation', 1)[0]
        self.assertIn('std::ofstream fileStream(filename, std::ios::binary)', io)
        self.assertIn('std::ifstream fileStream(filename, std::ios::binary)', io)
        self.assertNotIn('filename.string()', io)
        for header in ('ReadSimulationRequestData.h', 'ReadSimulationResultData.h', 'SaveSimulationRequestData.h'):
            self.assertIn('std::filesystem::path filename;', read(ROOT / 'source/Persister/Interface' / header))
        helper = read(ROOT / 'source/Base/PathHelper.h')
        self.assertIn('path.u8string()', helper)
        self.assertIn('std::filesystem::path(text) == path', helper)
        self.assertNotIn('SetConsoleOutputCP', helper + source + io)
        self.assertNotIn('setlocale', helper + source + io)


if __name__ == "__main__":
    unittest.main(verbosity=2)
