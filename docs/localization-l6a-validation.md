# L6-A validation: local dialogs and application messages

Date: 2026-10-23 (Europe/Moscow).
Branch: `feat/ru-localization-l6a`.
Baseline: `1e6438ff208166045575d1d08af102e8807de939` (`main`).

## Delivered components

| Component | Result |
| --- | --- |
| `AboutDialog` | Localized title, version/attribution text and OK; attribution/product names retained; wrapped body |
| `NetworkSettingsDialog` | Localized title, address/active-instance controls, actions and privacy help; measured label space includes accessory controls |
| `AutosaveWindow` | Localized toolbar, confirmation, table headers, queue/progress/error states, settings, mode display values, directory chooser caption and status; wrapped error/status text |
| `AutosaveController` | Localized create/delete/cleanup notifications and presentation of peak-task errors |
| `LogWindow` | Localized title, timestamp/message headers and verbose toggle; diagnostic payloads untouched |
| `FileTransferController` | Localized operation overlays and callback-produced load/save error explanations; raw diagnostics passed as literal arguments |
| `MainLoopController` | Localized startup failure, save-on-exit notification and version caption; Russian version uses the Cyrillic-capable font and measured centering |
| `HelpStrings` | Network active-instance help now stores its localization key; account/browser/MCP help remains outside this stage |
| `LocalDialogPresentation` | Small GUI-only adapter for display-time error prose and literal diagnostic arguments |
| `AlienGui::InputTextParameters` | Optional folder chooser caption; original default and all handlers remain unchanged |

Both catalogs now contain **1,275 keys**, including **43 additive `localui.*`
entries**. All 1,232 previous entries are unchanged. Existing common/menu keys
are reused. The terminology, audit and roadmap documents are unchanged.
L6-B, L6-C and L6-D were not implemented.

## Producer-to-display audit and language policy

- Simulation open: `FileTransferController::onOpenSimulation` emits the loading
  overlay, then schedules the existing Persister read. The completion callback
  still calls `NewSimulationService::loadSimulation` and displays the original
  UTF-8 filename. Failures now use a translated operation explanation with the
  original diagnostic argument. Save callbacks use the corresponding save
  explanation. Scheduling, callbacks, filenames and error handling are unchanged.
- `NewSimulationService` also returns errors to `McpController` and
  `NetworkTransferController`. Its canonical fallback and external `what()` text
  therefore remain unchanged in the service. The local GUI adapter recognizes
  only the service's exact owned fallback, `Failed to load simulation.`, and
  displays its catalog equivalent. Other diagnostic text is retained verbatim.
- `PictureGuiService` is shared by `SavePictureDialog`, resource previews and
  MCP image export. Its resolution/scaling/encoding/write exceptions and preview
  log entries are canonical diagnostics, not independently displayed dialog
  prose. The service remains byte-identical. `SavePictureDialog` already catches
  the failure, writes the diagnostic to the log and displays the L2 localized
  picture-save error. That path is retained and exercised again. No image
  encoding, I/O, exception type or machine-facing error string was changed.
- Autosave creation/deletion/cleanup overlays resolve at emission. Peak-task
  errors pass their original ordered diagnostics to the display adapter. Failed
  savepoint rows retain their existing error-state behavior; no new error
  retrieval, recovery, retry or success notification was introduced.
- Captions, help, table headers, confirmation dialogs and owned generic error
  prose resolve **at display time**. An open message can switch languages while
  keeping its arguments, identity and action callback.
- Overlay notifications resolve **at event emission** and remain in that language
  until replaced. Filenames displayed as successful-operation notifications are
  always original data. Log records remain stored event-time text, including
  diagnostics and the existing technical message-dialog log prefix.
- Vendor file chooser titles/filters retain the existing **open-time** policy.
  Switching languages does not reopen the chooser or reset its navigation. The
  autosave chooser receives a localized caption when opened; its canonical
  `GenericFileDialog` ID, path and callback remain unchanged. Vendor controls
  remain part of L8-B.

The translated network help retains the original 25-minute in-memory hash claim.
It adds no assertion about anonymity, encryption, security or stronger privacy.
Login/security/account help is unchanged.

## Identity, state and preservation

Original window names, settings nodes, field names, tree scopes, table names,
row IDs and toolbar IDs are retained. Table column captions append their
canonical `###` suffixes and continue through native `TableHeadersRow`, retaining
column-index header scopes, native clipping/tooltips and context menus. Columns
can still be resized/reordered/hidden. Mode values remain canonical English
internally; only display values are translated.

No autosave directory, filename, request/sender ID, state enum, snapshot/peak
identifier, timestamp or numeric format, settings key, serialized field or file
filter grammar changed. Existing autosave entries and user-authored names are
not translated. The rendering test writes an isolated fixture table, then checks
its bytes and entry fields across all language/scale combinations.

About/network default dimensions and autosave's default dimensions were enlarged;
loaded user geometry remains authoritative. Network labels reserve accessory
space. About and autosave error/status text wrap; generic errors retain their
existing scrollable wrapped message child. Autosave reserves room for two status
lines. Very narrow user-resized windows and long table cell values retain native
scrolling/clipping and tooltips.

