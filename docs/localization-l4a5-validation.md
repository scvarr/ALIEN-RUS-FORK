# L4-A5 validation: genome diagnostics, preview and signals

Date: 2026-10-22 (Europe/Moscow).
Branch: `feat/ru-localization-l4a5`.
Baseline: `aa69598bd2962b2daa455978c5a6fc4f73983b43` (`main`).

## Delivered scope

`GenomeIssueDescription.cpp` resolves all seven diagnostic families at display
time: donor boundary nodes, disconnected nodes, cycles avoiding the root,
unreachable genes, excess separation, construction of removed genes and injection
of removed genes. Titles, problems, explanations, correction consequences,
gene/node locations, cycle labels and meta-mutation notes are translated.
Follow-up warnings retain their existing icon, relevance and cause semantics.

Messages use complete sentence templates with named arguments. Russian counted
sentences distinguish one/few/many, including the 11–14 exception and 21; the
empty disconnected-node list has a separate complete zero sentence. Index lists
use the existing range formatter; cycle paths retain their original arrow and
order. The term Void consistently uses the approved donor terminology.
Disconnecting donors and affected nodes have all four independent singular/plural
sentence variants, avoiding plural wording when either list contains one node.

`GeneGraphWidget.cpp` translates gene captions, the root designation, construction
edges, constructor locations and separation messages. User-authored gene names
remain unchanged, and their existing UTF-8 truncation stays intact.

`CreaturePreviewWidget.cpp` translates primary/secondary and trimmed headers as
complete counted templates and wraps long headers. `PreviewWidget.cpp` translates
the TPS caption and adds localized hints to the existing run, pause, backward,
forward and restart buttons. `PreviewDescView.cpp` translates and measures the
front marker caption. Existing A2 activity, settings and speed translations are
reused. `PreviewSettingsDialog.cpp/.h` remain byte-identical to the baseline.

`SignalsBufferDialog.cpp` and `AlienGui::SignalMemoryEditor` translate signal
counts, selection, channel labels and their hints. The modal reuses existing
Adopt/Cancel translations and the stable-title API. Label widths are measured
with the active font; the modal has a wider initial size and horizontally
scrollable content for narrow windows. `SignalsBufferDialog.h` only adds test
friendship.

Both catalogs contain **932 keys**, adding **78 keys** under `genomeissues.*`,
`genegraph.*`, `preview.*` and `signals.*`. All 854 existing entries are unchanged.
The embedded English fallback is generated through the existing CMake mechanism.
The Base catalog completeness/reference test includes the new consumers.

## Identity and data preservation

Canonical widget names, parent/child scopes, channel numbers, signal indices,
button IDs and modal titles remain the identity source. Display labels resolve
each frame without resetting drafts or editor selection. Preview simulations,
savepoints, caches and timing calculations are unchanged.

Genome validation/correction algorithms, gene structures, signal values, numeric
formats/ranges, enum ordering and serialization are unchanged. Both signal-buffer
`open` overloads and `onAdopt` retain their original implementations. The shared
editor still uses its original minimum signal count of one; the buffer dialog
still permits zero. The existing shared selection logic is preserved.

No engine, Data, persistence, application composition root or dependency source
is modified. No CodeQL scan, submodule update or IDE-tree build was run.

## Automated validation

Windows Release command:

```text
build-windows-ninja.bat agent Release 16
```

Only `build-agent` was configured and built. Logs and XML results are retained
under `temp/l4a5/`. The initial compilation exposed an ADL ambiguity with
`std::format`; the call is explicitly qualified and the subsequent Release build
passes. No build or test was cancelled.

| Suite | Passed | Other results |
| --- | ---: | --- |
| BaseTests | 41 | 0 failures |
| DataTests | 247 | 0 failures |
| EngineInterfaceTests | 2 | 0 failures |
| McpToolsTests | 40 | 0 failures |
| NetworkTests | 38 | 0 failures |
| PersisterTests | 142 | 1 existing disabled test |
| GuiLocalizationTests | 124 | 6 new A5 tests; 0 failures |
| EngineTests | 2196 | 2 existing runtime skips, 2 disabled tests |

