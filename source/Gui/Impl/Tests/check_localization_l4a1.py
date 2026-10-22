"""Genome shell presentation contracts; real ImGui interactions live in LocalizationGuiTests."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
GUI = ROOT / "source/Gui/Impl"
BASE = "729ea59746f35c53ca9f48e7181d56e4a447933d"


def read(path):
    return path.read_text(encoding="utf8")


def original(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"], cwd=ROOT).decode("utf8").replace("\r\n", "\n")


def catalog(language):
    def unique(pairs):
        result = dict(pairs)
        if len(result) != len(pairs):
            raise ValueError("Duplicate catalog key")
        return result
    return json.loads(read(ROOT / f"resources/locales/{language}.json"), object_pairs_hook=unique)


def function(text, name):
    start = re.search(r"^.*::" + re.escape(name) + r"\(", text, re.M).start()
    end = re.search(r"^}\n", text[start:], re.M).end() + start
    return text[start:end]


class L4a1Contracts(unittest.TestCase):
    def test_catalog_parity_existing_entries_and_safe_semantic_keys(self):
        en, ru = catalog("en"), catalog("ru")
        self.assertEqual(set(en), set(ru))
        for language, values in [("en", en), ("ru", ru)]:
            old = json.loads(original(f"resources/locales/{language}.json"))
            self.assertEqual(old, {key: values[key] for key in old})
            self.assertEqual(134, len(values) - len(old))
            for key, text in values.items():
                self.assertRegex(key, r"^[a-z][a-z0-9]*(\.[a-z][a-z0-9]*)+$")
                self.assertTrue(text)
                self.assertNotIn("##", text)
                self.assertEqual(set(re.findall(r"\{[a-z]+}", en[key])), set(re.findall(r"\{[a-z]+}", ru[key])))
        self.assertEqual("Большой леденец", ru["genome.shape.largelolli"])
        self.assertEqual("Донор", ru["genome.cell.void"])
        self.assertEqual("Конвертер", ru["genome.cell.digestor"])

    def test_every_new_key_has_a_display_consumer_or_typed_count_family(self):
        source = "\n".join(read(GUI / name) for name in (
            "GenomeEditorWindow.cpp", "GenomeEditorWidget.cpp", "GeneEditorWidget.cpp", "GenomeTabWidget.cpp",
            "GenomeEditorPresentation.cpp", "FileTransferController.cpp", "ChangeColorDialog.cpp", "UploadSimulationDialog.cpp"))
        en = catalog("en")
        for key in en:
            if key.startswith("genome."):
                if re.search(r"\.(one|few|many)$", key) and ".used." not in key:
                    self.assertIn(key.rsplit(".", 1)[0], source)
                else:
                    self.assertIn('"' + key + '"', source)

    def test_all_canonical_property_names_and_parent_scopes_survive(self):
        for name in ("GenomeEditorWidget.cpp", "GeneEditorWidget.cpp"):
            old, new = original("source/Gui/Impl/" + name), read(GUI / name)
            self.assertEqual(re.findall(r'\.name\("([^"]*)"\)', old), re.findall(r'\.name\("([^"]*)"\)', new))
            for pattern in (r'ImGui::BeginChild\("([^"]*)"', r'ImGui::BeginTable\("([^"]*)"', r'ImGui::PushID\(([^;]+)\);'):
                self.assertEqual(re.findall(pattern, old), re.findall(pattern, new))
        tree = read(GUI / "GenomeEditorWidget.cpp")
        self.assertIn('TableRowTreeNode("##gene",', tree)
        self.assertIn('TableRowTreeNode("##node",', tree)
        tabs = read(GUI / "GenomeTabWidget.cpp")
        for name in ("Phenotype", "Gene network"):
            self.assertIn('.id("' + name + '")', tabs)
        self.assertIn('.id("##GenomeView")', tabs)
        upload = read(GUI / "UploadSimulationDialog.cpp")
        self.assertIn('.hintId(BrowserDataTypeToUpperString.at(_resourceType) + " name")', upload)
        self.assertIn('.name("Share with the community")', upload)

    def test_genome_operations_generated_names_and_serialization_calls_are_unchanged(self):
        for name, functions in {
            "GenomeEditorWidget.cpp": ["onAddGene", "onAddNode", "onRemoveNode", "removeGeneIntern", "moveGeneUpwardIntern", "moveGeneDownwardIntern", "fixAllIssuesIntern"],
            "GenomeEditorWindow.cpp": ["openTab", "onCloneGenome", "onCopyGenome", "onPasteGenome", "onCloseOtherTabs", "onSavepointGenome", "getDefaultGenome"],
            "GenomeTabWidget.cpp": ["normalizeForEditor", "revertChanges", "resetOriginal", "getName", "setGenomeDesc"]
        }.items():
            before, after = original("source/Gui/Impl/" + name), read(GUI / name)
            for method in functions:
                self.assertEqual(function(before, method), function(after, method), method)
        name = "source/Gui/Impl/FileTransferController.cpp"
        before, after = original(name), read(ROOT / name)
        for pattern in (r'SerializerService::get\(\)\.[^\n]+', r'\(\*\.genome\)\{\.genome},\.\*'):
            self.assertEqual(re.findall(pattern, before), re.findall(pattern, after))

    def test_attribute_adapter_matches_nine_canonical_descriptions_and_keeps_fallback(self):
        adapter = read(GUI / "GenomeEditorPresentation.cpp")
        help_source = read(GUI / "EntityAttributeHelp.cpp")
        en = catalog("en")
        attrs = re.findall(r'case EntityAttribute::(\w+):\s*return tr\("(genome.attribute.[^"]+)"\)', adapter)
        self.assertEqual(9, len(attrs))
        for attr, key in attrs:
            block = re.search(r'\{EntityAttribute::' + attr + r',\s*((?:"(?:[^"\\]|\\.)*"\s*)+)\}', help_source)[1]
            self.assertEqual(en[key], "".join(json.loads(s) for s in re.findall(r'"(?:[^"\\]|\\.)*"', block)))
        self.assertIn("return EntityAttributeHelp::get(attribute);", adapter)
        self.assertEqual(original("source/Gui/Impl/EntityAttributeHelp.cpp"), help_source)
        self.assertIn("static_assert(keys.size() == ConstructorShape_Count)", adapter)
        self.assertIn("static_assert(keys.size() == CellType_Count)", adapter)

    def test_later_editors_and_data_contracts_remain_byte_identical(self):
        for name in ("NeuralNetEditorWidget.cpp", "NodeEditorWidget.cpp", "MutationRatesWidget.cpp", "MutationRatesDialog.cpp",
                     "GenomeIssueDescription.cpp", "GeneGraphWidget.cpp"):
            self.assertEqual(original("source/Gui/Impl/" + name), read(GUI / name))
        for path in ("source/Data/GenomeDesc.h", "source/Data/CellTypeConstants.h",
                     "source/Data/ShapeGenerator.h", "source/Persister/Interface/SerializerService.cpp"):
            self.assertEqual(original(path), read(ROOT / path))


if __name__ == "__main__":
    unittest.main()
