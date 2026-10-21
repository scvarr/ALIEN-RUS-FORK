# L2-B1 simulation parameters shell validation

Date: 2026-10-21 (Europe/Moscow).

## Baseline and repository protection

Base: `93c3c6b3ebdf55a591a418b08b2d4760bb140aae` (`Use concise Russian README`).
An origin fetch confirmed that local `main` and `origin/main` both pointed to this
commit. The checkout was clean, main was active, and no other worktree existed.
Branch `feat/ru-localization-l2b1` was created from that verified main.
The audit, terminology and L1/L2-A validation reports were consulted.

The user's locally customized `AGENTS.md` and its `skip-worktree` flag are retained
and excluded from the commit. README, README.upstream.md, LICENSE, the normative
terminology document, dependency versions and the vcpkg submodule are unchanged.
No upstream synchronization, submodule update, CodeQL scan, merge into main,
force push or published-history reset was performed.

## Changed files and translated scope

- `resources/locales/en.json`, `resources/locales/ru.json`: 49 new semantic keys;
  each catalog contains 138 keys. Existing `menu.parameters` supplies the main
  window title. English remains canonical, including the generated embedded fallback.
- `source/Gui/Impl/SimulationParametersMainWindow.cpp`: main title, Overview,
  Expert settings, selected-object heading and filtered state; all 12 toolbar
  command names and all existing command tooltips; overflow tooltip; copy/paste/
  reference notifications; five column headings and three system row types;
  display-only Base name; slider status hint and filter placeholder; parameter
  file-picker titles/filter description; load/save, reference mismatch and limit errors.
- `source/Gui/Impl/AlienGui.h`, `source/Gui/Impl/AlienGui.cpp`: optional tree label
  and optional toolbar overflow tooltip, with existing behavior as the default.
- `source/Gui/Impl/SimulationParametersGuiText.h`: small presentation helper that
  substitutes one fixed `{name}` slot in complete localized messages. It does not
  interpret user text or introduce a general formatter. Base, layer and source
  window titles have their own complete message keys.
- `source/Gui/Impl/SimulationParametersBaseWidget.cpp`,
  `source/Gui/Impl/SimulationParametersLayerWidget.cpp`,
  `source/Gui/Impl/SimulationParametersSourceWidget.cpp`: localized window captions.
- `source/Gui/Impl/SimulationParametersMainWindow.h`: access for the existing
  headless fixture, allowing the production table and toolbar to run without an engine.
- `source/Gui/Impl/Tests/LocalizationGuiTests.cpp`: five new integration tests.
- `source/Base/Tests/LocalizationServiceTests.cpp`,
  `source/Gui/Impl/Tests/check_localization_l2a.py`: updated catalog completeness
  expectations and source coverage while retaining prior checks.
- `source/Gui/Impl/Tests/check_localization_l2b1.py`: four additional contracts.
- `docs/localization-l2b1-validation.md`: this report.

No parameter specifications, engine values/enumerations, CUDA, serialization,
JSON storage fields, CLI or MCP contracts are changed. User-defined names remain
verbatim. The English Base value in the presentation model remains unchanged;
its translation is selected only when rendering. Numeric strings, camera action,
row selection and parameter mutation callbacks retain their original code.

Terminology: Strength is translated as the explicitly requested radiation share
("Доля"). The system parameter-level Base is displayed as "Базовый уровень",
following the task's level terminology; it is distinct from the glossary's Base
cell type. No new unresolved domain term was identified, and the normative
terminology file was not extended.

## Identity and state invariants

`AlienWindow` already separates its custom titlebar from the internal Begin name.
Only `setTitleKey("menu.parameters")` is added. `Simulation parameters`, its ID
stack and all `windows.simulation parameters...` settings keys remain unchanged.

`TreeNodeParameters::name` still controls ImGui identity, wrapper tracking and
`_savedTreeNodeStatesByName`. Optional `label` supplies display text via the
explicit-string-ID TreeNodeEx overload using a fixed `%s` format. Calls without
labels use their original overload. Highlighting uses the visible label.
Overview and Expert settings retain their English internal names. The dynamic
section uses `###parameters`, which hashes identically to the previous dynamic
heading ending in `###parameters`. Its saved-name key is now constant, so changing
language, filter or selected object cannot split the in-memory saved state.
Persisted section-open booleans still use the original GlobalSettings keys.

The Locations table keeps its name, column count, order, flags, initial widths
and original zero UserIDs (legacy index-based settings mapping). Column captions
have fixed `###Name`, `###Type`, `###Position`, `###Strength`, `###Opacity` suffixes,
so translated text cannot create a new header interaction ID. Headless tests
retain the actual table object, changed width, reordered columns, hidden column,
selected layer and fixture row data through EN -> RU -> EN. Persistent table
identity is unchanged; transient header labels now have explicit stable suffixes.