Total: **2830 passing C++ tests**, two runtime skips and three disabled tests.
The engine skips are `provideEnergy_insufficientEnergy/2` and `/5`, matching A4.
The full engine suite completed in 158.543 seconds.
After the final singular/plural wording refinement, Release, the complete GUI
suite, Base catalog suite and all Python localization checks were rerun and passed.

Six new production ImGui tests cover:

- Every diagnostic family in EN/RU/EN, direct/follow-up text, locations,
  meta-mutations, removed-target references, empty tooltips and missing catalogs.
- Russian 0/1/2/5/11/21 diagnostic forms, donor conversion and gene-removal
  consequences, without mutating the diagnostic source genome.
- Graph rendering and all primary/secondary/trimmed header variants at
  100/150/200%, 0/1/2/5/21 cell counts, Cyrillic glyph availability and measured
  diagnostic wrapping, preserving the genome and node selection.
- Actual Ctrl-click signal-channel editing across EN→RU with the active ImGui
  ID retained, independent second-signal values, rendering at all three scales
  and serialization/reload of the edited memory genome.
- Modal draft and selected signal retention, actual Cancel while switching
  language with the button held, actual Adopt for both genome/live signal
  callbacks and independent channel values.
- An open production preview-settings modal across languages and all three
  scales, preserving its draft and the source display mode.

All **76 localization Python checks** pass, including four new A5 contracts.
Historical tests exclude only the new A5 catalog namespaces and superseded A5
presentation sites. The shared legacy adapter substitutes only the signal-editor
function for historical byte comparisons; A5 separately checks its canonical IDs,
limits, numeric formats, selection storage and data access. Other historical
contracts remain enforced. Both Windows Ninja dependency tests pass without skips.

clang-format **19.1.5** was applied only to modified C++ files clean at HEAD.
`AlienGui.cpp` was already not clean and received local edits only.
`git diff --check` passes.

## Graph analysis

Fresh upstream impact evidence is retained in `temp/l4a5/impact-fresh.json`.
Diagnostic helpers report LOW and lead to `getIssueText`, `getIssueTooltip` and
`getTooltip`. Graph text helpers lead to `processGraph`/`process`; the preview
title leads to the creature preview `process` flows (navigation/move/scale).
The front marker leads to `draw` and `drawCollage`. The signal-dialog class has
six indexed dependents and reports LOW.

The dialog constructor/render override, shared signal editor and test fixture
report UNKNOWN, which was investigated rather than treated as an all-clear.
Source inspection confirms the dialog's node-editor/inspector consumers and
main-window registration. The shared signal editor currently has no production
call sites; it remains implemented and is exercised directly by the GUI tests.
Test entry points use gtest/unittest registration.

Index refresh succeeds for the graph; full-text/BM25 construction fails in this
environment, as in prior stages. Global process enumeration is bounded, so lack
of process membership is not proof of no consumers. Final pre-commit graph
analysis is retained under `temp/l4a5/`.
The complete `scope: all` result in `graph-final.json` returns all **76 changed
indexed symbols across 30 files**, LOW risk and no indexed affected processes;
neither partial nor truncated output is reported. The index contains 730217
nodes, 950039 edges and 432 reported flows. These bounded graph results are
supplemented by the source-call inspection and complete test results above.

## Isolated acceptance build

The final Release libraries are separately linked into:

```text
E:\Projects\ALIEN\temp\l4a5\review\alien-l4a5.exe
```

The wrapper redirects HKCU for its own process with `SettingsTestScope`, disables
autosave/save-on-exit and enables the genome editor. Only tracked runtime assets,
catalogs, licensed fonts and required DLLs are packaged. Profile and sample
generation probes both return 0. No user world/genome is copied.

