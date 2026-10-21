# L2-B3a localization validation

Date: 2026-10-21 (Europe/Moscow). Branch: `feat/ru-localization-l2b3a`.
Base: `628868dbc9135750c38210179fdada0f20335ca3` (local `main` and
`origin/main` verified after fetching origin). Visual acceptance is pending;
this branch must not be merged before the user's acceptance. L2-B3b is outside scope.

## Scope and files

- `resources/locales/en.json`, `resources/locales/ru.json`: two groups, twenty
  complete names/descriptions, five coloring options and common color-control captions.
- `source/Gui/Impl/SimulationParametersPresentation.{h,cpp}`: twenty additional
  typed member bindings and contextual alternative display labels.
- `source/Gui/Impl/SpecificationGuiService.cpp`: presentation labels/descriptions
  for checkbox, color and alternative controls, preserving canonical names.
- `source/Gui/Impl/AlienGui.{h,cpp}`: optional color/switcher display labels and
  optional switcher display values; common color captions and reset tooltip.
- `source/Base/CMakeLists.txt`: split the embedded English JSON into adjacent raw
  literals at line boundaries to avoid MSVC C2026 (one literal exceeded 16 KiB).
  The resulting catalog bytes and fallback behavior are preserved.
- `source/Base/Tests/LocalizationServiceTests.cpp`: expanded catalog count.
- `source/Gui/Impl/Tests/LocalizationGuiTests.cpp`: additional runtime coverage;
  the previous untranslated-group assertion now uses Shape instead of Visualization.
- `source/Gui/Impl/Tests/check_localization_l2{a,b1,b2}.py`: expanded catalog counts.
- `source/Gui/Impl/Tests/check_localization_l2b3a.py`: new source contracts.
- `docs/localization-l2b3a-validation.md`: this report.

The local AGENTS.md and its skip-worktree flag are preserved. README, glossary,
submodule, dependencies, upstream, Data specifications/filter, engine/CUDA, physics,
serialization, CLI and MCP are unchanged. Accepted General/Location catalog entries
are unchanged. No energy-balance functionality is added.

## Translated declarations

Each row has its own `parameters.<group>.<lowercase-member>.name` and `.description`
keys. The member pointer and specification type, never the English caption, select it.

| Canonical name | Member | Russian caption |
| --- | --- | --- |
| Background color | backgroundColor | Цвет фона |
| Customization colors | customizationColors | Цвета настройки |
| Object coloring | objectColoring | Окраска объектов |
| Glow | glow | Свечение |
| Fluid illumination | fluidIllumination | Освещение жидкости |
| Borderless rendering | borderlessRendering | Бесшовное отображение |
| Grid lines | gridLines | Линии сетки |
| Mark reference domain | markReferenceDomain | Рамка основной области |
| Show radiation center | sourceShowRadiationCenter | Показывать центр излучения |
| Light color | sourceLightColor | Цвет света |
| Light brightness | sourceLightBrightness | Яркость света |
| Light range | sourceLightRange | Дальность света |
| Show force field | layerShowForceField | Показывать силовое поле |
| External energy amount | externalEnergy | Количество внешней энергии |
| Inflow for constructors | externalEnergyInflowForConstructor | Передача энергии конструкторам |
| Inflow threshold factor | externalEnergyInflowThresholdFactor | Порог собственной энергии |
| Inflow only for first offspring | externalEnergyInflowOnlyForFirstOffspring | Только первому потомку |
| Inflow for sources | externalEnergyInflowForSources | Передача энергии источникам |
| Backflow | externalEnergyBackflowFactor | Возврат энергии |
| Backflow limit | externalEnergyBackflowLimit | Предел внешнего запаса |

Group captions: Visualization -> Визуализация; Guided energy supply -> Управляемая
подача энергии. Actual types are **3 ColorSpec, 6 BoolSpec, 10 FloatSpec and 1
AlternativeSpec**. The request's five-checkbox subtotal omits one declaration;
all six are covered, including the per-color first-offspring setting.

| Original index | Canonical coloring option | Russian display |
| ---: | --- | --- |
| 0 | Energy | Энергия |
| 1 | Customization | Настройка |
| 2 | Lineage + Customization | Происхождение + Настройка |
| 3 | Lineage | Происхождение |
| 4 | Creature | Организм |

The five explanations, energy-dependent brightness, creature versus other-object
behavior, original Font Awesome chevrons and six double-newline separators are
preserved. There is no global alternative-name replacement dictionary.

Descriptions distinguish object bloom from fluid illumination, periodic rendering
from physical boundaries, source emission from lighting, and field visualization
from physical forces. Energy descriptions retain proportional shortage handling,
the constructor's own-energy fraction and 60% example, first-offspring limitation
and own-creature exception, per-color source amounts and relative shares, residual
energy particles, the external pool cap and the excessive-cell-production warning.

