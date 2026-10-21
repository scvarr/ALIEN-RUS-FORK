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

## Follow-up: switcher layout after visual review

Baseline for this additional commit: `da2b5c98f2d6a18c71d23f5889e024b9dc7d4859`.
The user reported clipping of Происхождение + Настройка at scale 1.5; scale 1.0
in a maximized application fit. This is a reported defect, not visual acceptance
of the original commit. Repeat human visual acceptance remains pending.

The original Switcher width calculation reserved `parameters._textWidth` in every
expanded color row, although only row zero renders the reset/caption/help controls.
In the parameter editor this unnecessarily removed 360 logical units from every
row. Row zero also reserved 360 units instead of measuring its much shorter
Окраска объектов caption. Color swatches, the expansion button, arrows and frame
padding further reduce the real input width. A maximized window at scale 1.0 hid
this problem. The previous test measured text against assumed spare space; it did
not exercise actual scaled widget geometry and could not detect this defect.

The common Switcher now:

- Reserves trailing text space only for row zero. Other expanded rows use the full
  remaining width after their color swatch and arrows.
- Measures an independent display caption plus the actual reset/help controls,
  keeping the configured reservation as an upper bound. If caption and a complete
  individual mode cannot fit on one line, the caption/reset/help move below the
  input and arrows. The editor's 650/360 constants need no further increase.
- Clamps input width to at least one pixel instead of passing a negative width
  to ImGui, where negative widths have a different meaning.
- Compares the complete value against the actual input rectangle, frame padding
  and window clip rectangle. Hover reveals the full, wrapped value when clipped,
  including the deduplicated mixed-color list. Disabled/read-only fields also
  expose this tooltip. No alternative text is shortened or reordered.

Canonical names, row scopes, input/arrow/reset IDs, selected integers, arrow
handlers, defaults and expansion-state IDs are unchanged. Plain legacy Switcher
callers retain their configured width/textWidth calculation; they also gain the
full-value tooltip when clipped. SpecificationGuiService still supplies canonical
values and independent localized labels; it requires no special-case implementation.

Four additional headless GUI tests run with real fonts, ImGui style and the active
WindowController scale at 1.0, 1.5 and 2.0. They verify expanded-row geometry at
220/400/650 logical widths, first-row caption wrapping and full hover text at
200/420/650 widths, uniform and mixed color modes, EN -> RU -> EN preservation,
read-only tooltips and actual canonical input IDs through hit-testing. Legacy
examples cover browser sorting, creation material, timeline/autosave modes,
constructor branch counts, signal editing and preview mode. Existing real arrow,
reset, palette, filter, active-edit and mode-index tests remain passing. Python
contracts compare arrow handlers and mixed-list ordering against normalized hashes
of the baseline implementation.

WindowController.h and OverlayController.h only gain friend access for the existing
headless fixture; they gain no fields, public APIs or production behavior. Pre-edit
impact for OverlayController is HIGH (19 importers), so that risk was reported
before editing. The ambiguous WindowController query also included a CRITICAL
forward declaration; resolving the actual header gives MEDIUM with 13 direct
importers and a dispatch-boundary limitation. These shared-header changes prompted
a clean build of all ALIEN translation units in build-agent.

## Follow-up: Unicode simulation paths

The failure occurs at the GUI path-to-text boundary, not in the notification font.
A Windows diagnostic on this machine reports ACP 1251. For `Start - мир.sim`:

- Native UTF-16 contains U+043C U+0438 U+0440.
- `path.string()` produces bytes EC E8 F0 for those characters.
- `path.u8string()` produces UTF-8 D0 BC D0 B8 D1 80.

ImGui expects the latter. Sending the ACP bytes to OverlayController produced
invalid UTF-8 display even though converting them back to a native path on this
machine round-tripped. The diagnostic records actual bytes/code points in
`temp/l2b3a/followup/path-diagnostic.cpp` and its executable output; it does not
infer data loss from a screenshot. The overlay's Noto Sans Mono font already
includes Russian glyphs and U+2116. No font or code-page change is needed.

The reviewed path flow is:

1. ImFileDialog finalizes its UTF-8 input as `std::filesystem::path`, with native
   directory selections. GenericFileDialog forwards the path without narrowing.
2. FileTransferController now passes native paths directly into the existing
   ReadSimulationRequestData/SaveSimulationRequestData path members. The save
   callback uses its supplied native path instead of retrieving/narrowing it again.
3. PersisterWorker already passes a native path to SerializerService and returns a
   native basename in ReadSimulationResultData. Those request/result contracts
   and worker processing remain unchanged.
4. Only display/log strings use `PathHelper::toUtf8`. In particular, the completion
   notification converts the result's basename explicitly to UTF-8.
5. The .sim file-opening boundary used zstd's narrow-string file wrappers. It now
   opens std::ifstream/ofstream with the native path and wraps that stream in the
   same zstd::istream/ostream. Compression level, worker count and serialization
   routines are unchanged. Output is explicitly flushed while errors are caught.

No Persister public contract or .sim layout is changed. This is a small I/O-boundary
fix, not a migration of the persistence subsystem. Existing CLI/MCP callers continue
using the same native-path APIs. The new tests compare the complete output bytes
against serializeSimulationToString, confirming identical compressed .sim data for
the same synthetic simulation.

