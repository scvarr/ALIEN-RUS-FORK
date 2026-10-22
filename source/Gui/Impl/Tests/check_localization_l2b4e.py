"""Final parameter groups: typed presentation and unchanged transition editor contracts."""
import json
import re
import subprocess
import unittest
from check_localization_l2b2 import ROOT, GUI, catalog, read
from check_localization_l2b3b import declarations

BASE = '80f106a6485a0caedc5d77849f221e12db671790'
FIELDS = ['muscleEnergyCost', 'muscleMovementAcceleration', 'muscleCrawlingAcceleration',
          'muscleBendingAcceleration', 'sensorRadius', 'reconnectorRadius', 'detonatorRadius',
          'detonatorChainExplosionProbability', 'colorTransitionRules']
GROUPS = ['muscle'] * 4 + ['sensor', 'reconnector'] + ['detonator'] * 2 + ['colortransition']
NEW_KEYS = {f'parameters.{g}.{f.lower()}.{suffix}' for f, g in zip(FIELDS, GROUPS) for suffix in ('name', 'description')}
NEW_KEYS |= {f'parameters.{g}.name' for g in GROUPS}
NEW_KEYS |= {'parameters.colortransition.description', 'parameters.expert.prefix'}


def original(file):
    return subprocess.check_output(['git', 'show', f'{BASE}:{file}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


def legacy_transition_text(file, text):
    """Remove only the exact B4e presentation additions for older whole-file contracts."""
    if file.endswith('AlienGui.h'):
        return text.replace('        MEMBER(InputColorTransitionParameters, std::optional<std::string>, label, std::nullopt);\n', '')
    if file.endswith('AlienGui.cpp'):
        a = text.index('void AlienGui::InputColorTransition(')
        b = text.index('\nnamespace', a)
        block = text[a:b].replace('TextParameters().text(parameters._label.value_or(parameters._name)).truncate(parameters._label.has_value())',
                                  'TextParameters().text(parameters._name.c_str())')
        return text[:a] + block + text[b:]
    if file.endswith('SpecificationGuiService.cpp'):
        text = text.replace('.label(SimulationParametersPresentation::heading(groupSpec))',
                            '.label(isExpertSettings ? name : SimulationParametersPresentation::label(groupSpec))')
        text = text.replace('                    .label(SimulationParametersPresentation::label(groupSpec))\n', '')
        text = text.replace('.tooltip(SimulationParametersPresentation::description(groupSpec))', '.tooltip(groupSpec._description)')
        text = text.replace('widgetParameters.name(parameterSpec._name)\n'
                            '                .label(SimulationParametersPresentation::label(parameterSpec))\n'
                            '                .tooltip(SimulationParametersPresentation::description(parameterSpec));',
                            'widgetParameters.name(parameterSpec._name).tooltip(parameterSpec._description);')
    return text


class L2b4eContracts(unittest.TestCase):
    def test_exact_extension_retains_all_408_existing_entries_and_typed_identity(self):
        self.assertEqual(set(catalog('en')), set(catalog('ru')))
        adapter = read(GUI / 'SimulationParametersPresentation.cpp')
        for lang in ('en', 'ru'):
            old = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(408, len(old))
            self.assertEqual(NEW_KEYS, {key for key in catalog(lang) if not key.startswith("genome.")} - set(old))
            self.assertEqual(old, {k: catalog(lang)[k] for k in old})
        for field, group in zip(FIELDS, GROUPS):
            typ = 'ColorTransitionRulesSpec' if field == 'colorTransitionRules' else 'FloatSpec'
            self.assertEqual(1, adapter.count(f'references<{typ}>(spec, &SimulationParameters::{field})'))
            prefix = f'parameters.{group}.{field.lower()}'
            self.assertIn(f'return "{prefix}";', adapter)
            block = declarations()[field]
            self.assertEqual(re.search(r'\.name\("([^"]+)"\)', block)[1], catalog('en')[prefix + '.name'])
            description = block.split('.description(', 1)[1]
            expected = ''.join(json.loads(m[0]) for m in re.finditer(r'"(?:[^"\\]|\\.)*"', description))
            self.assertEqual(expected, catalog('en')[prefix + '.description'])
            self.assertNotEqual(expected, catalog('ru')[prefix + '.description'])

    def test_ranges_formats_defaults_and_infinity_representation_are_unchanged(self):
        blocks = declarations()
        header = read(ROOT / 'source/Data/SimulationParameters.h')
        for i, (field, minimum, maximum, default) in enumerate(zip(FIELDS[:8], [0, 0, 0, 0, 10, 0, 0, 0],
                                                               [5, 10, 10, 10, 800, 3, 10, 1], [0, 1, 1, 1, 255, 2, 10, 0.2])):
            block = blocks[field]
            self.assertEqual(minimum, float(re.search(r'\.min\(([0-9.]+)f\)', block)[1]))
            self.assertEqual(maximum, float(re.search(r'\.max\(([0-9.]+)f\)', block)[1]))
            self.assertEqual(i < 4, '.logarithmic(true)' in block)
            self.assertEqual(['%.5f'] if i == 0 else [], re.findall(r'\.format\("([^"]+)"\)', block))
            declaration = next(line for line in header.splitlines() if re.search(r'\b' + field + r'\s*=', line))
            self.assertIn('BaseParameter<ColorVector<float>>', declaration)
            self.assertEqual(default, float(re.search(r'uniform\(([0-9.]+)f\)', declaration)[1]))
        widget = read(GUI / 'AlienGui.h').split('struct InputColorTransitionParameters', 1)[1].split('static void InputColorTransition', 1)[0]
        for text in ('int, min, 1', 'int, max, 10000000', 'bool, logarithmic, false', 'bool, infinity, false'):
            self.assertIn(text, widget)
        types = read(ROOT / 'source/Data/SimulationParametersTypes.h')
        self.assertIn('int duration = Infinity<int>::value;', types)
        self.assertIn('BaseLayerParameter<ColorVector<ColorTransitionRule>> colorTransitionRules;', header)

    def test_shared_editor_changes_are_only_display_and_keep_every_handler(self):
        for file in ('AlienGui.cpp', 'AlienGui.h', 'SpecificationGuiService.cpp'):
            path = 'source/Gui/Impl/' + file
            self.assertEqual(original(path), legacy_transition_text(path, read(ROOT / path)), file)
        service = read(GUI / 'SpecificationGuiService.cpp')
        self.assertIn('name = "Expert settings: " + name;', service)
        self.assertIn('ImGui::PushID(name.c_str())', service)
        self.assertIn('.name(groupSpec._name)\n                    .label(SimulationParametersPresentation::label(groupSpec))', service)
        self.assertIn('for (int color = 0; color < MAX_COLORS; ++color)', service)
        self.assertIn('AlienGui::InputColorTransition(widgetParameters, color, value[color].targetColor, value[color].duration)', service)
        self.assertIn('.logarithmic(true)\n                                    .infinity(true)', service)
        adapter = read(GUI / 'SimulationParametersPresentation.cpp')
        self.assertIn('*spec._expertToggle == &SimulationParameters::colorTransitionRulesToggle', adapter)
        self.assertIn('heading(group), *filter.containedText', adapter)

    def test_semantics_and_protected_sources_remain_intact(self):
        ru = catalog('ru')
        for key, phrase in [('muscle.musclemovementacceleration', 'исключительно'), ('muscle.musclecrawlingacceleration', 'сигналы клетки'),
                            ('muscle.musclebendingacceleration', 'сигналы клетки'), ('sensor.sensorradius', 'дополнительно ограничивается'),
                            ('detonator.detonatorradius', 'в восемь раз больше'), ('detonator.detonatorchainexplosionprobability', 'других детонаторов'),
                            ('colortransition.colortransitionrules', 'возраст клетки сбрасывается')]:
            self.assertIn(phrase, ru['parameters.' + key + '.description'])
        for file in ['source/Data/SimulationParameters.cpp', 'source/Data/SimulationParameters.h', 'source/Data/SimulationParametersTypes.h',
                     'source/Data/SimulationParametersSpecification.h', 'source/Data/SpecificationEvaluationService.cpp',
                     'source/Persister/Interface/SettingsParserService.cpp', 'source/Persister/Interface/SerializerService.cpp',
                     'source/Engine/Impl/Kernels/MuscleProcessor.cuh', 'source/Engine/Impl/Kernels/SensorProcessor.cuh',
                     'source/Engine/Impl/Kernels/ReconnectorProcessor.cuh', 'source/Engine/Impl/Kernels/DetonatorProcessor.cuh',
                     'source/Engine/Impl/Kernels/CellProcessor.cuh', 'docs/localization-terminology.md']:
            self.assertEqual(original(file), read(ROOT / file), file)


if __name__ == '__main__':
    unittest.main(verbosity=2)
