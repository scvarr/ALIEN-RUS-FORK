# Genome preview selection fix validation

Date: 2026-10-26. Platform: Windows, MSVC 14.44, Release.
Base: `c789b2ed749699a2cbe66af1f337ff925b3b7998`.
Branch: `fix/genome-preview-selection`.

## Root cause and reproduction

1. Open a genome containing multiple genes and nodes.
2. Select a gene row in the editor tree. `selectGene()` sets
   `GenomeSelectionLevel::Gene` while preserving remembered node indices.
3. Click a valid cell in the creature preview.

Before the fix, `_CreaturePreviewWidget::updateSelection()` assigned
`selectedGeneIndex` and called `setSelectedNodeIndex()` directly. Neither operation
restored the Node selection level. The cell ID and indices could change while
`_GenomeTabWidget::processEditors()` continued dispatching to `GeneEditor`, because
`isNodeLevelSelected()` remained false. Reopening the editor happened to restore
the initial Node selection; it was not necessary to reproduce or repair the bug.

A related inconsistency existed in preview highlighting: an external selection
was compared only with the remembered node index. Selecting another gene with the
same node index, or selecting a gene row with a remembered node, could retain the
previous preview cell ID and its neural activity selection.

## Minimal production change

- Call the existing `selectNode(geneIndex, nodeIndex)` for a valid preview hit.
  This updates the gene, remembered node, and selection level together.
- Retain a selected preview cell only while its ID exists and its gene and node
  match the current valid Node selection. Clear the local preview highlight when
  the selection moves elsewhere or the preview cell disappears.

No editor recreation, forced global reset, label changes, or ID changes were added.
The inspector runs before the preview in `processEditors()`. A click therefore
updates selection immediately and the inspector follows on the next frame.

## Regression tests

Build and run the focused regressions from the repository root:

```bat
build-windows-ninja.bat agent 16
build-agent\Release\GuiLocalizationTests.exe --gtest_filter=LocalizationGuiTests.genomePreviewSelection*
```

The existing `GuiLocalizationTests` infrastructure creates a real ImGui context.
The new tests submit mouse position, press, and release events through ImGui IO.
They execute the production tree row renderers, `GenomeEditorWindow` tab dispatch,
`_GenomeTabWidget::processEditors()`, inspector widgets, preview conversion,
viewport mapping, hit testing, and preview rendering. Tree rows are also rendered
in a small test window to obtain deterministic click coordinates, using the
existing production-row testing pattern.

Only the simulation facade is substituted: it returns fixed seed/offspring and
cell descriptions. Preview playback is paused and the production creature widget
is injected through the fixture's friend access. No GPU computation is required.

`genomePreviewSelectionRestoresNodeInspectorAcrossLanguagesScalesAndTabs` covers:

- Gene tree row selection followed by a preview cell click.
- Node tree row selection followed by a different preview cell click.
- Two genes sharing node index 0, plus another node in the first gene.
- Repeated clicks, including clicking the same cell again, without closing.
- Switching to another real genome tab and returning, then selecting again.
- EN/RU/EN at 100%, 150%, and 200% content scale, using rebuilt fonts and style.
- Matching preview cell ID, selected gene/node, valid Node level, and active
  `NodeEditor` after the release frame; the next-frame inspector delay is allowed.
- Stable `CellGraphWidget` and `NodeEditor` window IDs across language changes
  and selections within a tab. Production item labels and ID scopes are unchanged.
- Exact equality of the complete genome and facade data before/after selection,
  including nodes, neural networks, neural activity, and simulation descriptions;
  the tab remains unmodified and the other tab's genome is unchanged.

`genomePreviewSelectionClearsStaleCellAcrossGenesAndPreviewRefresh` covers:

- Clearing a preview selection when a different gene's node with the same index
  is selected in the tree.
- Clearing the local cell selection when the gene level is selected.
- Selecting again and returning to the node inspector.
- Refreshing the preview after a genome color edit and a cell position change.
- Retaining an existing matching ID, clearing a replaced ID, and clicking its
  replacement at the refreshed coordinates.
- Removing the selected node and rejecting a click on its stale preview cell.

## Results

The tests were built and run against the unchanged production source from the
base commit before applying the fix. The primary regression failed with
`isNodeLevelSelected() == false` and no active `NodeEditor` after the click.
The secondary regression failed because the old preview ID remained selected
after choosing another gene's node with the same index and after selecting a
gene row. The final test source was rerun against the base production file:
both tests failed for these expected reasons (exit code 1). After restoring the
fix, both focused tests pass, including all language/scale combinations.
The final Release binary also passed 10 shuffled repetitions of both focused
tests (seeds 27 through 36).

| Check | Result |
| --- | --- |
| `build-windows-ninja.bat agent 16` (Windows Release) | PASS |
| New focused GUI regressions with base production file | 2/2 FAIL as expected |
| New focused GUI regressions | 2/2 PASS after fix |
| Complete `GuiLocalizationTests` | 150/150 PASS |
| `BaseTests` | 41/41 PASS |
| `DataTests` | 247/247 PASS |
| `EngineInterfaceTests` | 2/2 PASS |
| `McpToolsTests` | 40/40 PASS |
| `NetworkTests` | 38/38 PASS |
| `PersisterTests` | 142/142 PASS; 1 existing disabled test |
| `AccountLocalizationTests` | 6/6 PASS |
| `McpLocalizationTests` | 7/7 PASS |
| clang-format 19.1.5, modified C++ files | PASS |
| `git diff --check` | PASS |

Total: 773 passing tests across the complete suites; the focused tests are
included in the GUI total. The enlarged existing GUI test translation unit hit
MSVC C1128 during development. Adding `/bigobj` to `GuiLocalizationTests` resolved
the object section limit; this affects only that test target.

GitNexus impact analysis resolved `updateSelection()` to the creature preview's
`process()` caller, with LOW risk and preview rendering/navigation flows. The
test fixture reported UNKNOWN because the index does not resolve GTest macro
callers; text inspection confirmed its uses in `TEST_F`. The index was refreshed.
Graph change analysis reports the preview selection flow with MEDIUM aggregate
risk, including test fixture changes, without a partial or truncated result.

## Changed files

- `source/Gui/Impl/CreaturePreviewWidget.cpp`: selection transition and stale
  preview highlight validation.
- `source/Gui/Impl/PreviewWidget.h`: friend access for the existing GUI fixture.
- `source/Gui/Impl/Tests/LocalizationGuiTests.cpp`: deterministic integration tests
  and fixture helpers.
- `source/Gui/Impl/Tests/CMakeLists.txt`: MSVC `/bigobj` for the GUI test target.
- `docs/genome-preview-selection-fix-validation.md`: this report.

## Limits and preservation

These are headless ImGui integration tests, without a graphics backend or manual
desktop visual inspection. Scale checks exercise the application's content scale
and fonts, not operating-system monitor DPI transitions. Preview refresh uses
synthetic facade snapshots and the real converter; live CUDA regeneration and
simulation timing are outside this test. The full GPU engine suite was not run:
no engine, shared data, kernel, or simulation algorithm dependencies changed.

`main` stays at the specified base. `AGENTS.md` and its skip-worktree flag are
preserved. Dependencies, user settings/data, and IDE build trees are untouched;
builds use `build-agent`. L7 was not started.
