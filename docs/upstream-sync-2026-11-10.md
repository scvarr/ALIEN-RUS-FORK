# Upstream synchronization — 2026-11-10

Status: **COMPLETE — technical integration verified; manual user acceptance pending.** This is not approval to merge to main.

## Scope and revisions

- Update worktree: E:\Projects\ALIEN-upstream, branch sync/upstream-2026-11-10.
- Fork baseline d1365091a; original fork main f86aa220863167b520ded9a5162c072b309bf836.
- Equivalent upstream baseline c855178dc; target e12e235d59a899f8f2b7a2ab8f8e084e6dab1e28.
- Existing bridge f395e3530ba8331de105069efc63d91ffe5faf79 links fork main and upstream baseline without changing the fork tree. History linking was not repeated. The existing merge was continued without reset or history rewriting.
- The primary worktree E:\Projects\ALIEN and its independent fix/ru-localization-quality work were not edited. Main was not changed. No work from the independent translation-quality branch was imported.

## Conflict resolutions

| File | Working-tree resolution |
| --- | --- |
| CMakeLists.txt | Keep GuiLocalizationTests and VideoTests; search both CUDA bin and bin/x64, excluding directories, with missing/ambiguous DLL diagnostics. |
| README.md | Preserve the full Russian fork README from stage 2. Refresh README.upstream.md from target upstream. |
| BrowserData.cpp | Keep the new encapsulated workspace API and selection maintenance; retain BrowserMetadataSearch in the new tree-building implementation. |
| BrowserGalleryWidget.cpp | Use the new filter getter with fork metadata search. |
| BrowserUserListWidget.cpp | Keep multiple badges and active/24-hour counts, Russian labels, and stable tab ID. |
| BrowserWindow.cpp | Adapt translated controls to getters/setters and removeRawTOs, retaining localized delete confirmation. |
| GenericMessageDialog.cpp | Keep the new action button and localized OK handling. |
| GenomeTabWidget.cpp | Keep translated labels/stable IDs and migrate the gene network badge to the new badge vector. |
| HelpStrings.h | Keep fork localization key and new upstream simulator-count explanation. Corresponding old catalog text still needs updating. |
| MainLoopController.cpp | Keep Russian menus and stable IDs; retain recorder entry and recorder guard on console mode. |
| NetworkTransferController.cpp | Combine includes; retain the upstream refresh-on-delete-error path with localized error presentation. New upload confirmation still needs localization. |
| StyleService.cpp | Keep Cyrillic-capable monospace font and merge the new video forward icon into it. |
| UploadSimulationDialog.cpp | Retain new public/private actions, validation, suggested names, per-type/folder description memory and responsive height; localize new controls and preserve stable input ID. |


These resolutions retain both application functionality and Russian presentation; they are validated by compilation and the runtime suites below, not merely by removing markers.

## Automatically merged overlaps

All 20 overlapping files were inspected for API compatibility and preservation of fork presentation behavior:

| Files | Contract checked |
| --- | --- |
| Base/CMakeLists.txt, Base/Tests/CMakeLists.txt | CrashLogger/Callstack sources and CallstackTests added alongside fork localization sources/tests. |
| Base/GlobalSettings.cpp | Upstream removes destructor-time Linux settings save; fork localization initialization remains. Explicit settings persistence paths require Linux acceptance. |
| Gui/Impl/AlienGui.cpp, AlienGui.h | Vector badge API retained; fork stable tab IDs and separate visible labels remain. |
| BrowserController.cpp | Refresh populates data through setters; localized refresh errors retained. |
| BrowserHelper.cpp | New leaf factory preserves raw resource identity and folder parsing; translated download presentation retained. |
| BrowserTableWidget.cpp, BrowserTableWidget.h | Sorting and collapse mutations delegated to data setters; pending collapse applied after table traversal; stable translated columns and reaction actions retained. |
| BrowserWindow.h | Resource reveal and const cache accessor match the implementation. |
| Gui/Impl/CMakeLists.txt | Recorder sources and Video link added alongside fork localization targets. |
| EditorController.cpp | Velocity now passed as a vector, matching ShallowUpdateSelectionData. |
| FileTransferController.cpp | Camera-flight dialogs preserve extensions and JSON suffix rules; captions/errors/overlays localized. |
| GenericMessageDialog.h | informationWithAction declaration and optional action text match implementation and retain localized APIs. |
| MainWindow.cpp | Recorder singleton setup added without removing localization/font setup. |
| NeuralNetEditorWidget.cpp | Weight sliders use the upstream model's connection-weight bounds. |
| OverlayController.h | Optional persistent recording status and rendering helper declarations match implementation. |
| SimulationInteractionController.cpp, SimulationInteractionController.h | Separate optional left/right click modes and guarded panning match; selection consumption no longer leaks stale click modes. |
| UploadSimulationDialog.h | Explicit workspace argument and type/folder description cache replace the removed toggle and old cache fields. |


