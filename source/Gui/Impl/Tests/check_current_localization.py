"""Mandatory current-version invariants; historical snapshot decisions are explicit in the manifest."""
import importlib
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[4]
manifest = json.loads((ROOT / "docs/upstream-sync-2026-11-10-test-contracts.json").read_text(encoding="utf8"))
suite = unittest.TestSuite()
for entry in manifest:
    if entry["required"]:
        module = importlib.import_module(entry["module"])
        suite.addTest(getattr(module, entry["class"])(entry["method"]))
suite.addTests(unittest.defaultTestLoader.loadTestsFromModule(importlib.import_module("check_upstream_sync")))
if __name__ == "__main__":
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    raise SystemExit(not result.wasSuccessful())