Toolbar and overflow rows keep their existing index scopes and hidden button IDs.
The production narrow-toolbar popup survives the language sequence. A normalized
baseline comparison checks the complete toolbar function: actions, availability,
icons, ordering and control flow match the base commit, allowing only presentation
changes and the additive tooltip parameter.

`LocationController`, `LocationWindow` and `LocationWidget` are unchanged.
LocationWindow continues to use its numeric `_id` suffix; localized captions and
renamed user objects therefore do not change window identity. Tests exercise the
same numeric-suffix ImGui window construction with changing captions, retained
position, mixed user text, percent signs and literal `{name}`. The real base-widget
caption is also checked. The layer/source name caches and engine lifecycle are
not reimplemented in the test.

Parameter file patterns remain exactly ` (*.settings.json){.settings.json,.json},.*`.
Only the description preceding this grammar is translated. ALIEN errors use the
existing localizedInformation API, retaining message keys while open and the
localized OK button. Tests switch all five error variants through both languages
and use real mouse events to press OK and execute the production delayed close.

## Build and automated results

Required `build-windows-ninja.bat agent` succeeded in Release.
A fresh agent tree completed all 411 build steps. To avoid the documented
Ninja/MSVC header-dependency defect, the previous build-agent tree was reversibly
archived at `temp/l2b1/build-before-clean`; only `vcpkg_installed` was moved into the
new tree. No compiled ALIEN object/library was reused. All consumers of the changed
AlienGui header were compiled afresh. IDE-owned build trees were untouched.
Subsequent required-script builds after formatting and stronger test assertions
also passed. The final source build, including the language-selector removal,
is recorded in `temp/l2b1/selector-removal-build.log`.

| Suite | Passed | Failed | Disabled |
| --- | ---: | ---: | ---: |
| BaseTests | 41 | 0 | 0 |
| DataTests | 247 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 |
| PersisterTests | 141 | 0 | 1 |
| GuiLocalizationTests | 19 | 0 | 0 |
| Total | 528 | 0 | 1 |

All 14 existing GUI tests are retained. The five added GUI tests cover tree ID
and saved-state behavior, the production table, dynamic window captions/identity,
the production toolbar overflow, and actual confirmation of localized errors.
All 138 embedded English values are checked with external catalogs absent.
Existing malformed-catalog, duplicate-key, invalid-UTF-8, font, fallback,
frame-boundary switching and isolated settings tests remain passing.

Both Python programs pass: six L2-A checks and four L2-B1 checks (10 total after
the language-selector placement correction). They supplement
runtime tests with strict UTF-8/duplicate-free key parity, message-slot parity,
complete toolbar text coverage, unchanged actions/conditions, file patterns and
settings/camera contracts. These checks run from any working directory.

An intermediate stronger OK-click test failed because it captured the last item
rectangle after leaving the modal scope. Capturing it inside the modal content
fixed the test; no production dialog change was needed. The final 19-test run
passes, including real button clicks. Initial passes are not counted twice.

EngineTests and stress tests were deliberately not run: production changes are
restricted to GUI presentation and catalogs, and the engine/CUDA/data/protocol
code is unchanged. The CPU/interface/persistence suites plus fresh compilation
and headless GUI integration are the relevant regression scope. This is not a
new GPU qualification claim.

clang-format 19.1.5 was used only on modified files clean at HEAD, plus the new
header. Already non-clean AlienGui.cpp, SimulationParametersLayerWidget.cpp and
SimulationParametersMainWindow.h received narrow edits without whole-file reflow.
Whitespace checks pass. Detailed logs and XML stay in ignored `temp/l2b1`.

GitNexus upstream impact checks reported LOW for the main-window methods (callers
include processIntern, processMasterWidget and toolbar action paths). Common
wrappers and virtual getLocationName calls returned UNKNOWN; actual tree/toolbar
call sites and LocationWindow's virtual dispatch were inspected in source.
No missing graph edge was treated as proof of no callers. An index-only refresh
completed; the long-lived MCP server continued reporting its earlier snapshot,
so the CLI fallback was also used. Final detect_changes returned 22 changed
symbols and the Toolbar -> Scale process, risk medium, without partial/truncated
flags. This graph result is supplemented by runtime tests and the baseline
contracts; line-based symbol attribution and absent GUI flows are not treated as
proof that other presentation paths are unaffected.

## User-data protection and visual acceptance

No real GUI window or user simulation was opened by the agent. Tests use
SettingsTestScope's process-local registry redirection. Real profile exports
before/after have identical SHA-256:
`7145C865FB4B43BEBE5E6F9DA201027AF081D9247DD81838998140A2564C0F55`.
The previous output autosave was retained in the archived tree and copied back
to the fresh normal output tree, with identical SHA-256:
`F7E34AC256686C8CC22D3AB8780937992A83C900CF6C236991089D3F477C84F6`.
The isolated review directory excludes that autosave.

