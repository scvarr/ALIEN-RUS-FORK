# L5-B validation: world and selection tools

Date: 2026-10-23 (Europe/Moscow).
Branch: `feat/ru-localization-l5b`.
Baseline: `f801d0bd5254954a73e2d0abfa4004fb11bec0f0` (`main`).

## Delivered scope

| Component | Localized presentation |
| --- | --- |
| `EditorWidget` | Tool names, scope/contact options, overflow actions, placement/drawing/scissors/force instructions and shortcuts |
| `SelectionWidget` | HUD summary with Russian count forms, action names, sticky/static/inspection popups, position/velocity/rotation controls |
| `CreatorWidget` | Materials, shape dimensions, radii, spacing, color, energy, stiffness, glow, sticky/static flags and point completion/cancellation |
| `MultiplierWidget` | Grid/random modes, row and column captions, ranges, overlap checking, Build/Undo and overlap failure message |
| `ResizeWorldDialog` | Title, dimensions, content scaling and actions |
| `MassEditDialog` | Title, selection filter, all operation groups, ranges, mutation-rate integration and actions |
| `ImageToPatternDialog` | Open-image caption, descriptive file-filter caption and unreadable-image error |
| `SimulationInteractionController` | Edit-mode HUD label and exit hint |
| `EditorController` | Four directly related copy/paste/delete/glue notifications emitted by the localized selection actions |

`ChangeColorDialog` was already fully localized in L4-A1. It remains byte-identical;
its filter, color rule and actions reuse the existing catalog entries and regression
tests. No duplicate implementation or translation was added.

Both catalogs contain **1,176 keys**, including **132 new `worldtools.*` keys**.
All 1,044 previous entries are preserved exactly. Common actions, object names,
attribute descriptions, mutation controls and plural selection reuse existing
L1–L5-A presentation APIs. The nine creator tooltips formerly displayed directly
from `HelpStrings.h` now resolve through the catalogs. Their English text matches
the canonical declarations exactly; unrelated help constants remain unchanged.
No terminology-document edits were made. The mutant-operation heading describes
the actual lineage-ID randomization; it does not imply genome mutation.

## Identity, state and behavior

Canonical field names, parent scopes, grid-row IDs, material order, numeric formats,
limits and stored settings keys are retained. Toolbar buttons retain their original
index-based IDs. Selection-menu items draw translated captions under the exact
original ImGui IDs; adding a `###` suffix alone failed that legacy-hash check and
was replaced before delivery. Grid column captions use stable canonical suffixes.
Dialog titles use the existing localized `ModalWindow` API and retain the same
live popup instance across language changes.

The only shared control extension is an optional display `label` on
`SliderInputFloatParameters`, matching the existing field APIs. The angle slider
and input retain their original names, parent scope, ranges and focused input ID.
Callers without a label retain the old presentation.

`WorldToolPresentation.h` provides the world-tool display map, measured label
widths and the narrow selectable-caption adapter. Selection counts reuse
`GenomeEditorPresentation::count` and retain `StringHelper` number grouping.
Label widths use the current font and scale. Creator/transform/multiplier panels
and mass-operation columns accommodate the measured labels. Initial resize and
mass-dialog sizes are enlarged; existing saved dialog sizing continues to apply.
Toolbar overflow hints reuse the existing translation.

Source contracts compare creation, selection, clipboard, undo, mass-edit and
world-resize operations against the requested baseline. The four editor-controller
changes are restricted to message lookup. No engine, CUDA, persistence, simulation
data, user names, paths, extensions, file-filter grammar or settings identifiers
were changed. Temporal/spatial control and statistics production files are
byte-identical: L5-C was not started.

Failure dialogs resolve the title and whole message at display time, including
when already open. Short overlay notifications retain the existing emit-time
language policy until the next notification. The image file-dialog caption and
filter description use the language at opening; an already-open vendor dialog
keeps that caption and its current path/selection until closed. No vendor-dialog
implementation or dependency was modified.

## Automated validation

Windows Release: `build-windows-ninja.bat agent Release 16`.
Only `build-agent` was used. No kernel/header change required a clean CUDA rebuild.
Build logs, GTest XML, Python results, formatting eligibility and graph results
are retained locally under `temp/l5b/` and are not committed.

