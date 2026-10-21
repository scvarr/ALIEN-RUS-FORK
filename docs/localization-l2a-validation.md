# L2-A localization validation

Date: 2026-10-21 (Europe/Moscow).

## Repository baseline and scope

Branch: `feat/ru-localization-l2a`, created from local `main` at
`e7f1eaa0ed168937c68469b67ad2fc67ad612cd5`. An `origin` fetch confirmed that
`origin/main` had the same SHA. The existing unstaged GitNexus appendix in
`AGENTS.md` predates this work and is deliberately excluded from the commit.
No upstream synchronization, published-history rewrite, merge, push, dependency
update, submodule update, CodeQL scan, or binary release was performed.

The audit, current L1 validation (including its startup and language-popup fixes),
fork README and repository instructions were read. This change extends L1's
existing catalogs, translation service, fonts, language selector and identity
conventions. It adds no localization system.

## Translated components and counts

**64 new semantic keys**, with 64 English and 64 Russian values; both catalogs now
contain **89 keys**. Counts describe catalog messages, including reused messages,
rather than physical source lines or every rendered occurrence.

- Seven additional top-level menus: Simulation, Network, Windows, Editor, View,
  Tools and Help. Settings retains its L1 implementation. All eight menus have
  language-independent IDs.
- All 37 directly contained menu commands outside Settings: simulation creation,
  opening, saving, picture saving, Run/Pause; browser, login/logout, simulation
  and genome upload, account deletion; seven window toggles; genome editor,
  object-edit mode, three inspection commands, inspector closure, deselection,
  copying, pasting and deletion; cell/message overlays, interface/simulation
  visibility, console mode; mass operations, image converter; About and
  Documentation. The six existing Settings commands retain their L1 keys.
- Run/Pause overlay messages issued by these menu/keyboard commands.
- `NewSimulationDialog`: title, project name, dimensions, energy, parameter
  adoption, random-name tooltip, OK and Cancel.
- `ExitDialog`: title, confirmation, OK and Cancel.
- `SavePictureDialog`: title, dimensions, adoption of viewport size, OK, Cancel
  and the directly handled save error. Technical exception details are retained
  in the existing log; the error modal displays a complete translated message.
- ALIEN-owned open/save simulation file-picker titles and simulation filter
  description, plus picture-picker title and PNG filter description. The filter
  patterns ` (*.sim){.sim},.*` and ` (*.png){.png},.*` are unchanged.
- The existing language selector is available inside all three basic dialogs,
  allowing language changes while a modal blocks the main menu.

Dialog defaults and label space are enlarged for Russian. Existing icons,
shortcuts, selected/disabled conditions, callbacks and menu-closing behavior are
retained. Simulation/CUDA code, protocols, persistence formats and permanent
GlobalSettings keys are unchanged.

## Identity and compatibility

Menu captions use `tr(key) + "###stable-id"`, preserving both the menu button and
the custom menu window identity (including its existing `##1` suffix).
Every command has its own semantic ID, including commands with matching visible
captions. Menu-open booleans remain the existing controller members.

`InputIntParameters`, `InputFloatParameters` and `InputTextParameters` gain an
optional `.label()`, following L1's existing wrappers. `.name()` still defines
legacy input/revert scopes, while `.label()` only affects rendered text. The
random-name tooltip is an optional presentation parameter with the original
English default for existing callers. Other windows are not migrated.

The three dialogs reuse L1's `setTitleKey()` modal support and retain original
content scopes. Native buttons use constant `###` suffixes. The existing in-memory
INI-header migration now also covers `New simulation`, `Exit` and `Save picture`;
positions and sizes are retained under the same `gui.imgui settings` storage key.
No menu-open state is newly persisted across application restarts.

`GenericMessageDialog::localizedInformation()` is an additive counterpart of L1's
localized Yes/No API. It keeps title/message keys while open and localizes OK only
in this mode. Existing information and Yes/No callers retain their behavior.
`AlienDialog` grants the headless test fixture access to its modal wrapper, so
production dialog contents can be tested without starting the simulation engine.

