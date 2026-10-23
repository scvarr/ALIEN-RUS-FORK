"""Food-chain matrix and four cell-type presentation contracts."""
import json
import re
import subprocess
import unittest
from check_localization_l2b2 import ROOT, GUI, catalog, read
from check_localization_l2b3b import declarations

BASE = '03a3131ceac4d2f762d7de5e0ad510c7cae6b5d0'
FIELDS = ['attackerEnergyCost', 'attackerFoodChainColorMatrix', 'attackerStrength',
          'attackerRelatedLineageProtection', 'attackerSizeProtection', 'attackerRadius',
          'maxRawEnergyConductivity', 'maxRawEnergyConversion', 'defenderAntiAttackerStrength',
          'defenderAntiInjectorStrength', 'injectorEnergyCost', 'injectorRadius']
GROUPS = ['attacker'] * 6 + ['digestor'] * 2 + ['defender'] * 2 + ['injector'] * 2


def original(file):
    return subprocess.check_output(['git', 'show', f'{BASE}:{file}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


def legacy_float_matrix_text(file, text):
    """Remove exactly the optional float presentation extension for older byte contracts."""
    from check_localization_l2b4e import legacy_transition_text
    text = legacy_transition_text(file, text)
    if file.endswith('AlienGui.cpp'):
        start, end = 'void AlienGui::InputFloatColorMatrix(', '\nbool AlienGui::InputText('
        removals = ['    matrixParameters._' + name + ' = parameters._' + name + ';\n'
                    for name in ('label', 'localizationKey', 'rowLabel', 'columnLabel')]
    elif file.endswith('SpecificationGuiService.cpp'):
        start, end = 'void SpecificationGuiService::createWidgetsForFloatSpec(', '\nvoid SpecificationGuiService::createWidgetsForFloat2Spec('
        removals = [
            '    auto member = std::get_if<std::shared_ptr<decltype(&SimulationParameters::attackerFoodChainColorMatrix)>>(&floatSpec._member);\n',
            '    auto localizedMatrix = member && *member && **member == &SimulationParameters::attackerFoodChainColorMatrix;\n',
            '                .label(localizedMatrix ? std::optional(SimulationParametersPresentation::label(parameterSpec)) : std::nullopt)\n',
            '                .localizationKey(localizedMatrix ? "parameters.attacker.attackerfoodchaincolormatrix" : "")\n',
            '                .rowLabel(localizedMatrix ? tr("parameters.matrix.attackerrow") : "[cell color]")\n',
            '                .columnLabel(localizedMatrix ? tr("parameters.matrix.targetcolumn") : "[target cell color]")\n']
    elif file.endswith('AlienGui.h'):
        for line in ('std::optional<std::string>, label, std::nullopt', 'std::string, localizationKey, ""',
                     'std::string, rowLabel, "[cell color]"', 'std::string, columnLabel, "[target cell color]"'):
            text = text.replace(f'        MEMBER(InputFloatColorMatrixParameters, {line});\n', '')
        return text
    else:
        return text
    a, b = text.index(start), text.index(end, text.index(start))
    block = text[a:b]
    for line in removals:
        block = block.replace(line, '')
    block = block.replace('.tooltip(localizedMatrix ? SimulationParametersPresentation::description(parameterSpec) : parameterSpec._description)',
                          '.tooltip(parameterSpec._description)')
    return text[:a] + block + text[b:]


class L2b4dContracts(unittest.TestCase):
    def test_exact_catalog_extension_and_twelve_typed_bindings(self):
        en, ru = catalog('en'), catalog('ru')
        self.assertEqual(set(en), set(ru))
        expected = {f'parameters.{g}.{f.lower()}.{suffix}' for f, g in zip(FIELDS, GROUPS) for suffix in ('name', 'description')}
        expected |= {f'parameters.{g}.name' for g in GROUPS}
        expected |= {'parameters.matrix.attackerrow', 'parameters.matrix.targetcolumn'}
        from check_localization_l2b4e import NEW_KEYS
        expected |= NEW_KEYS
        adapter = read(GUI / 'SimulationParametersPresentation.cpp')
        for lang in ('en', 'ru'):
            old = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(expected, {key for key in catalog(lang) if not key.startswith(("genome.", "neural.", "node.", "mutation.", "genomeissues.", "genegraph.", "preview.", "signals.", "inspection."))} - set(old))
            self.assertEqual(old, {k: catalog(lang)[k] for k in old})
        for field, group in zip(FIELDS, GROUPS):
            prefix = f'parameters.{group}.{field.lower()}'
            self.assertEqual(1, adapter.count(f'references<FloatSpec>(spec, &SimulationParameters::{field})'))
            self.assertIn(f'return "{prefix}";', adapter)
            block = declarations()[field]
            self.assertEqual(re.search(r'\.name\("([^"]+)"\)', block)[1], en[prefix + '.name'])
            description = block[block.index('.description(') + len('.description('):]
            self.assertEqual(''.join(json.loads(m[0]) for m in re.finditer(r'"(?:[^"\\]|\\.)*"', description)), en[prefix + '.description'])
            self.assertNotEqual(en[prefix + '.description'], ru[prefix + '.description'])
        for enname, runame in [('Attacker', 'Атакующая'), ('Digestor', 'Конвертер'), ('Defender', 'Защитник'), ('Injector', 'Инжектор')]:
            self.assertEqual('Тип клетки: ' + runame, ru[f'parameters.{enname.lower()}.name'])

    def test_matrix_is_typed_numeric_opt_in_and_keeps_original_handlers(self):
        service = read(GUI / 'SpecificationGuiService.cpp')
        self.assertIn('member && *member && **member == &SimulationParameters::attackerFoodChainColorMatrix', service)
        self.assertIn('.tooltip(localizedMatrix ? SimulationParametersPresentation::description(parameterSpec) : parameterSpec._description)', service)
        for file in ('AlienGui.cpp', 'AlienGui.h', 'SpecificationGuiService.cpp'):
            path = 'source/Gui/Impl/' + file
            self.assertEqual(original(path), legacy_float_matrix_text(path, read(ROOT / path)))
        self.assertEqual(original('source/Gui/Impl/ColorMatrixDialog.cpp'), read(GUI / 'ColorMatrixDialog.cpp'))
        for text in ('matrixParameters._label = parameters._label', 'matrixParameters._localizationKey = parameters._localizationKey',
                     'matrixParameters._rowLabel = parameters._rowLabel', 'matrixParameters._columnLabel = parameters._columnLabel'):
            self.assertIn(text, read(GUI / 'AlienGui.cpp'))

    def test_semantics_ranges_and_model_algorithms_are_unchanged(self):
        ru = catalog('ru')
        self.assertIn('при каждой попытке атаки', ru['parameters.attacker.attackerenergycost.description'])
        self.assertIn('при каждой успешной инъекции', ru['parameters.injector.injectorenergycost.description'])
        matrix = ru['parameters.attacker.attackerfoodchaincolormatrix.description']
        for phrase in ('строки', 'столбца', 'строке 2 и столбце 3', 'Промежуточные значения'):
            self.assertIn(phrase, matrix)
        self.assertIn('доступную энергию', ru['parameters.digestor.maxrawenergyconversion.description'])
        for file in ['source/Data/SimulationParameters.cpp', 'source/Data/SimulationParameters.h',
                     'source/Data/SimulationParametersSpecification.h', 'source/Data/SpecificationEvaluationService.cpp',
                     'source/Persister/Interface/SerializerService.cpp', 'docs/localization-terminology.md']:
            self.assertEqual(original(file), read(ROOT / file), file)
        changed = subprocess.check_output(['git', 'diff', BASE, '--name-only'], cwd=ROOT, text=True).splitlines()
        for file in changed:
            self.assertFalse(file.startswith(('source/Engine/', 'source/Data/', 'source/Apps/', 'source/McpTools/',
                                              'source/Network/', 'source/Persister/', 'external/')), file)

    def test_original_numeric_ranges_formats_defaults_and_color_dependence(self):
        blocks = declarations()
        header = read(ROOT / 'source/Data/SimulationParameters.h')
        contracts = [
            (0, 1, True, '%.5f', 'BaseLayerParameter<ColorVector<float>>', '0.0f'),
            (0, 1, False, '%.2f', 'BaseLayerParameter<ColorMatrix<float>>', '1.0f'),
            (0, 0.5, True, None, 'BaseParameter<float>', '0.05f'),
            (0, 1, False, None, 'BaseParameter<ColorVector<float>>', '0.0f'),
            (0, 1, False, '%.2f', 'BaseLayerParameter<float>', '0.0f'),
            (0, 4, False, None, 'BaseParameter<ColorVector<float>>', '2.0f'),
            (0, 6, False, '%.3f', 'BaseParameter<ColorVector<float>>', '3.0f'),
            (0, 1, False, '%.3f', 'BaseParameter<ColorVector<float>>', '0.1f'),
            (0, 5, False, None, 'BaseParameter<ColorVector<float>>', '0.5f'),
            (0, 5, False, None, 'BaseParameter<ColorVector<float>>', '0.5f'),
            (0, 1000, True, None, 'BaseParameter<ColorVector<float>>', '250.0f'),
            (0.1, 4, False, None, 'BaseParameter<ColorVector<float>>', '3.0f')]
        for field, (minimum, maximum, logarithmic, fmt, typ, default) in zip(FIELDS, contracts):
            block = blocks[field]
            self.assertEqual(minimum, float(re.search(r'\.min\(([0-9.]+)f\)', block)[1]))
            self.assertEqual(maximum, float(re.search(r'\.max\(([0-9.]+)f\)', block)[1]))
            self.assertEqual(logarithmic, '.logarithmic(true)' in block)
            self.assertEqual([] if fmt is None else [fmt], re.findall(r'\.format\("([^"]+)"\)', block))
            declaration = next(line for line in header.splitlines() if re.search(r'\b' + field + r'\s*=', line))
            self.assertIn(typ, declaration)
            self.assertIn(default, declaration)


if __name__ == '__main__':
    unittest.main(verbosity=2)
