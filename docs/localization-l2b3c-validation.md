# L2-B3c validation: motion, thresholds and radiation

Date: 2026-10-22 (Europe/Moscow).

## Baseline and scope

Base SHA: `1605ded80f85ea2b3496f1cf2e0d6e65bf7a8306`.
An origin fetch confirmed HEAD, main and origin/main at that exact commit before
creating `feat/ru-localization-l2b3c`. The implementation was initially published
only on that feature branch; its authorized integration is recorded below.
No L2-B4 work is included. The customized AGENTS.md and its
skip-worktree flag are preserved. README, dependencies and submodules are unchanged.

The L2-B2/B3a/B3b presentation architecture is extended by 21 typed member bindings,
each with separate name and full-description entries. The catalogs now contain
306 entries each: 42 parameter strings and three group captions were added.
Group captions are Physics: Motion → Физика: движение; Physics: Thresholds →
Физика: пороговые значения; Radiation → Излучение.

## Changed files

- `resources/locales/en.json`
- `resources/locales/ru.json`
- `source/Base/Tests/LocalizationServiceTests.cpp`
- `source/Gui/Impl/AlienGui.cpp`
- `source/Gui/Impl/SimulationParametersPresentation.cpp`
- `source/Gui/Impl/SpecificationGuiService.cpp`
- `source/Gui/Impl/Tests/LocalizationGuiTests.cpp`
- `source/Gui/Impl/Tests/check_localization_l2a.py`
- `source/Gui/Impl/Tests/check_localization_l2b1.py`
- `source/Gui/Impl/Tests/check_localization_l2b2.py`
- `source/Gui/Impl/Tests/check_localization_l2b3a.py`
- `source/Gui/Impl/Tests/check_localization_l2b3b.py`
- `source/Gui/Impl/Tests/check_localization_l2b3c.py`
- `docs/localization-l2b3c-validation.md`

## All parameter declarations and typed bindings

Each catalog prefix below has independent `.name` and `.description` keys in both
languages. Parameter lookup compares the original typed pointer-to-member; it
never uses translated or canonical display names to identify a parameter.

| Original name | Russian display | Specification | Original member binding | Catalog prefix |
| --- | --- | --- | --- | --- |
| Time step size | Длительность шага | `FloatSpec` | `&SimulationParameters::timestepSize` | `parameters.motion.timestepsize` |
| Smoothing length | Радиус сглаживания | `FloatSpec` | `&SimulationParameters::smoothingLength` | `parameters.motion.smoothinglength` |
| Pressure | Давление | `FloatSpec` | `&SimulationParameters::pressureStrength` | `parameters.motion.pressurestrength` |
| Viscosity | Вязкость | `FloatSpec` | `&SimulationParameters::viscosityStrength` | `parameters.motion.viscositystrength` |
| Friction | Трение | `FloatSpec` | `&SimulationParameters::friction` | `parameters.motion.friction` |
| Inner friction | Внутреннее трение | `FloatSpec` | `&SimulationParameters::innerFriction` | `parameters.motion.innerfriction` |
| Rigidity of solids | Жёсткость связанных частиц | `FloatSpec` | `&SimulationParameters::rigidity` | `parameters.motion.rigidity` |
| Maximum velocity | Максимальная скорость | `FloatSpec` | `&SimulationParameters::maxVelocity` | `parameters.thresholds.maxvelocity` |
| Maximum force | Максимальная сила | `FloatSpec` | `&SimulationParameters::maxForce` | `parameters.thresholds.maxforce` |
| Minimum distance | Минимальное расстояние | `FloatSpec` | `&SimulationParameters::minObjectDistance` | `parameters.thresholds.minobjectdistance` |
| Maximum distance | Максимальная длина соединения | `FloatSpec` | `&SimulationParameters::maxBindingDistance` | `parameters.thresholds.maxbindingdistance` |
| Fusion velocity | Порог скорости соединения | `FloatSpec` | `&SimulationParameters::objectFusionVelocity` | `parameters.thresholds.objectfusionvelocity` |
| Relative strength | Доля излучения | `FloatSpec` | `&SimulationParameters::relativeStrengthBasePin` | `parameters.radiation.relativestrengthbasepin` |
| Disable radiation sources | Отключить источники излучения | `BoolSpec` | `&SimulationParameters::disableRadiationSources` | `parameters.radiation.disableradiationsources` |
| Absorption factor | Коэффициент поглощения | `FloatSpec` | `&SimulationParameters::radiationAbsorption` | `parameters.radiation.radiationabsorption` |
| Radiation type I: Strength | Излучение I: интенсивность | `FloatSpec` | `&SimulationParameters::radiationType1_strength` | `parameters.radiation.radiationtype1strength` |
| Radiation type I: Minimum age | Излучение I: минимальный возраст | `IntSpec` | `&SimulationParameters::radiationType1_minimumAge` | `parameters.radiation.radiationtype1minimumage` |
| Radiation type II: Strength | Излучение II: интенсивность | `FloatSpec` | `&SimulationParameters::radiationType2_strength` | `parameters.radiation.radiationtype2strength` |
| Radiation type II: Threshold | Излучение II: порог энергии | `FloatSpec` | `&SimulationParameters::radiationType2_energyThreshold` | `parameters.radiation.radiationtype2energythreshold` |
| Minimum split energy | Минимальная энергия разделения | `FloatSpec` | `&SimulationParameters::particleSplitEnergy` | `parameters.radiation.particlesplitenergy` |
| Radiation angle | Угол излучения | `FloatSpec` | `&SimulationParameters::sourceRadiationAngle` | `parameters.radiation.sourceradiationangle` |

