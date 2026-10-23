"""L4-A3 display contracts; production ImGui tests cover interactions and state."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
GUI = ROOT / 'source/Gui/Impl'
BASE = '0368045bdb359d16b7bd7eb05084b9ef1f678716'


def read(path):
    return path.read_text(encoding='utf8')


def original(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


def catalog(language):
    def unique(pairs):
        result = dict(pairs)
        if len(result) != len(pairs):
            raise ValueError('Duplicate key')
        return result
    return json.loads(read(ROOT / f'resources/locales/{language}.json'), object_pairs_hook=unique)


class L4a3Contracts(unittest.TestCase):
    def test_catalog_extension_keeps_old_entries_and_placeholder_signatures(self):
        en, ru = catalog('en'), catalog('ru')
        self.assertEqual(set(en), set(ru))
        for language, values in [('en', en), ('ru', ru)]:
            old = json.loads(original(f'resources/locales/{language}.json'))
            self.assertEqual(old, {key: values[key] for key in old})
            self.assertEqual(138, sum(not k.startswith(("mutation.", "genomeissues.", "genegraph.", "preview.", "signals.", "inspection.", "worldtools.", "simulationui.")) for k in values) - len(old))
            source = read(GUI / 'NodeEditorWidget.cpp') + read(GUI / 'GenomeEditorPresentation.cpp')
            for key in values.keys() - old.keys():
                if key.startswith(("mutation.", "genomeissues.", "genegraph.", "preview.", "signals.", "inspection.", "worldtools.", "simulationui.")):
                    continue
                self.assertTrue(key.startswith('node.'))
                self.assertRegex(key, r'^[a-z][a-z0-9]*(\.[a-z][a-z0-9]*)+$')
                self.assertTrue(values[key])
                self.assertNotIn('##', values[key])
                self.assertIn('"' + key + '"', source)
                self.assertEqual(re.findall(r'\{\w+}', en[key]), re.findall(r'\{\w+}', ru[key]))
                self.assertEqual(re.findall(r'#\d+', en[key]), re.findall(r'#\d+', ru[key]))

    def test_all_consumed_attributes_match_canonical_english_and_have_russian(self):
        adapter = read(GUI / 'GenomeEditorPresentation.cpp')
        node = read(GUI / 'NodeEditorWidget.cpp')
        help_source = read(GUI / 'EntityAttributeHelp.cpp')
        attributes = re.findall(r'case EntityAttribute::(\w+):\s*return tr\("(node.attribute.[^"]+)"\)', adapter)
        self.assertEqual(56, len(attributes))
        self.assertEqual(set(re.findall(r'attributeDescription\(EntityAttribute::(\w+)\)', node)), {a for a, _ in attributes})
        en, ru = catalog('en'), catalog('ru')
        for attribute, key in attributes:
            block = re.search(r'\{EntityAttribute::' + attribute + r',\s*(.*?)\},', help_source, re.S)[1]
            english = ''.join(json.loads(s) if s.startswith('"') else '\uf054'
                              for s in re.findall(r'"(?:[^"\\]|\\.)*"|ICON_FA_CHEVRON_RIGHT', block))
            self.assertEqual(english, en[key], attribute)
            self.assertNotEqual(en[key], ru[key], attribute)
        self.assertIn('return EntityAttributeHelp::get(attribute);', adapter)

    def test_enum_adapters_follow_canonical_order_without_changing_data_constants(self):
        constants = read(ROOT / 'source/Data/CellTypeConstants.h')
        self.assertEqual(original('source/Data/CellTypeConstants.h'), constants)
        adapter = read(GUI / 'GenomeEditorPresentation.cpp')
        en = catalog('en')
        for name in ['ProvideEnergyGenome', 'SensorMode', 'GeneratorMode', 'AttackerMode', 'MuscleMode',
                     'DefenderMode', 'ReconnectorMode', 'MemoryMode', 'CommunicatorMode']:
            canonical = re.findall(r'"([^"]*)"', re.search(name + r'Strings\s*=\s*\{([^}]+)\}', constants)[1])
            keys = re.findall(r'"(node.option.' + name.lower() + r'.[^"]+)"', adapter)
            self.assertEqual(canonical, [en[key] for key in keys])
            self.assertIn('static_assert(keys.size() == ' + name + '_Count)', adapter)
        self.assertNotIn('InjectorMode', read(GUI / 'NodeEditorWidget.cpp'))

    def test_node_ids_numeric_ranges_assignments_and_channel_masks_are_preserved(self):
        before = original('source/Gui/Impl/NodeEditorWidget.cpp')
        after = read(GUI / 'NodeEditorWidget.cpp')
        for pattern in [r'\.name\("([^"]+)"\)', r'ImGui::PushID\(([^;]+)\);', r'ImGui::BeginChild\("([^"]+)"',
                        r'\.(?:min|max|step|format|readOnly|infinity)\(([^\n]+)\)',
                        r'\b(?:node|cellTypeNode|constructor|sensor|generator|attacker|muscle|memory|communicator)\._\w+\s*(?:=|\|=)[^;]+;',
                        r'\bmemory\._signalEntries.resize\([^;]+;', r'bit\[[^\n;]+;',
                        r'create\w+GenomeDesc\(mode\)', r'digestor.setRawEnergyConversionRate\([^;]+;']:
            self.assertEqual(re.findall(pattern, before), re.findall(pattern, after), pattern)
        self.assertEqual(before[:before.index('void _NodeEditorWidget::processNodeAttributes()')],
                         after[:after.index('void _NodeEditorWidget::processNodeAttributes()')].replace('#include "GenomeEditorPresentation.h"\n', ''))

    def test_shared_extensions_are_opt_in_and_layout_measures_display_values(self):
        from localization_node_legacy import legacy_node_widget_text
        for name in ['AlienGui.h', 'AlienGui.cpp']:
            path = 'source/Gui/Impl/' + name
            self.assertEqual(original(path), legacy_node_widget_text(path, read(ROOT / path)))
        node = read(GUI / 'NodeEditorWidget.cpp')
        self.assertIn('ImGui::CalcTextSize(tr(key).c_str())', node)
        self.assertIn('ImGui::CalcTextSize(value.c_str())', node)
        self.assertIn('ImGuiWindowFlags_HorizontalScrollbar', node)
        self.assertRegex(node, r'\.values\(geneIds\)\s*\.displayValues\(genes\)')
        self.assertNotIn('EntityAttributeHelp::get(', node)

    def test_mutation_editor_engine_genomes_and_serialization_remain_unchanged(self):
        for path in ['source/Gui/Impl/EntityAttributeHelp.cpp',
                     'source/Data/GenomeDesc.h',
                     'source/Persister/Interface/SerializerService.cpp', 'source/Data/ShapeGenerator.h']:
            self.assertEqual(original(path), read(ROOT / path), path)


if __name__ == '__main__':
    unittest.main()