A local review runner was relinked against the final production libraries. It
wraps the original application main with SettingsTestScope, disables save-on-exit,
and starts with a fresh temporary profile. Its profile-only self-check passed
without opening the GUI. This is an ignored local acceptance aid, not a release.

```powershell
& 'E:\Projects\ALIEN\temp\l2b1\review\Launch-L2B1.ps1'
& 'E:\Projects\ALIEN\temp\l2b1\review\Launch-L2B1.ps1' -Scale 1.5
```

The user reported successful visual acceptance of branch HEAD
`258f6589943cebf72f86c4ab24108a075eb663a2` before integration. The acceptance
checklist is retained below for reference:

1. In the isolated runner create a small disposable simulation, open Parameters,
   and switch Settings -> Language EN -> RU -> EN. Check all three section captions,
   collapsed states, selected row and an edited parameter value.
2. Add a layer and radiation source; give them mixed user names. Resize, reorder
   and hide table columns; switch language and verify widths, order and selection.
   Check the camera-position button and unchanged numeric formatting.
3. Exercise copy, paste, reference paste, clone, delete and move buttons. Narrow
   the window to expose overflow and exercise the same available actions there.
4. Detach base/layer/source windows, move them, rename the layer and switch language.
   Confirm captions update while the same windows retain position and state.
5. Type an English specification filter; check the localized placeholder and active
   filter heading while preserving existing search behavior.
6. Open/cancel parameter file pickers. Save/load only disposable settings under
   `E:\Projects\ALIEN\temp\l2b1\review`; verify translated errors for invalid paths
   and reference mismatch. Inspect long Russian tooltips and captions at 1x/1.5x/2x.

## Limits and L2-B2 boundary

Real renderer/layout quality, DPI clipping, full simulation-backed selection/edit
round trips, layer/source rename lifecycle, successful file I/O and maximum-count
workflows are outside automated coverage; user visual acceptance is recorded
above. The headless fixture directly submits the actual
table and toolbar without initializing a simulation facade; it does not pretend
to run the whole simulation-dependent processIntern or mutate real engine data.

As in L2-A, ImFileDialog captures ALIEN-owned translated titles and filter text
when opened. Reopen it after switching language to refresh these captions. Its
own buttons, navigation, All Files and overwrite UI remain third-party English.
The isolated runner uses a fresh profile per run and does not prove restart
persistence; existing service tests cover language/settings persistence.

Parameter specifications, expert parameter names/help, specification groups,
parameter enum labels and a localized GUI/filter adapter are left entirely to
L2-B2. English parameter searching remains unchanged. No generalized formatter,
Unicode search overhaul or localization of unrelated windows is included.

## Language-selector placement correction

Following user review, the language selector was removed from New simulation,
Save picture and Display settings. Its only production placement is now the
Settings menu. Unused includes were removed; dialog actions, dimensions, widget
IDs and localization remain unchanged. Switch language before opening a modal;
there is no language selector inside a modal. Earlier L1/L2-A descriptions of
additional selectors are historical and are superseded by this placement rule.

The required agent Release build and all 19 headless GUI tests passed. All ten
Python contracts passed (six L2-A and four L2-B1), including a new check that
rejects any production selector placement outside MainLoopController's Settings
menu. The isolated L2-B1 review runner was relinked against the updated libraries.
No shared C++ header, engine or CUDA code changed; the earlier CPU regression
results remain applicable. GitNexus impact returned UNKNOWN for the three virtual
processIntern methods; their AlienDialog dispatch and actual call sites were
checked in source. Diff/format checks passed; the already non-clean new-simulation
file received only narrow removals. No user GUI or simulation was opened.

## Pre-integration evidence reconciliation

The integration audit checked the stored GoogleTest XML against the corresponding
logs. The correct total is **528 passed**, not 628: 41 + 247 + 2 + 40 + 38 + 141
+ 19. PersisterTests additionally has one disabled test, which is not counted as
passed. The incorrect earlier total was an arithmetic reporting error; no suite
was missing or failing.

All seven CPU/headless suites were rerun before integration and again passed
528 tests with zero failures and one disabled test. Fresh XML and logs are stored
in ignored `temp/l2b1/integration`. Both Python contract programs were rerun:
six L2-A checks and four L2-B1 checks passed, **10 total**. Stored final build,
GUI-test and review-runner build exit codes were all zero. This documentation-only
correction requires no rebuild or GPU/stress test.

The complete branch diff against main was reviewed, and whitespace/protected-path
checks passed. GitNexus compare analysis reported 27 changed symbols and three
presentation execution flows at medium risk, without partial/truncated flags.
No technical integration blocker was found. The local and origin branch heads
matched the user-accepted SHA; main remained an ancestor, allowing fast-forward
integration. No program code was changed by this reconciliation.
