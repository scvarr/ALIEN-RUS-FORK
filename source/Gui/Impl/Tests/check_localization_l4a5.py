"""A5 presentation contracts; production ImGui tests exercise editing and rendering."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
GUI = ROOT / 'source/Gui/Impl'
BASE = 'aa69598bd2962b2daa455978c5a6fc4f73983b43'
PREFIXES = ('genomeissues.', 'genegraph.', 'preview.', 'signals.')


def read(path):
    return path.read_text(encoding='utf8')


def original(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


def function(text, name):
    start = re.search(r'^.*::' + name + r'\(', text, re.M).start()
    return text[start:start + re.search(r'^}\n', text[start:], re.M).end()]


class L4a5Contracts(unittest.TestCase):
    def test_catalog_parity_preservation_and_argument_signatures(self):
        def unique(pairs):
            result = dict(pairs)
            self.assertEqual(len(pairs), len(result))
            return result
        en, ru = [json.loads(read(ROOT / f'resources/locales/{lang}.json'), object_pairs_hook=unique) for lang in ['en', 'ru']]
        self.assertEqual(set(en), set(ru))
        for lang, values in [('en', en), ('ru', ru)]:
            old = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(old, {key: values[key] for key in old})
            self.assertEqual(78, sum(not key.startswith(("inspection.", "worldtools.")) for key in values) - len(old))
            for key in values.keys() - old.keys():
                if key.startswith(("inspection.", "worldtools.")):
                    continue
                self.assertTrue(key.startswith(PREFIXES))
                self.assertTrue(values[key])
                self.assertNotIn('##', values[key])
                self.assertEqual(sorted(re.findall(r'\{\w+}', en[key])), sorted(re.findall(r'\{\w+}', ru[key])), key)

    def test_canonical_scopes_channel_order_limits_formats_and_callbacks(self):
        for name in ['SignalsBufferDialog.cpp', 'GeneGraphWidget.cpp', 'CreaturePreviewWidget.cpp', 'PreviewWidget.cpp']:
            before, after = original('source/Gui/Impl/' + name), read(GUI / name)
            for pattern in [r'\.name\(([^)]+)\)', r'ImGui::PushID\(([^;]+)\);', r'ImGui::BeginChild\("([^"]+)"',
                            r'\.min\(([^)]+)\)', r'\.max\(([^)]+)\)', r'\.format\(([^)]+)\)']:
                self.assertEqual(re.findall(pattern, before), re.findall(pattern, after), name + pattern)
        dialog = read(GUI / 'SignalsBufferDialog.cpp')
        self.assertIn('setTitleKey("signals.title")', dialog)
        for action in ['Adopt', 'Cancel']:
            self.assertIn(f'DisplayButton("{action}",', dialog)
        old = original('source/Gui/Impl/SignalsBufferDialog.cpp')
        self.assertEqual(function(old, 'onAdopt'), function(dialog, 'onAdopt'))
        self.assertEqual(function(old, 'open'), function(dialog, 'open'))
        self.assertIn('std::clamp(numEntries, 0, MAX_CELL_MEMORY_ENTRIES)', dialog)
        shared = function(read(GUI / 'AlienGui.cpp'), 'SignalMemoryEditor')
        self.assertIn('std::clamp(numEntries, 1, MAX_CELL_MEMORY_ENTRIES)', shared)
        old = function(original('source/Gui/Impl/AlienGui.cpp'), 'SignalMemoryEditor')
        for pattern in [r'\.name\(([^)]+)\)', r'\.min\(([^)]+)\)', r'\.max\(([^)]+)\)', r'\.format\(([^)]+)\)',
                        r'entries\.resize\([^;]+;', r'_signalMemorySelection[^;]+;', r'&entries\.at[^;]+;']:
            self.assertEqual(re.findall(pattern, old), re.findall(pattern, shared), pattern)

    def test_algorithms_data_preview_settings_dependencies_and_glossary_unchanged(self):
        paths = ['source/Data/GenomeDesc.h', 'source/Data/GenomeIssue.h', 'source/Data/DescValidationService.cpp',
                 'source/Gui/Impl/PreviewSettingsDialog.cpp', 'source/Gui/Impl/PreviewSettingsDialog.h',
                 'source/Gui/Impl/EntityAttributeHelp.cpp', 'source/Persister/Interface/SerializerService.cpp',
                 'docs/localization-terminology.md']
        for path in paths:
            self.assertEqual(original(path), read(ROOT / path), path)
        for name in ['updatePreview', 'createSubGenomesForPreview', 'setupPreviewData', 'calcPreview', 'calcTpsForPreview',
                     'onRun', 'onStepBackward', 'onStepForward', 'onRestart']:
            old, new = original('source/Gui/Impl/PreviewWidget.cpp'), read(GUI / 'PreviewWidget.cpp')
            self.assertEqual(function(old, name), function(new, name), name)
        changed = subprocess.check_output(['git', 'diff', BASE, '--name-only'], cwd=ROOT, text=True).splitlines()
        for path in changed:
            self.assertFalse(path.startswith(('source/Engine/', 'source/Data/', 'source/Persister/', 'source/Apps/', 'external/')), path)

    def test_long_labels_wrap_and_russian_donor_terminology(self):
        for name in ['AlienGui.cpp', 'SignalsBufferDialog.cpp']:
            source = read(GUI / name)
            self.assertIn('ImGui::CalcTextSize(tr("signals.count").c_str())', source)
            self.assertIn('ImGui::CalcTextSize(tr("signals.edit").c_str())', source)
        self.assertIn('ImGuiWindowFlags_HorizontalScrollbar', read(GUI / 'SignalsBufferDialog.cpp'))
        self.assertIn('ImGui::PushTextWrapPos', read(GUI / 'CreaturePreviewWidget.cpp'))
        self.assertIn('CalcTextSizeA(textSize, FLT_MAX, 0, label.c_str())', read(GUI / 'PreviewDescView.cpp'))
        ru = json.loads(read(ROOT / 'resources/locales/ru.json'))
        self.assertIn('донор', ru['genomeissues.first'])
        self.assertNotIn('пуст', ru['genomeissues.first'])
        for suffix in ['one', 'few', 'many']:
            self.assertIn('{count}', ru['genomeissues.cut.void.' + suffix])


if __name__ == '__main__':
    unittest.main()
