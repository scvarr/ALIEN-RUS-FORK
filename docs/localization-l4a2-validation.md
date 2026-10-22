# L4-A2 validation: neural network editor

Date: 2026-10-22 (Europe/Moscow).
Branch: `feat/ru-localization-l4a2`.
Baseline: `2ec767deff324d1eee7b5c4552c76a12003e1a01` (`main`).

## Delivered scope

The neural network editor now resolves English/Russian input/output, memory,
telemetry, weight/bias, channel-role, module, action and activation captions at
render time. Both embedded and modal editors use the same presentation adapter.
The six activation functions retain their enum order and mathematical evaluation.
The mathematical graph notation `tanh`, `abs` and `mod` is shared; the other short
annotations and all chooser names are localized. Canonical Data constants remain
unchanged.

`CellFunctionChannels` resolves captions while constructing its presentation
modules each frame. Read/write roles, channel indices, ordering, masks and mode
branches are unchanged. No neural algorithm, kernel, genome structure, numeric
range, randomization algorithm, clipboard data structure or serialization format
was changed.

The graph measures translated node and channel strings using the actual label
font. The weight/bias slider reserves the wider of both captions; inspector card
width follows the widest possible translated connection header. The rotated rail
labels are measured before rotation and fit their available vertical span without
byte truncation. Module blocks that need extra height stay inside the graph.
Glyph generation bypasses the unrotated CPU clip rectangle while retaining the
original GPU scissor for the final rotated vertices. Horizontal graph scrolling
continues to expose long channel labels. Tool buttons wrap using translated
widths; modal Adopt/Cancel occupy their own row and reserve that row's height.

The related preview neural-activity overlay translates signals, memory, heading,
empty states and hide hint, measures its columns from their display labels and
scrolls horizontally when needed. Preview detail/activity hints and speed format
are localized. Preview settings resolve their title, display mode, choices and
actions without changing their stored mode.

Canonical ImGui names, numeric node scopes, parent children, slider IDs, window
identity and editing state remain independent of language. All seven action
buttons use `DisplayButton` with their original canonical names. The modal uses
the existing stable title-key API. Preview sliders retain `Signal N` / `Mem N` as
their names and supply translated labels separately.

The only change in `NodeEditorWidget` is the heading of its neural editor. The
additive optional `GroupParameters.label` displays that heading while keeping
`GroupParameters.text` as the original expand-button scope. This is a narrow A2
integration change, not the start of A3: node properties, cell modes, mutation
panels and their help are unchanged. Existing Group callers keep their behavior.

The EN/RU catalogs each contain **633 keys**, including **66 new `neural.*` keys**.
All 567 pre-existing entries are unchanged. Translations follow the normative
terminology; no glossary decisions or canonical enum labels were modified.

## Automated validation

Windows Release completed successfully:

```powershell
.\build-windows-ninja.bat agent Release 16
python -m unittest discover -s source/Gui/Impl/Tests -p 'check_localization*.py'
```

Only `build-agent` was configured/built. No IDE build tree was used. No CUDA
structure/header changed, so no clean kernel rebuild was needed. An incremental
build retained an old localization object despite an updated generated English
catalog header. Removing only
`build-agent/source/Base/CMakeFiles/Base.dir/Release/LocalizationService.cpp.obj`
and rebuilding corrected the embedded fallback; the final Base suite passes.
This is a resolved build-cache issue, not a missing-catalog exception.

| Executable | Passed | Other results |
| --- | ---: | --- |
| BaseTests | 41 | 0 failures |
| DataTests | 247 | 0 failures |
| EngineInterfaceTests | 2 | 0 failures |
| McpToolsTests | 40 | 0 failures |
| NetworkTests | 38 | 0 failures |
| PersisterTests | 142 | 1 previously disabled test |
| GuiLocalizationTests | 108 | 9 new A2 tests; 0 failures |
| EngineTests | 2196 | 2 existing runtime skips, 2 disabled; 0 failures |

Total: **2814 passing C++ tests**, 2 runtime skips and 3 disabled tests. The engine
skips remain `provideEnergy_insufficientEnergy/2` and `/5`. No build or test was
cancelled. Logs, XML results and exit codes are retained under `temp/l4a2/`.

The nine new production ImGui tests cover:

