"""Source contracts complement the headless ImGui and catalog tests.

Run from any directory with Python 3: python source/Gui/Impl/Tests/check_localization_l2a.py
"""

import hashlib
import json
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[4]
GUI = ROOT / "source/Gui/Impl"


def read(path):
    return path.read_text(encoding="utf-8", errors="strict")


def menu_body(source):
    return source.split("void MainLoopController::processMenubar()", 1)[1].split(
        "void MainLoopController::pushGlobalStyle()", 1
    )[0]


def behavior_digest(body):
    body = re.sub(r'\.name\((?:tr\("[a-z.]+"\) \+ )?"[^"\n]*"\)', '.name(CAPTION)', body)
    body = re.sub(r'AlienGui::BeginMenu\([^\n]+?, (_[a-zA-Z]+MenuOpened)', r'AlienGui::BeginMenu(CAPTION, \1', body)
    body = re.sub(r'printOverlayMessage\((?:tr\()?"[^"\n]*"\)?\)', 'printOverlayMessage(CAPTION)', body)
    return hashlib.sha256(re.sub(r'\s+', '', body).encode()).hexdigest()


class LocalizationL2aContracts(unittest.TestCase):
    def test_menu_actions_shortcuts_and_conditions_match_accepted_l1(self):
        # Digest of e7f1eaa0e processMenubar, with only presentation strings removed.
        self.assertEqual("b4587ae877e219cd27aece7d15e3ca1aee893da00e3f217e34dd2a8cea6e4292", behavior_digest(menu_body(read(GUI / "MainLoopController.cpp"))))

    def test_all_eight_menus_and_items_have_language_independent_ids(self):
        body = menu_body(read(GUI / "MainLoopController.cpp"))
        menus = re.findall(r'AlienGui::BeginMenu\(([^\n]+)', body)
        self.assertEqual(8, len(menus))
        for menu in menus:
            self.assertRegex(menu, r'tr\("[a-z.]+"\) \+ " ###[a-z.]+"')
        items = re.findall(r'\.name\(([^\n]+?)\)', body)
        self.assertEqual(43, len(items))
        for item in items:
            self.assertTrue(item.startswith('tr("'), item)
        ids = re.findall(r'\.name\(tr\("[a-z.]+"\) \+ "###([a-z.]+)"\)', body)
        self.assertEqual(len(items), len(ids))
        self.assertEqual(len(ids), len(set(ids)))

    def test_dialog_captions_and_tooltip_are_translated(self):
        for file in ("NewSimulationDialog.cpp", "ExitDialog.cpp", "SavePictureDialog.cpp"):
            source = read(GUI / file)
            self.assertIn('setTitleKey("', source)
            if file == "ExitDialog.cpp":
                self.assertNotIn('LocalizationGui::languageSelector()', source)
            else:
                self.assertIn('LocalizationGui::languageSelector()', source)
            self.assertNotRegex(source, r'AlienGui::Button\("[A-Za-z]')
            self.assertNotRegex(source, r'ImGui::TextWrapped\("%s", "[A-Za-z]')
            for name in re.findall(r'\.name\("([^"\n]+)"\)', source):
                self.assertRegex(source, re.escape(f'.name("{name}")') + r'\s*\.label\(tr\("')
        self.assertIn('.generateValueTooltip(tr("new.generate"))', read(GUI / "NewSimulationDialog.cpp"))

    def test_filters_keep_extensions_and_grammar(self):
        source = read(GUI / "FileTransferController.cpp")
        for key in ("file.open", "file.save"):
            self.assertIn(f'tr("{key}"), tr("file.simulation") + " (*.sim){{.sim}},.*"', source)
        self.assertIn('tr("picture.title"), tr("file.picture") + " (*.png){.png},.*"', read(GUI / "SavePictureDialog.cpp"))
        generic = read(GUI / "GenericFileDialog.cpp")
        self.assertIn('Open("GenericFileDialog", title, filter', generic)
        self.assertIn('Save("GenericFileDialog", title, filter', generic)

    def test_catalogs_have_no_duplicate_keys_and_all_references_exist(self):
        def unique(pairs):
            result = {}
            for key, value in pairs:
                self.assertNotIn(key, result)
                result[key] = value
            return result

        catalogs = [json.loads(read(ROOT / f"resources/locales/{lang}.json"), object_pairs_hook=unique) for lang in ("en", "ru")]
        self.assertEqual(set(catalogs[0]), set(catalogs[1]))
        self.assertEqual(138, len(catalogs[0]))
        for file in ("MainLoopController.cpp", "NewSimulationDialog.cpp", "ExitDialog.cpp", "SavePictureDialog.cpp", "FileTransferController.cpp"):
            for key in re.findall(r'(?:tr|setTitleKey)\("([a-z.]+)"\)', read(GUI / file)):
                self.assertIn(key, catalogs[0])


if __name__ == "__main__":
    unittest.main(verbosity=2)
