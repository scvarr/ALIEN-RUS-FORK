"""Structural meta-mutation presentation contracts; native interaction tests live in C++."""
import json
import re
import subprocess
import unittest
from check_localization_l2b2 import ROOT, GUI, catalog, read
from check_localization_l2b3b import declarations

BASE = '634970082bd0c1e628cba91f5c40cbca2d098879'
MEMBERS = ['addNode', 'deleteNode', 'extendGene', 'trimGene', 'copyNodeSection',
           'moveNodeSection', 'addGene', 'duplicateGene', 'deleteGene', 'swapGene']
TERMS = ['Add node', 'Delete node', 'Extend gene', 'Trim gene', 'Copy node section',
         'Move node section', 'Add gene', 'Duplicate gene', 'Delete gene', 'Swap gene']
CAPTIONS = ['Добавление узла', 'Удаление узла', 'Расширение гена', 'Укорочение гена',
            'Копирование участка узлов', 'Перемещение участка узлов', 'Добавление гена',
            'Дублирование гена', 'Удаление гена', 'Перестановка генов']


def original(file):
    return subprocess.check_output(['git', 'show', f'{BASE}:{file}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


class L2b4cContracts(unittest.TestCase):
    def test_ten_typed_bindings_and_full_catalogs(self):
        en, ru = catalog('en'), catalog('ru')
        self.assertEqual(set(en), set(ru))
        self.assertEqual(433, sum(not key.startswith("genome.") for key in en))
        adapter = read(GUI / 'SimulationParametersPresentation.cpp')
        blocks = declarations()
        for i, member in enumerate(MEMBERS):
            field = member + 'MetaMutationsSigma'
            prefix = 'parameters.' + ('metamgene.' if i < 6 else 'metamgenome.') + field.lower()
            self.assertEqual(1, adapter.count(f'references<FloatSpec>(spec, &SimulationParameters::{field})'))
            self.assertIn(f'return "{prefix}";', adapter)
            self.assertEqual(TERMS[i] + ' sigma', en[prefix + '.name'])
            self.assertEqual(CAPTIONS[i] + ': сигма', ru[prefix + '.name'])
            self.assertEqual(re.search(r'\.name\("([^"]+)"\)', blocks[field])[1], en[prefix + '.name'])
            expected = ('Standard deviation of the Gaussian change of the probability of the ' + TERMS[i].lower()
                        + " mutations stored in a genome. It only acts on genomes for which 'Apply meta-mutations' is enabled, and is applied once per offspring.")
            self.assertEqual(expected, en[prefix + '.description'])
            for phrase in ('Стандартное отклонение', 'гауссова изменения', 'сохранённой в геноме вероятности',
                           'только на геномы', 'Apply meta-mutations', 'один раз на потомка'):
                self.assertIn(phrase, ru[prefix + '.description'])

    def test_groups_and_previous_entries_unchanged(self):
        for lang in ('en', 'ru'):
            entries = catalog(lang)
            old = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(old, {k: entries[k] for k in old})
            for scope, english, russian in [('gene', 'Gene', 'гена'), ('genome', 'Genome', 'генома')]:
                self.assertEqual('Meta-mutations: ' + english + ' structure' if lang == 'en'
                                 else 'Метамутации: структура ' + russian, entries[f'parameters.metam{scope}.name'])

    def test_defaults_ranges_formats_and_operation_distinctions(self):
        blocks = declarations()
        header = read(ROOT / 'source/Data/SimulationParameters.h')
        for member in MEMBERS:
            field = member + 'MetaMutationsSigma'
            self.assertIn('.min(0.0f).max(1.0f).logarithmic(true).format("%.5f")', blocks[field])
            self.assertRegex(header, field + r'\s*=\s*\{0\.?0?f?\}')
        ru = catalog('ru')
        def description(member, genome=False):
            return ru['parameters.' + ('metamgenome.' if genome else 'metamgene.') + member.lower() + 'metamutationssigma.description']
        self.assertIn('любую позицию', description('addNode'))
        self.assertIn('только в начало или конец', description('extendGene'))
        self.assertIn('как минимум один узел', description('deleteNode'))
        self.assertIn('только с начала или конца', description('trimGene'))
        self.assertIn('внутри одного генома', description('swapGene', True))
        self.assertIn('независимую копию', description('duplicateGene', True))

    def test_engine_model_serialization_widgets_and_glossary_unchanged(self):
        files = ['source/Data/SimulationParameters.cpp', 'source/Data/SimulationParameters.h',
                 'source/Data/SimulationParametersSpecification.h', 'source/Data/ParametersValidationService.cpp',
                 'source/Data/SpecificationEvaluationService.cpp', 'source/Engine/Impl/Kernels/MutationProcessor.cuh',
                 'source/Persister/Interface/SerializerService.cpp', 'docs/localization-terminology.md',
                 'source/Gui/Impl/SpecificationGuiService.cpp', 'source/Gui/Impl/MutationRatesWidget.cpp',
                 'source/Gui/Impl/AlienGui.cpp', 'source/Gui/Impl/AlienGui.h', 'source/Gui/Impl/ColorMatrixDialog.cpp']
        for file in files:
            from check_localization_l2b4d import legacy_float_matrix_text
            self.assertEqual(original(file), legacy_float_matrix_text(file, read(ROOT / file)), file)
        changed = subprocess.check_output(['git', 'diff', BASE, '--name-only'], cwd=ROOT, text=True).splitlines()
        for file in changed:
            self.assertFalse(file.startswith(('source/Engine/', 'source/Data/', 'source/Apps/', 'source/McpTools/',
                                              'source/Network/', 'source/Persister/', 'external/')), file)


if __name__ == '__main__':
    unittest.main(verbosity=2)