- Enum-ordered activation adapters and unchanged canonical activation names.
- Memory modes with masks 0/1/85/255, channel read/write roles and ordering through
  EN/RU/EN, plus the sensor's dual-role trigger/found channel.
- Both editor layout modes, live memory/telemetry and selected indices at
  100/150/200%, with narrow/wide windows, measured bounds and finite draw vertices.
- Actual mouse clicks for Clear, Identity, Randomize, Copy and Paste through a
  language switch, with exact value checks and unchanged connection weights.
- An actual open neural modal, retained popup ID and buffered edits on a language
  switch, mouse Cancel rollback and Adopt of both neural and connection values.
- Mouse input/output selection, retained selected connection and active numeric
  weight input ID on a language switch.
- Preview overlay rendering, bounded placement, unchanged signals/memory and
  selection at all three scales in both languages.
- Canonical expand-button identity with the localized neural heading.
- Mouse resets of weight/bias and activation choice affecting only the selected
  neuron.

Language switching inside a modal is driven through the existing deferred locale
API in tests; it does not depend on being able to operate a blocked settings menu.
These tests render real ImGui widgets without a graphics backend. They establish
interaction/state contracts, not human visual acceptance of rasterized text.

All **61 Python checks pass**, including six new A2 contracts for catalog parity,
old-entry preservation, placeholders, display consumers, canonical scopes,
unchanged operation bodies, paired measurements and later-stage boundaries.
Historical catalog assertions now isolate their original slice from `neural.*`.
The historical shared-widget byte checks exclude exactly the additive optional
Group-label declaration/rendering expression; their original handler checks remain.
The new A2 boundary check permits only the neural heading change in NodeEditorWidget.
The Base catalog test covers all 633 entries and embedded English fallback.

clang-format **19.1.5** was used. All changed C++ files clean at HEAD and the new
header pass formatting checks. `AlienGui.cpp` was already not format-clean; only
its one display expression was changed. `git diff --check` passes.

## Graph analysis

The index was refreshed with `node .gitnexus/run.cjs analyze --index-only` at the
specified baseline. Graph indexing completed (729985 nodes, 949093 edges, 435
flows). The full-text/BM25 index build failed; graph traversal remains available.
The long-lived MCP service kept stale metadata, so fresh LocalBackend instances
were used for the final impact/change checks.

Method-level upstream impact for editor rendering/layout/actions, channels and
preview paths reports LOW. The indexed flows are neural `processGraph` and
creature-preview `process`; concrete neural-header importers include
InspectionWindow and NodeEditorWidget. Constructors, macro-generated tests and
virtual dialog/Group dispatch may return UNKNOWN or have no indexed target.
Those results were not treated as unused code: creation, singleton, virtual
callback and Group call sites were checked in source. The optional Group API is
covered by its identity test and the complete GUI suite.

Final graph change analysis uses `detect_changes(scope="all")` on the complete
staged diff. Its result is **78 changed symbols, 5 affected graph flows, MEDIUM
risk**, with `partial=false`, `truncated=false` and all 78 symbols returned.
Machine-readable output is `temp/l4a2/graph-final.json`; upstream results are in
`temp/l4a2/impact-fresh.json`. Newly added presentation/test code is additionally
covered by runtime and source-contract tests; the graph was indexed at baseline.

## Isolated visual acceptance build

Launch from PowerShell:

```powershell
& 'E:\Projects\ALIEN\temp\l4a2\review\Launch-L4A2.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l4a2\review\Launch-L4A2.ps1' -Scale 1.5
& 'E:\Projects\ALIEN\temp\l4a2\review\Launch-L4A2.ps1' -Scale 2.0 -English
```

Default: scale 1.5, Russian. Executable:
`E:\Projects\ALIEN\temp\l4a2\review\alien-l4a2.exe`.

The separately linked executable uses the final Release production libraries.
Its entry-point wrapper uses `SettingsTestScope` to redirect HKCU only for that
process. It enables the genome editor and detailed preview, disables autosave and
save-on-exit, and generates disposable samples with sensor/constructor, memory,
nonzero weights and bias. It contains DLLs, tracked resources, current catalogs
and licensed fonts; no user settings, worlds or autosaves were copied.
`--review-check-profile` and `--review-generate-samples` return 0; the launcher
parses without errors. Package sources/build log are in `temp/l4a2/` and remain
ignored local review artifacts.