Terminology proposals specific to this slice: Customization -> Настройка,
reference domain -> основная область, backflow -> возврат энергии. These are
contextual presentation choices, not additions to the normative glossary. The
approved Происхождение, Внешняя энергия, Передача энергии and Расход энергии
are retained (with grammatical inflection where required).

## Architecture and invariants

L2-B2's copy-and-visibility filter remains unchanged. Original groups, parameter
indices, alternative ordering, member references, default values, ranges, logarithmic
flags, formats and infinity handling all come from immutable specifications.
Search matches English canonical names and the current display language, including
group names. It keeps the query string during EN -> RU -> EN. UTF-8 case folding and
highlight boundaries use the existing Utf8Search implementation. Clearing the
filter restores the complete original list. Alternative values are deliberately
not additional search terms; no global search mechanism is changed.

`alternativeLabels` accepts a ParameterSpec, checks the typed objectColoring member
and translates each original index. Other AlternativeSpec instances fall back to
their original names, even when their names/options equal this control's names.
Switcher retains its canonical `.values()` and receives independent `.displayValues()`;
a size mismatch falls back to canonical values. Arrow cycling, per-color expansion,
selected integers, reset callbacks and original nested parameter/name/row scopes
are unchanged.

ColorButton and ColorVectorButtons use optional `.label()` without changing `.name()`.
Picker, swatch, previous/current, palette, checkbox and reset IDs stay canonical.
The translated built-in default button explicitly retains the hash of its original
`Default##<canonical-name>` via PushOverrideID and an empty native button label;
its independently rendered caption determines its size. All palettes, drag/drop
payloads and original event handlers are retained. Existing callers without labels
keep their previous captions. Common wrapper-owned captions Choose/Current/Previous/
Palette/Default/Color and reset tooltips are localized. The common slider translates its formatted
Infinity caption only when a distinct display label is present; stored sentinels,
formats and numeric processing are unchanged. RGB/HSV/Hex channel notation
continues to come from the unchanged ImGui picker.

The editor's text width is 360 (previously 287) and minimum column width 650
(previously 550). These two layout constants reserve room for Russian captions,
help/reset controls and the longer built-in default button. No approved term is
abbreviated to fit.

## Verification

Windows Release succeeds with `build-windows-ninja.bat agent`. The old build-agent
was reversibly archived at `temp/l2b3a/build-before-clean`; only vcpkg_installed
packages were reused, with no compiled ALIEN objects/libraries retained. All ALIEN
translation units were rebuilt after the common GUI header layout changed. The
later generated DefaultLocale.h update was explicitly followed by recompiling
LocalizationService.cpp to avoid the known Ninja/MSVC include-prefix dependency gap.

| Suite | Passed | Failed | Disabled |
| --- | ---: | ---: | ---: |
| BaseTests | 41 | 0 | 0 |
| DataTests | 247 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 |
| PersisterTests | 141 | 0 | 1 |
| GuiLocalizationTests | 37 | 0 | 0 |
| Total | 546 | 0 | 1 |

Nine new headless tests cover the twenty bindings and complete descriptions;
unchanged groups; five canonical and localized alternative indices; production
widgets at base/layer/source with all twenty values and five coloring modes;
bilingual/case-insensitive group and parameter searches, original indices and
empty/no-match filters; English fallback including modes; real switcher arrow
and reset clicks; original picker/default IDs; palette color selection, Previous,
color/enabled-state resets and built-in customization defaults; Russian label/mode
font metrics at scales 1.0, 1.5 and 2.0; active numeric edits and reset, plus the
localized infinity display without changing its stored sentinel.

The first GUI run caught an attempted `###` default-button suffix that preserved
language stability but not the legacy hash. Explicit original-ID rendering fixes
that and is checked against the exact original hash, rather than only comparing
EN/RU IDs to each other. A test's nonexistent Physics group name was corrected to
Physics: Motion/Physics: Thresholds. A numeric logging assertion was moved inside
the same ImGui frame; it now verifies the actual Russian infinity text. These
intermediate failures are retained in ignored diagnostic logs.

The final seven-suite run leaves the real Windows profile byte-identical before
and after, SHA-256 `7FC787C3237120143F69C445756A962B34F5E8D1FDAB36BDFBDB4FA55501F5C5`.
Tests use process-local SettingsTestScope and synthetic values. The review
profile-only self-check also leaves its bracketed registry exports identical,
SHA-256 `93F18F157CE838C0E7D06ADFE4D221F5DC07FAD355C371514A1E3C45698DD2E9`.
An independent installed alien.exe process was observed and was not controlled or
modified. Between the two bracketed checks only its gui.imgui settings value
changed; those independent changes were not restored over.

