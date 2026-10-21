"""L2-B1 presentation and unchanged-action contracts; complements real ImGui tests."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
GUI = ROOT / 'source/Gui/Impl'
BASE = '93c3c6b3ebdf55a591a418b08b2d4760bb140aae'


def read(file):
    return file.read_text(encoding='utf-8-sig', errors='strict')


def baseline(file):
    return subprocess.check_output(['git', 'show', f'{BASE}:source/Gui/Impl/{file}'], cwd=ROOT).decode('utf-8-sig')


def toolbar(text):
    return text.split('void SimulationParametersMainWindow::processToolbar()', 1)[1].split('void SimulationParametersMainWindow::processMasterWidget()', 1)[0]


def behavior(text):
    text = re.sub(r'"(?:[^"\\]|\\.)*"\s*"', '"', text)
    text = re.sub(r'\.overflowTooltip\(tr\("[a-z.]+"\)\)', '', text)
    text = re.sub(r'\.(name|tooltip)\((tr\("[a-z.]+"\)|"[^"\n]*")\)', r'.\1(CAPTION)', text)
    text = re.sub(r'printOverlayMessage\((tr\("[a-z.]+"\)|"[^"\n]*")\)', 'printOverlayMessage(CAPTION)', text)
    text = re.sub(r'GenericMessageDialog::get\(\)\.(information|localizedInformation)\([^;]+;', 'MESSAGE;', text)
    return re.sub(r'\s+', '', text)


class L2b1Contracts(unittest.TestCase):
    def test_toolbar_preserves_all_actions_icons_order_and_conditions(self):
        self.assertEqual(behavior(toolbar(baseline('SimulationParametersMainWindow.cpp'))), behavior(toolbar(read(GUI / 'SimulationParametersMainWindow.cpp'))))

    def test_all_twelve_commands_and_all_tooltips_are_localized(self):
        body = toolbar(read(GUI / 'SimulationParametersMainWindow.cpp'))
        names = re.findall(r'\.name\(([^\n]+?)\)', body)
        self.assertEqual(12, len(names))
        self.assertTrue(all(name.startswith('tr("parameters.') for name in names))
        self.assertNotRegex(body, r'\.tooltip\("')
        wrapper = read(GUI / 'AlienGui.cpp')
        self.assertIn('ImGui::PushID(toInt(index))', wrapper)
        self.assertIn('ImGui::Selectable("##row"', wrapper)
        self.assertIn('pendingAction = parameters._action;', wrapper)

    def test_file_patterns_settings_identity_and_camera_action_remain(self):
        current = read(GUI / 'SimulationParametersMainWindow.cpp')
        self.assertEqual(2, current.count('" (*.settings.json){.settings.json,.json},.*"'))
        self.assertIn('AlienWindow("Simulation parameters", "windows.simulation parameters"', current)
        self.assertIn('setTitleKey("menu.parameters")', current)
        self.assertIn('onCenterLocation(row);\n                        _selectedOrderNumber = row;', current)
        self.assertIn('ImGui::BeginTable("Locations", 5, flags', current)
        self.assertIn('.name("###parameters").label(title)', current)
        location = read(GUI / 'LocationWindow.cpp')
        self.assertEqual(baseline('LocationWindow.cpp'), location)

    def test_catalog_keys_utf8_and_named_slots(self):
        def unique(pairs):
            result = {}
            for key, value in pairs:
                self.assertNotIn(key, result)
                result[key] = value
            return result
        en, ru = [json.loads(read(ROOT / f'resources/locales/{lang}.json'), object_pairs_hook=unique) for lang in ('en', 'ru')]
        self.assertEqual(set(en), set(ru))
        self.assertEqual(261, len(en))
        for key in en:
            self.assertEqual(re.findall(r'\{[^}]*\}', en[key]), re.findall(r'\{[^}]*\}', ru[key]))
            self.assertNotIn('##', en[key])
            self.assertNotIn('##', ru[key])
        for file in ('SimulationParametersMainWindow.cpp', 'SimulationParametersGuiText.h', 'SimulationParametersBaseWidget.cpp', 'SimulationParametersLayerWidget.cpp', 'SimulationParametersSourceWidget.cpp'):
            for key in re.findall(r'"((?:parameters|common)\.[a-z.]+)"', read(GUI / file)):
                self.assertIn(key, en)


if __name__ == '__main__':
    unittest.main(verbosity=2)