The visual acceptance checklist is retained below:

1. Open `samples/synthetic-a.genome`; select its sensor/constructor node. Inspect
   embedded input/output, memory, telemetry, channel roles and the neural card.
   Select several input/output nodes and confirm the selected connection follows.
2. Expand the neural heading. Compare weight/bias values with the embedded view.
   Test each activation graph/name, sliders and resets. Cancel must discard modal
   edits; Adopt must transfer the edited net and connection weights.
3. Copy the net, Randomize, then Paste and compare values. Clear must zero weights
   and biases and select identity activations; Identity must restore the diagonal.
   Switch EN/RU between edits and verify values, selection and clipboard survive.
4. Select the memory node; inspect its channel labels and constructor block if
   enabled. Enable detailed simulation and the neural-activity preview overlay,
   select a preview cell and edit a signal/memory value. Inspect the overlay's hide
   hint and preview settings; Cancel/Adopt must retain their existing semantics.
5. Repeat at 1.0/1.5/2.0 and with narrow/wide editor panes. Scroll long graphs and
   overlay columns. Check rotated Cyrillic rails, clipping, button wrapping,
   inspector header, weight/bias captions, activation hints and tooltip wrapping.
   Save any disposable edits only under this package's `samples/` directory.

## Limitations and preservation

The user confirms successful visual acceptance at different UI scales; see the
closure investigation below. No native-window screenshots are claimed. Automated
font/layout/state checks and isolated launch preparation are complete. General preview titles/counts, preview cell-type
annotations, SignalsBufferDialog and other A5 work remain outside this stage.
Node properties/cell modes and mutation panels retain their planned A3/A4 scope.
The shared channel captions also appear in live inspectors, but this does not
localize the remaining inspector properties. Third-party file-dialog controls
retain the previously documented English vendor strings.

`main` remained at the specified baseline during the initial implementation. `AGENTS.md` retains skip-worktree and
SHA-256 `090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`.
The real `HKCU\Software\alien` exports before/after checks are compared under
`temp/l4a2/` (both SHA-256 `891CF1B4278DAD9DB22B571DC7A7EA897196EA45E717187C99CA9D41B0C1DB37`). User worlds/genomes/autosaves and pinned dependency sources are
unchanged. No CodeQL, submodule update, IDE-tree build or dependency edit ran.
The initial implementation was committed/pushed only on the feature branch.
Integration is authorized by the user in the closure follow-up below.

## Exact change inventory

- `resources/locales/en.json`
- `resources/locales/ru.json`
- `source/Base/Tests/LocalizationServiceTests.cpp`
- `source/Gui/Impl/AlienGui.cpp`
- `source/Gui/Impl/AlienGui.h`
- `source/Gui/Impl/CellFunctionChannels.cpp`
- `source/Gui/Impl/CreaturePreviewWidget.cpp`
- `source/Gui/Impl/CreaturePreviewWidget.h`
- `source/Gui/Impl/NeuralNetEditorPresentation.h`
- `source/Gui/Impl/NeuralNetEditorWidget.cpp`
- `source/Gui/Impl/NeuralNetEditorWidget.h`
- `source/Gui/Impl/NodeEditorWidget.cpp`
- `source/Gui/Impl/PreviewSettingsDialog.cpp`
- `source/Gui/Impl/PreviewWidget.cpp`
- `source/Gui/Impl/Tests/LocalizationGuiTests.cpp`
- `source/Gui/Impl/Tests/check_localization_l2a.py`
- `source/Gui/Impl/Tests/check_localization_l2b1.py`
- `source/Gui/Impl/Tests/check_localization_l2b2.py`
- `source/Gui/Impl/Tests/check_localization_l2b3a.py`
- `source/Gui/Impl/Tests/check_localization_l2b3b.py`
- `source/Gui/Impl/Tests/check_localization_l2b3c.py`
- `source/Gui/Impl/Tests/check_localization_l2b4a.py`
- `source/Gui/Impl/Tests/check_localization_l2b4b.py`
- `source/Gui/Impl/Tests/check_localization_l2b4c.py`
- `source/Gui/Impl/Tests/check_localization_l2b4d.py`
- `source/Gui/Impl/Tests/check_localization_l2b4e.py`
- `source/Gui/Impl/Tests/check_localization_l4a1.py`
- `source/Gui/Impl/Tests/check_localization_l4a2.py`
- `docs/localization-l4a2-validation.md`