## Upstream functionality

Integrated camera flights, video recording/scheduling and the Video component; browser encapsulation and selection preservation; public/private upload actions and confirmation navigation; active/last-day simulator counts including CLI reporting; crash logs/call stacks; installer/CI changes; CUDA geometry/editing/constructor changes and additional engine tests; Vulkan/ray-tracing changes. The new rendering parameters have typed localization bindings.

## Localization review

Reviewed all 80 new English/Russian pairs and their production bindings. Catalogs contain 1593 entries per language. Existing fork entries are preserved except the intentional correction of the network privacy explanation to 24-hour in-memory hash retention after the last report. Russian README is preserved, README.upstream.md is updated from the target.

Reviewed recorder controls, flight dialogs, tooltips, status/count formatting, upload notifications, browser counts and visualization parameters. Control labels are separated from programmatic names/IDs. Cyrillic monospace fonts retain the merged video icon. Six Russian chapters were updated: energy, files, generative-art, keyboard-and-mouse, simulation-parameters and user-interface. Numeric/channel/shortcut checking found and fixed a missing video example and an extra repeated shortcut. UTF-8 source/catalog validation and real documentation rendering/navigation pass.

## Current and historical Python checks

Mandatory current command: `python source/Gui/Impl/Tests/check_current_localization.py`.

The explicit method-level decision register is upstream-sync-2026-11-10-test-contracts.json, with a readable table in upstream-sync-2026-11-10-current-validation.md. It accounts for every one of the 125 historical test methods across all 26 scripts:

- A: current invariants are executed directly.
- B: immutable stage snapshots/source hashes/old scope boundaries remain historical evidence; they do not require the updated application to equal an obsolete version.
- A+B: current catalog/behavior requirements are covered by the specified current Python contracts and stage-matched runtime C++ tests; obsolete exact stage identity is retained as historical evidence.

No historical failure is relabeled PASS and no test is disabled. The old complete-script outcomes (0/26 on the isolated fork baseline, 3/26 after earlier adaptations) remain recorded with exact output and classification. Their 23 failing script totals are not treated as 23 application regressions. The current suite runs 52 tests, including duplicate/key/placeholder/Unicode checks, preservation of every baseline catalog entry, actual new-key consumers, typed parameter bindings, stable control identities, flight serialization contracts and current numeric/shortcut inventories.

The old menu behavior digest was not changed: the test verifies the recorder action and recording guard explicitly, then requires the remaining menu behavior to match the original digest. Historical count-only checks were replaced with preservation of actual baseline keys. Model counts changed only where upstream adds one boolean and one float, and all labels/descriptions are still checked against typed model specifications.

## New runtime interaction checks

BrowserLocalizationTests adds four observed-behavior tests:

1. Public/private upload event handlers dispatch the requested visibility and payload; completion displays a confirmation/action; an invalid name schedules no request. Existing multilingual failure tests retain literal raw error details.
2. Refreshing data with replacement objects preserves selection by resource ID and rebinds it to the new object; removing the resource clears selection.
3. Recorder playback requires a path, enters playback with two waypoints and stops when the path is removed; invalid recording startup leaves recording inactive. Window visibility opens/closes through the real window API.
4. Console activation refuses to enter while recording state is active. Production now enforces this at the controller boundary as well as disabling the menu. The state-only fixture creates no encoder and claims no MP4 success.

Test facades implement the new capture/count APIs and fail on unexpected capture/network operations. No obsolete production API was reintroduced to satisfy tests.

## Build and validation

Tools: Visual Studio 2026 v18.11.3, MSVC 19.51.36260 (toolset directory 14.51.36231), CUDA 13.2.51, CMake 4.2.0-rc2, local Ninja package 1.13.2 and clang-format 19.1.5. No global PATH changes. Existing modified C++ files were not formatted wholesale because their HEAD versions are not formatting-clean.

The update worktree uses the exact vcpkg gitlink af752f21c9d79ba3df9cb0250ce2233933f58486, with vcpkg executable 2025-12-16-44bb3ce006467fc13ba37ca099f64077b8bbf84d. Partial-clone objects needed by historical port versions were fetched before building; no arbitrary dependency versions were substituted and git submodule update was not used.

A recursive build-directory deletion was rejected by automatic policy review (blocked by policy). Clean verification instead used Ninja's generated Release clean target, removing 441 generated files, followed by the standard `build-windows-ninja.bat agent 12`. This rebuilds all objects without touching IDE trees or deleting a worktree. The latest run follows the final console-guard changes.