## Automated validation

Windows Release: `build-windows-ninja.bat agent Release 16` — **passed**.
Only `build-agent` was used. No CUDA struct, kernel or constant-memory header
changed; a clean CUDA rebuild was not required. Logs/XML/graph and formatting
records are retained locally under `temp/l6a/` and are not committed.

| Suite | Passed | Other results |
| --- | ---: | --- |
| BaseTests | 41 | 0 failures |
| DataTests | 247 | 0 failures |
| EngineInterfaceTests | 2 | 0 failures |
| McpToolsTests | 40 | 0 failures |
| NetworkTests | 38 | 0 failures |
| PersisterTests | 142 | 1 existing disabled test; 0 failures |
| EngineTests | 2,196 | 2 skipped and 2 existing disabled tests; 0 failures |
| GuiLocalizationTests | 142 | Includes full L1–L5 regression and four new L6-A runtime tests; 0 failures |
| Python localization contracts | 93 | Includes six new L6-A contracts; 0 failures |

The full GPU suite completed in approximately **90 seconds**, without
cancellation. Skipped cases remain
`ConstructorTests_ProvideEnergy_Separation.provideEnergy_insufficientEnergy/2`
and `/5`, matching the preceding stage. GUI tests were repeated after the final
native-table-header adjustment; all 142 passed.

New production ImGui tests cover **EN/RU/EN at 100/150/200%**:

1. Open About/network dialogs retain actual window instances, position/size,
   open state, edited server address, active input ID/input state and checkbox
   value. Content bounds fit the tested sizes; no ImGui frame errors occur.
2. Autosave/log panels retain actual table instances, column widths, hidden
   autosave column, entry selection, settings, all four savepoint states and raw
   log contents. The isolated savepoint file remains byte-identical.
3. Long generic load/save/error messages retain exact diagnostic arguments
   containing Cyrillic paths, `%`, braces and `###`. Their actual rendered text
   and open dialog identity survive language switching. Already-emitted overlays
   keep their original language; filename notifications remain data.
4. PNG output and shared picture exceptions remain identical across languages;
   the existing localized picture error dialog remains functional.

Source contracts additionally check callback producers, request payloads,
settings/path/ID literals, all previous catalog entries, named-argument parity,
shared MCP-facing service preservation, the opt-in folder-caption adapter and
stage boundaries. Historical checks exclude only the additive namespace and
normalize only the new optional folder-caption site. The L5-C historical scope
check is fixed to its accepted implementation commit; L6-A has its own current
scope check. Earlier computation and data contracts remain active.

`git diff --check` passes. clang-format **19.1.5** was used for eligible modified
C++ files clean at HEAD and the new header. `AlienGui.cpp` and
`FileTransferController.cpp` already have formatting differences at HEAD, so
whole-file formatting was not applied to them.

## Graph analysis

Upstream impact covered dialogs, autosave, file-transfer/picture/load services,
startup methods, shared input and test fixtures. Resolved display/control methods
reported LOW; controller/service classes reported MEDIUM. Callers/processes
include `AutosaveWindow::processIntern`,
`AutosaveController::processAutomaticSavepoints`/`process`,
`FileTransferController::onOpenSimulationDialog` and main-loop startup/fade/exit
flows. UNKNOWN virtual-entry/constructor/test-fixture results were checked against
registration, menus, callbacks and actual test execution. Empty caller sets were
not treated as evidence of non-use.

`GenericMessageDialog` class impact reported **HIGH** (29 direct consumers); this
was reported before its header edit. That edit only grants the test fixture
access to stored diagnostic arguments; production behavior/API is unchanged.

The graph was refreshed with `analyze --index-only`. Full-text/BM25 index creation
still fails, as recorded in L5-C; graph analysis succeeds. Final all-scope analysis
uses the CLI LocalBackend with an expanded result limit, avoiding the running
MCP server's stale metadata. Its complete structured result is retained in
`temp/l6a/graph-complete.json`: **42 changed files, 99 listed changed symbols,
five affected processes, aggregate MEDIUM risk**. Neither `partial` nor
`truncated` is set.

## Limitations and acceptance evidence

This is automated ImGui/font/layout execution and source/callback review,
not interactive Vulkan desktop or screenshot-based visual acceptance. Final
desktop composition, hovered tooltip appearance, vendor controls and unusually
narrow saved windows still need ordinary-use visual verification. No separate
manual-acceptance build was prepared.

Failure dialogs were exercised with representative ordered Persister diagnostics;
real asynchronous disk/network failures were not injected through a running GUI.
Successful persistence is covered by Persister/engine suites and the isolated
savepoint fixture; no user world or autosave was opened or overwritten. Existing
picture/preview/MCP diagnostic text remains canonical English by design, after
checking its consumers. This report does not claim localization of those raw
technical payloads or completion of L8-B.

`main` remains at the requested baseline. Only the feature branch is to be
committed/pushed. `AGENTS.md` retains skip-worktree and SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`.
User data, dependencies and pinned vcpkg are unchanged. No CodeQL, submodule update
or IDE-tree build was run. Manual visual acceptance is not claimed.
