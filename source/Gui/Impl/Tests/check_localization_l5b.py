"""World-tool presentation coverage and unchanged simulation-operation contracts."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
GUI = ROOT / 'source/Gui/Impl'
BASE = 'f801d0bd5254954a73e2d0abfa4004fb11bec0f0'
COMPONENTS = ['EditorWidget', 'SelectionWidget', 'CreatorWidget', 'MultiplierWidget',
              'ResizeWorldDialog', 'MassEditDialog', 'ImageToPatternDialog', 'SimulationInteractionController', 'EditorController']


def read(path):
    return path.read_text(encoding='utf8')


def original(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


def function(text, name):
    start = re.search(r'^.*::' + name + r'\(', text, re.M).start()
    return text[start:start + re.search(r'^}\n', text[start:], re.M).end()]


class L5bContracts(unittest.TestCase):
    def test_catalog_preservation_parity_arguments_and_references(self):
        def unique(pairs):
            result = dict(pairs)
            self.assertEqual(len(pairs), len(result))
            return result
        en, ru = [json.loads(read(ROOT / f'resources/locales/{lang}.json'), object_pairs_hook=unique) for lang in ['en', 'ru']]
        self.assertEqual(set(en), set(ru))
        consumers = '\n'.join(read(GUI / (name + '.cpp')) for name in COMPONENTS) + read(GUI / 'WorldToolPresentation.h')
        for lang, values in [('en', en), ('ru', ru)]:
            old = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(old, {key: values[key] for key in old})
            added = values.keys() - old.keys()
            self.assertEqual(132, len(added))
            for key in added:
                self.assertTrue(key.startswith('worldtools.'))
                self.assertRegex(key, r'^[a-z][a-z0-9]*(\.[a-z][a-z0-9]*)+$')
                self.assertTrue(values[key])
                self.assertNotIn('##', values[key])
                self.assertEqual(re.findall(r'\{\w+}', en[key]), re.findall(r'\{\w+}', ru[key]), key)
                if '.count.' not in key:
                    self.assertIn('"' + key + '"', consumers, key)
        for key in re.findall(r'"(worldtools\.[a-z0-9.]+)"', consumers):
            if not key.startswith('worldtools.count.'):
                self.assertIn(key, en)

    def test_creation_selection_undo_mass_and_resize_algorithms_are_unchanged(self):
        for name, methods in {
            'EditorWidget': ['processShortcuts', 'selectTool'],
            'SelectionWidget': ['process', 'calcViewBounds', 'processFrame', 'processColorPopup'],
            'CreatorWidget': ['process'],
            'MultiplierWidget': ['init', 'shutdown', 'processGridPreview', 'validateAndCorrect', 'onUndo'],
            'ResizeWorldDialog': ['open', 'onResizing'],
            'MassEditDialog': ['initIntern', 'shutdownIntern', 'onExecute', 'isOkEnabled', 'validateAndCorrect'],
            'SimulationInteractionController': ['processEvents', 'setEditMode', 'drawCursor']
        }.items():
            before, after = original(f'source/Gui/Impl/{name}.cpp'), read(GUI / (name + '.cpp'))
            for method in methods:
                self.assertEqual(function(before, method), function(after, method), name + '::' + method)
        before = function(original('source/Gui/Impl/MultiplierWidget.cpp'), 'onBuild')
        after = function(read(GUI / 'MultiplierWidget.cpp'), 'onBuild')
        after = re.sub(r'localizedInformation\(\s*"Random multiplication",\s*"[^"]+",\s*"[^"]+"\)',
                       'information("Random multiplication", "Non-overlapping copies could not be created.")', after)
        self.assertEqual(before, after)
        before = original('source/Gui/Impl/EditorController.cpp')
        after = read(GUI / 'EditorController.cpp').replace('#include <Base/LocalizationService.h>\n', '')
        en = json.loads(read(ROOT / 'resources/locales/en.json'))
        after = re.sub(r'printOverlayMessage\(tr\("([^"]+)"\)\)', lambda m: 'printOverlayMessage(' + json.dumps(en[m[1]]) + ')', after)
        self.assertEqual(before, after)
        for path in ['source/Gui/Impl/CreatorController.cpp', 'source/Gui/Impl/SelectionController.cpp',
                     'source/Gui/Impl/EditorModel.cpp',
                     'source/Gui/Impl/ChangeColorDialog.cpp', 'source/Gui/Impl/EntityAttributeHelp.cpp',
                     'source/Data/CreatorService.cpp', 'source/Data/MultiplierService.cpp',
                     'source/Data/MassEditService.cpp', 'source/Persister/Interface/SerializerService.cpp',
                     'source/Gui/Impl/TemporalControlWindow.cpp', 'source/Gui/Impl/SpatialControlWindow.cpp',
                     'source/Gui/Impl/EvolutionDashboardWindow.cpp', 'docs/localization-terminology.md']:
            self.assertEqual(original(path), read(ROOT / path), path)

    def test_canonical_input_scopes_options_ranges_formats_and_file_contracts(self):
        for name in COMPONENTS:
            before, after = original(f'source/Gui/Impl/{name}.cpp'), read(GUI / (name + '.cpp'))
            for pattern in [r'ImGui::PushID\(([^;]+)\);', r'\.(?:min|max|step|format)\(([^)]+)\)',
                            r'\.values\(([^)]+)\)', r'ImGui::(?:Begin|BeginPopup|OpenPopup)\("([^"\n]+)"',
                            r'ImGui::(?:InputFloat|InputInt|Checkbox|InvisibleButton)\("([^"\n]+)"']:
                self.assertEqual(re.findall(pattern, before), re.findall(pattern, after), name + pattern)
            if name != 'EditorWidget':
                self.assertEqual(re.findall(r'\.name\("([^"]+)"\)', before), re.findall(r'\.name\("([^"]+)"\)', after), name)
        image = read(GUI / 'ImageToPatternDialog.cpp')
        self.assertIn(' + "{.png},.*"', image)
        for statement in ['_startingPath = firstFilenameCopy.remove_filename().string();',
                          'auto image = ImageFileService::get().loadRgbImage(firstFilename);',
                          'CreatorService::get().createPatternFromImage(*image, Viewport::get().getCenterInWorldPos(), customizationColors)']:
            self.assertIn(statement, image)
        self.assertIn('localizedInformation("Error",', image)
        creator = read(GUI / 'CreatorWidget.cpp')
        self.assertNotIn('EntityAttributeHelp::get(', creator)
        self.assertNotIn('Const::Creator', creator)
        en = json.loads(read(ROOT / 'resources/locales/en.json'))
        help_source = read(GUI / 'HelpStrings.h')
        for name, body in re.findall(r'std::string const (Creator\w+Tooltip) = (.*?);', help_source, re.S):
            canonical = ''.join(json.loads(t) for t in re.findall(r'"(?:\\.|[^"\\])*"', body.replace('ICON_FA_CHEVRON_RIGHT', '""')))
            self.assertEqual(canonical, en['worldtools.help.' + name.lower()])


if __name__ == '__main__':
    unittest.main()