## Terminology and preserved meaning

These Russian captions are working choices for user acceptance, not additions to
the normative terminology glossary. The glossary is unchanged.

- Time step size describes simulated duration per integration step, independently
  of TPS, FPS or display refresh. Smaller steps improve accuracy; larger steps can
  cause numerical instability. Smoothing length retains both the neighborhood
  definition and the consequences of excessively small/large values.
- Rigidity describes uniform motion of connected objects in solid structures;
  it does not introduce a new object category. Inner friction describes relative
  velocity along a connection and remains hidden through the original `.visible(false)`.
- Maximum force retains probabilistic loss of all connections when exceeded.
  Maximum binding distance remains a connection-length limit. Fusion velocity
  explicitly means the minimum relative collision speed, plus stickiness of at
  least one object and a free connection slot on each object.
- Base radiation share is the fraction emitted directly next to cells. Together
  with source shares it totals one. It is distinct from General/source share.
- Both radiation Strength captions use intensity as shorthand. Their full Russian
  descriptions explicitly define a fraction of cell energy emitted on average
  per simulation step, not absolute power in physical units. Type I is age based;
  its age counts simulation steps. Type II is energy based; usable and raw energy
  are considered separately.
- Absorption is a fraction of incoming particle energy. Split energy is a threshold
  for possible delayed division into two particles, retaining the small momentum
  note. It does not assert doubled energy or immediate division.
- Radiation angle retains its independent enable flag, −180…180 bounds and `%.1f`
  format. Enabled means the specified direction; disabled means away from the
  source center.

## Implementation boundaries and state

SimulationParameters.cpp/.h, SpecificationEvaluationService, RadiationStrengthService,
the original getter/setter lambdas, physics, CUDA, serialization, MCP and CLI have
no implementation changes. No numeric defaults, constraints, formats, member
types or original visibility are changed. Fixed-source regression checks compare
these boundaries with the baseline, including the previously fixed Unicode paths.

The ordinary IntSpec dispatch now passes the localized label and description to
SliderInt while retaining `.name(parameterSpec._name)` for identity. The color-matrix
branch is unchanged. Existing optional-label widget calls remain compatible.
Slider/checkbox captions with a display label use the existing UTF-8 truncation
and immediate full-text hover path. This reuses the L2-B3a text presentation helper;
the previously adaptive switcher layout is unchanged. Canonical name/ID stacks,
numeric calculations, enable handling, pin handling and callbacks are unchanged.

Search continues to copy the specification and alter visibility only. Runtime
tests cover all three Russian/English group names, mixed case, original ordering,
hidden inner friction and unchanged complete SimulationParameters across language
and filter changes. Parameter-label checks cover all 21 declarations, including
typed lookup after changing a canonical caption and fallback with missing catalogs.
Existing UTF-8 highlight/filter-input tests still verify complete characters and
retained filter text/focus through EN → RU → EN. New groups are included in the
existing canonical-ID/collapse-state test.

Real ImGui mouse/keyboard input edits and resets all 17 visible ordinary new
FloatSpec controls. The remaining numeric declarations are covered separately:
hidden inner friction is inspected but never rendered, the custom base-share
control is edited/reset/pinned through production dispatch, and IntSpec is edited
both collapsed and expanded by color with retained focus/IDs, logarithmic flag,
localized infinity, reset and state across languages. Both threshold arrays are
expanded through real buttons, edited by color, checked for untouched neighbors
and reset. Direction enable and disable-source checkboxes are clicked through
production dispatch and compared across language changes.

The base-share test combines its getter/setter with General/source share. A pinned
base and second source reject an impossible source change. Unpinning, editing and
resetting the base redistributes only eligible shares, preserves the pinned source
and retains the sum of one. Float infinity is rendered for both new infinite
FloatSpec declarations. A disposable parameter-file round trip checks all new
color arrays, infinite values, source share/pin and angle/enable state. Floating
values use the normal floating-point assertion tolerance across the existing JSON
serializer's decimal conversion; this format and its implementation are unchanged.