ImFileDialog already appends `###GenericFileDialog` to the caller's title. ALIEN
passes translated display titles and preserves that internal key. File-picker
titles/filters are captured when opened; no third-party code is modified.

## Build and automatic results

Required command: `build-windows-ninja.bat agent`, Release, successful.
The first full build completed all **411 steps** with MSVC 19.44, CUDA 13.2 and
the existing pinned dependencies. The final required-script build also passed.

To avoid L1's known Ninja/MSVC header-tracking defect, the previous `build-agent`
was moved to `temp/l2a-build-before-clean-20261021`. A new agent tree reused only
`vcpkg_installed`; no compiled ALIEN objects or libraries were reused. IDE-owned
build trees were untouched. All consumers of the changed common headers were
therefore compiled with the new layouts.

One intermediate retry overlapped the running GPU suite and could not relink its
locked executable (`LNK1104: Release/EngineTests.exe`). The suite completed without
interruption. A subsequent build after its completion passed; final suites were
then run against the resulting binaries without a concurrent build. An initial
new GUI test incorrectly activated an input before the modal's placement settled;
the corrected test waits for layout and uses actual mouse/keyboard events.

| Suite | Passed | Failed | Skipped | Disabled |
| --- | ---: | ---: | ---: | ---: |
| BaseTests | 41 | 0 | 0 | 0 |
| DataTests | 247 | 0 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 | 0 |
| PersisterTests | 141 | 0 | 0 | 1 |
| EngineTests | 2196 | 0 | 2 | 2 |
| GuiLocalizationTests | 14 | 0 | 0 | 0 |
| **Total** | **2719** | **0** | **2** | **3** |

Relative to the latest L1 result (40 BaseTests and 9 GUI tests), this preserves all
existing passes, skips and disabled tests and adds one service test and five GUI
tests. The final GPU run passed in **81.842 seconds**; the earlier run also passed,
with 2196 passes in 88.664 seconds. The five Python checks are additional to the
2719 C++ test passes above.

The existing L1 tests remain, covering malformed/missing catalogs, duplicate keys
(including escaped duplicates), invalid UTF-8, deferred switching, persistence,
fonts, startup atlas readiness and actual language-popup mouse interaction.
Catalog completeness now checks all 89 keys and the additional source files.
The new fallback test verifies every shipped English message with both external
catalogs absent, including all L2-A messages.

New headless GUI tests check actual basic dialog windows through EN → RU → EN,
retaining an active input containing `73` in the new-simulation and picture dialogs;
legacy IDs and values of the three extended input types; all eight menu windows,
open-state retention and distinct IDs for identical visible captions; the localized
picture-error modal; and migration of all three legacy dialog INI sections.
The simulation-dependent dialog initialization/action is intentionally not invoked
by these headless tests; creating a real simulation remains manual acceptance.

Five additional standard-library Python source-contract tests pass:

```powershell
python source/Gui/Impl/Tests/check_localization_l2a.py
```

They verify strict UTF-8/duplicate-free catalog parity and referenced keys, all
eight menu IDs and 43 unique command IDs, translated basic-dialog captions and
tooltip, unchanged filter grammar/internal file-dialog key, and a baseline digest
of the entire menu function with only presentation captions normalized. The
digest compares callbacks, icon macros, shortcuts, conditions, selections and
control flow against accepted L1 commit `e7f1eaa0e`.

clang-format **19.1.5** was applied only to modified files already clean at HEAD.
`AlienGui.cpp`, `FileTransferController.cpp` and `NewSimulationDialog.cpp` were
already non-clean and received only localized edits, without whole-file reflow.
Whitespace checks pass.

