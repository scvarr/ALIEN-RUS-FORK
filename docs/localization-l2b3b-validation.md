# L2-B3b geometry and force-field localization validation

Date: 2026-10-21 (Europe/Moscow).

## Baseline and changes

Branch: `feat/ru-localization-l2b3b`. Initial HEAD and local main:
`2f0f2b59edaaeb2e6bba1bee763b52b68b7a39cf`. The checkout was clean. A final fetch confirms origin/main at the same SHA.
The customized AGENTS.md and its skip-worktree flag are preserved.
The normative terminology and L2-B2/L2-B3a reports were consulted.

This slice extends only the GUI presentation adapter and catalogs: two groups,
17 parameter declarations, their complete descriptions and 11 alternatives across
four independent switchers. Each EN/RU catalog now has 261 entries (47 added).
Prior catalog entries are unchanged. No public adapter API change is needed.
Existing GUI/headless and Python suites are extended; catalog counts in earlier
checks are updated, and untranslated-group fixtures use Physics: Motion.

## Typed bindings

Every row has independent `.name` and `.description` keys under the shown prefix.
Bindings compare typed pointers to SimulationParameters members, never captions.
Names and descriptions remain distinct even for repeated Shape and Strength labels.

| Member | Spec type | Catalog prefix | Russian caption |
| --- | --- | --- | --- |
| `layerShape` | `AlternativeSpec` | `parameters.shape.layershape` | Форма слоя |
| `layerCoreRadius` | `FloatSpec` | `parameters.shape.layercoreradius` | Радиус полного воздействия |
| `layerCoreRect` | `Float2Spec` | `parameters.shape.layercorerect` | Размер полного воздействия (ширина, высота) |
| `layerFadeoutRadius` | `FloatSpec` | `parameters.shape.layerfadeoutradius` | Ширина зоны затухания |
| `sourceShapeType` | `AlternativeSpec` | `parameters.shape.sourceshapetype` | Форма источника |
| `sourceCircularRadius` | `FloatSpec` | `parameters.shape.sourcecircularradius` | Радиус |
| `sourceRectangularRect` | `Float2Spec` | `parameters.shape.sourcerectangularrect` | Размер (ширина, высота) |
| `layerForceFieldType` | `AlternativeSpec` | `parameters.forcefield.layerforcefieldtype` | Тип поля |
| `layerRadialForceFieldOrientation` | `AlternativeSpec` | `parameters.forcefield.layerradialforcefieldorientation` | Направление вращения |
| `layerRadialForceFieldStrength` | `FloatSpec` | `parameters.forcefield.layerradialforcefieldstrength` | Сила |
| `layerRadialForceFieldDriftAngle` | `FloatSpec` | `parameters.forcefield.layerradialforcefielddriftangle` | Угол отклонения |
| `layerCentralForceFieldStrength` | `FloatSpec` | `parameters.forcefield.layercentralforcefieldstrength` | Сила |
| `layerLinearForceFieldAngle` | `FloatSpec` | `parameters.forcefield.layerlinearforcefieldangle` | Угол |
| `layerLinearForceFieldStrength` | `FloatSpec` | `parameters.forcefield.layerlinearforcefieldstrength` | Сила |
| `layerPerlinNoiseForceFieldStrength` | `FloatSpec` | `parameters.forcefield.layerperlinnoiseforcefieldstrength` | Сила |
| `layerPerlinNoiseForceFieldSpatialSize` | `FloatSpec` | `parameters.forcefield.layerperlinnoiseforcefieldspatialsize` | Пространственный размер структур |
| `layerPerlinNoiseForceFieldTemporalSize` | `FloatSpec` | `parameters.forcefield.layerperlinnoiseforcefieldtemporalsize` | Временной размер структур |

Group captions are Shape → Форма and Force field → Силовое поле.
Shape has seven declarations (two alternatives, two Float2, three Float).
Force field has ten declarations (two alternatives, eight Float).

| Parameter | Index | Canonical value | Russian display |
| --- | ---: | --- | --- |
| `layerShape` | 0 | Circular | Круглая |
| `layerShape` | 1 | Rectangular | Прямоугольная |
| `sourceShapeType` | 0 | Circular | Круглая |
| `sourceShapeType` | 1 | Rectangular | Прямоугольная |
| `layerForceFieldType` | 0 | None | Нет |
| `layerForceFieldType` | 1 | Radial | Радиальное |
| `layerForceFieldType` | 2 | Central | Центральное |
| `layerForceFieldType` | 3 | Linear | Линейное |
| `layerForceFieldType` | 4 | Perlin noise | Шум Перлина |
| `layerRadialForceFieldOrientation` | 0 | Clockwise | По часовой стрелке |
| `layerRadialForceFieldOrientation` | 1 | Counter clockwise | Против часовой стрелки |