Disabled layer values in the original widget are normalized to base values during
rendering. The state-invariance fixture therefore explicitly enables the synthetic
layer overrides it tests. That setup avoids attributing pre-existing disabled-value
normalization to language switching. The direction test separately confirms the
existing enable/disable behavior. No production behavior was changed to satisfy a test.

## Build and automated verification

Release built successfully with `build-windows-ninja.bat agent`, using only
build-agent. To bypass the known Ninja/MSVC dependency issue, the objects for
LocalizationService (embedded English), SimulationParametersPresentation,
SpecificationGuiService, LocalizationServiceTests and LocalizationGuiTests were
explicitly removed and rebuilt. Build logs record their compilation and relinking.
Subsequent changed implementation/test objects are compiled by the same script.
No CUDA header/struct change requires a clean engine rebuild.

An initial test-only compilation failure (private widget-state access) was fixed
by exercising the expansion button through real input. Old B2/B3a untranslated-group
expectations were moved to still-untranslated groups. These checks remain active.

| Suite | Passed | Failed | Disabled |
| --- | ---: | ---: | ---: |
| BaseTests | 41 | 0 | 0 |
| DataTests | 247 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 |
| PersisterTests | 142 | 0 | 1 |
| GuiLocalizationTests | 56 | 0 | 0 |
| Total | 566 | 0 | 1 |

All 29 Python checks pass. Eight new headless GUI tests supplement the original
48. Previous General, Location, Visualization, Guided energy supply, Shape and
Force field tests remain active. Catalog-count assertions were updated, and the
old full-file widget hash check permits exactly the new IntSpec presentation
arguments and two caption truncation options while requiring the rest of the
previous widget source to match. The new test also checks these additions directly.
This preserves the regression contract rather than replacing its expected hash.

clang-format 19.1.5 formats only modified C++ files confirmed clean at HEAD.
AlienGui.cpp and SpecificationGuiService.cpp retain their existing formatting;
only the intended lines are edited. `git diff --check` passes.

Logs and test XML are ignored under `temp/l2b3c`. GPU/EngineTests and stress tests
were not run: only presentation changes are made, and CPU/headless coverage is
available. No simulation engine is initialized by the new tests.

## User data and isolated review

Tests use synthetic parameters and SettingsTestScope. The real HKCU/Software/alien
exports before/after testing and the review profile self-check are byte-identical,
SHA-256 `B6885659AF87F2DF7A2D26EB4CCCE9FFC6D6C36149228D976510CF02C0E8D821`.
No user profile is restored over independent user activity.

The ignored review executable links the final production libraries and uses a
disposable registry profile, Russian language, disabled autosave/save-on-exit and
an isolated working directory. Only runtime assets/DLLs and previously generated
synthetic empty samples are packaged. No user worlds, autosaves or settings are
copied. Its profile-only self-check passes without starting a GPU simulation.

```powershell
& 'E:\Projects\ALIEN\temp\l2b3c\review\Launch-L2B3c.ps1'
& 'E:\Projects\ALIEN\temp\l2b3c\review\Launch-L2B3c.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l2b3c\review\Launch-L2B3c.ps1' -Scale 2.0
```

Default scale is 1.5. Automated headless text checks exercise all 21 Russian
captions and full descriptions at 1.0/1.5/2.0 using the existing clipped-text hover
path. The subsequent user visual acceptance is recorded below.
The isolated runner is prepared, not automatically opened on a user simulation.

## User acceptance checklist

1. Launch the isolated runner at scale 1.5. Create a small disposable world or
   use a synthetic sample from its samples directory. Add a layer and source.
2. Inspect all three groups at base/layer/source locations. Read each full tooltip,
   especially minimum fusion speed, probabilistic connection loss, radiation
   fractions, separate raw/usable energy and delayed particle splitting.
3. Expand Maximum force and Maximum binding distance by color, enter distinct
   values, switch EN → RU → EN, reset and save/reload a disposable parameter file.
4. Edit the Type I minimum age, inspect logarithmic/infinite values and color rows.
   Inspect both radiation strengths and infinite energy thresholds.
5. Pin/unpin base radiation share and source shares. Edit/reset eligible shares;
   confirm their sum and pinned constraints survive language/filter changes.
6. Enable directed radiation, set an angle and switch language; confirm the enable
   flag persists. Disable it and confirm the original radial-source behavior.
7. Search each group in English/Russian and names in mixed case. Check Cyrillic
   highlighting, unchanged query, group/color expansion state, order after clearing
   and the absence of Inner friction even when searching its exact name.
8. Resize narrow/wide, repeat at 1.0 and 2.0, and hover any clipped caption to read
   the complete term.