Final C++ suite numbers are stored in upstream-sync-2026-11-10-cpp-results.json. All 11 suites are mandatory, including VideoTests and the full GPU EngineTests. Existing engine skips/disabled cases are reported rather than concealed. Build/test logs remain in build-agent-tools.

Packaging now refreshes documentation together with fonts/catalogs even when the application does not relink, fixing the stale-help defect exposed by runtime tests. The mandatory current Python suite and resource-rendering tests validate the deployed copies.

## Addressed GitNexus review

A separate ALIEN-UPSTREAM-SYNC index was rebuilt with --index-only (20592 nodes, 44524 edges at the last run); no instruction injection and no primary index rebuild. Local scan exclusions omit build outputs and external dependencies.

- BrowserWindow: HIGH, 17 dependent objects/15 direct imports. Reviewed browser data/table/gallery, refresh, deletion/visibility and upload navigation; tested replacement-object selection and multilingual UI state.
- RecorderController: MEDIUM, 7 impacted/5 direct imports. Reviewed main-loop sequencing, window/file-transfer integration, playback/recording scheduling and capture APIs. Video primitives/writer tests and controller-state tests cover automated scope; complete UI recording remains manual.
- NetworkTransferController: MEDIUM, 9 impacted/7 direct imports. Reviewed asynchronous payload capture, upload completion and delete-error refresh. Added upload/confirmation and preserved pending-task/error tests.
- Parameter localization key dispatch: LOW, three resolved direct callers, with additional unresolved receiver-typed sites. Reviewed typed references and exercised all visualization descriptions/labels via actual GUI tests.
- Geometry: resolved getNumRenderObjects by exact UID, inspected culling/kernel/helper calls and confirmed its SimulationCudaFacade caller in source. Interface/class lookup had UNKNOWN ambiguity; it was not treated as an all-clear. Full engine/interface suites validate the changed contracts.
- Console activation: UNKNOWN caller walk; text search confirms the main-menu caller. Added a controller-level guard and direct refusal test.

The focused unstaged change analysis observes 85 symbols/42 files/13 flows with HIGH risk and no capped listing at that checkpoint. The complete upstream merge analysis observes 1291 symbols/214 files/80 flows and CRITICAL aggregate risk, with an internal 1000-symbol listing cap. No attempt was made to patch that cap. C++ receiver/interface dispatch and flow enumeration remain incomplete, so absence of a graph finding is not proof of safety. Addressed subsystem review and runtime suites, rather than a false zero-impact claim, support technical acceptance.

## Protected state and remaining manual acceptance

AGENTS.md is the fork HEAD edition, with its original H index flag, and is excluded from the merge changes. .gitattributes, simulation files and the vcpkg gitlink are unchanged from fork HEAD. LFS pointer checks pass for both parents. No API/protocol/serialization keys were translated. Reviewed deletions are upstream's old CLI session implementation and two replaced ray-tracing shaders; no fork-only file was accidentally deleted.

The compact user checklist is upstream-sync-2026-11-10-manual-acceptance.md. Pending manual checks include complete camera/MP4 output, minimized recording, Cyrillic video overlays, layout/scaling and real-server visibility/navigation. Passing state tests does not establish successful UI MP4 encoding. User acceptance remains pending and this branch must not be merged to main on the basis of this technical report alone.

## Future synchronization procedure

Once this two-parent merge enters the accepted fork history, the histories already share their bridge; never link them again. In a separate update worktree/branch from accepted fork main, fetch upstream develop, record both revisions and verify merge-base. Run `git merge --no-commit --no-ff upstream/develop` once. Continue an existing MERGE_HEAD instead of repeating or resetting the merge. Compare base/ours/theirs, inspect automatic overlaps, preserve fork localization and protected instructions, update the current contract register, and run isolated graph/source review, clean agent Release build, all C++ suites, current Python checks, resource/LFS/dependency checks and manual acceptance. Stage explicit reviewed paths, create a normal two-parent merge, verify upstream as second parent and push only the update branch. Never force-push or modify main during preparation.


## Final technical result

The final clean Release rebuild completed all 442 build steps after the last production/test changes. All 11 C++ executables passed: 2929 tests ran, 2927 passed and 2 skipped by existing engine conditions; the engine also retains 2 previously disabled tests. GuiLocalizationTests passes 166/166, including the four added interaction tests; VideoTests passes 21/21. The mandatory current Python suite passes 52/52. Test details are in the C++ results JSON and the explicit current-validation register.

Final pointer checks for HEAD and MERGE_HEAD pass. Conflict-marker scan and git diff --check pass. The prepared merge excludes AGENTS.md changes, simulation/LFS attribute changes and dependency gitlink changes. Its parent requirements are f395e3530 and e12e235d5. Publication is limited to sync/upstream-2026-11-10; inspect that branch's Git history for the resulting merge SHA and remote tracking state. Main/user acceptance remains separate.