## Preserved semantics and invariants

- Layer core means the area of full effect. Fade-out is the surrounding transition
  zone in which influence decreases to zero. It is not a cell nucleus.
- Source geometry defines where energy particles are created. It does not define
  the extent of visible illumination.
- Radial describes rotation around the layer center. Orientation controls clockwise
  versus counterclockwise rotation. Drift angle rotates the force away from the
  tangent: zero is tangential, other values add inward or outward movement.
- Central force is weak at the center, peaks a short distance away and then decreases.
  Linear force keeps its direction and constant strength inside the full-effect area.
- Perlin strength, spatial vortex scale and renewal duration remain independent.
  Temporal structure size counts simulation steps, not seconds.

SimulationParameters::getSpec(), its English strings, original member references,
indices, nested structures, limits, formats, logarithmic flags and dimensions are
unchanged. The adapter translates display text and returns canonical fallback when
unbound or unresolved. Embedded English is verified with missing external catalogs.
Default/revert behavior, selected modes, enable flags, callbacks, stable ImGui IDs,
ID stacks and section state use unchanged production code. Existing adaptive
Switcher layout and full-text hover behavior from L2-B3a are reused without edits.

The unchanged L2-B2 GUI filter copies the tree and alters visibility only. English
canonical names and current-language labels participate, including group matches.
All four Strength declarations survive a Strength/Сила query. Descendant matches
keep parents visible, but production rendering selects children only from the
user's active alternative. Searching for Temporal structure/Временной while Radial
is selected neither changes that selection nor renders Perlin settings as active.
Clearing restores the original list and indices. UTF-8 matching/highlighting retains
whole Cyrillic characters. EN → RU → EN preserves values and the query.
Global SpecificationFilterService and StringHelper are unchanged.

README, normative glossary, dependencies, vendored file dialog, pinned vcpkg,
upstream, CUDA/physics, serialization, MCP and CLI are unchanged. No L2-B3c work,
main merge, GPU or stress tests are performed.

## Automated validation

Release builds use `build-windows-ninja.bat agent` and build-agent only.
To avoid Ninja/MSVC's include-prefix dependency problem, the four affected objects
were explicitly removed and recompiled: LocalizationService (embedded catalog),
SimulationParametersPresentation, LocalizationServiceTests and LocalizationGuiTests.
The successful forced-build log lists all four compilations and relinks production
and test binaries. No shared layouts or CUDA headers changed; no full engine
rebuild is required. Later test additions are recompiled by the required script.
clang-format 19.1.5 is used only on modified C++ files confirmed clean at HEAD.

The initial numeric-test build exposed Windows' `small` macro; the local test
variable was renamed to isSmallRange. No production fix was needed.

| Suite | Passed | Failed | Disabled |
| --- | ---: | ---: | ---: |
| BaseTests | 41 | 0 | 0 |
| DataTests | 247 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 |
| PersisterTests | 142 | 0 | 1 |
| GuiLocalizationTests | 48 | 0 | 0 |
| Total | 558 | 0 | 1 |

All 25 Python checks pass. The 48 GUI tests include seven new L2-B3b tests.
The final Release build and isolated review wrapper link successfully.


The new runtime tests recursively inspect every binding, verify distinct descriptions,
member-based lookup after canonical-caption changes, every alternative index and
English fallback. Production dispatch renders both shapes, five fields, both
orientations and nested numeric controls across language switches, comparing all
SimulationParameters values. A CPU-only facade returns a synthetic 1024x768 world
size; no GPU session or simulation is initialized. All 11 numeric fields and both
axes of both Float2 controls are edited with real ImGui input events, retain input
focus/IDs across languages and revert through real buttons. Switcher arrow handlers
cycle all indices and revert. Group collapse state is tested for both new groups.
Narrow/ample switcher captions and hover are checked at scales 1.0, 1.5 and 2.0.
Existing General, Location, Visualization and Guided energy supply tests still run.

Python contracts verify catalog equality/completeness, all typed bindings, exact
English descriptions from nested original builder chains, all 11 alternatives,
critical translation semantics and fixed hashes of original specs/widgets/filtering
and serialization. These supplement actual widget tests rather than replacing them.

## User data and isolated review

All tests use synthetic parameters and SettingsTestScope; no real simulation is
opened. The full CPU/GUI test bracket plus review-profile self-check left real
HKCU/Software/alien exports byte-identical, SHA-256:
`E01D408342412C28386041FDC429C1F329B88A4C148D89900227E9580B634DBF`.
No registry/profile restoration is performed over independent user activity.
Logs/XML/exports are ignored under temp/l2b3b.

