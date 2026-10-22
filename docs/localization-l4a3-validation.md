# L4-A3 validation: node editor

Date: 2026-10-22 (Europe/Moscow).
Branch: `feat/ru-localization-l4a3`.
Baseline: `0368045bdb359d16b7bd7eb05084b9ef1f678716` (`main`).

## Delivered scope

`NodeEditorWidget` resolves EN/RU captions every frame: selected/empty states,
base properties, construction selection and parameters, all existing cell panels,
mode selectors, lineage choices, color restrictions, channel masks, signal-buffer
entry buttons, deduced-angle annotation and property tooltips. Existing localized
first/last void-node error messages are reused.

The L4-A1 `GenomeEditorPresentation` adapter is extended with nine enum-ordered
option families and the 56 `EntityAttributeHelp` descriptions actually consumed by
the node editor. Its existing cell-type adapter is reused. The canonical Data
constants and canonical help table remain unchanged; English descriptions match
that table exactly. All adapters resolve through the existing localization
service and its embedded English fallback. There are 138 new `node.*` entries;
both catalogs now contain 771 entries. Existing entries are unchanged.

Approved terminology is retained, including Конвертер, Донор, Сглаживание сигнала,
signal-controlled muscle modes and the distinction between raw and usable energy.
The constructor selector contains only its original two genome options. The
service-only `Free` name remains literally `Free` in the explanatory tooltip;
no unapproved Russian name or additional control was introduced. No injector
mode selector was added. L4-A4 mutation editing and L4-A5 signal-buffer dialog
contents remain outside this change.

## Identity, behavior and layout

Canonical property names, parent child scopes, gene/node indices and constructor
scopes remain the ID inputs. Shared color selectors, color/channel checkboxes and
labeled buttons gain optional display fields with their original fallback.
The buffer button uses the existing `DisplayButton` implementation and retains
the original `Edit` ID. Integer infinity text uses the existing `common.infinity`
entry only for controls opting into a localized label.

`ComboParameters` gains optional `displayValues`. Its localized path keeps the
canonical combo name, numeric option scope and canonical option ID while showing
translated previews and choices. Missing or mismatched display arrays use the
original ImGui combo path. Construction choices keep a separate canonical list;
user gene names are never translated or rewritten. Option order, enum values,
indices and callbacks are unchanged.

Label and option widths are measured with the current font. The table's minimum
column width reserves both the longest translated label and chooser value.
Narrow panes can scroll horizontally; existing vertical scrolling exposes long
panels. The measured widths are recomputed after a language change. Tooltips use
the existing wrapping infrastructure.

No cell algorithm, genome structure, neural calculation, parameter range,
serialization, channel assignment or mask mapping changed. Shared adapter
consumers were checked: existing genome/gene captions retain their entries;
unmapped attributes retain canonical fallback. Shared widget callers include
InspectionWindow, CreatorWidget, ChangeColorDialog, InputColorTransition and
the parameter widgets. They retain the original paths when no display extension
is supplied. Inspection properties themselves are not localized by this stage.

## Automated validation

Windows Release passed using the root script:

```text
build-windows-ninja.bat agent Release 16
```

Only `build-agent` was used. Logs and XML results are retained in `temp/l4a3/`.

| Suite | Passed | Other results |
| --- | ---: | --- |
| BaseTests | 41 | 0 failures |
| DataTests | 247 | 0 failures |
| EngineInterfaceTests | 2 | 0 failures |
| McpToolsTests | 40 | 0 failures |
| NetworkTests | 38 | 0 failures |
| PersisterTests | 142 | 1 existing disabled test |
| GuiLocalizationTests | 114 | 6 new A3 tests; 0 failures |
| EngineTests | 2196 | 2 existing runtime skips, 2 disabled; 0 failures |

Total: **2820 passing C++ tests**, 2 runtime skips and 3 disabled tests.
The engine skips are `provideEnergy_insufficientEnergy/2` and `/5`.
No build or test was cancelled.

The six new production ImGui tests cover:

- All nine option adapters in EN/RU/EN, canonical order and reload behavior.
- All 56 descriptions, exact English text and missing-catalog English fallback.
- All 14 cell types and every existing mode, nested constructors, masks and
  selected-node/genome preservation at 100/150/200% with narrow/wide panes.
- An actively edited numeric node angle retaining its ImGui ID and edited value
  through EN/RU/EN.
- An already open canonical type chooser surviving translation, actual mouse
  selection across a language change, and a held buffer button retaining its ID
  and firing on release after translation.
- Actual color/channel checkbox clicks across a language change, retained IDs
  and exact resulting bit order.

These tests render production ImGui widgets without a graphics backend. They
verify state and interaction contracts, not human visual acceptance of pixels.

All **67 localization Python checks pass**, including six new A3 contracts for
catalog parity/preservation, placeholders and channel references, exact consumed
descriptions, enum order, canonical IDs/ranges/assignments/masks, opt-in shared
extensions, measured layout and unchanged later-stage/data files. Historical
catalog checks now exclude the new `node.*` slice. Their shared-widget checks
remove only the exact additive A3 presentation expressions; original behavior
comparisons remain. The former A2 node-editor freeze is superseded by A3's
explicit property/algorithm contracts. The A1 fallback assertion now uses an
attribute outside the expanded map. Both Windows Ninja dependency regression
tests pass from the MSVC environment, with no skips.

