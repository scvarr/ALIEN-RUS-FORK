"""L2-B4b typed presentation, sigma semantics, and unchanged model contracts."""
import json
import re
import subprocess
import unittest
from check_localization_l2b2 import ROOT, GUI, catalog, read
from check_localization_l2b3b import declarations

BASE = 'a3d2ae563c4be86b2c307a3ec1d69ec07e1ed78d'
MEMBERS = ['neurons', 'connections', 'cellTypeProperties', 'constructor', 'geometry',
           'cellType', 'cellTypeMode', 'void', 'customization']
TERMS = ['neuron', 'connection', 'cell type property', 'constructor', 'geometry',
         'cell type', 'cell type mode', 'void', 'customization']


def original(file):
    return subprocess.check_output(['git', 'show', f'{BASE}:{file}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


def legacy_widget_text(file, text):
    """Exclude only B4b presentation extension sites from earlier whole-file contracts."""
    from localization_node_legacy import legacy_node_widget_text
    text = legacy_node_widget_text(file, text)
    from check_localization_l2b4d import legacy_float_matrix_text
    text = legacy_float_matrix_text(file, text)
    old = original(file)
    def restore_region(start, end):
        nonlocal text
        a, b = text.index(start), text.index(end, text.index(start))
        x, y = old.index(start), old.index(end, old.index(start))
        text = text[:a] + old[x:y] + text[b:]
    if file.endswith('AlienGui.cpp'):
        restore_region('void AlienGui::CheckboxColorMatrix(', 'void AlienGui::InputIntColorMatrix(')
        restore_region('void AlienGui::BasicInputColorMatrix(', '\nnamespace\n')
        text = text[:text.index('\nbool AlienGui::DisplayButton(')]
    elif file.endswith('AlienGui.h'):
        for struct in ('ExpandedColorMatrixParameters', 'CheckboxColorMatrixParameters'):
            text = text.replace(f'        MEMBER({struct}, std::optional<std::string>, label, std::nullopt);\n', '')
            text = text.replace(f'        MEMBER({struct}, std::string, localizationKey, "");\n', '')
        text = text.replace('    static bool DisplayButton(std::string const& name, std::string const& label, ImVec2 size = ImVec2(0, 0));\n', '')
    elif file.endswith('SpecificationGuiService.cpp'):
        restore_region('void SpecificationGuiService::createWidgetsForBoolSpec(', 'void SpecificationGuiService::createWidgetsForIntSpec(')
    return text


class L2b4bContracts(unittest.TestCase):
    def test_ten_typed_bindings_complete_catalogs_and_exact_english(self):
        en, ru = catalog('en'), catalog('ru')
        self.assertEqual(set(en), set(ru))
        self.assertEqual(433, sum(not key.startswith(("genome.", "neural.", "node.", "mutation.", "genomeissues.", "genegraph.", "preview.", "signals.")) for key in en))
        adapter = read(GUI / 'SimulationParametersPresentation.cpp')
        blocks = declarations()
        for i, member in enumerate(MEMBERS + ['customizationTransitionMatrix']):
            field = member + 'MetaMutationsSigma' if i < 9 else member
            prefix = 'parameters.' + ('metamproperties.' if i < 5 else 'metamidentity.') + field.lower()
            typ = 'FloatSpec' if i < 9 else 'BoolSpec'
            self.assertEqual(1, adapter.count(f'references<{typ}>(spec, &SimulationParameters::{field})'))
            self.assertIn(f'return "{prefix}";', adapter)
            self.assertEqual(re.search(r'\.name\("([^"]+)"\)', blocks[field])[1], en[prefix + '.name'])
            if i < 9:
                description = ('Standard deviation of the Gaussian change of the probability of the ' + TERMS[i]
                    + " mutations stored in a genome. It only acts on genomes for which 'Apply meta-mutations' is enabled, and is applied once per offspring.")
            else:
                description = 'Determines which customizations a customization mutation may produce. Row = old customization, column = new one.'
            self.assertEqual(description, en[prefix + '.description'])
            self.assertNotEqual(en[prefix + '.name'], ru[prefix + '.name'])
            self.assertNotEqual(description, ru[prefix + '.description'])
        for language in ('en', 'ru'):
            old = json.loads(original(f'resources/locales/{language}.json'))
            self.assertEqual(old, {k: catalog(language)[k] for k in old})

    def test_sigma_ranges_defaults_and_meaning(self):
        ru = catalog('ru')
        blocks = declarations()
        header = read(ROOT / 'source/Data/SimulationParameters.h')
        for i, member in enumerate(MEMBERS):
            field = member + 'MetaMutationsSigma'
            self.assertIn('.min(0.0f).max(1.0f).logarithmic(true).format("%.5f")', blocks[field])
            self.assertRegex(header, field + r'\s*=\s*\{0\.?0?f?\}')
            prefix = 'parameters.' + ('metamproperties.' if i < 5 else 'metamidentity.') + field.lower()
            for phrase in ('Стандартное отклонение', 'гауссова изменения', 'сохранённой в геноме вероятности',
                           'только на геномы', 'Apply meta-mutations', 'один раз на потомка'):
                self.assertIn(phrase, ru[prefix + '.description'])
        void = ru['parameters.metamidentity.voidmetamutationssigma.description']
        self.assertIn('между типом «Донор» (Void) и типом, отличным от «Донор»', void)
        self.assertNotIn('Кастомизация', json.dumps(ru, ensure_ascii=False))

    def test_model_engine_persistence_and_glossary_are_unchanged(self):
        files = ['source/Data/SimulationParameters.cpp', 'source/Data/SimulationParameters.h',
                 'source/Data/Colors.h', 'source/Data/SimulationParametersSpecification.h',
                 'source/Data/ParametersValidationService.cpp', 'source/Data/SpecificationEvaluationService.cpp',
                 'source/Engine/Impl/Kernels/MutationProcessor.cuh',
                 'source/Persister/Interface/SerializerService.cpp', 'docs/localization-terminology.md']
        for file in files:
            self.assertEqual(original(file), read(ROOT / file), file)
        for file in ('AlienGui.cpp', 'AlienGui.h', 'SpecificationGuiService.cpp'):
            path = 'source/Gui/Impl/' + file
            self.assertEqual(original(path), legacy_widget_text(path, read(ROOT / path)), path)

    def test_matrix_direction_ids_defaults_and_optional_compatibility(self):
        gui = read(GUI / 'AlienGui.cpp')
        dialog = read(GUI / 'ColorMatrixDialog.cpp')
        service = read(GUI / 'SpecificationGuiService.cpp')
        engine = read(ROOT / 'source/Engine/Impl/Kernels/MutationProcessor.cuh')
        self.assertIn('customizationTransitionMatrix.value[candidateSourceColor][color]', engine)
        self.assertIn('ImGui::Checkbox(("##" + parameters._name).c_str(), &value[row - 1][col - 1])', gui)
        self.assertIn('parameters._disableDiagonal && row == col', gui)
        self.assertIn('.disableDiagonal(true)', service)
        self.assertIn('ImGui::PushID(parameters._name.c_str())', gui)
        self.assertIn('auto id = ImGui::GetID(name.c_str())', gui)
        self.assertIn('ImGui::PushOverrideID(id)', gui)
        self.assertIn('_modalWindow.setTitle(parameters._name)', dialog)
        self.assertIn('Row = old customization, column = new one.', catalog('en')['parameters.metamidentity.customizationtransitionmatrix.description'])
        self.assertEqual('Исходная настройка', catalog('ru')['parameters.matrix.row'])
        self.assertEqual('Новая настройка', catalog('ru')['parameters.matrix.column'])
        self.assertIn('parameters._localizationKey.empty() ? "Edit matrix"', gui)
        for key in ('edit', 'row', 'column', 'adopt', 'cancel', 'clear', 'selectall', 'invert', 'randomize'):
            self.assertTrue(catalog('en')['parameters.matrix.' + key])
            self.assertTrue(catalog('ru')['parameters.matrix.' + key])


if __name__ == '__main__':
    unittest.main(verbosity=2)