The old build output autosave was retained in the archive and copied back to the
fresh normal output only while the destination was absent. Both copies have
SHA-256 `7C8BDDA22DAC8C746DD7AA170CAD3E1A266BE9A85C733331616F4D0F48B84156`.
It was never opened in these tests and is excluded from the isolated review package.

clang-format 19.1.5 checks pass for modified files that were clean at HEAD;
AlienGui.cpp was already non-clean and received narrow edits only. git diff --check
passes. Logs, XML, profile exports and review binaries are ignored under temp/l2b3a.

Python contracts pass: 18 tests across L2-A, L2-B1, L2-B2 and L2-B3a. They include
both catalogs (214 entries each), all twenty typed bindings/descriptions, exact
English specification text, five indexed options, chevrons/separators, common
captions and unchanged baseline hashes of Data specs/filter and related code.

The initial clean Release build exposed MSVC C2026 for the enlarged embedded
catalog. Line-separated adjacent raw literals address that build limit without
changing the LocalizationService API or catalog contents. The existing complete
embedded-English fallback test also exercises all new entries.

GPU/stress tests are unnecessary for this GUI/localization slice and are not run.
No build/test is cancelled. build-ninja, build-ninja-hip and .vs are untouched.

## GitNexus review

Pre-edit impact analysis reports MEDIUM for SpecificationGuiService (five direct
class dependencies), LOW for checkbox/alternative/color rendering (the parameter
group dispatcher, table and alternative recursion), LOW for the color helper
(ColorVectorButtons) and LOW for BasicSlider (SliderFloat/SliderInt and their
consumers). Presentation key lookup reaches label/description. Several public
wrapper entry points return UNKNOWN because callers are not resolved; explicit
source searches confirmed production parameter-editor and legacy browser/creator/
inspection/preview/signal-editor callers. UNKNOWN was not treated as unused code.
No HIGH or CRITICAL pre-edit verdict was returned.

The forced index-only refresh completed successfully (729526 nodes, 946881 edges,
436 flows). AGENTS.md contents and skip-worktree are preserved. The MCP metadata
still reports the previous documentation-only commit as one behind, while local
index metadata records the specified main SHA. The refresh does not remove known
receiver-typing and process-enumeration limits: absent indexed flows are not proof
of unaffected behavior. Source review and the production GUI tests supplement it.

The final all-scope analysis includes staged new files: **15 files, 44 changed
symbols, two affected processes, MEDIUM risk**. The full MCP response lists all
44 symbols without partial/truncated flags. The CLI agrees on the two processes:
Switcher -> Get and ColorVectorButtons -> Get. Context review confirms their
value-text/default helpers, canonical scopes, palette operations, localization and
style dependencies. No engine process is reported, and the explicit forbidden-path
diff and baseline-hash checks independently confirm unchanged simulation code.
The saved complete response is temp/l2b3a/graph-final.json.

## Isolated manual acceptance

The review runner uses SettingsTestScope to redirect HKCU only in its own process,
a fresh disposable profile, Russian by default, disabled save-on-exit and resources
copied without autosaves. It links the final production libraries. The original
application entry point and normal executable are unchanged. The profile-only self-check exits successfully. The launcher defaults
to scale 1.5 and accepts other scales explicitly:

```powershell
& 'E:\Projects\ALIEN\temp\l2b3a\review\Launch-L2B3a.ps1'
& 'E:\Projects\ALIEN\temp\l2b3a\review\Launch-L2B3a.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l2b3a\review\Launch-L2B3a.ps1' -Scale 2.0
```

1. Create a small disposable simulation; open Parameters, add a layer and radiation
   source. Inspect both new groups on their applicable base/layer/source entries.
2. Inspect every help marker, especially ray tracing, periodic repetition, the
   external pool warning, constructor threshold/first-offspring exception and source
   shares/backflow. Check long tooltip wrapping and all labels at 1.0, 1.5 and 2.0.
3. Change all three color parameters. Open each picker; inspect common Russian
   captions, change swatches and the palette, use Previous, the built-in defaults
   and reset. Check layer enable state and per-color expansion/reset behavior.
4. Cycle all five coloring modes, including individual colors and a mixed collapsed
   state. Switch EN -> RU -> EN while a picker is open and while values are edited.
   Verify colors, active mode, numbers, infinity, toggles and group states persist.
5. Search Visualization, визуализация, Glow, сВеЧеНие, Guided energy, управляемая,
   Передача энергии, BACKFLOW and an absent term. Inspect complete Cyrillic
   highlighting, retained filter text across language changes and the full list after
   clearing. Russian terms participate while Russian is selected, as in L2-B2.
6. Confirm General/Location still work and Shape/Force field/Physics/Radiation retain
   their original English presentation and behavior. Close the isolated runner.

Human visual acceptance is pending. Automated checks do not establish subjective
readability or qualify CUDA/ray-tracing behavior. Do not open user simulations,
import real settings, merge main or start L2-B3b during this acceptance step.
