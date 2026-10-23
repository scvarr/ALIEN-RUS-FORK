"""Mutation editor presentation, data and identity contracts; interactions are tested in C++."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
GUI = ROOT / 'source/Gui/Impl'
BASE = 'ba2e7748a8b294875d5ac5fe384f46a37a82eea2'


def read(path):
    return path.read_text(encoding='utf8')


def original(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


def catalog(language):
    def unique(pairs):
        result = dict(pairs)
        if len(result) != len(pairs):
            raise ValueError('Duplicate catalog key')
        return result
    return json.loads(read(ROOT / f'resources/locales/{language}.json'), object_pairs_hook=unique)


def function(text, name):
    start = re.search(r'^.*::' + name + r'\(', text, re.M).start()
    return text[start:start + re.search(r'^}\n', text[start:], re.M).end()]


class L4a4Contracts(unittest.TestCase):
    def test_catalog_parity_preservation_and_message_signatures(self):
        en, ru = catalog('en'), catalog('ru')
        self.assertEqual(set(en), set(ru))
        consumers = '\n'.join(read(GUI / p) for p in ['MutationRatesPresentation.h', 'MutationRatesWidget.cpp',
            'MutationRatesDialog.cpp', 'GenomeEditorPresentation.cpp'])
        for language, values in [('en', en), ('ru', ru)]:
            old = json.loads(original(f'resources/locales/{language}.json'))
            self.assertEqual(old, {k: values[k] for k in old})
            self.assertEqual(83, sum(not k.startswith(("genomeissues.", "genegraph.", "preview.", "signals.", "inspection.", "worldtools.", "simulationui.", "localui.", "browser.", "account.", "mcp.")) for k in values) - len(old))
            for key in values.keys() - old.keys():
                if key.startswith(("genomeissues.", "genegraph.", "preview.", "signals.", "inspection.", "worldtools.", "simulationui.", "localui.", "browser.", "account.", "mcp.")):
                    continue
                self.assertTrue(key.startswith('mutation.'))
                self.assertRegex(key, r'^[a-z][a-z0-9]*(\.[a-z][a-z0-9]*)+$')
                self.assertTrue(values[key])
                self.assertNotIn('##', values[key])
                self.assertIn('"' + key + '"', consumers)
                self.assertEqual(re.findall(r'\{\w+}', en[key]), re.findall(r'\{\w+}', ru[key]))
        self.assertIn('{count}', ru['mutation.editsummary'])
        self.assertIn('Донор', ru['mutation.void'])

    def test_all_consumed_help_is_translated_with_exact_english_fallback(self):
        source = read(GUI / 'GenomeEditorPresentation.cpp')
        canonical = read(GUI / 'EntityAttributeHelp.cpp')
        attributes = re.findall(r'case EntityAttribute::(\w+):\s*return tr\("(mutation.attribute.[^"]+)"\)', source)
        self.assertEqual(32, len(attributes))
        consumed = '\n'.join(read(GUI / p) for p in ['MutationRatesWidget.cpp', 'MutationRatesDialog.cpp'])
        self.assertEqual(set(re.findall(r'EntityAttribute::(\w+)', consumed)), {a for a, _ in attributes})
        en, ru = catalog('en'), catalog('ru')
        for attribute, key in attributes:
            block = re.search(r'\{EntityAttribute::' + attribute + r',\s*(.*?)\},', canonical, re.S)[1]
            self.assertEqual(''.join(json.loads(t) for t in re.findall(r'"(?:[^"\\]|\\.)*"', block)), en[key])
            self.assertNotEqual(en[key], ru[key])
        self.assertIn('return EntityAttributeHelp::get(attribute);', source)

    def test_parameter_order_ids_ranges_formats_and_settings_are_unchanged(self):
        path = 'source/Gui/Impl/MutationRatesDialog.cpp'
        old, new = original(path), read(ROOT / path)
        for pattern in [r'\.name\(([^)]+)\)', r'\.id\(([^)]+)\)', r'\.min\(([^)]+)\)',
                        r'\.max\(([^)]+)\)', r'\.format\(([^)]+)\)', r'\.logarithmic\(([^)]+)\)',
                        r'&mutation\.(\w+)', r'"(?:CMR|NMR|CTPM|GEOM|CTMM|CTM|CUM|VM|EXGM|ADNM|TRGM|DLNM|ADGM|DPGM|DLGM|SWGM|CNSM|MNSM|COM)\d*"']:
            self.assertEqual(re.findall(pattern, old), re.findall(pattern, new), pattern)
        for name in ['loadSettings', 'saveSettings', 'open', 'onAdopt']:
            self.assertEqual(function(old, name), function(new, name))
        for name in ['MutationRatesWidget.cpp', 'MutationRatesDialog.cpp']:
            self.assertNotIn('EntityAttributeHelp::get(', read(GUI / name))

    def test_measured_layout_and_stable_buttons(self):
        dialog = read(GUI / 'MutationRatesDialog.cpp')
        widget = read(GUI / 'MutationRatesWidget.cpp')
        presentation = read(GUI / 'MutationRatesPresentation.h')
        for name in ['Adopt', 'Cancel', 'Copy', 'Paste', 'Reset']:
            self.assertIn(f'DisplayButton("{name}"', dialog)
        self.assertIn('setTitleKey("mutation.title")', dialog)
        self.assertIn('ImGuiWindowFlags_HorizontalScrollbar', dialog)
        self.assertIn('parameterLabelWidth()', dialog)
        self.assertIn('ImGui::CalcTextSize(label(name).c_str())', presentation)
        for name in ['copy', 'paste']:
            self.assertIn(f'ImGui::CalcTextSize(tr("mutation.{name}").c_str())', dialog)
        self.assertIn('.buttonText("Edit (" + std::to_string(activeMutationTypes.size()) + " active)")', widget)
        self.assertIn('.buttonLabel(GenomeEditorPresentation::format(', widget)
        self.assertIn('_mutation = MutationRatesDesc();', dialog)

    def test_later_stages_data_dependencies_and_glossary_are_preserved(self):
        for path in ['source/Gui/Impl/NodeEditorWidget.cpp',
                     'source/Gui/Impl/EntityAttributeHelp.cpp', 'source/Data/GenomeDesc.h',
                     'source/Persister/Interface/SerializerService.cpp', 'docs/localization-terminology.md',
                     'source/Engine/Impl/Kernels/MutationProcessor.cuh']:
            self.assertEqual(original(path), read(ROOT / path), path)
        changed = subprocess.check_output(['git', 'diff', BASE, '--name-only'], cwd=ROOT, text=True).splitlines()
        for path in changed:
            self.assertFalse(path.startswith(('source/Engine/', 'source/Data/', 'source/Apps/',
                                             'source/Persister/', 'external/')), path)


if __name__ == '__main__':
    unittest.main()
