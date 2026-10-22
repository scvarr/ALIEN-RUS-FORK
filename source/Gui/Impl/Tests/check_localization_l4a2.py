"""L4-A2 catalog, identity, algorithm and stage-boundary contracts."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
GUI = ROOT / 'source/Gui/Impl'
BASE = '2ec767deff324d1eee7b5c4552c76a12003e1a01'


def read(path):
    return path.read_text(encoding='utf8')


def original(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


def catalog(lang):
    def unique(pairs):
        result = dict(pairs)
        if len(result) != len(pairs):
            raise ValueError('Duplicate key')
        return result
    return json.loads(read(ROOT / f'resources/locales/{lang}.json'), object_pairs_hook=unique)


def function(text, name):
    start = re.search(r'^.*::' + name + r'\(', text, re.M).start()
    return text[start:start + re.search(r'^}\n', text[start:], re.M).end()]


class L4a2Contracts(unittest.TestCase):
    def test_catalog_extension_parity_and_placeholder_signatures(self):
        en, ru = catalog('en'), catalog('ru')
        self.assertEqual(set(en), set(ru))
        for lang, values in [('en', en), ('ru', ru)]:
            old = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(old, {k: values[k] for k in old})
            self.assertEqual(66, sum(not k.startswith(("node.", "mutation.")) for k in values) - len(old))
            for k in values.keys() - old.keys():
                if k.startswith(("node.", "mutation.")):
                    continue
                self.assertTrue(k.startswith('neural.'))
                self.assertTrue(values[k])
                self.assertNotIn('##', values[k])
                self.assertEqual(re.findall(r'\{\w+}', en[k]), re.findall(r'\{\w+}', ru[k]))
                self.assertEqual(en[k].count('%'), ru[k].count('%'))

    def test_every_key_has_a_display_consumer(self):
        source = '\n'.join(read(GUI / p) for p in ['NeuralNetEditorWidget.cpp', 'NeuralNetEditorPresentation.h',
            'CellFunctionChannels.cpp', 'CreaturePreviewWidget.cpp', 'PreviewWidget.cpp', 'PreviewSettingsDialog.cpp'])
        for k in catalog('en'):
            if k.startswith('neural.'):
                self.assertIn('"' + k + '"', source)

    def test_canonical_scopes_and_actions_are_preserved(self):
        for name in ['NeuralNetEditorWidget.cpp', 'CreaturePreviewWidget.cpp', 'PreviewWidget.cpp', 'PreviewSettingsDialog.cpp']:
            before, after = original('source/Gui/Impl/' + name), read(GUI / name)
            for pattern in [r'ImGui::PushID\(([^;]+)\);', r'ImGui::BeginChild\("([^"]+)"']:
                self.assertEqual(re.findall(pattern, before), re.findall(pattern, after), name)
        net = read(GUI / 'NeuralNetEditorWidget.cpp')
        for action in ['Adopt', 'Cancel', 'Clear', 'Identity', 'Randomize', 'Copy', 'Paste']:
            self.assertIn(f'DisplayButton("{action}", tr("neural.{action.lower()}"))', net)
        preview = read(GUI / 'CreaturePreviewWidget.cpp')
        self.assertIn('.name(entry.name).label(entry.label)', preview)
        group = function(read(GUI / 'AlienGui.cpp'), 'Group')
        self.assertIn('ImGui::PushID(parameters._text.c_str())', group)
        self.assertIn('_label.value_or(parameters._text)', group)

    def test_neural_operations_and_modal_copy_algorithm_are_unchanged(self):
        before = original('source/Gui/Impl/NeuralNetEditorWidget.cpp')
        after = read(GUI / 'NeuralNetEditorWidget.cpp')
        for name in ['processDialog', 'drawWeightCurves', 'calcWeightColor']:
            self.assertEqual(function(before, name), function(after, name))
        for pattern in [r'weights\[.*;', r'biases\[.*;', r'activationFunctions\[.*;', r'weights = .*;', r'biases = .*;', r'activationFunctions = .*;']:
            self.assertEqual(re.findall(pattern, before), re.findall(pattern, after))

    def test_later_node_modes_and_shared_data_contracts_are_unchanged(self):
        # Node properties are covered by the L4-A3 state and source contracts.
        for path in ['source/Data/GenomeDesc.h', 'source/Data/CellTypeConstants.h',
                     'source/Persister/Interface/SerializerService.cpp', 'source/Gui/Impl/GenomeIssueDescription.cpp',
                     'source/Gui/Impl/SignalsBufferDialog.cpp', 'source/Gui/Impl/GeneGraphWidget.cpp']:
            self.assertEqual(original(path), read(ROOT / path), path)

    def test_rotated_and_measured_labels_use_the_same_display_text(self):
        source = read(GUI / 'NeuralNetEditorWidget.cpp')
        self.assertIn('font->RenderText(', source)
        self.assertIn('ImVec4 cpuClip{-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX}', source)
        self.assertIn('calcNodeLabelSize(_labelFont, name)', source)
        self.assertIn('tr("neural.connections").c_str()', source)
        self.assertNotIn('StringHelper::toUpper(functionModule.name)', source)
        self.assertNotIn('substr(', source)
        self.assertIn('static_assert(keys.size() == ActivationFunction_Count)', read(GUI / 'NeuralNetEditorPresentation.h'))


if __name__ == '__main__':
    unittest.main()
