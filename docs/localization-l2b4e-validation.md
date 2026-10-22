# L2-B4e validation: final simulation parameter groups

Date: 2026-10-22 (Europe/Moscow).

## Baseline and scope

Base SHA: `80f106a6485a0caedc5d77849f221e12db671790`. After fetching origin,
HEAD, main and origin/main matched this SHA. The index and working tree were
clean. The implementation was delivered on `feat/ru-localization-l2b4e` without
changing main. Subsequent user acceptance and integration authorization are
recorded below.

AGENTS.md, the normative glossary, all four L2-B4a/B4b/B4c/B4d reports,
SimulationParameters.cpp/.h, SimulationParametersSpecification.h,
SimulationParametersTypes.h, the presentation adapter, GUI dispatch, shared
widgets, relevant cell handlers, settings parser and existing tests were read.
The terminology document is unchanged. This completes the requested five groups
and nine parameters; other GUI work, documentation translation and the genome
editor remain separate stages.

## Groups and exact parameter contracts

| Canonical group | Russian heading |
| --- | --- |
| Cell type: Muscle | Тип клетки: Мышца |
| Cell type: Sensor | Тип клетки: Сенсор |
| Cell type: Reconnector | Тип клетки: Соединитель |
| Cell type: Detonator | Тип клетки: Детонатор |
| Object color transition rules | Правила переходов цветов объектов |

All eight numeric fields below have the exact declared type
`BaseParameter<ColorVector<float>>`, with ten independent color entries.
Their numeric specification type is `FloatSpec`. All member pointers are typed;
neither duplicate `Radius` nor duplicate `Energy cost` strings identify a field.

| Canonical parameter | Field in SimulationParameters | Russian caption | Range | Format | Logarithmic | Default |
| --- | --- | --- | --- | --- | --- | --- |
| Energy cost | muscleEnergyCost | Расход энергии | 0..5 | %.5f | yes | 0 |
| Movement acceleration | muscleMovementAcceleration | Ускорение движения | 0..10 | %.3f | yes | 1 |
| Crawling acceleration | muscleCrawlingAcceleration | Ускорение ползания | 0..10 | %.3f | yes | 1 |
| Bending acceleration | muscleBendingAcceleration | Ускорение изгиба | 0..10 | %.3f | yes | 1 |
| Radius | sensorRadius | Радиус сканирования | 10..800 | %.3f | no | 255 |
| Radius | reconnectorRadius | Радиус соединения | 0..3 | %.3f | no | 2 |
| Blast radius | detonatorRadius | Радиус взрыва | 0..10 | %.3f | no | 10 |
| Chain explosion probability | detonatorChainExplosionProbability | Вероятность цепного взрыва | 0..1 | %.3f | no | 0.2 |

`Target color and duration` becomes `Целевой цвет и длительность`. It binds
through `ColorTransitionRulesSpec` to
`BaseLayerParameter<ColorVector<ColorTransitionRule>> SimulationParameters::colorTransitionRules`.
The associated field is `ExpertToggle SimulationParameters::colorTransitionRulesToggle`.
Each rule retains `int targetColor` and `int duration`, with default target 0
and `Infinity<int>::value` duration. The existing widget uses minimum 1,
maximum 10,000,000, logarithmic scaling and its original Infinity conversion.
Infinity remains the sentinel, rather than a finite maximum in the model.

EN descriptions reproduce the original specification verbatim. RU descriptions
preserve activation-dependent emitted muscle energy, exclusive direct movement,
automatic and signal-controlled crawling/bending, the individual sensor range
limit, possible connection creation, direct blast versus subsequent eightfold
shock-wave reach, and externally triggered chain explosions. The original mode
names and activation algorithms are unchanged. The transition description covers
cells and free cells, subsequent customization color, duration in simulation
steps and resetting cell age when color changes.

## Presentation, identity and search