Run from PowerShell:

```powershell
& 'E:\Projects\ALIEN\temp\l4a5\review\Launch-L4A5.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l4a5\review\Launch-L4A5.ps1' -Scale 1.5
& 'E:\Projects\ALIEN\temp\l4a5\review\Launch-L4A5.ps1' -Scale 2.0 -English
```

Default: Russian, 150%. Each run uses a disposable profile. Save review edits
only inside the package's `samples/` directory. The package recreates its
synthetic inputs on launch.

The user accepted L4-A5 without an additional manual launch, based on the
automated checks and localization review. No visual acceptance was performed or
is claimed. The isolated build remains available for optional future inspection.

Optional manual checklist retained for future inspection; not executed for closure:

1. Open `samples/diagnostics.genome`, `cut-off.genome` and `separation.genome`.
   Inspect warning icons in the editor and graph, root and edge tooltips, all
   causes/consequences, locations, cycles and follow-up arrows. Toggle relevant
   structural/meta-mutation rates to inspect alternate boundary corrections.
2. Open `samples/signals.genome`. Select storage/recorder memory cells, open the
   signal buffer and edit different channels in different signals. Switch EN/RU
   while editing and with the modal open. Cancel must discard the draft; Adopt
   must apply it once. Save under a new name and reload to compare values.
3. Inspect primary/secondary preview cards and the front marker. Use detailed
   preview, neural activity, run/pause/step/restart and settings. Switch languages
   while paused, with a selected node and with settings open; confirm the same
   selection, view and draft remain.
4. Repeat at 100/150/200% and narrow/wide windows. Inspect long Russian messages,
   Cyrillic gene names, header/tooltips wrapping, channel labels, buttons and
   scrollbars. `synthetic-a.genome` and `synthetic-b.genome` retain the A1–A4
   acceptance scenarios, including all cell modes and active mutation groups.

## Preservation

At implementation delivery, `main` remained at the requested baseline and only
the feature branch was committed and pushed. `AGENTS.md` retains skip-worktree and SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`.
User worlds, genomes, autosaves and dependency sources are preserved. Profile
exports before/after validation are retained under `temp/l4a5/`.
Both real-profile exports have SHA-256
`09F9632B91EF06A375D98A1C5F9F0BE67602D145861E10A4E79B515BC4830758`.

## Acceptance and L4 closure

Date: 2026-10-22 (Europe/Moscow).

The user accepted L4-A5 implementation
`231a9195b9819a10aa939ec7ff7ab270e89bb446` on the basis of the recorded automated
validation and localization review, without an additional manual launch.
This is acceptance of the implementation, not completed visual acceptance.
L4-A1–A5 are complete and L4 is closed. L5 is not started or authorized by this
closure.

The user authorizes a documentation-only acceptance commit and push,
fast-forward-only integration into `main`, pushing `main`, synchronization
verification and deletion of the integrated feature branch. This closure changes
only this report and the roadmap. Builds and tests are not rerun; the existing
passing results above remain the validation evidence. Catalogs, implementation,
`AGENTS.md`, skip-worktree flags, user data and dependencies are preserved.

### Deferred editorial review

The following wording candidates are recorded for later review and do not block
the accepted implementation. No catalog changes or replacement translations are
approved by this documentation update:

- `genomeissues.cut.void.one` and `genomeissues.cut.become.one`: consider a
  singular index caption when the counted sentence describes one node, while
  retaining plural captions and range formatting for larger lists.
- `preview.primary.trimmed.*` and `preview.secondary.trimmed.*`: review whether
  the shortened header makes the grammatical referent of the trimmed qualifier
  sufficiently clear, especially next to the cell count.
- `preview.front`: review the clarity of the short front-direction caption in
  context, balancing readability with the measured marker width.

Any later wording changes should preserve the agreed terminology, whole-sentence
count variants, argument signatures, canonical IDs and data contracts. They are
separate follow-up work and do not reopen L4 or start L5.