New catalog keys (same set in EN/RU):

```text
neural.title
neural.connections
neural.weight
neural.bias
neural.incoming
neural.outgoing
neural.memory
neural.telemetry
neural.energy
neural.attacked
neural.age
neural.velocity
neural.yes
neural.no
neural.input
neural.output
neural.mem
neural.signal
neural.adopt
neural.cancel
neural.clear
neural.identity
neural.randomize
neural.copy
neural.paste
neural.activation.tanh
neural.activation.step
neural.activation.identity
neural.activation.abs
neural.activation.gaussian
neural.activation.mod
neural.channel.trigger
neural.channel.found
neural.channel.angle
neural.channel.mass
neural.channel.distance
neural.channel.activation
neural.channel.delayed
neural.channel.replayed
neural.channel.restored
neural.channel.smoothed
neural.channel.readwrite
neural.channel.transfer
neural.channel.value
neural.channel.success
neural.module.gen
neural.module.attack
neural.module.inject
neural.module.recon
neural.module.deton
neural.module.comm
neural.module.constructor
neural.preview.activity
neural.preview.disabled
neural.preview.noselection
neural.preview.detailed
neural.preview.showactivity
neural.preview.settings
neural.preview.speed
neural.preview.mode
neural.preview.node
neural.preview.cell
neural.preview.hideactivity
neural.activation.shortstep
neural.activation.shortidentity
neural.activation.shortgaussian
```

## User acceptance, scale-change investigation and closure

Date: 2026-10-22 (Europe/Moscow).

The user visually accepted implementation
`fcb292643b6ef55ea3da4b881e8f8f99ff7e98bc`: neural editor translation and rendering
were correct at different scales. The user also reported one crash while changing
UI scale with the genome editor open; a subsequent launch/change succeeded.
This report was investigated before integration, rather than recorded as an
unexplained or fixed-by-restart event.

### Crash evidence and established cause

Application Error events and local minidumps were available. Events at 17:18:26
and 17:20:16 on October 22 report `0xc0000005` in the agent `alien.exe`. Their dump
stacks lead through:

```text
MainLoopController::process
  -> MainLoopEntityController::process
  -> DelayedExecutionController::process
  -> DisplaySettingsDialog::onChangeContentScaling
  -> GenericMessageDialog::localizedYesNo
  -> std::function assignment / invalid virtual call
```

The intervening 17:18:51 dump faults during destruction of the same generic
message dialog on exit. Older Application Error records also exist, but their
individual causes are not inferred from the event summaries alone.

An offline diagnostic relink produced a linker map with an identical `.text`
SHA-256 to the crashing executable. Minidump exception context, captured memory
and PE unwind records were used to map the stacks; broad export-name fallback
was not used as evidence of a Network/cereal fault.

The build contained incompatible object layouts. `MainWindow.cpp.obj` and
`DisplaySettingsDialog.cpp.obj` dated from October 21, while the updated
`GenericMessageDialog.cpp.obj` dated from October 22. The old singleton allocator
in MainWindow allocated **0x238 bytes** for GenericMessageDialog; the current
layout requires **0x248 bytes** after the earlier addition of message arguments.
The current callback pointer at offset **0x240** lies outside the old allocation.
The clean executable allocates 0x248 bytes. This explains corruption during scale
confirmation and dialog destruction; it is not a neural evaluation or font graph
algorithm defect.

Ninja reported **zero header dependencies** for the stale display-settings
object. CMake had decoded the localized MSVC `/showIncludes` output with the
wrong Windows encoding, writing a garbled prefix to `rules.ninja`. Ninja therefore
failed to recognize include lines and retained stale consumers when headers
changed. The same build-cache failure accounts for why a generated locale header
could previously change without rebuilding its consumer.

Source inspection confirmed that applying content scale opens a deferred
confirmation, stores the new scale and calls `scheduleClosing()`. It requests an
application restart; it does not rebuild the font atlas inside the open editor.
The scale setting, confirmation callback and shutdown paths are otherwise unchanged.

### Reproduction and fix

