"""L6-C account presentation and browser-only UTF-8 search contracts."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
BASE = 'cfbde45fd0e086962e377b4c3e490cafcaeabfb6'
GUI = 'source/Gui/Impl/'
COMPONENTS = ['LoginDialog', 'CreateUserDialog', 'ActivateUserDialog',
              'ResetPasswordDialog', 'NewPasswordDialog', 'DeleteUserDialog', 'LoginController']


def read(path):
    return (ROOT / path).read_text(encoding='utf8')


def original(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


class L6cContracts(unittest.TestCase):
    def test_catalogs_preserve_existing_values_and_argument_signatures(self):
        def unique(pairs):
            self.assertEqual(len(pairs), len(dict(pairs)))
            return dict(pairs)
        catalogs = {lang: json.loads(read(f'resources/locales/{lang}.json'), object_pairs_hook=unique) for lang in ['en', 'ru']}
        self.assertEqual(catalogs['en'].keys(), catalogs['ru'].keys())
        for lang, catalog in catalogs.items():
            before = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(before, {key: catalog[key] for key in before})
            for key in catalog.keys() - before.keys():
                self.assertTrue(key.startswith('account.'))
                self.assertTrue(catalog[key])
                self.assertNotIn('##', catalog[key])
                self.assertEqual(re.findall(r'\{\w+}', catalogs['en'][key]), re.findall(r'\{\w+}', catalogs['ru'][key]))
        adapter = read(GUI + 'AccountPresentation.h')
        for key in re.findall(r'"(account\.[^"]+)"', adapter):
            self.assertIn(key, catalogs['en'])
        self.assertIn('account.information', catalogs['en'])

    def test_request_arguments_branches_and_settings_are_preserved(self):
        for name in COMPONENTS:
            path = GUI + name + '.cpp'
            before, after = original(path), read(path).replace('AccountPresentation::button(', 'AlienGui::Button(')
            for pattern in [r'NetworkService::get\(\)\.\w+\([^;\n]+',
                            r'if \(([^\n]+)\)', r'ImGui::BeginDisabled\(([^\n]+)\)',
                            r'"(?:dialogs\.|controller\.)[^"\n]+"',
                            r'(?:schedule|fetch)\w+\(', r'LoginRequestData\{[^}]+}',
                            r'\.(?:clear|empty)\(\)', r'result \|= [^;]+;']:
                # Only display-time user-name interpolation moved to a named argument.
                if 'NetworkService' in pattern and name == 'DeleteUserDialog':
                    continue
                self.assertEqual(re.findall(pattern, before), re.findall(pattern, after), (name, pattern))
        for name in COMPONENTS[:6]:
            text = read(GUI + name + '.cpp')
            self.assertIn('setTitleKey("account.', text)
            self.assertNotIn('AlienGui::Button(', text)
            self.assertNotRegex(text, r'AlienGui::(?:Text|HelpMarker)\(\s*"')
            for hint in re.findall(r'\.hintId\("([^"]+)"\)', text):
                self.assertIn(f'.hint("{hint}")', original(GUI + name + '.cpp'))
        adapter = read(GUI + 'AccountPresentation.h')
        self.assertIn('AlienGui::DisplayButton(canonical, label)', adapter)
        self.assertIn('localizedInformation(', adapter)
        self.assertIn('LocalDialogPresentation::error("account.loginerror", criticalErrors)', read(GUI + 'LoginController.cpp'))

    def test_security_help_is_exact_original_english(self):
        en = json.loads(read('resources/locales/en.json'))
        help_text = original(GUI + 'HelpStrings.h')
        for name in ['LoginHowToCreateNewUseTooltip', 'LoginForgotYourPasswordTooltip', 'LoginSecurityInformationTooltip', 'LoginRememberTooltip']:
            body = re.search(r'const ' + name + r'\s*=([^;]+);', help_text)[1]
            canonical = ''.join(json.loads(s) for s in re.findall(r'"(?:\\.|[^"\\])*"', body))
            self.assertEqual(canonical, en['account.help.' + name.lower()])
        self.assertEqual(original(GUI + 'HelpStrings.h'), read(GUI + 'HelpStrings.h'))

    def test_search_changes_only_browser_matching_and_keeps_fields(self):
        adapter = read(GUI + 'BrowserMetadataSearch.h')
        old = original('source/Network/NetworkResourceRawTO.cpp')
        old_fields = re.findall(r'containsCaseInsensitive\((.*?), filter\)', old)
        new_fields = re.findall(r'contains\((resource\.\w+|std::to_string\(resource\.\w+\))\)', adapter)
        self.assertEqual(old_fields, [field.replace('resource.', '') for field in new_fields])
        self.assertIn('Utf8Search::find(field, filter).has_value()', adapter)
        for name, old_call, new_call in [
                ('BrowserData', 'rawTO->matchWithFilter(filter)', 'BrowserMetadataSearch::matches(*rawTO, filter)'),
                ('BrowserGalleryWidget', 'rawTO->matchWithFilter(_data->filter)', 'BrowserMetadataSearch::matches(*rawTO, _data->filter)')]:
            path = GUI + name + '.cpp'
            actual = read(path).replace('#include "BrowserMetadataSearch.h"\n\n', '').replace(new_call, old_call)
            self.assertEqual(original(path), actual)
        for path in ['source/Base/StringHelper.cpp', 'source/Base/StringHelper.h', GUI + 'Utf8Search.h',
                     'source/Network/NetworkResourceRawTO.cpp', 'source/Network/NetworkResourceRawTO.h',
                     'source/Network/NetworkService.cpp', 'source/Network/NetworkService.h',
                     'source/McpTools/Impl/McpNetworkTools.cpp', GUI + 'NetworkTransferController.cpp']:
            self.assertEqual(original(path), read(path))

    def test_test_executable_has_link_time_network_isolation(self):
        stub = read(GUI + 'Tests/AccountNetworkStub.cpp')
        header = read('source/Network/NetworkService.h').split('public:')[1].split('private:')[0]
        for name in re.findall(r'\b(\w+)\([^;]+\);', header, re.S):
            self.assertIn('NetworkService::' + name + '(', stub)
        self.assertNotIn('httplib', stub)
        self.assertNotIn('client.Post', stub)
        self.assertIn('add_executable(AccountLocalizationTests AccountLocalizationTests.cpp AccountNetworkStub.cpp)', read(GUI + 'Tests/CMakeLists.txt'))


if __name__ == '__main__':
    unittest.main()
