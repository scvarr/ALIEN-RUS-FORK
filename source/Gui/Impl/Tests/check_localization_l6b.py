"""L6-B contracts: presentation boundaries and unchanged resource operations."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
BASE = 'f97d6d0373fe5592740401fc1a8e724d703da7f4'
GUI = 'source/Gui/Impl/'
COMPONENTS = ['BrowserWindow', 'BrowserGalleryWidget', 'BrowserTableWidget',
              'BrowserUserListWidget', 'BrowserHelper', 'BrowserLoginHintWidget',
              'BrowserLoginBannerWidget', 'BrowserController', 'BrowserData',
              'EditSimulationDialog', 'UploadSimulationDialog', 'ReplaceSimulationDialog',
              'NetworkTransferController']


def read(path):
    return (ROOT / path).read_text(encoding='utf8')


def original(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


def function(text, name):
    start = re.search(r'^.*::' + re.escape(name) + r'\(', text, re.M).start()
    return text[start:start + re.search(r'^}\n', text[start:], re.M).end()]


class L6bContracts(unittest.TestCase):
    def test_catalog_preservation_parity_and_arguments(self):
        def unique(pairs):
            self.assertEqual(len(pairs), len(dict(pairs)))
            return dict(pairs)
        catalogs = {lang: json.loads(read(f'resources/locales/{lang}.json'), object_pairs_hook=unique) for lang in ['en', 'ru']}
        self.assertEqual(catalogs['en'].keys(), catalogs['ru'].keys())
        for lang, values in catalogs.items():
            before = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(before, {key: values[key] for key in before})
            for key in values.keys() - before.keys():
                self.assertTrue(key.startswith('browser.'))
                self.assertTrue(values[key])
                self.assertNotIn('##', values[key])
                self.assertEqual(sorted(re.findall(r'\{\w+}', catalogs['en'][key])), sorted(re.findall(r'\{\w+}', catalogs['ru'][key])))
        adapter = read(GUI + 'BrowserPresentation.h')
        for key in re.findall(r'"(browser\.[^"]+)"', adapter):
            self.assertIn(key, catalogs['en'])

    def test_settings_payloads_folder_ids_and_sort_ids_are_unchanged(self):
        for name in COMPONENTS:
            path = GUI + name + '.cpp'
            before, after = original(path), read(path)
            for pattern in [r'"(?:windows\.|dialogs\.|settings\.)[^"\n]+"',
                            r'\.sortId = \w+', r'\.resourceId\s*=\s*[^,;}]+',
                            r'\.(?:folderName|resourceWithoutFolderName|resourceDescription|workspaceType|downloadCache|data)\s*=\s*[^,;}]+',
                            r'(?:schedule|fetch)\w+\(', r'\.entries\.emplace_back\([^;]+;',
                            r'\.values\(([^)]+)\)',
                            r'ImGui::(?:BeginTable|BeginChild|PushID|BeginPopup|OpenPopup)\(\s*"([^"]+)"']:
                self.assertEqual(re.findall(pattern, before), re.findall(pattern, after), (name, pattern))

    def test_access_filter_and_reaction_algorithms_are_unchanged(self):
        methods = {'BrowserData': ['createTreeTOs', 'isOwner', 'isSelected', 'onDownloadResource', 'sortUserList'],
                   'BrowserWindow': ['isLoginRequired', 'onMoveResource', 'onExpandFolders', 'onCollapseFolders'],
                   'BrowserController': ['isVisibleInWorkspace'],
                   'BrowserGalleryWidget': ['getSortedEntries', 'requestMissingPictures', 'resetPage', 'onSelectEntry'],
                   'BrowserTableWidget': ['processSortSpecs', 'processFolderTreeSymbols']}
        for name, names in methods.items():
            before, after = original(GUI + name + '.cpp'), read(GUI + name + '.cpp')
            for method in names:
                if method == 'isVisibleInWorkspace':
                    pattern = r'    bool isVisibleInWorkspace\(.*?\n    }'
                    self.assertEqual(re.search(pattern, before, re.S)[0], re.search(pattern, after, re.S)[0])
                else:
                    self.assertEqual(function(before, method), function(after, method), (name, method))
        before, after = original(GUI + 'BrowserData.cpp'), read(GUI + 'BrowserData.cpp')
        after = after.replace('LocalDialogPresentation::error("browser.error.reaction", errors);', 'GenericMessageDialog::get().information("Error", errors);')
        self.assertEqual(function(before, 'onToggleLike'), function(after, 'onToggleLike'))

    def test_display_ids_and_dynamic_language_resolution(self):
        window = read(GUI + 'BrowserWindow.cpp')
        self.assertIn('"###workspace"', window)
        self.assertIn('overflowTooltip(tr("genome.toolbar.more"))', window)
        self.assertIn('.id("Simulations")', window)
        self.assertIn('.id("Genomes")', window)
        self.assertIn('.id("Filter (case insensitive)")', window)
        self.assertIn('localizedYesNo("Delete", "browser.delete", messageKey', window)
        self.assertIn('BrowserPresentation::column(column.name)', read(GUI + 'BrowserTableWidget.cpp'))
        upload = read(GUI + 'UploadSimulationDialog.cpp')
        self.assertIn('"genome.upload.title" : "browser.upload.title"', upload)
        self.assertIn('.hintId(BrowserDataTypeToUpperString.at(_resourceType) + " name")', upload)
        for name in ['BrowserLoginHintWidget', 'BrowserLoginBannerWidget']:
            text = read(GUI + name + '.cpp')
            self.assertNotRegex(text, r'auto const (Headline|SubText|ButtonText) =')
            self.assertIn('"###loginInvitation"', text)

    def test_callback_errors_keep_raw_details_and_requests(self):
        controller = read(GUI + 'NetworkTransferController.cpp')
        for name in ['download', 'upload', 'replace', 'delete', 'edit', 'move']:
            self.assertIn(f'LocalDialogPresentation::error("browser.error.{name}", errors)', controller)
        self.assertIn('printOverlayMessage(data.resourceName)', controller)
        self.assertIn('LocalDialogPresentation::simulationLoadError(*errorMessage)', controller)
        self.assertIn('localizedMarkdownInformation(', controller)
        message = read(GUI + 'GenericMessageDialog.cpp')
        self.assertIn('MarkdownParser::parse(LocalizationGuiText::format(_messageKey, _messageArguments))', message)
        self.assertEqual(original(GUI + 'LocalDialogPresentation.h'), read(GUI + 'LocalDialogPresentation.h'))

    def test_later_stages_protocols_and_user_data_remain_outside_scope(self):
        changed = subprocess.check_output(['git', 'diff', '--name-only', BASE], cwd=ROOT).decode().splitlines()
        for path in changed:
            self.assertFalse(path.startswith(('source/Network/', 'source/Persister/', 'source/Data/', 'source/Engine/',
                                             'source/McpTools/', 'source/Apps/', 'external/')), path)
            self.assertNotIn('AGENTS.md', path)
        for name in ['LoginController', 'LoginDialog', 'McpWindow', 'McpSettingsDialog', 'PictureGuiService', 'NewSimulationService']:
            self.assertEqual(original(GUI + name + '.cpp'), read(GUI + name + '.cpp'))
        for path in ['docs/localization-roadmap.md', 'docs/localization-terminology.md', 'docs/localization-l3-audit.md']:
            self.assertEqual(original(path), read(path))


if __name__ == '__main__':
    unittest.main()
