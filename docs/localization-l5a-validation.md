# L5-A validation: object inspector localization

Date: 2026-10-23 (Europe/Moscow).
Branch: `feat/ru-localization-l5a`.
Baseline: `fe7269a5b909e8f6c36ff9c1db6a603d778aacac` (`main`).

## Delivered scope

Localized the energy-particle, solid-particle, fluid, free-cell, organism-cell
and organism inspection panels. This includes object IDs, connections, energy
forms, age, lifecycle states, constructors, every active cell type/mode,
lineage restrictions, neural activity/memory and genome actions.

`InspectionPresentation.h` reuses L4's cell types, modes, construction-energy
options, count formatting and attribute descriptions. It adds the remaining
47 inspector attribute descriptions without changing the canonical complete
`EntityAttributeHelp` table or the existing L4 adapters. Both catalogs contain
1,044 keys: 112 inspector keys added, all 932 previous entries preserved.
English attribute descriptions match the canonical strings exactly.

Controller limit errors use complete messages with named count arguments and
the existing deferred-language modal API. Already-open limit dialogs resolve
their language when displayed. The genome-inspection overlay uses the existing
one/few/many adapter, including 0, 11–14 and 21. Overlays retain their existing
emit-time language policy until the next message.

## Identity, behavior and layout

Canonical widget names, tree parents, dynamic connection/channel names, combo
option identities and gene options remain unchanged. User-authored genome/gene
names and numeric values are not translated. Display labels and combo display
values are resolved on each frame.

The inspector retains its exact original `generateTitle()` implementation and
window hash. Its localized title is drawn separately while native title-bar
close/collapse/drag behavior and IDs remain ImGui's responsibility. Merely adding
`###` would have changed the legacy root hash; an initial regression test caught
that approach and it was replaced before delivery. Native title text is hidden
only during `Begin`; title text and button glyphs are then drawn with the restored
text color. This narrowly uses the existing ImGui internal drawing helpers.

The only shared widget extension is an optional display `label` on `InputFloat2`.
It changes its visible caption while retaining its canonical input name, values,
format, read-only/default behavior and revert button. Callers without a label
retain their previous behavior.

Label widths are measured using the current font and UI scale. Initial inspector
widths and dynamic table column widths account for the translated labels; the
existing horizontal scrollbar remains available after manual narrowing.

No engine, object/genome data definitions, inspection selection/filtering,
validation, simulation updates or serialization algorithms were changed. No
L5-B production files were modified. The tests retain the existing inspector's
initialization of undersized neural arrays and normalization of unused high
memory-mask bits; test fixtures use a valid eight-channel mask.

## Automated validation

Windows Release command: `build-windows-ninja.bat agent Release 16`.
Only `build-agent` was configured/built. No `.cuh` change required a clean rebuild.
Build logs, test logs and GTest XML are retained under `temp/l5a/` (not committed).

| Suite | Passed | Other results |
| --- | ---: | --- |
| BaseTests | 41 | 0 failures |
| DataTests | 247 | 0 failures |
| EngineInterfaceTests | 2 | 0 failures |
| McpToolsTests | 40 | 0 failures |
| NetworkTests | 38 | 0 failures |
| PersisterTests | 142 | 1 existing disabled test |
| EngineTests | 2,196 | 2 skipped, 2 existing disabled tests; 0 failures |

The GPU suite completed in approximately 82 seconds. The skipped cases are
`ConstructorTests_ProvideEnergy_Separation.provideEnergy_insufficientEnergy/2`
and `/5`.

All 128 `GuiLocalizationTests` pass, including the complete L4 regression and
four added production ImGui tests. The added tests cover:

- Every object category and all cell types, active modes, lifecycle/detonator
  states and constructors in EN -> RU -> EN at 100/150/200%.
- Exact legacy window hashes, reuse of the same live window, unchanged object
  data and inspection registration; Russian label widths with actual fonts.
- User genome names, neural state values, complete wrapped descriptions, option
  order, the `Free` exception and count forms for 0/1/2/5/11/14/21.
- Focused `InputFloat2` identity/values across languages and scales.
- Already-open object/genome/organism limit dialogs, their full messages,
  translated title, unchanged modal instance and count argument at every scale.