An isolated full-application diagnostic executable, linked to the original review
libraries and using SettingsTestScope, opened a synthetic genome and invoked the
production scale-confirmation path. It crashed before the confirmation callback
with `0xc0000005` (probe PID 50000, event at 17:34:26). The real user profile was
not used. Native window capture was attempted twice but failed with
`FrameArrived timed out` / `window capture timed out`; no mouse/visual automation
result is claimed. The reproduction instead runs the actual GUI/engine loop and
production methods through a local diagnostic entry point.

The fix adds `cmake/MsvcShowIncludes.cmake`, included immediately after project
initialization. For MSVC/Ninja it compiles a small include probe, preserves its raw
output bytes with `ENCODING NONE`, extracts the actual include prefix and supplies
it to Ninja. It caches the verified prefix and rejects existing unverified or
incompatible dependency caches, requiring a clean tree. It supports a localized
compiler without requiring an English language pack. Other generators/platforms
skip this probe. Existing pre-fix Ninja caches must be recreated by their owner;
agents must never clean the IDE-owned trees.

Only the agent tree was archived/recreated; its installed vcpkg dependency tree
was preserved. The original stale binaries and diagnostic evidence remain under
`temp/l4a2/closure/`. No IDE build directory, dependency source, application
algorithm or user asset was changed. After the clean Release build, Ninja records
**51 dependencies** for DisplaySettingsDialog and **347 dependencies** for
MainWindow, including GenericMessageDialog.h.

The original review executable was relinked against the corrected fresh
libraries. Its launch path remains
`E:\Projects\ALIEN\temp\l4a2\review\Launch-L4A2.ps1`.
The build fix and regression tests are recorded in commit
`fce7151a6f3c89d40e610491185e4f515645bd63`.

### Focused validation after the fix

No full test-suite repetition was performed. The affected checks passed:

| Check | Result |
| --- | --- |
| Windows Release, root build script with `agent Release 16` | Clean build passed |
| BaseTests | 41 passed |
| GuiLocalizationTests | 108 passed |
| L4-A2 Python contracts | 6 passed |
| Windows Ninja dependency regression tests | 2 passed, no skips |
| Incompatible agent cache | Rejected before build as expected |

The new `tests/test_windows_ninja_dependencies.py` configures/builds a small
MSVC/Ninja project with the production module. It verifies that an included
header appears in the dependency database, that changing only the header changes
the rebuilt executable's result from 1 to 2, and that a tampered cache prefix is
rejected. It also checks the raw-output and guard wiring.

Three independent isolated application runs then opened the synthetic genome,
selected its neural node, rendered the editor, created the production scale
confirmation, invoked its callback and shut down normally:

| Initial scale / target | Language | Exit code |
| --- | --- | ---: |
| 100% / 150% | RU | 0 |
| 150% / 200% | RU | 0 |
| 200% / 100% | EN | 0 |

All runs reached `PROBE shutdown complete`. The selected node exposes the
embedded neural graph; the real rendering/engine loop is used. These are focused
scale-path regression probes, not a second human visual acceptance session.
The CMake dependency regression prevents recurrence of the stale-build mechanism;
this is not a claim that every historical access violation has been diagnosed.

Additional changed files in this closure: `CMakeLists.txt`,
`cmake/MsvcShowIncludes.cmake`, `tests/test_windows_ninja_dependencies.py` and this
report. No L4-A3 work was started. Complete graph change analysis is required
before both the fix and documentation commits. The fix check returned 9 changed
indexed symbols, no indexed affected flows, LOW risk, `partial=false` and
`truncated=false`. CMake entry-point impact was UNKNOWN (no resolved graph
callers), so the configuration scope was reviewed directly and verified by the
compile/header-change tests. The subsequent documentation-only check returned
4 indexed symbols, no affected flows, LOW risk, with neither partial nor
truncated output. Results are retained in `temp/l4a2/closure/`.

The real HKCU\Software\alien exports before/after the investigation match:
SHA-256 `BDB897D5FA8D736A8D12E895D3D65F6910F6E3325568D77239BC0B57E075B37E`.
AGENTS.md retains its previously recorded SHA-256 and skip-worktree flag.
User worlds, genomes, autosaves and pinned dependencies are preserved.

There are no remaining blocking findings for this stage. The user authorizes
closing L4-A2, committing this report on the feature branch, fast-forward-only
integration into main, pushing main and deleting the integrated feature branch.
L4-A3 remains outside this task.
