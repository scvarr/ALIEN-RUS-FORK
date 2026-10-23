"""L6-D presentation boundary and exact MCP baseline contracts."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
BASE = '5f83ed9a4c6024f3cd325af8f930649ae8f3a7ea'
GUI = 'source/Gui/Impl/'


def original(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT)


def read(path):
    return (ROOT / path).read_text(encoding='utf8')


class L6dContracts(unittest.TestCase):
    def test_machine_implementation_and_contracts_match_base_bytes(self):
        paths = subprocess.check_output(
            ['git', 'ls-tree', '-r', '--name-only', BASE, 'source/Network', 'source/McpTools', 'resources/help'], cwd=ROOT
        ).decode().splitlines()
        self.assertGreater(len(paths), 50)
        for path in paths:
            # Compare Git blobs: Windows checkout line-ending conversion is immaterial.
            current = subprocess.check_output(['git', 'hash-object', '--path=' + path, path], cwd=ROOT).strip()
            baseline = subprocess.check_output(['git', 'rev-parse', f'{BASE}:{path}'], cwd=ROOT).strip()
            self.assertEqual(baseline, current, path)

    def test_entire_controller_changes_only_three_gui_messages(self):
        path = GUI + 'McpController.cpp'
        actual = read(path).replace('#include "LocalizationGuiText.h"\n', '')
        substitutions = {
            'printOverlayMessage(LocalizationGuiText::format("mcp.started", {{"url", getServerUrl()}}));':
                'printOverlayMessage("MCP server running at " + getServerUrl());',
            'printOverlayMessage(tr("mcp.serverstopped"));': 'printOverlayMessage("MCP server stopped");',
            'GenericMessageDialog::get().localizedInformation("MCP server", "mcp.title", "mcp.startfailed", {{"port", std::to_string(_port)}});':
                'GenericMessageDialog::get().information("MCP server", std::format("The MCP server could not be started because port {} is not available.", _port));',
        }
        for localized, canonical in substitutions.items():
            self.assertEqual(1, actual.count(localized))
            actual = actual.replace(localized, canonical)
        self.assertEqual(original(path).decode('utf8').replace('\r\n', '\n'), actual)

    def test_catalogs_are_additive_and_preserve_placeholders(self):
        def unique(pairs):
            self.assertEqual(len(pairs), len(dict(pairs)))
            return dict(pairs)
        catalogs = {lang: json.loads(read(f'resources/locales/{lang}.json'), object_pairs_hook=unique) for lang in ['en', 'ru']}
        self.assertEqual(catalogs['en'].keys(), catalogs['ru'].keys())
        for lang, catalog in catalogs.items():
            before = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(before, {key: catalog[key] for key in before})
            added = catalog.keys() - before.keys()
            self.assertEqual(30, len(added))
            for key in added:
                self.assertTrue(key.startswith('mcp.'))
                self.assertTrue(catalog[key])
                self.assertNotIn('##', catalog[key])
                self.assertEqual(re.findall(r'\{\w+}', catalogs['en'][key]), re.findall(r'\{\w+}', catalogs['ru'][key]))
        for file in ['McpWindow.cpp', 'McpSettingsDialog.cpp', 'McpController.cpp']:
            for key in re.findall(r'"(mcp\.[^"]+)"', read(GUI + file)):
                self.assertIn(key, catalogs['en'])
        for key in ['step1', 'help']:
            for lang in catalogs:
                self.assertIn('HTTP', catalogs[lang]['mcp.' + key])
        for lang in catalogs:
            self.assertIn('Streamable HTTP', catalogs[lang]['mcp.help'])

    def test_payload_display_and_configuration_remain_literal(self):
        window = read(GUI + 'McpWindow.cpp')
        before = original(GUI + 'McpWindow.cpp').decode('utf8')
        for expression in ['.text(entry.command)', '.text(entry.result)', 'StringHelper::format(entry.time)',
                           'boost::algorithm::join(toolGroup.toolNames, ", ")', 'toolGroup.name',
                           'ImGui::SetClipboardText(url.c_str())', 'ImGui::InputText("##url", url.data(), url.size() + 1, ImGuiInputTextFlags_ReadOnly)']:
            self.assertIn(expression, window)
            self.assertIn(expression, before)
        for expression in ['"##content"', '"##mcpCommands"', '.id("McpServer")', '"windows.mcp server"',
                           '"MCP server"', 'auto constexpr CopyButtonText = ICON_FA_COPY "  Copy";']:
            self.assertIn(expression, window)
            self.assertIn(expression, before)
        self.assertIn('DisplayButton(CopyButtonText, copyLabel)', window)
        settings = read(GUI + 'McpSettingsDialog.cpp')
        for expression in ['.name("Port")', 'DisplayButton("Adopt",', 'DisplayButton("Cancel",', 'controller.setPort(_port)', '_port = McpController::get().getPort()']:
            self.assertIn(expression, settings)
        # No GUI configuration block existed at this baseline; the URL remains the runnable example.
        self.assertNotIn('mcpServers', before)
        self.assertNotIn('mcpServers', window)

    def test_shared_headers_add_only_fixture_access(self):
        for name in ['AlienDialog', 'GenericMessageDialog', 'OverlayController', 'WindowController',
                     'McpController', 'McpWindow', 'McpSettingsDialog']:
            path = GUI + name + '.h'
            actual = read(path).replace('    friend class McpLocalizationTests;\n', '')
            if name == 'McpController':
                actual = actual.replace('{\n\n    MAKE_SINGLETON', '{\n    MAKE_SINGLETON')
            self.assertEqual(original(path).decode('utf8').replace('\r\n', '\n'), actual, name)

    def test_server_test_target_has_no_network_listener(self):
        stub = read(GUI + 'Tests/McpServerStub.cpp')
        for method in ['McpServer', '~McpServer', 'start', 'stop', 'isRunning', 'getPort', 'handleMessage']:
            self.assertIn('McpServer::' + method + '(', stub)
        self.assertNotIn('#include <httplib', stub)
        self.assertNotIn('listen(', stub)
        self.assertIn('add_executable(McpLocalizationTests McpLocalizationTests.cpp McpServerStub.cpp)', read(GUI + 'Tests/CMakeLists.txt'))

    def test_all_machine_groups_have_gui_only_captions(self):
        facade = original('source/McpTools/Impl/McpToolsFacadeImpl.cpp').decode('utf8')
        groups = re.findall(r'addGroup\(\s*"([^"]+)"', facade)
        adapter = read(GUI + 'McpPresentation.h')
        self.assertEqual(6, len(groups))
        for group in groups:
            self.assertIn('"' + group + '"', adapter)
        self.assertIn('McpPresentation::groupCaption(toolGroup.name)', read(GUI + 'McpWindow.cpp'))
        for key in re.findall(r'"(mcp\.group\.[^"]+)"', adapter):
            for lang in ['en', 'ru']:
                self.assertIn(key, json.loads(read(f'resources/locales/{lang}.json')))


if __name__ == '__main__':
    unittest.main()