All 79 Python localization contract checks pass, including 27 L4 checks and
3 new L5-A checks. Historical catalog checks exclude the new inspector namespace
from their original stage counts; existing key preservation is still checked.
L4's obsolete inspector-freeze assertion is replaced by L5-A's comparison against
the requested baseline. The shared-widget legacy adapter removes only the new
optional caption site. L5-A checks canonical names/options, formats/ranges,
object mutations, factories, complete attribute coverage, catalog/argument parity,
shared fallback behavior and unchanged L5-B/data/serialization sources.

Formatting uses Visual Studio's clang-format 19.1.5. Only modified files already
clean at HEAD are eligible for whole-file formatting. `AlienGui.cpp` has existing
formatting differences; its single caption expression is changed without whole-file
formatting. The GUI test file was clean at HEAD; clang-format was applied after adding the new tests.
`git diff --check` passes.

## Graph analysis

Upstream analysis covered the changed inspector helpers/panels and controller
commands. Most resolved inspector methods report LOW risk; the properties helper
reports MEDIUM with seven direct panel callers. Relevant processes include
`process -> generateTitle`, `table -> processPropertiesSubNode` and `table -> get`.
UNKNOWN command/`InputFloat2` results were checked by text search: controller
commands are called by `MainLoopController` and `SelectionWidget`; `InputFloat2`
has four production call sites, all in the inspector. Test methods are invoked
by GTest/unittest registration rather than resolved graph callers.

The index was refreshed to the requested baseline with
`node .gitnexus/run.cjs analyze --index-only`. Graph analysis completed, but the
analyzer reported a full-text-index checkpoint/rename failure. Full-text search
is degraded; this does not make empty graph results an all-clear. CLI impact
analysis was also used against the refreshed on-disk graph because the running
MCP server continued to advertise its old cached commit metadata.

Final `detect_changes({scope: "all"})` returned 29 changed files, all 72 changed
symbols and four affected processes, with MEDIUM risk. The result is complete:
72 listed symbols equal the reported count, with neither `partial` nor `truncated`
set. The CLI check was also rerun with `--limit 500`. Broad diff hunks include
adjacent factory/inspection methods in the graph report; the source contracts
confirm that their algorithms remain unchanged.

## Limitations and manual acceptance

The required 100/150/200% checks are automated ImGui/font/layout checks.
No interactive Vulkan desktop session or screenshot-based visual acceptance was
performed. Desktop visual acceptance remains advisable for title-bar appearance,
long hovered tooltips, manually narrow windows and live simulation refresh.
No separate isolated manual-acceptance build was prepared, as allowed by the task.

The service construction-energy option `Free` remains in English, as explicitly
required by the approved terminology document until a translation is agreed.
Its enum/value/availability is unchanged; the other two options reuse L4.
ImGui/vendor navigation/debug views may still show canonical English window names;
vendor UI remains the separately planned L8-B scope.

## Preservation

At implementation delivery, `main` remained at the requested baseline and only
the feature branch was committed and pushed. `AGENTS.md` retains skip-worktree and SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`.
No user worlds, genomes, autosaves, dependency sources or pinned-vcpkg state were
changed. GUI tests use the existing isolated `SettingsTestScope`; no production
GUI was launched. No CodeQL scan, submodule update or IDE-tree build was run.

## Acceptance and L5-A closure

Date: 2026-10-23 (Europe/Moscow).

The user accepted L5-A implementation
`c766b7fc7525f8e4fb59d7ca50fb4df8526abd4d` based on the recorded automated
tests and review of this report. L5-A is complete. Manual visual acceptance was
not performed and is not claimed by this closure.

Known limitation retained: the inspector's separately drawn title requires
visual verification during subsequent use, including its appearance and native
close/collapse controls at the supported UI scales. Acceptance on automated
evidence does not resolve this visual-verification limitation. The other known
limitations above, including the untranslated service option `Free`, remain.

The user authorizes a documentation-only commit and push, fast-forward-only
integration into `main`, pushing `main`, synchronization verification and deletion
of the integrated feature branch. Only this report and the roadmap are changed.
Builds and tests are not rerun; the implementation's existing passing results
remain the validation evidence. `AGENTS.md`, skip-worktree flags, user data and
dependencies are preserved. L5-B is not started or authorized.