The remembered directory has a distinct contract: GlobalSettings strings are UTF-8,
whereas the unchanged vendored dialog's startingDir string is locale encoded.
FileTransferController now stores its shared reference directory as UTF-8 and
converts only at that legacy dialog boundary. This adaptation is also applied to
its genome-dialog entry points because they share the same reference directory;
genome serialization and file-opening code are unchanged. `toLegacyString` accepts
only exact native-path round trips, catching MSVC's std::system_error when a
character is unrepresentable. It never passes replacement question marks. For an
unrepresentable starting directory, the dialog retains its current folder rather
than losing the stored UTF-8 path. Selected native paths still load/save correctly.

This patch does not rewrite all internal strings of the vendored file dialog.
Its breadcrumb/favorite/new-entry handling and arbitrary non-ACP starting folders
would require a separate dialog-level Unicode stage. No dependency source/version
is changed here. The four requested Russian names and Cyrillic folders are
representable on this Windows installation and are covered in the tests below.

Regression coverage uses only synthetic data and temporary directories:

| Filename | ASCII directory | Cyrillic directory |
| --- | --- | --- |
| start.sim | Save/read/bytes verified | Save/read/bytes verified |
| Старт.sim | Save/read/bytes verified | Save/read/bytes verified |
| Start - мир.sim | Save/read/bytes verified | Save/read/bytes verified |
| Тестовый мир №1.sim | Save/read/bytes verified | Save/read/bytes verified |

A ninth save/read case uses a directory with 漢, outside CP1251, to verify that
native .sim I/O no longer depends on the system code page. GUI tests verify native
request/result paths, UTF-8 code points, filename/parent-directory round trips,
UTF-8 settings persistence in an isolated registry, notification contents, glyph
coverage and actual overlay drawing. No real simulation or autosave is loaded.

## Follow-up verification and repeat acceptance

`build-windows-ninja.bat agent` succeeds in Release after a clean build-agent
rebuild. The previous tree is retained in temp/l2b3a/followup/build-before-clean;
only vcpkg_installed packages were reused. IDE trees and .vs are untouched.
The first link exposed GTest's missing char8_t string printer in its prebuilt DLL;
the test uses an equivalent boolean equality assertion, without changing GTest.
The initial unsupported-character test exposed MSVC's std::system_error exception
type; the legacy-boundary handler was corrected and the final test passes.

| Suite | Passed | Failed | Disabled |
| --- | ---: | ---: | ---: |
| BaseTests | 41 | 0 | 0 |
| DataTests | 247 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 |
| PersisterTests | 142 | 0 | 1 |
| GuiLocalizationTests | 41 | 0 | 0 |
| Total | 551 | 0 | 1 |

All 20 Python checks pass. The full CPU/GUI run leaves real registry exports
byte-identical, SHA-256
`9DF2C4DC97003AC89613B1517F5F8A4FB0B33CC9F41FB52178D36245D90B24C6`.
Tests use SettingsTestScope and temporary files; no existing profile is restored
over user changes. GPU/stress suites are not run because engine/CUDA algorithms
are unchanged. clang-format 19.1.5 is used only on files clean at HEAD and the new
PathHelper header; AlienGui.cpp and FileTransferController.cpp receive narrow edits.

The new isolated package is separate from the original review executable and uses
a disposable profile, Russian, disabled autosaves/save-on-exit and scale 1.5 by
default. It links the final production libraries. Its profile-only check passes.
Eight synthetic, empty 256x256 .sim files are included under samples/ascii and
samples/Тестовый каталог, with all four requested filenames in both directories.

```powershell
& 'E:\Projects\ALIEN\temp\l2b3a\followup\review\Launch-L2B3a.ps1'
& 'E:\Projects\ALIEN\temp\l2b3a\followup\review\Launch-L2B3a.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l2b3a\followup\review\Launch-L2B3a.ps1' -Scale 2.0
```

For repeat acceptance, open one of these synthetic samples and Parameters.
Choose Происхождение + Настройка, expand all colors, resize the window and compare
1.0/1.5/2.0. Set different per-color modes, collapse, and hover the composite list.
Check the full tooltip at narrow sizes, then switch EN -> RU -> EN and confirm the
same values and modes. Open each named sample from both sample directories, inspect
the notification and save a disposable copy into a Cyrillic directory. Reopen it
and check that the dialog returns to the saved folder. Do not use real simulations.

Repeat visual acceptance is pending. No merge into main or L2-B3b work is performed.

Follow-up graph review: the index-only refresh and forced refresh both completed.
The MCP transport closed during reindexing; the required CLI fallback and its
same LocalBackend `detect_changes` implementation were used. The saved full JSON
reports **10 changed files, 39 symbols, LOW risk**, with all 39 symbols listed and
neither partial nor truncated. No indexed process is reported; the index also
warns about its process-enumeration/receiver-resolution limits, so this is not
proof of zero GUI impact. Explicit call-site review, source contracts and real
headless widget/persistence tests supply the additional evidence. CLI and structured
results agree; see temp/l2b3a/followup/graph-final.json and graph-final.log.

The new review executable's profile-only check also leaves bracketed real registry
exports identical to the full-suite hash above. The archived output autosave is
copied back only if the new normal-output destination is absent; both copies retain
SHA-256 `18E655712326C4CBF63AA9DCFB032C1B3263F72DBA9DFB23ADB41F8A7AF7E181`.
It is not included in the review package. A separate sample helper uses /utf-8;
the eight resulting basenames and the Cyrillic directory are verified against
exact Unicode strings before delivery. No real files are used as sample data.