| Suite | Passed | Other results |
| --- | ---: | --- |
| BaseTests | 41 | 0 failures |
| DataTests | 247 | 0 failures |
| EngineInterfaceTests | 2 | 0 failures |
| McpToolsTests | 40 | 0 failures |
| NetworkTests | 38 | 0 failures |
| PersisterTests | 142 | 1 existing disabled test; 0 failures |
| EngineTests | 2,196 | 2 skipped, 2 existing disabled tests; 0 failures |
| GuiLocalizationTests | 135 | Complete L4/L5-A regression; 0 failures |
| Python localization contracts | 82 | Complete existing stage checks plus 3 L5-B checks; 0 failures |

The GPU suite completed in approximately 202 seconds without cancellation. The
skipped cases are `ConstructorTests_ProvideEnergy_Separation.provideEnergy_insufficientEnergy/2`
and `/5`, matching the previous stage's exclusions.

Seven added production ImGui tests cover EN -> RU -> EN at 100/150/200%:

- All creator materials and shapes, visible labels, parameter values and field bounds.
- Grid/random multiplier panels, persistent numeric values and focused input.
- Open resize/mass dialogs, stable popup instances, filters and draft parameters.
- Selection-menu popup continuity, exact legacy selectable IDs and count families
  for 0/1/2/5/11/14/21/22/25/1000.
- Long scope/material help measured with actual fonts and wrapping.
- Angle-input legacy hash, focused input and unchanged value.
- Already-open error messages, real editor/selection HUD windows, selected counts,
  tool state and original HUD window hashes.

Historical source checks exclude the new namespace from their original-stage
counts while retaining checks of every previous entry. The obsolete whole-file
freeze on the interaction controller and L5-B widgets is superseded by explicit
baseline operation/identity contracts. Shared-widget legacy adapters remove only
the new optional caption site when testing earlier-stage invariants.

The initial BaseTests failure was the stale expected catalog size; its expectation
was updated and all 41 tests rerun successfully. Early focused GUI tests caught
the selectable-hash issue and test-fixture font/popup lifecycle mistakes. These
were corrected before the final passing runs. Final Release build and
`git diff --check` pass. Formatting uses clang-format **19.1.5**; only modified
files already clean at HEAD were eligible for whole-file formatting. Existing
formatting differences in `AlienGui.cpp` and `ResizeWorldDialog.cpp` were preserved.

## Graph analysis

Upstream impact analysis covered the changed production methods and shared angle
control before edits. Resolved panel methods report LOW; the interaction-controller
class reports MEDIUM, with nine direct dependencies and an interface-dispatch
boundary. Callers include the creator process, selection action bar and multiplier
content panels. Relevant execution flows include `Process -> Get`,
`ProcessActionBar -> Get`, `ProcessContent -> IntRow`, `FloatRow`, `Move` and `Scale`.

UNKNOWN results were investigated in source: main-loop/dialog entry points use
virtual registration; image opening is called from `EditorWidget` and
`MainLoopController`; multiplier content is called by the selection popup; editor
notifications are reached by selection actions and keyboard/menu commands.
`SliderInputFloat` has one production consumer, the selection transform panel.
GTest/unittest registration explains unresolved test-method callers.

The index was refreshed with `node .gitnexus/run.cjs analyze --index-only`.
Graph analysis completed, but full-text index creation failed, so BM25/keyword
search remains degraded. The running MCP server retained stale metadata and later
closed its transport; the refreshed on-disk graph was queried through the CLI.
Final all-scope analysis is also saved as the backend's complete structured result
because the CLI formatter prints only the first 15 symbols even with `--limit 500`.
The final result covers 43 changed files, all 106 listed changed symbols and eight
affected processes, with neither `partial` nor `truncated` set.
The aggregate risk is HIGH because this change spans many panel methods and
historical tests; that is not a claim of an engine-algorithm modification.

## Limitations and preservation

100/150/200% checks are automated ImGui/font/layout checks. No interactive Vulkan
desktop session or screenshot-based visual acceptance was performed. Very narrow
manually resized windows, hovered tooltip appearance, vendor file-dialog controls
and the final desktop composition still need visual acceptance during ordinary
use. No separate manual-acceptance build was prepared, as requested.

Vendor file-dialog controls and other vendor UI remain the planned L8-B scope.
The existing L5-A inspector-title visual-verification limitation is unchanged.
The untranslated construction-energy service option `Free` remains unchanged;
this stage introduces no new exposure or terminology decision for it.

`main` remains at the specified baseline; only the feature branch is committed
and pushed. `AGENTS.md` retains skip-worktree and SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`.
User worlds, genomes, autosaves, dependencies and pinned-vcpkg state are unchanged.
Tests use the existing isolated settings scope; no production GUI was launched.
No CodeQL scan, submodule update or IDE-tree build was run.