GitNexus impact checks reported LOW for the menu function and the resolved
picture-save caller. Several singleton/virtual calls returned UNKNOWN; actual
call sites and lifecycle registration were inspected in source. Common GUI
headers reported HIGH/CRITICAL fan-out (23, 29 and 70 direct consumers); these
warnings were reported and are covered by the fresh build. Change analysis
completed without `partial` or `truncated` flags.
Its empty affected-process list is not treated as evidence that GUI paths are
unaffected; source contracts and runtime headless tests supply that evidence.

Local logs, XML, formatting results and private backups are under `temp/l2a/`,
outside version control. No credentials or profile contents are committed.

## User-data protection and manual acceptance

**User visual acceptance is incomplete. No real GUI window was launched by the
agent.** The application and a local isolated review runner are ready.

The real `HKCU\\SOFTWARE\\alien` profile was exported before and after testing.
Both exports have SHA-256
`297BC4E4C0060462C461E32BC9F6ECB3B29EA6090292BF5080798EAD9ECB1196`.
Existing autosave contents were hashed and verified unchanged. Tests use L1's
process-specific registry redirection; the real profile was not restored or
overwritten because it did not change.

For safe local review:

```powershell
& 'E:\Projects\ALIEN\temp\l2a\review\Launch-L2A.ps1'
# Repeat with a different startup interface scale:
& 'E:\Projects\ALIEN\temp\l2a\review\Launch-L2A.ps1' -Scale 1.5
```

The ignored local `alien-l2a-review.exe` links the same production libraries with
the original application main wrapped by L1's `SettingsTestScope`. This redirects
HKCU only inside that process to a new temporary test profile. A profile-only
self-check passed without opening the GUI, and the real profile export remained
identical. The review working directory contains copied DLLs/resources, excludes
existing autosaves and starts with save-on-exit disabled. Each run starts with
fresh settings; this runner does not validate persistence across restarts.
It is a local test aid, not a distributable binary release or a tracked build change.

Manual checklist, still pending:

1. Switch all eight menus EN → RU → EN; inspect captions, icons, checks, disabled
   entries, popup focus and text fit.
2. Check Ctrl+N/O/S, Space, Alt window/editor/view/tool shortcuts, Escape, Delete
   and F1. Cancel file/network dialogs; do not submit account operations.
3. Create a small test simulation (for example 100 × 100); Run/Pause it and check
   translated overlay messages.
4. Open/cancel New simulation, Exit and Save picture. Edit dimensions/project name,
   switch language using each dialog's selector, and confirm values remain.
5. Save a small PNG only to
   `E:\Projects\ALIEN\temp\l2a\review\l2a-picture.png`; inspect the result.
   Exercise an invalid test path and check the translated error and log details.
6. Repeat layout inspection at 1x and 1.5x/2x (as screen size permits), including
   long Russian network commands and parameter-adoption text.

Use only test files under the review directory. Do not open or overwrite real
user simulations. Main build output remains `build-agent/Release/alien.exe`;
the isolated review runner above is preferred for acceptance with protected data.

## Limits and proposed L2-B

ImFileDialog's own buttons, navigation, column captions, overwrite prompts and
`All Files` label remain English, as explicitly deferred. Its captions/filters do
not update live while open. No third-party identity/filter logic is changed.

Network dialogs, window interiors (including Autosave/MCP), Resize world,
specialized editors/inspectors, About contents, documentation, console contents,
loading/startup messages, genome file operations and raw engine/persistence
diagnostics remain outside this slice. Generated project names and filenames are
data rather than translated messages. Existing small persisted window sizes can
still require user resizing; Russian default sizes are larger. Real-window fit,
GPU rendering, simulation creation and PNG export remain pending manual checks.

Proposed L2-B: basic temporal/spatial-control and resize-world dialogs, followed by
the explicitly selected window interiors; plan network/account dialogs as a
separate bounded slice. Keep third-party ImFileDialog localization separately
scoped, retain stable IDs and extend the same catalog/test infrastructure.
No L2-B implementation is included here.
