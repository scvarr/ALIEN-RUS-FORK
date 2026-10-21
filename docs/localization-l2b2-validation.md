# L2-B2 simulation parameter presentation validation

Date: 2026-10-21 (Europe/Moscow).

## Baseline and scope

Branch `feat/ru-localization-l2b2` starts at current `main`/`origin/main`,
`c36bf891802035486a454cb9c0586d805a5f4c4e`, confirmed by fetching origin.
The checkout was clean. The local customized `AGENTS.md` and its skip-worktree
flag are preserved and excluded. The architectural audit, terminology and L1,
L2-A and L2-B1 reports were consulted.

Only General and Location receive specification translations: two group labels,
nine parameter labels, nine full descriptions and two mouse-picker messages
(22 added entries; 160 entries in each catalog). The English descriptions exactly
match the original declarations. No new ambiguous domain terminology was required.
Relative strength uses the accepted radiation-distribution term "Доля".

README, upstream, dependencies, the pinned vcpkg submodule, engine parameters,
CUDA/physics, enum values, serialization, MCP/CLI and simulation-field cell labels
are unchanged. The implementation phase left main unchanged; integration review
and user acceptance are recorded below. No L2-B3 work is included.

## Presentation adapter and extension path

`SimulationParametersPresentation` lives entirely in Gui/Impl. Its explicit
bindings compare typed pointers to members of `SimulationParameters`, extracted
from each original reference variant. Display names are not lookup identities.
Copied specifications and repeated display names therefore retain the correct
context. Renaming a display name in a test does not change its translation binding.
Group labels use the existing canonical General/Location group identifiers.

| Canonical group | Stable member | Catalog prefix after `parameters.` | Russian label |
| --- | --- | --- | --- |
| General | projectName | general.project | Название проекта |
| General | layerName | general.layer | Название слоя |
| General | sourceName | general.source | Название источника |
| General | layerOpacity | general.opacity | Непрозрачность |
| General | sourceRelativeStrength | general.share | Доля |
| Location | layerPosition | location.layerposition | Положение (x, y) |
| Location | layerVelocity | location.layervelocity | Скорость (x, y) |
| Location | sourcePosition | location.sourceposition | Положение (x, y) |
| Location | sourceVelocity | location.sourcevelocity | Скорость (x, y) |

Each prefix has `.name` and `.description`. The group keys are
`parameters.general.name` and `parameters.location.name`. Four independent
Location descriptions distinguish layer/source centers and per-time-step
velocities. Opacity retains 0..1 and the central-area condition. Radiation share
explains energy particles, creation next to cells versus at sources, the relative
distribution between base and sources, external inflow scaling and a total of 1.
Coordinate bounds, formats and mouse-picker flags come directly from unchanged
Float2Spec values; they are never reconstructed from translated strings.

`LocalizationService` supplies the existing per-key English fallback and embedded
English catalog. Unbound specifications return their existing English text.
The adapter also returns original text when a requested key cannot be resolved.
`SimulationParameters::getSpec()` stays immutable and entirely English.

To add a group, add catalog records, add typed member bindings and its explicit
group-label binding. The filter, UTF-8 matcher, identity handling and fallback
need no architectural change. Char64/Float/Float2 controls already consume the
adapter. Other control families can be wired with the same explicit `.label()`
and `.tooltip()` calls; wrappers without separate labels will need the same small
additive label option as SliderFloat2. Alternative mode names remain canonical
and outside this slice.

## Filtering, identity and cache

The GUI stops using Data's byte-wise `SpecificationFilterService`; that service
and global `StringHelper` are unchanged. The GUI filter copies the original tree,
retains every group, parameter and alternative at its original index and changes
only parameter `_visible` flags. Existing evaluation rules still determine which
location types may render each entry. Duplicate Position/Velocity declarations
are retained, and invisible siblings no longer renumber ImGui scopes.

A matching canonical or displayed group name keeps its entire subtree. Otherwise,
parameters match their canonical English names or current localized labels;
nested alternatives are traversed without deleting or reordering branches. A
matching descendant keeps its parent visible. Empty/absent filters preserve the
complete original ordering, visibility, references, ranges and descriptions.
No matches leave no visible parameter groups.

A Russian query can also match a group substring: "положение" matches
"Расположение" and consequently displays all four Location declarations.
"Положение (" selects the two position declarations only. English queries work
in both EN and RU; Russian labels participate while RU is selected. Switching
languages retains the filter text even when its current result set changes.

`_specCache` is removed. Each submission derives visibility from the current
catalog and language, so reloads and language switches cannot reuse stale cached
results or accumulate cache entries. The existing small specification tree is
copied per submission; no engine data is copied into a translation catalog.

