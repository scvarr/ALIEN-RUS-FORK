"""L7-C resource, terminology, numeric and preservation contracts.

Markdown structure, duplicate anchors, F1 navigation, search and scales are checked
by DocumentationLocalizationTests with the real ALIEN MarkdownParser.
"""
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
BASE = '3bc6977fb3d5d62a3df7545191a74717beae717d'
FILES = ('cell-types.md', 'energy.md', 'simulation-parameters.md',
         'glossary.md', 'how-life-works.md')
NUMBER = r'(?<![A-Za-z])[-+]?\d+(?:[.,]\d+)*'


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT).strip()


def chapter(filename, russian=False):
    return (ROOT / 'resources/docs' / ('ru' if russian else '') / filename).read_text(encoding='utf8')


class L7cContracts(unittest.TestCase):
    def test_complete_utf8_numeric_and_channel_inventory(self):
        for filename in FILES:
            source, target = chapter(filename), chapter(filename, True)
            self.assertFalse(target.startswith('\ufeff'), filename)
            self.assertNotIn('\ufffd', target, filename)
            self.assertRegex(target, '[А-Яа-яЁё]', filename)
            for pattern in (NUMBER, r'#\d+', r'\b(?:Alt|Ctrl|Shift)\+[A-Z0-9]\b'):
                self.assertEqual(Counter(re.findall(pattern, source)),
                                 Counter(re.findall(pattern, target)), (filename, pattern))
            source_rows = [line.split('|')[1:-1] for line in source.splitlines() if line.startswith('|')]
            target_rows = [line.split('|')[1:-1] for line in target.splitlines() if line.startswith('|')]
            self.assertEqual(len(source_rows), len(target_rows), filename)
            for row, (english, russian) in enumerate(zip(source_rows, target_rows)):
                self.assertEqual(len(english), len(russian), (filename, row))
                for column, (left, right) in enumerate(zip(english, russian)):
                    self.assertEqual(re.findall(NUMBER, left), re.findall(NUMBER, right),
                                     (filename, row, column))

    def test_parameter_names_match_current_ui_catalog(self):
        english = json.loads((ROOT / 'resources/locales/en.json').read_text(encoding='utf8'))
        russian = json.loads((ROOT / 'resources/locales/ru.json').read_text(encoding='utf8'))
        names = {}
        for key, value in english.items():
            if key.startswith('parameters.') and key.endswith('.name'):
                names.setdefault(value, set()).add(russian[key])
        source_rows = [line.split('|')[1:-1] for line in chapter('simulation-parameters.md').splitlines() if line.startswith('|')]
        target_rows = [line.split('|')[1:-1] for line in chapter('simulation-parameters.md', True).splitlines() if line.startswith('|')]
        checked = 0
        for source, target in zip(source_rows, target_rows):
            name = source[0].strip()
            if name in names:
                self.assertIn(target[0].strip(), names[name], name)
                checked += 1
        self.assertGreaterEqual(checked, 65)

    def test_cell_types_energy_and_memory_terminology(self):
        target = chapter('cell-types.md', True)
        for name in ('Базовая', 'Накопитель', 'Сенсор', 'Генератор', 'Атакующая',
                     'Инжектор', 'Мышца', 'Защитник', 'Соединитель', 'Детонатор',
                     'Конвертер', 'Память', 'Коммуникатор', 'Донор', 'Конструктор'):
            self.assertIn('\n## ' + name + '\n', target)
        for mode in ('Автоизгиб', 'Управляемый изгиб', 'Изгиб по углу', 'Автоползание',
                     'Управляемое ползание', 'Прямое движение', 'Сглаживание сигнала'):
            self.assertIn('| ' + mode + ' |', target)
        energy = chapter('energy.md', True)
        for term in ('Доступная энергия', 'Сырая энергия', 'Запасённая энергия', 'Зарезервированная энергия'):
            self.assertIn('| ' + term + ' |', energy)
        self.assertIn('**Нейрон памяти.** Один из четырёх выходов нейросети', chapter('glossary.md', True))
        self.assertIn('Генератор создаёт периодический сигнал', target)
        self.assertIn('Это не тип клетки, а дополнительная функциональность', target)
        catalog = json.loads((ROOT / 'resources/locales/ru.json').read_text(encoding='utf8'))
        for key in ('genome.field.resistance', 'inspection.state.detonator.ready',
                    'inspection.state.detonator.activated', 'inspection.state.detonator.exploded'):
            self.assertIn('**' + catalog[key] + '**' if key.startswith('genome.') else '*' + catalog[key] + '*', target)

    def test_all_nineteen_image_references_and_release_resources(self):
        count = 0
        for filename in FILES:
            source = re.findall(r'!\[([^\]]*)\]\(([^)]+)\)', chapter(filename))
            target = re.findall(r'!\[([^\]]*)\]\(([^)]+)\)', chapter(filename, True))
            self.assertEqual(len(source), len(target), filename)
            for (caption, path), (translated, relative) in zip(source, target):
                self.assertEqual('../' + path, relative)
                self.assertNotEqual(caption, translated)
                self.assertRegex(translated, '[А-Яа-яЁё]')
                image = ROOT / 'resources/docs/ru' / relative
                self.assertTrue(image.is_file(), str(image))
                self.assertTrue(image.read_bytes().startswith(b'\x89PNG\r\n\x1a\n'))
                count += 1
        self.assertEqual(19, count)
        for output in ('build-agent/resources', 'build-agent/Release/resources'):
            paths = [ROOT / 'resources/docs/ru' / name for name in FILES]
            paths.extend((ROOT / 'resources/docs/images').glob('*.png'))
            for path in paths:
                deployed = ROOT / output / path.relative_to(ROOT / 'resources')
                self.assertEqual(hashlib.sha256(path.read_bytes()).digest(),
                                 hashlib.sha256(deployed.read_bytes()).digest(), str(deployed))

    def test_protected_resources_production_and_data_match_base(self):
        paths = git('ls-tree', '-r', '--name-only', BASE, 'resources/docs', 'resources/help',
                    'resources/locales', 'source/Data', 'source/Engine', 'source/McpTools',
                    'source/Network', 'source/Persister').decode().splitlines()
        self.assertGreater(len(paths), 400)
        for path in paths:
            if path.startswith('resources/docs/ru/'):
                previous = subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8')
                current = (ROOT / path).read_text(encoding='utf8')
                caption = r'!\[([^\]]*)\]\(([^)]+)\)'
                self.assertEqual(re.sub(caption, r'![](\2)', previous.replace('\r\n', '\n')),
                                 re.sub(caption, r'![](\2)', current), path)
                continue
            self.assertEqual(git('rev-parse', f'{BASE}:{path}'),
                             git('hash-object', '--path=' + path, path), path)
        self.assertEqual('090c381a6f2b1f62597108de236e53131559ffef796146011a5796dbb7ef3368',
                         hashlib.sha256((ROOT / 'AGENTS.md').read_bytes()).hexdigest())
        self.assertEqual(b'S AGENTS.md', git('ls-files', '-v', 'AGENTS.md'))
        self.assertEqual(b'27d9d82a2c4a5a506db5947c8464ab76f406b413', git('rev-parse', 'main'))
        self.assertEqual(b'', git('diff', BASE, '--', 'external/vcpkg'))


if __name__ == '__main__':
    unittest.main()