9. Recheck the previously localized groups. Close the isolated runner without
   opening or saving real user files.

## Limitations and graph analysis

Human visual acceptance is complete, as reported by the user below.
Headless ImGui and synthetic persistence tests
do not claim physical/GPU execution or a new physical model validation.
The user reports no remarks on the reviewed terminology or interface. This does
not add terms to the normative glossary, which remains unchanged.

GitNexus upstream analysis reported LOW for the adapter key (three direct consumers:
label, description and alternativeLabels), IntSpec dispatch (parameter-group
dispatcher and its upstream consumers), SliderInt and BasicSlider (Float/Int wrappers).
No indexed process was returned for these GUI paths. Group label and Checkbox
returned UNKNOWN/no resolved callers; source searches explicitly confirmed the
SpecificationGuiService consumers. Test-file lookup is also unresolved, with CMake
registration and fixture use checked directly. None of these zero results is
treated as evidence of unused code. Receiver-resolution/process-enumeration limits
remain relevant. The CLI index-only workflow is used to refresh the index without
rewriting AGENTS.md. Final all-scope graph results are recorded below.

The index-only refresh completed successfully: 729617 nodes, 947469 edges and
435 flows. Its diagnostics report unresolved receivers, capped process enumeration
and malformed/over-bound sites; those are graph coverage limits, not proof that
unlisted GUI callers or flows are unaffected. No PDG or CodeQL scan was run.
After staging all 14 changed files, the all-scope LocalBackend check reports
35 changed symbols, zero indexed affected processes and LOW risk. All 35 symbols
are listed, with `partial: false` and `truncated: false`. The complete response is
saved in `temp/l2b3c/graph-final.json`; the summary is in graph-final.log.
These graph findings are supplemented by the production widget tests and baseline
source contracts described above.

## Integration review and user acceptance

Date: 2026-10-22 (Europe/Moscow).

The user explicitly confirmed that all nine manual acceptance scenarios were
completed without remarks for implementation commit
`1b71848d54871c5dcc13ea97d79d26ff53e9804c`. The user authorized integration into
main and deletion of the completed feature branch after successful publication.
Repeated visual acceptance is not required. This records the user's reported
manual acceptance separately from the automated evidence; it does not claim a
new validation of the physical model or GPU stress behavior.

A fresh origin fetch confirmed main and origin/main at the requested baseline
`1605ded80f85ea2b3496f1cf2e0d6e65bf7a8306`, and both feature refs at the reviewed
implementation SHA. The Git index and working tree were clean.

The complete branch diff was reviewed against main. No blocking issue was found.
All 21 declarations retain distinct typed member bindings and separate complete
name/description keys. English fallback and canonical ImGui names/ID scopes remain
intact. Original specification ranges, values, hidden Inner friction visibility,
color vectors, pin/getter/setter distribution and the independent Radiation angle
enable flag remain in unchanged production specifications and evaluation code.
Only presentation text is selected through localization. The two caption options
reuse existing UTF-8 truncation/full-text hover behavior. IntSpec matrix dispatch
is unchanged. Physics, CUDA, serialization, MCP, CLI, README, normative glossary,
dependencies, upstream and the pinned submodule have no branch changes.

The integration rerun passes all seven CPU/headless GUI suites: **566 C++ tests
passed, zero failures/errors, one existing disabled test**. All **29 Python checks
pass**. Logs/XML are retained under `temp/l2b3c/integration`. The previously successful
forced Release build applies to the reviewed implementation. This integration
changes documentation only, so no recompilation or GPU/stress run is required.
Tests use SettingsTestScope and synthetic data. Real HKCU/Software/alien exports
before and after the rerun are byte-identical, SHA-256:
`9F7B9EC376DEB1E415ACF8F07F4D0BC8CA77DDE067B4AF171EE22A41C306F247`.
No real worlds or autosaves are opened; real settings are not restored or modified.
The customized AGENTS.md and its skip-worktree flag are preserved.

The index-only GitNexus refresh completed successfully (729630 nodes, 947448
edges, 435 flows). Comparison with main reports 14 changed files, 45 changed
symbols and LOW risk; the documentation-only all-scope check reports one file,
five symbols and LOW risk. Both responses list every reported symbol without
partial/truncated results and are saved as graph-compare.json and graph-all.json
under the integration log directory. Indexed affected processes are empty, subject
to the previously documented receiver-resolution and process-enumeration limits;
source review and runtime checks provide the complementary evidence. No code
change is needed for integration. The final documentation commit is prepared on
the feature branch for fast-forward publication to main, followed by safe deletion
of the completed local and remote feature refs.

L2-B3a, L2-B3b and L2-B3c form a completed localization block covering physics,
visualization, geometry, force fields and radiation. This does not mean that the
entire parameter editor is translated. L2-B4 remains outside scope.