`Utf8Search` strictly decodes scalar values and returns byte ranges at character
boundaries. It folds ASCII A..Z, Russian А..Я and Ё to their lowercase partners;
other Unicode scalars compare exactly. It does not equate е with ё, normalize
combining marks or claim general Unicode case folding. Truncated sequences,
overlong encodings, surrogate encodings, invalid continuation bytes and values
above U+10FFFF produce no match. Cyrillic never reaches byte-wise `tolower`.
The shared GUI Text/tree highlighter uses this same matcher; its two overlay draws
use complete substrings. ASCII matching and first-match highlighting are retained.

## Widget integration

- `SpecificationGuiService` retains `.name(parameterSpec._name)`, original group
  scopes, original parameter indices and numeric formats. Separate labels and
  descriptions feed Char64, scalar Float and Float2 controls. The layer/source
  lookup, evaluation, getter/setter and mutation callbacks are unchanged.
- `SliderFloat2Parameters` gains optional `.label()` and mouse-picker tooltip/message
  values with the original English defaults. Revert localizes its existing common
  tooltip when a display label is supplied. Hidden slider IDs, default values,
  callbacks and coordinates are unchanged.
- The parameter filter previously used its translated hint as its ImGui ID.
  Optional `InputFilterParameters::id` / `InputTextParameters::hintId` now allow
  an independent ID. Only the parameter editor opts in, preserving its original
  English ID `Filter (case insensitive)`. Existing callers keep their defaults.
- Char64 storage, input capacity, text handling and user-defined names are unchanged.
  The main-window filter remains a separate UTF-8 string.

## Build and automated verification

`build-windows-ninja.bat agent` succeeded in Release, initially compiling all
412 steps from a fresh agent tree. The previous build-agent was reversibly
archived under `temp/l2b2/build-before-clean`; only `vcpkg_installed` was reused.
No compiled ALIEN objects/libraries were reused. This excludes stale common-header
layouts from the known Ninja/MSVC dependency-prefix defect. IDE-owned build trees
were untouched. Final required-script builds after test/revert refinements passed.

| Suite | Passed | Failed | Disabled |
| --- | ---: | ---: | ---: |
| BaseTests | 41 | 0 | 0 |
| DataTests | 247 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 |
| PersisterTests | 141 | 0 | 1 |
| GuiLocalizationTests | 28 | 0 | 0 |
| Total | 537 | 0 | 1 |

The nine additional headless GUI tests cover all nine member bindings and complete
English descriptions, all four contextual descriptions, untouched groups, language
switches, empty/no-match/group/parameter/nested filtering, original indices,
UTF-8 rejection and Ё/ё case matching, actual highlight vertex counts, production
Char64 focus and edits, production layer/source velocity controls, General values
at all three location types, tree IDs/collapsed states, active UTF-8 filter content,
English fallback with missing external catalogs and real picker/revert clicks.
All 19 earlier GUI tests remain passing, including editors, dialogs, table and
menu state contracts. Existing Base tests verify all 160 embedded English values,
strict UTF-8, duplicate keys and per-key missing-translation fallback.

All 14 Python contracts pass: L2-A (6), L2-B1 (4), L2-B2 (4). New contracts verify
catalog completeness, original English descriptions and fixed baseline hashes of
specifications, SimulationParameters, Data's filter, StringHelper and interaction
controller. These supplement runtime checks; they do not replace them.

The first 27-test GUI run had one incorrect expectation: the Russian position
query also matched the group name. Correcting that expectation and adding the
more specific position query resolved it without changing filtering behavior.
The final 28-test suite passes, including the additional General-value test and
picker revert assertions. Repeated runs are counted once in the table.

GPU/stress suites were not run: engine/CUDA/data/protocol code is unchanged.
This is GUI/CPU regression validation, not a new GPU qualification. clang-format
19.1.5 was applied only to modified files clean at HEAD and new C++ files.
Already non-clean AlienGui.cpp received narrow edits only. Whitespace checks pass.
Logs, XML, registry exports and local review binaries remain ignored in temp/l2b2.

## GitNexus

Pre-edit upstream impact reported MEDIUM for SpecificationGuiService (five direct
class dependencies), LOW for parameter-group/scalar/vector/text rendering methods
and the highlighter (group/alternative dispatch and Text/tree consumers), and LOW
for processDetailWidget via processIntern. Several wrapper entry points returned
UNKNOWN; source searches confirmed base/layer/source rendering and existing filter,
login, browser and dialog callers. UNKNOWN was not treated as an all-clear.
No HIGH/CRITICAL result was returned. Index refreshes and the final all-scope
change analysis are recorded in temp/l2b2; graph coverage limitations are not taken
as proof that unlisted GUI paths are unaffected.

