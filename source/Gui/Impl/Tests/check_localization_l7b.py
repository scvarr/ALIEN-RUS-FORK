"""L7-B real-resource translation and unchanged English/MCP contracts.

Anchor, Markdown structure and rendering checks live in DocumentationLocalizationTests
and use the project's MarkdownParser rather than reconstructing its rules here.
"""
from collections import Counter
import hashlib
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
BASE = '9ab2171ef8e13d8b1d2dc56791152cf24a3fe0e7'
FILES = ('contents.md', 'getting-started.md', 'user-interface.md',
         'keyboard-and-mouse.md', 'genomes.md', 'neural-networks.md', 'first-creature.md')


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT).strip()


class L7bContracts(unittest.TestCase):
    def test_exact_package_and_utf8(self):
        root = ROOT / 'resources/docs/ru'
        expected = set(FILES) | {'cell-types.md', 'energy.md', 'simulation-parameters.md',
                                 'glossary.md', 'how-life-works.md'}
        self.assertEqual(expected, {path.name for path in root.glob('*.md')})
        for filename in FILES:
            text = (root / filename).read_bytes().decode('utf8')
            self.assertFalse(text.startswith('\ufeff'), filename)
            self.assertRegex(text, '[А-Яа-яЁё]', filename)
            self.assertNotIn('\ufffd', text, filename)

    def test_numbers_channels_and_keyboard_shortcuts(self):
        patterns = (r'(?<![A-Za-z])[-+]?\d+(?:[.,]\d+)*', r'#\d+',
                    r'\b(?:Alt|Ctrl|Shift)\+[A-Z0-9]\b|\b(?:F1|F7|Space|Esc|Del)\b|\*\*Enter\*\*|(?m:^\| Enter \|)')
        for filename in FILES:
            source = (ROOT / 'resources/docs' / filename).read_text(encoding='utf8')
            target = (ROOT / 'resources/docs/ru' / filename).read_text(encoding='utf8')
            for pattern in patterns:
                self.assertEqual(Counter(re.findall(pattern, source)), Counter(re.findall(pattern, target)), (filename, pattern))

    def test_english_assets_and_machine_contracts_match_base(self):
        paths = git('ls-tree', '-r', '--name-only', BASE, 'resources/docs', 'resources/help',
                    'source/McpTools', 'source/Network', 'resources/locales').decode().splitlines()
        self.assertGreater(len(paths), 90)
        for path in paths:
            self.assertEqual(git('rev-parse', f'{BASE}:{path}'),
                             git('hash-object', '--path=' + path, path), path)

    def test_release_resources_match_source_bytes(self):
        for output in ('build-agent/resources', 'build-agent/Release/resources'):
            root = ROOT / output
            self.assertTrue(root.is_dir(), output)
            paths = [ROOT / 'resources/docs/ru' / filename for filename in FILES]
            paths.extend((ROOT / 'resources/docs/images').glob('*.png'))
            for source in paths:
                deployed = root / source.relative_to(ROOT / 'resources')
                self.assertTrue(deployed.is_file(), str(deployed))
                self.assertEqual(hashlib.sha256(source.read_bytes()).digest(),
                                 hashlib.sha256(deployed.read_bytes()).digest(), str(deployed))

    def test_repository_instructions_remain_intact(self):
        self.assertEqual('090c381a6f2b1f62597108de236e53131559ffef796146011a5796dbb7ef3368',
                         hashlib.sha256((ROOT / 'AGENTS.md').read_bytes()).hexdigest())
        self.assertEqual(b'S AGENTS.md', git('ls-files', '-v', 'AGENTS.md'))


if __name__ == '__main__':
    unittest.main()
