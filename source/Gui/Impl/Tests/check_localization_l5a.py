"""L5-A presentation coverage and immutable data/identity contracts."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
GUI = ROOT / 'source/Gui/Impl'
BASE = 'fe7269a5b909e8f6c36ff9c1db6a603d778aacac'


def read(path):
    return path.read_text(encoding='utf8')


def original(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


class L5aContracts(unittest.TestCase):
    def test_catalog_parity_existing_entries_arguments_and_complete_attribute_coverage(self):
        def unique(pairs):
            result = dict(pairs)
            self.assertEqual(len(pairs), len(result))
            return result
        en, ru = [json.loads(read(ROOT / f'resources/locales/{lang}.json'), object_pairs_hook=unique) for lang in ['en', 'ru']]
        self.assertEqual(set(en), set(ru))
        for lang, values in [('en', en), ('ru', ru)]:
            old = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(old, {k: values[k] for k in old})
            self.assertEqual(112, sum(not key.startswith(('worldtools.', 'simulationui.', 'localui.')) for key in values) - len(old))
            for key in values.keys() - old.keys():
                if key.startswith(('worldtools.', 'simulationui.', 'localui.')):
                    continue
                self.assertTrue(key.startswith('inspection.'))
                self.assertTrue(values[key])
                self.assertNotIn('##', values[key])
                self.assertEqual(sorted(re.findall(r'\{\w+}', en[key])), sorted(re.findall(r'\{\w+}', ru[key])), key)
        source = read(GUI / 'InspectionWindow.cpp')
        adapters = read(GUI / 'InspectionPresentation.h') + read(GUI / 'GenomeEditorPresentation.cpp')
        self.assertLessEqual(set(re.findall(r'EntityAttribute::(\w+)', source)), set(re.findall(r'case EntityAttribute::(\w+)', adapters)))
        canonical = read(GUI / 'EntityAttributeHelp.cpp')
        entries = {}
        for name, body in re.findall(r'\{EntityAttribute::(\w+),\s*((?:.|\n)*?)\},', canonical):
            entries[name.lower()] = ''.join(json.loads(t) for t in re.findall(r'"(?:\\.|[^"\\])*"', body.replace('ICON_FA_CHEVRON_RIGHT', '""')))
        for key in en:
            if key.startswith('inspection.attribute.'):
                self.assertEqual(entries[key.split('.')[-1]], en[key], key)

    def test_widget_names_numeric_contracts_and_data_mutations_are_unchanged(self):
        before = original('source/Gui/Impl/InspectionWindow.cpp')
        after = read(GUI / 'InspectionWindow.cpp')
        for pattern in [r'\.name\(([^)]+)\)', r'\.buttonText\(([^)]+)\)', r'\.values\(([^)]+)\)',
                        r'\.(?:min|max|step|format|readOnly|infinity)\(([^)]+)\)',
                        r'ImGui::PushID\(([^;]+)\);', r'\b(?:cell|object|constructor|sensor|generator|attacker|muscle|memory|communicator)\._\w+\s*(?:=|\|=)[^;]+;',
                        r'\bmemory\._signalEntries.resize\([^;]+;', r'\bsignals.resize\([^;]+;',
                        r'digestor.setRawEnergyConversionRate\([^;]+;', r'_SimulationFacade::get\(\)->change\w+\([^;]+;']:
            self.assertEqual(re.findall(pattern, before), re.findall(pattern, after), pattern)
        for name in ['createObjectTypeDesc', 'createCellTypeDesc', 'createSensorModeDesc', 'createGeneratorModeDesc', 'createAttackerModeDesc',
                     'createMuscleModeDesc', 'createReconnectorModeDesc', 'createMemoryModeDesc', 'createCommunicatorModeDesc']:
            pattern = r'    \w+ ' + name + r'\(.*?\n    }'
            self.assertEqual(re.search(pattern, before, re.S).group(), re.search(pattern, after, re.S).group())
        self.assertNotIn('EntityAttributeHelp::get(', after)
        pattern = r'std::string _InspectionWindow::generateTitle\(\) const.*?\n}'
        self.assertEqual(re.search(pattern, before, re.S).group(), re.search(pattern, after, re.S).group())
        self.assertIn('ImGui::Begin(generateTitle().c_str(), &_on, ImGuiWindowFlags_HorizontalScrollbar)', after)

    def test_shared_extension_is_opt_in_and_l5b_data_dependencies_are_unchanged(self):
        from localization_node_legacy import legacy_local_dialog_text
        before = original('source/Gui/Impl/AlienGui.cpp')
        after = legacy_local_dialog_text('AlienGui.cpp', read(GUI / 'AlienGui.cpp'))
        start = after.index('void AlienGui::InputFloat2(')
        end = after.index('\n}', start)
        after = after[:start] + after[start:end].replace('.text(parameters._label.value_or(parameters._name))', '.text(parameters._name.c_str())') + after[end:]
        start = after.index('void AlienGui::SliderInputFloat(')
        end = after.index('\n}', start)
        after = after[:start] + after[start:end].replace('.text(parameters._label.value_or(parameters._name))', '.text(parameters._name.c_str())') + after[end:]
        self.assertEqual(before, after)
        header = legacy_local_dialog_text('AlienGui.h', read(GUI / 'AlienGui.h')).replace('        MEMBER(InputFloat2Parameters, std::optional<std::string>, label, std::nullopt);\n', '')
        header = header.replace('        MEMBER(SliderInputFloatParameters, std::optional<std::string>, label, std::nullopt);\n', '')
        self.assertEqual(original('source/Gui/Impl/AlienGui.h'), header)
        paths = ['source/Data/Descs.h', 'source/Data/CellTypeConstants.h', 'source/Gui/Impl/EntityAttributeHelp.cpp',
                 'source/Gui/Impl/GenomeEditorPresentation.cpp', 'source/Gui/Impl/EditorModel.cpp',
                 'source/Persister/Interface/SerializerService.cpp', 'docs/localization-terminology.md']
        for path in paths:
            self.assertEqual(original(path), read(ROOT / path), path)


if __name__ == '__main__':
    unittest.main()