Final refreshed all-scope analysis reports 17 changed files, 58 changed symbols,
zero indexed affected processes and LOW risk. The complete MCP response contains
all 58 symbols without partial/truncated flags. Both the CLI and MCP checks agree.
The process inventory was inspected; its limited GUI coverage is supplemented by
the production headless tests and explicit call-site review.

## User data and isolated acceptance

The agent did not open a real simulation. Tests use process-local SettingsTestScope
registry redirection and synthetic parameters. A final bracketed run of all seven
CPU/GUI suites left the real registry export byte-identical, SHA-256:
`11445F7A813BE21E17F54327B9E4A9022D136E3247AA7AFDA839A0A09A4955EE`.
The broader session's first/last exports differed in browser-history/editor-active
values; a separate alien process was observed during the session. No profile was
restored over those independent changes. The final isolated test bracket is the
basis of the unchanged-profile verification.

The old output autosave was retained in the archived build and copied back into
the fresh normal output tree only while its destination was absent. Both copies
have SHA-256 `4716A1046175F0D7EB9B59BEB16F2E4239D8F7C1F52A8FD5914A00AB64B98F69`.
It was not loaded or used in a test and is excluded from the review directory.

The ignored review executable was linked against the final production libraries.
It wraps the original application entry point with SettingsTestScope, disables
save-on-exit and uses a fresh disposable profile. Its profile-only self-check
passed without launching the GUI. Resources/DLLs are copied without autosaves.
The launcher defaults to interface scale 1.5 and permits 1..4 explicitly:

```powershell
& 'E:\Projects\ALIEN\temp\l2b2\review\Launch-L2B2.ps1'
& 'E:\Projects\ALIEN\temp\l2b2\review\Launch-L2B2.ps1' -Scale 1.5
```

User-reported visual acceptance passed for implementation commit
`9d221aac23c1da11465e8c644f3738fc40fac7d6`. The user reported checking the interface,
numeric input and filtering while switching languages, with no problems found.
This records only that reported scope. It does not assert that the user completed
every item in the original checklist, tested a particular scale, inspected every
tooltip, exercised mouse picking or reviewed other editors individually.

Original visual acceptance checklist (retained as reference, not a record that
every item was performed):

1. Create a small disposable simulation; open Parameters and add a layer and a
   radiation source. Check General/Location on the base, layer and source.
2. In Settings switch EN -> RU -> EN. Keep a name edit active, adjust opacity/share
   and velocity, and collapse a group. Verify values, input contents, selection,
   focus and group states persist. User names must remain verbatim.
3. In RU inspect all nine help markers, especially the distinct layer/source
   coordinate descriptions and the energy-distribution explanation for "Доля".
   Confirm labels/tooltips fit at scale 1.5.
4. Search for Общие, General, Доля, Opacity, Расположение, Положение (, Velocity,
   Glow, an absent name and an empty query. Check complete Cyrillic highlighting,
   retained filter text during switches and the original order after clearing.
5. For both layer and source, enable mouse position selection, check Russian help
   and the screen message, select a disposable coordinate, then revert it. Verify
   both axes and unchanged %.2f/%.3f formatting and bounds.
6. Check an untranslated group and an existing editor for normal English labels,
   working controls and unchanged behavior. Close the isolated runner without
   opening user simulations or importing real settings.

## Integration review

Date: 2026-10-21 (Europe/Moscow).

An origin fetch confirmed main and origin/main at
`c36bf891802035486a454cb9c0586d805a5f4c4e` and both feature-branch refs at the
user-tested implementation commit above. The working tree was clean and no other
worktree was registered. No parallel branch update was observed at this check.

Final review against main found no blocking issue. Engine specifications and
structures, ranges, physics, serialization and protocols have no diff. The nine
typed member bindings distinguish repeated names. Original vector positions and
canonical widget/group names preserve ImGui scopes; filtering changes visibility
without renumbering. The filter ID is independent of its translated hint, and
UTF-8 matching/highlighting uses validated character boundaries. Unbound groups
retain English text, and additive widget options preserve existing defaults.
UNKNOWN graph results were cross-checked against the actual base/layer/source,
browser, documentation and Text/tree callers rather than treated as unused code.

The existing Release build success and saved test XML were inspected. A fresh
integration run of all seven CPU/GUI executables confirms 537 successful tests,
zero failures and one disabled Persister test; all 14 Python checks pass again.
Logs/XML are retained under `temp/l2b2/integration`. The implementation code is
unchanged from the user-tested commit. This follow-up changes documentation only;
no rebuild or heavy GPU test is required for it.

The user authorized fast-forward integration into main and removal of the completed
feature branch after successful synchronization. Local AGENTS.md content and its
skip-worktree flag, user data, autosaves and diagnostic materials are preserved.
