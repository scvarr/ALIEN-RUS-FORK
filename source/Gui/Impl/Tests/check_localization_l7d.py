"""Complete F1 package, command, image and preservation contracts.

Real Markdown parsing, anchors, search and ImGui rendering are exercised by
DocumentationLocalizationTests, rather than approximated by this script.
"""
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
BASE = '27d9d82a2c4a5a506db5947c8464ab76f406b413'
FILES = ('sandbox.md', 'evolution.md', 'generative-art.md', 'ai-agents.md',
         'files.md', 'troubleshooting.md', 'format-reference.md')
DOCS = ROOT / 'resources/docs'
IMAGE = r'!\[([^\]]*)\]\(([^)]+)\)'


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT).strip()


class L7dContracts(unittest.TestCase):
    def test_complete_utf8_numeric_and_machine_contracts(self):
        self.assertEqual(19, len(list(DOCS.glob('*.md'))))
        self.assertEqual({p.name for p in DOCS.glob('*.md')},
                         {p.name for p in (DOCS / 'ru').glob('*.md')})
        for filename in FILES:
            en = (DOCS / filename).read_text(encoding='utf8')
            ru = (DOCS / 'ru' / filename).read_text(encoding='utf8')
            self.assertFalse(ru.startswith('\ufeff'))
            self.assertNotIn('\ufffd', ru)
            self.assertRegex(ru, '[А-Яа-яЁё]')
            for pattern in (r'(?<![A-Za-z])[-+]?\d+(?:[.,]\d+)*', r'#\d+',
                            r'\b(?:Alt|Ctrl|Shift)\+[A-Z0-9]\b'):
                self.assertEqual(Counter(re.findall(pattern, en)), Counter(re.findall(pattern, ru)), filename)
            en_code = re.findall(r'```\n(.*?)```', en, re.S)
            ru_code = re.findall(r'```\n(.*?)```', ru, re.S)
            en_code = [value.replace('Code inside a list item', 'Код внутри пункта списка') for value in en_code]
            self.assertEqual(en_code, ru_code, filename)
            self.assertEqual(re.findall(r'https?://[^\s)`]+', en),
                             re.findall(r'https?://[^\s)`]+', ru), filename)
        en = (DOCS / 'ai-agents.md').read_text(encoding='utf8')
        ru = (DOCS / 'ru/ai-agents.md').read_text(encoding='utf8')
        tools = re.findall(r'`([a-z_]+)`', en)
        self.assertEqual(tools, re.findall(r'`([a-z_]+)`', ru))
        implementations = '\n'.join(p.read_text(encoding='utf8') for p in (ROOT / 'source/McpTools/Impl').glob('*.cpp'))
        for tool in tools:
            self.assertIn('.name = "' + tool + '"', implementations, tool)
        parser = (ROOT / 'source/Apps/Alien-cli/CommandLineParser.cpp').read_text(encoding='utf8')
        for flag in ('-i', '-o', '-t', '-u', '-p', '--upload-name', '--upload-interval', '-d', '--plain'):
            self.assertIn(flag, parser)
            self.assertIn(flag, (DOCS / 'ru/files.md').read_text(encoding='utf8'))

    def test_actual_ui_captions(self):
        catalog = json.loads((ROOT / 'resources/locales/ru.json').read_text(encoding='utf8'))
        for filename, keys in {
            'sandbox.md': ('worldtools.drawfreehand', 'worldtools.singleobject', 'worldtools.patternfromimage',
                           'worldtools.selectandmove', 'worldtools.makeuniformvelocities', 'parameters.motion.rigidity.name',
                           'inspection.label.static', 'inspection.label.sticky'),
            'evolution.md': ('node.label.autotriggerinterval', 'node.label.provideenergy', 'node.label.separation',
                             'genome.field.metamutations', 'mutation.rateone', 'mutation.ratetwo'),
            'generative-art.md': ('parameters.visualization.customizationcolors.name',
                                  'parameters.visualization.objectcoloring.option1',
                                  'parameters.visualization.fluidillumination.name', 'menu.renderui'),
            'files.md': ('menu.uploadsimulation', 'menu.uploadgenome', 'localui.autosave.files', 'localui.autosave.directory'),
            'troubleshooting.md': ('display.title', 'display.fps', 'display.scale', 'display.autoscale', 'simulationui.syncwithrendering'),
            'ai-agents.md': ('mcp.start', 'mcp.running', 'mcp.settings', 'mcp.copy')
        }.items():
            text = (DOCS / 'ru' / filename).read_text(encoding='utf8')
            for key in keys:
                self.assertIn(catalog[key], text, (filename, key))
        art = (DOCS / 'ru/generative-art.md').read_text(encoding='utf8')
        for shape in ('segment', 'triangle', 'rectangle', 'hexagon', 'tube', 'largelolli', 'smalllolli', 'zigzag'):
            self.assertIn(catalog['genome.shape.' + shape], art)

    def test_inventory_all_formats_paths_and_copied_bytes(self):
        inventory = json.loads((ROOT / 'docs/localization-l7d-images.json').read_text(encoding='utf8'))
        used = {}
        for directory in (DOCS, DOCS / 'ru'):
            for path in directory.glob('*.md'):
                for caption, relative in re.findall(IMAGE, path.read_text(encoding='utf8')):
                    resource = (path.parent / relative).resolve()
                    self.assertTrue(resource.is_file(), (path.name, relative))
                    self.assertRegex(caption, '[А-Яа-яЁё]' if directory.name == 'ru' else '[A-Za-z]')
                    used.setdefault(resource.relative_to(ROOT).as_posix(), set()).add(path.relative_to(ROOT).as_posix())
        self.assertEqual(29, len(used))
        self.assertEqual(set(used), {row['path'] for row in inventory})
        for row in inventory:
            path = ROOT / row['path']
            self.assertEqual(sorted(used[row['path']]), row['chapters'])
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), row['sha256'])
            self.assertGreater(row['width'], 0)
            self.assertGreater(row['height'], 0)
            self.assertIn(row['format'], ('PNG', 'JPEG'))
        for output in ('build-agent/resources', 'build-agent/Release/resources'):
            for source in list(DOCS.rglob('*.md')) + [ROOT / path for path in used]:
                target = ROOT / output / source.relative_to(ROOT / 'resources')
                self.assertEqual(hashlib.sha256(source.read_bytes()).digest(),
                                 hashlib.sha256(target.read_bytes()).digest(), str(target))

    def test_preservation(self):
        paths = git('ls-tree', '-r', '--name-only', BASE, 'resources/docs', 'resources/images/logo.png',
                    'resources/help', 'resources/locales', 'source/Data', 'source/Engine', 'source/Apps',
                    'source/Base', 'source/McpTools', 'source/Network', 'source/Persister', 'external').decode().splitlines()
        for path in paths:
            if path == 'external/vcpkg':
                continue
            if path.startswith('resources/docs/ru/'):
                en = subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')
                ru = (ROOT / path).read_text(encoding='utf8')
                self.assertEqual(re.sub(IMAGE, r'![](\2)', en), re.sub(IMAGE, r'![](\2)', ru), path)
            else:
                self.assertEqual(git('rev-parse', f'{BASE}:{path}'), git('hash-object', '--path=' + path, path), path)
        self.assertEqual(BASE.encode(), git('rev-parse', 'main'))
        self.assertEqual(b'S AGENTS.md', git('ls-files', '-v', 'AGENTS.md'))
        self.assertEqual('090c381a6f2b1f62597108de236e53131559ffef796146011a5796dbb7ef3368',
                         hashlib.sha256((ROOT / 'AGENTS.md').read_bytes()).hexdigest())
        self.assertEqual(b'', git('diff', BASE, '--', 'external/vcpkg'))


if __name__ == '__main__':
    unittest.main()