The existing SimulationParametersPresentation adapter has nine new typed
bindings and five group translations. Both catalogs contain exactly 433 keys:
all previous 408 keys and values are unchanged, with 25 additions (18 parameter
name/description keys, five group names, an expert description and an expert
prefix). Every new key is present in EN and RU. Native missing-catalog tests
exercise the embedded English catalog as well as canonical adapter fallback.

The expert checkbox keeps `.name(groupSpec._name)` and receives a separate
localized label and description. Its expanded header uses `heading(groupSpec)`:
`Экспертные настройки: Правила переходов цветов объектов`. Its name and PushID
scope still use `Expert settings: Object color transition rules`. The checkbox
scope and `##checkbox` ID, tree scope, parameter index, both source-color scopes,
target combo, duration slider and revert IDs retain their canonical inputs.

InputColorTransitionParameters adds only an optional display label. The first
row receives the adapter label and description; the other nine retain empty
captions and their original IDs. The shared widget changes only displayed text
and UTF-8-safe caption truncation. Numeric conversion, bounds, Infinity mapping,
selection and reset handlers are byte-for-byte unchanged after removing that
single presentation expression. It remains an ordered vector of ten rules.

Filtering searches canonical and localized parameter/group captions, including
the full English and Russian expert heading. It only changes visibility in a
copied specification, retaining original indices and references. Native tests
also check that a sensor query does not select the reconnector radius and that
muscle energy cost does not change attacker or injector captions.

## Native interaction and regression coverage

Nine new headless GUI tests call the production dispatcher and widgets. They
cover the following behaviors:

- All eight typed numeric bindings, ranges, formats, logarithmic flags, defaults,
  independent color values, actual Ctrl-click editing, focus during EN/RU/EN,
  reset and synthetic settings save/load.
- Group expansion and canonical IDs; numeric captions and full help text at
  scales 1.0, 1.5 and 2.0 and editor widths 650 and 950 logical units.
- Finding and clicking the actual expert checkbox, matching its canonical ID,
  reading localized help, expanding/collapsing its production group and retaining
  the legacy header ID and stored expansion across language changes.
- Ten ordered transition rows with distinct canonical slider IDs. Actual combo
  selections set source 2 to target 7 and source 7 to target 2; actual duration
  input sets 123 and 987, respectively. The caption-bearing first row is also
  edited, language-switched and reset through its original controls. Active editor focus and complete
  SimulationParameters equality survive EN/RU/EN. Resetting source 2 restores
  only that row; source 7 and all other rows stay intact, including Infinity.
- The same transition edits in the base and an enabled layer override, checking
  independence in both directions. Expert off/on retains all rules. Synthetic
  serialization round-trips both vectors, the layer flag, Infinity and the
  expert switch. Full snapshots are compared before/after language changes.
- Special caption/help accessibility in narrow editors at all three scales.

The original 83 GUI test definitions remain. Two earlier passthrough assertions
used the then-untranslated Reconnector/Detonator groups as fixtures. Those now
use synthetic unbound extension specifications so that the same untranslated
fallback assertions remain meaningful. Newly localized real fields are covered
by typed binding and missing-catalog checks. No previous test is disabled.
Earlier Python whole-file contracts remove only the exact B4e presentation
additions before comparison; the new B4e contract verifies those additions
directly against the baseline. Existing matrix handlers, earlier catalog values,
model, algorithms and serialization remain covered.

## Build and test results

Windows Release uses only `build-windows-ninja.bat agent` and `build-agent`.
The known Ninja/MSVC stale embedded-English object was reproduced by the native
missing-catalog test. Only
`build-agent/source/Base/CMakeFiles/Base.dir/Release/LocalizationService.cpp.obj`
was removed and rebuilt. The complete embedded-English test then passed.
The IDE-owned build trees were not touched.

The Release build completes successfully. All required CPU suites pass:

| Suite | Passed | Failures | Previously disabled |
| --- | --- | --- | --- |
| BaseTests | 41 | 0 | 0 |
| DataTests | 247 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 |
| PersisterTests | 142 | 0 | 1 |
| GuiLocalizationTests | 92 | 0 | 0 |
| Total | 602 | 0 | 1 |

Python: all 49 checks pass (45 previous and four new contracts). Native GUI
coverage grows from 83 to 92 tests; the previous disabled Persister test remains
disabled. Logs and native XML reports are under ignored `temp/l2b4e`.
During fixture development, click coordinates, table/tree ID scopes and ImGui
logging's automatic tree expansion were corrected in tests only. Two obsolete
untranslated-group expectations were updated as described above. All final
results use the completed Release build.

Formatting uses clang-format 19.1.5 at
`F:/VS/VC/Tools/Llvm/bin/clang-format.exe`. Whole-file formatting is applied only
where the HEAD version passes the prescribed dry run. AlienGui.cpp is already
unclean at HEAD, so its single edited expression retains local formatting.

## Data isolation and review package

All tests use synthetic SimulationParameters and the existing process-local
SettingsTestScope. No user world, genome, autosave or profile is loaded or edited.
Real HKCU/Software/alien exports around the verification run match byte for byte:
SHA-256 `76494EF7CF5FDC773FD47147E13A04081C0C0AB598649FFACBB533E53F8DFFB3`.
No registry restoration is performed. AGENTS.md retains SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`
and its original skip-worktree flag.

The separately linked review executable uses the final production libraries and
SettingsTestScope, selects Russian, and disables autosave and save-on-exit.
DLLs and runtime resources are copied into its own directory. Real worlds,
genomes and saved settings are not copied. Profile-only checks exit 0 at scales
1.0, 1.5 and 2.0 without initializing a simulation. The PowerShell launcher parses
successfully and defaults to 1.5:

```powershell
& 'E:\Projects\ALIEN\temp\l2b4e\review\Launch-L2B4e.ps1'
& 'E:\Projects\ALIEN\temp\l2b4e\review\Launch-L2B4e.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l2b4e\review\Launch-L2B4e.ps1' -Scale 2.0
```

The review launcher/executable and raw logs are ignored local acceptance
artifacts, rather than a tracked alternate application. At initial delivery,
interactive visual acceptance remained for the user; headless layout and input
checks were automated. Successful user acceptance is recorded below.

## GitNexus and limitations

Pre-edit analysis finds three direct consumers of the adapter key (label,
description and alternativeLabels), LOW risk. The color-transition dispatcher
has one direct group-rendering caller and two indirect consumers, LOW risk.
Shared InputColorTransition, expert/group rendering, group presentation and
filtering have unresolved caller anchors (UNKNOWN). Source review confirms the
single InputColorTransition caller in SpecificationGuiService, base/layer/source
parameter renderers and the main-window expert section. UNKNOWN is not treated
as evidence of safety. Test-file graph lookups also lack anchors; CMake/Python
discovery and native execution verify their consumers. No HIGH/CRITICAL result
was returned. The process resource was read; its sampled inventory does not
resolve these GUI flows. Receiver-resolution gaps limit graph coverage.

Both index refreshes succeed with the existing full-text/BM25 build warning;
graph analysis remains available. The final fresh-backend
`detect_changes(scope: all)` reports 21 files, 61 listed changed symbols, LOW
risk and no indexed affected processes, with `partial=false` and
`truncated=false`. The summary count matches the complete listing. Global
process enumeration is separately bounded, so the absent GUI flows are not
claimed to be unaffected. Results are retained in
`temp/l2b4e/graph-final.json`; index logs also preserve receiver-resolution and
parsed-site coverage warnings. Direct caller inspection and passing native
regressions supplement these graph limitations. `git diff --check` passes.

SimulationParameters::getSpec(), model definitions, CUDA/physics, cell
algorithms, serialization, CLI, MCP, README, glossary, dependencies, upstream
and the pinned submodule are unchanged. No CodeQL, submodule update, GPU/stress
tests, L3 work or genome-editor changes are performed. There is no technical
reason for GPU tests for presentation-only changes. Known injector/converter
defects are not fixed. The existing transition boundary conversion maps the
widget maximum to Infinity; this behavior is intentionally preserved, including
its existing numeric-entry semantics. No new Infinity/reset defect was found
in the exercised scenarios. Layer enable/disable UI behavior is unchanged;
tests exercise an independently enabled override and verify its stored flag.

## Manual acceptance

1. Launch the isolated package at 1.5 and create only a small disposable
   simulation. Open Parameters and inspect all five headings, nine captions and
   complete help texts. Confirm signal-controlled meanings of Manual modes,
   the sensor's individual limit, connection possibility, and the distinction
   between blast radius and subsequent shock-wave reach.
2. Search English and Russian group/parameter names, including the full expert
   headings and mixed case. Clear the filter and check original ordering.
3. Expand color-dependent numeric controls; edit two colors and switch
   EN -> RU -> EN while a numeric editor has focus. Check values, precision,
   expansion and Reset. Confirm attacker/injector energy-cost captions too.
4. Enable the expert transition switch. Expand its group and inspect all ten
   source colors in order. Set source 2 to target 7 with duration 123 and source
   7 to target 2 with duration 987. Keep another row at Infinity. Switch
   EN -> RU -> EN with duration input active; then reset only source 2.
5. Repeat transition edits in an enabled synthetic layer override, checking
   independent base values. Toggle expert rules off/on and confirm retained
   rules. Save/load only a synthetic parameter file; recheck durations, targets,
   Infinity and flags. Exercise both prior 10-by-10 matrix dialogs as regression.
6. Repeat at 1.0 and 2.0, with narrow/wide parameter windows. Check full captions
   or their hover text, help markers, row controls, ten-row ordering and absence
   of overlap. Close without opening or saving real user worlds.

## User acceptance and integration review

Date: 2026-10-22 (Europe/Moscow).

The user confirms successful visual acceptance of implementation commit
`30e6afa39c18b72190212d095d1b54f6c7edfa59` across all six manual acceptance
scenarios above:

1. All five group headings, nine captions and complete descriptions.
2. English/Russian search, expert headings and original ordering.
3. Color-dependent numeric editing, language switching, focus and reset.
4. The ten-row transition editor, target colors, durations, Infinity and row reset.
5. Independent layer overrides, expert off/on retention, synthetic save/load
   and both prior matrix dialogs.
6. Narrow/wide window layouts and scales 1.0, 1.5 and 2.0.

Result reported by the user: successful acceptance of all six scenarios. This
records the user's visual verification; no new GPU or biological-dynamics
validation is claimed. The user authorizes closing L2-B4e, fast-forward
integration into main, pushing main to origin and removing the integrated
feature branch locally and remotely. L3 remains outside scope.

After fetching origin, main and origin/main both match
`80f106a6485a0caedc5d77849f221e12db671790`; the feature branch and its origin
tracking ref both match the accepted implementation SHA. The index and working
tree are clean, the feature is ahead one / behind zero, and there is no other
worktree. The baseline is an ancestor of the feature, allowing fast-forward.
The reviewed diff contains only the documented L2-B4e implementation, catalogs,
tests and validation report; it has no unrelated changes. `git diff --check`
passes.

This follow-up changes only this report. Implementation code and catalogs remain
identical to the accepted SHA. The successful Release build, 602 passing C++
tests, 49 passing Python checks and one previously disabled test remain the
validation evidence; no new implementation change justifies repeating them.
No full build or test suite is rerun during integration.

AGENTS.md retains its original bytes (SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`)
and skip-worktree flag. Integration does not run the application or touch user
worlds, genomes, profiles, settings, dependencies or submodules. No CodeQL,
submodule update or L3 work is performed. This documentation commit is published
on the feature branch before the authorized fast-forward integration.