The ignored review executable links the final production libraries with a wrapper
using a disposable SettingsTestScope profile, Russian, disabled autosaves and
save-on-exit. Its profile-only self-check passes. Runtime resources and DLLs are
copied, with synthetic empty samples copied from the earlier isolated review
package; no user saves or autosaves are included.

```powershell
& 'E:\Projects\ALIEN\temp\l2b3b\review\Launch-L2B3b.ps1'
& 'E:\Projects\ALIEN\temp\l2b3b\review\Launch-L2B3b.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l2b3b\review\Launch-L2B3b.ps1' -Scale 2.0
```

The launcher defaults to scale 1.5. Human visual acceptance is pending; automated
headless verification does not imply user approval of terminology or appearance.

## Working terminology choices

The normative glossary is unchanged. The following remain presentation proposals
for visual acceptance: Форма слоя / Форма источника distinguish the two objects;
Радиус полного воздействия / Размер полного воздействия describe the core;
Ширина зоны затухания describes its surrounding falloff; Радиальное retains the
canonical mode name with an explicitly rotational explanation; Сила distinguishes
force strength from radiation share (Доля); Угол отклонения means deviation from
the tangent. Временной размер структур retains the source caption while its
help text explicitly specifies simulation steps. These choices are not automatic
additions to the approved normative terminology.

## Manual acceptance

1. Launch the isolated runner at default 1.5; create a small temporary simulation
   or load a sample under its samples directory. Open Parameters, add a layer and
   a radiation source. Do not open real user simulations.
2. Inspect Форма for both locations. Cycle Круглая/Прямоугольная, edit radius,
   width and height, inspect full-effect and fade-out descriptions, and revert.
   Confirm source descriptions refer to particle creation, not light boundaries.
3. Enable Силовое поле and cycle all five modes. Check both rotation directions,
   strength, drift, central dependence, linear direction, spatial scale and the
   step-based temporal scale. Edit distinct values in every mode and revisit them.
4. Switch EN → RU → EN, including an active numeric edit; confirm every selected
   mode, value, focus and collapsed section remains. Check revert and enable state.
5. Search Shape/Форма, Force field/Силовое поле, Strength/Сила and a nested numeric
   name. While Radial is active search Temporal structure/Временной: the parent
   remains discoverable but no Perlin controls appear active and mode is unchanged.
   Check whole-character Cyrillic highlighting, then clear and confirm full order.
6. Resize narrow/wide and repeat at 1.0/2.0. Long values must fit when space permits
   and be fully readable on hover when clipped. Inspect both rectangular axes and
   all help markers. Recheck the four previously localized groups. Close without
   saving user data.

## GitNexus

Repository binding: ALIEN-RUS-FORK at E:/Projects/ALIEN (the edited checkout).
The initial index was one documentation-only commit behind HEAD, at
14ce92cf28a2ab542c9292b53ccd78f99af0b2bb. Explicit repo selection was used.
Pre-edit upstream impact on key reports LOW: label and description are its two
direct consumers, with no indexed process reported. Group label and alternativeLabels
return UNKNOWN because their consumers are unresolved; source searches confirm
SpecificationGuiService group rendering and alternative dispatch, plus GUI tests.
Those zero-caller results are not interpreted as unused or safe. No HIGH/CRITICAL
risk was returned. Test macro/file symbols are not consistently resolved either;
their CMake registration and existing call-site use were checked directly.

The index-only refresh completed successfully (729580 nodes, 947208 edges, 435
flows). Its warnings explicitly report receiver-resolution and process-enumeration
limits; the process resource was inspected and has limited GUI coverage. Missing
indexed flows are not proof of no effect. Actual production widget tests and fixed
source hashes provide the complementary evidence.

The final refresh includes the last runtime tests and report (729592 nodes,
947218 edges, 435 flows). All-scope detect_changes reports **11 changed files,
39 changed symbols, zero indexed affected processes and LOW risk**. The structured
response lists all 39 symbols and has neither partial nor truncated results.
It is saved in temp/l2b3b/graph-final.json; graph-final.log contains the summary.
The earlier MCP check and final LocalBackend/CLI-backed check agree on LOW risk.
Zero indexed processes is qualified by the coverage limits above, not taken as
proof that no GUI path is affected. Final source contracts independently confirm
unchanged simulation specifications, widgets and serialization. Both main refs
remain at the baseline; only the feature branch is prepared for push.
