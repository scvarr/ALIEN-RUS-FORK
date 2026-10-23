"""L6-A contracts: callback producers, persistent identity and canonical diagnostics."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
BASE = '1e6438ff208166045575d1d08af102e8807de939'
COMPONENTS = ['AboutDialog', 'NetworkSettingsDialog', 'AutosaveWindow', 'AutosaveController',
              'LogWindow', 'FileTransferController', 'MainLoopController']


def read(path):
    return (ROOT / path).read_text(encoding='utf8')


def original(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


class L6aContracts(unittest.TestCase):
    def test_catalog_additions_preserve_every_previous_entry(self):
        def unique(pairs):
            self.assertEqual(len(pairs), len(dict(pairs)))
            return dict(pairs)
        catalogs = {lang: json.loads(read(f'resources/locales/{lang}.json'), object_pairs_hook=unique) for lang in ['en', 'ru']}
        self.assertEqual(catalogs['en'].keys(), catalogs['ru'].keys())
        consumers = '\n'.join(read('source/Gui/Impl/' + name + '.cpp') for name in COMPONENTS)
        consumers += read('source/Gui/Impl/HelpStrings.h') + read('source/Gui/Impl/LocalDialogPresentation.h')
        for lang, values in catalogs.items():
            before = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(before, {key: values[key] for key in before})
            added = values.keys() - before.keys()
            self.assertEqual(43, len(added))
            for key in added:
                self.assertTrue(key.startswith('localui.'))
                self.assertIn('"' + key + '"', consumers)
                self.assertNotIn('##', values[key])
                self.assertEqual(re.findall(r'\{\w+}', catalogs['en'][key]), re.findall(r'\{\w+}', catalogs['ru'][key]))

    def test_paths_payloads_settings_and_request_identity_are_unchanged(self):
        for component in COMPONENTS:
            path = f'source/Gui/Impl/{component}.cpp'
            before, after = original(path), read(path)
            for pattern in [r'"(?:windows\.|dialogs\.|settings\.)[^"\n]+"',
                            r'\.values\(([^)]+)\)', r'\.id\("([^"]+)"\)',
                            r'ImGui::(?:BeginTable|BeginChild|PushID)\(\s*"([^"]+)"',
                            r'auto constexpr \w*(?:SenderId|Interval) = [^;]+;',
                            r'\.(?:filename|senderId|generateNameFromTimestep|resetDeserializedSimulation|wishResultData|wishErrorInfo)\s*=\s*[^,;}]+',
                            r'newEntry->\w+ = [^;]+;']:
                self.assertEqual(re.findall(pattern, before), re.findall(pattern, re.sub(r'setTitleKey\([^;]+;', '', after)), (component, pattern))
        controller = read('source/Gui/Impl/AutosaveController.cpp')
        before = original('source/Gui/Impl/AutosaveController.cpp')
        for start in ['void AutosaveController::updateSavepoint(', 'void AutosaveController::updateSavepointTableFromFile(']:
            self.assertEqual(before[before.index(start):], controller[controller.index(start):])

    def test_shared_picture_and_simulation_diagnostics_keep_machine_consumers(self):
        # These errors also reach MCP. The GUI explains the operation separately.
        for name in ['PictureGuiService', 'NewSimulationService', 'McpController', 'NetworkTransferController', 'GuiLogger']:
            path = f'source/Gui/Impl/{name}.cpp'
            self.assertEqual(original(path), read(path))
        file = read('source/Gui/Impl/FileTransferController.cpp')
        self.assertIn('LocalDialogPresentation::simulationLoadError(*errorMessage)', file)
        self.assertIn('LocalDialogPresentation::error("localui.error.open", criticalErrors)', file)
        self.assertIn('LocalDialogPresentation::error("localui.error.save", criticalErrors)', file)
        self.assertIn('printOverlayMessage(PathHelper::toUtf8(data.filename))', file)
        self.assertEqual(original('source/Gui/Impl/SavePictureDialog.cpp'), read('source/Gui/Impl/SavePictureDialog.cpp'))

    def test_security_help_and_display_time_message_boundary(self):
        en = json.loads(read('resources/locales/en.json'))
        original_help = original('source/Gui/Impl/HelpStrings.h')
        body = re.search(r'std::string const NetworkCountAsActiveTooltip =\s*(.*?);', original_help, re.S)[1]
        self.assertEqual(''.join(json.loads(s) for s in re.findall(r'"(?:\\.|[^"\\])*"', body)), en['localui.network.activehelp'])
        self.assertIn('.tooltip(tr(Const::NetworkCountAsActiveTooltip))', read('source/Gui/Impl/NetworkSettingsDialog.cpp'))
        message = read('source/Gui/Impl/LocalDialogPresentation.h')
        self.assertIn('localizedInformation("Error", "localui.error", messageKey, {{"details", details}})', message)
        self.assertNotIn('tr(details)', message)
        self.assertEqual(original('source/Gui/Impl/GenericMessageDialog.cpp'), read('source/Gui/Impl/GenericMessageDialog.cpp'))

    def test_folder_chooser_adapter_is_opt_in(self):
        from localization_node_legacy import legacy_local_dialog_text
        for name in ['AlienGui.cpp', 'AlienGui.h']:
            path = 'source/Gui/Impl/' + name
            self.assertEqual(original(path), legacy_local_dialog_text(path, read(path)))
        self.assertIn('.folderDialogTitle(tr("localui.autosave.selectdirectory"))', read('source/Gui/Impl/AutosaveWindow.cpp'))

    def test_scope_excludes_later_stages_and_dependencies(self):
        changed = subprocess.check_output(['git', 'diff', '--name-only', BASE], cwd=ROOT).decode().splitlines()
        allowed = {'resources/locales/en.json', 'resources/locales/ru.json',
                   'docs/localization-l6a-validation.md', 'source/Base/Tests/LocalizationServiceTests.cpp',
                   'source/Gui/Impl/AlienGui.cpp', 'source/Gui/Impl/AlienGui.h',
                   'source/Gui/Impl/GenericMessageDialog.h', 'source/Gui/Impl/HelpStrings.h',
                   'source/Gui/Impl/LocalDialogPresentation.h',
                   'source/Gui/Impl/Tests/LocalizationGuiTests.cpp', 'source/Gui/Impl/Tests/localization_node_legacy.py'}
        allowed.update(f'source/Gui/Impl/{name}.{ext}' for name in COMPONENTS for ext in ['cpp', 'h'])
        allowed.update(str(path.relative_to(ROOT)).replace('\\', '/') for path in (ROOT / 'source/Gui/Impl/Tests').glob('check_localization_*.py'))
        self.assertLessEqual(set(changed), allowed)
        for path in changed:
            self.assertFalse(path.startswith(('source/Engine/', 'source/Data/', 'source/Persister/', 'source/Network/', 'source/McpTools/', 'source/Apps/', 'external/')), path)
            self.assertNotIn('AGENTS.md', path)
        for path in ['docs/localization-terminology.md', 'docs/localization-roadmap.md', 'docs/localization-l3-audit.md']:
            self.assertEqual(original(path), read(path))


if __name__ == '__main__':
    unittest.main()