clang-format **19.1.5** was used only on modified C++ files clean at HEAD. Those
files pass formatting checks. `AlienGui.cpp` was already not format-clean and
received only local edits. `git diff --check` passes.

## Graph analysis

GitNexus indexing was refreshed with `node .gitnexus/run.cjs analyze --index-only`:
730105 nodes, 949422 edges, 432 flows. Full-text/BM25 index construction failed;
graph traversal completed. Fresh LocalBackend instances avoid stale metadata in
the long-lived MCP service. Upstream checks are retained in
`temp/l4a3/impact-fresh.json`.

Node attribute/empty-state methods report LOW, with `process()` as the direct
caller and no indexed affected process. The parameterized Button overload has
four indexed direct shared-widget callers (BasicInputColorMatrix, BasicSlider,
Checkbox and Switcher), ten upstream symbols and LOW risk. Other shared helpers
and the namespace presentation function report UNKNOWN where callers are not
resolved. Those results were not treated as unused code: all concrete GUI call
sites were checked in source and the complete GUI suite was run.

Complete staged-diff graph change analysis is recorded in
`temp/l4a3/graph-final.json`: **45 changed indexed symbols, 25 changed files,
no indexed affected flows, LOW risk**. All 45 symbols are returned, with neither
partial nor truncated output. Unresolved shared callers remain a limitation of
the graph, rather than evidence of no consumers. New presentation code is
additionally covered by source contracts and production widget tests.

## Isolated visual acceptance build

Launch from PowerShell:

```powershell
& 'E:\Projects\ALIEN\temp\l4a3\review\Launch-L4A3.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l4a3\review\Launch-L4A3.ps1' -Scale 1.5
& 'E:\Projects\ALIEN\temp\l4a3\review\Launch-L4A3.ps1' -Scale 2.0 -English
```

Default: Russian, 150%. Executable:
`E:\Projects\ALIEN\temp\l4a3\review\alien-l4a3.exe`.

The separately linked executable uses the final Release production libraries.
Its entry-point wrapper uses `SettingsTestScope` to redirect HKCU for that process,
disables autosave/save-on-exit and enables the genome editor. Only tracked runtime
resources, current catalogs, licensed fonts and required DLLs are packaged.
Disposable `samples/synthetic-a.genome` and `synthetic-b.genome` contain all cell
types/modes and constructors referring to a second gene. Profile isolation and
sample generation probes both return 0. No existing user world or genome is copied.

Manual acceptance checklist retained for reference (user acceptance recorded below):

1. Open `samples/synthetic-a.genome`. Inspect base/empty states, first/inner/last
   node angles, cell-type choices and constructor gene choices. User gene names
   must remain unchanged. Check first/last donor restrictions.
2. Visit every cell type and mode. Compare option order in EN/RU, inspect every
   tooltip, and check sensor/reconnector/communicator color and lineage filters.
   Injector must expose only its original gene-index field.
3. Toggle construction, both energy options, separation, branches, optional
   interval and infinite repetitions. Switch language during numeric editing and
   with a chooser open; values, selected node and focus must survive.
4. Inspect all memory modes, masks 0–3/4–7 and the buffer entry button. Open the
   buffer editor and verify existing Cancel/Adopt semantics; its internal captions
   are intentionally deferred. Check homogeneous-cell-type genes as well.
5. Repeat at 100/150/200%, narrow and wide panes. Inspect long chooser values,
   Cyrillic, headers, tooltips and horizontal/vertical scrolling. Save disposable
   edits only inside this package's `samples/` directory.

## Preservation

During implementation, `main` remained at the specified baseline. `AGENTS.md` retains skip-worktree and
SHA-256 `090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`.
The real HKCU profile exports before/after checks match:
SHA-256 `BDB897D5FA8D736A8D12E895D3D65F6910F6E3325568D77239BC0B57E075B37E`.
User worlds, genomes, autosaves and dependency sources are preserved. No CodeQL,
submodule update, IDE-tree build or dependency edit ran. Implementation commit and
push targeted only the requested feature branch.

## User acceptance and closure

Date: 2026-10-22 (Europe/Moscow).

The user confirmed successful visual acceptance of L4-A3 implementation
`7a71aed7027102211dbda85af765f99c5d8c4535` on 2026-10-22. Visual acceptance is
complete; the checklist above is retained as the review record. This records
the user's confirmation and does not claim an additional agent visual session.

The user authorizes a documentation-only acceptance commit and push, followed by
fast-forward-only integration into `main`, pushing `main`, verifying its
synchronization with `origin/main`, and deleting the integrated feature branch.
Builds and tests are not repeated: the passing results above remain the validation
for the accepted implementation. No L4-A4 work is started.

Before closure, the working tree was clean, local and remote feature refs matched
the accepted implementation, and local and remote `main` both matched the recorded
baseline. The closure changes only this report. `AGENTS.md`, its skip-worktree flag,
user data and dependencies are preserved. Documentation-only graph change analysis
reports one changed file, no changed indexed symbols or affected flows, and LOW
risk, with neither partial nor truncated output. Its local evidence is retained
separately under `temp/l4a3/closure/`.
