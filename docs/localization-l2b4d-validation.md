# L2-B4d validation: attacker, digestor, defender and injector

Date: 2026-10-22 (Europe/Moscow).

## Baseline and scope

Base SHA: `03a3131ceac4d2f762d7de5e0ad510c7cae6b5d0`.
After fetching origin, main, origin/main and HEAD matched this SHA. The working
tree and index were clean. Work is isolated on `feat/ru-localization-l2b4d`.
The initial delivery did not merge main or begin L2-B4e. The subsequent
user acceptance and integration review are recorded below.

The user-provided AGENTS.md, normative terminology, L2-B3a/B3b/B3c and
L2-B4a/B4b/B4c validation reports, canonical parameter declarations, presentation
adapter, specification dispatcher, AlienGui, ColorMatrixDialog and relevant
AttackerProcessor, DigestorProcessor, CellProcessor and InjectorProcessor paths
were consulted. SimulationParameters::getSpec() and all model declarations are
unchanged. Identical captions such as Energy cost use different typed member
pointers; strings and positions do not identify parameter bindings.

## Parameters and terminology

| Canonical caption | Typed SimulationParameters member | Russian caption |
| --- | --- | --- |
| Energy cost | `attackerEnergyCost` | Расход энергии |
| Food chain color matrix | `attackerFoodChainColorMatrix` | Цветовая матрица пищевой цепи |
| Attack strength | `attackerStrength` | Сила атаки |
| Same lineage protection | `attackerRelatedLineageProtection` | Защита своего происхождения |
| Size protection | `attackerSizeProtection` | Защита по размеру |
| Attack radius | `attackerRadius` | Радиус атаки |
| Max raw energy conductivity | `maxRawEnergyConductivity` | Максимальная проводимость сырой энергии |
| Max raw energy conversion | `maxRawEnergyConversion` | Максимальное преобразование сырой энергии |
| Anti-attacker strength | `defenderAntiAttackerStrength` | Сила защиты от атак |
| Anti-injector strength | `defenderAntiInjectorStrength` | Сила защиты от инъекций |
| Energy cost | `injectorEnergyCost` | Расход энергии |
| Injection radius | `injectorRadius` | Радиус инъекции |

The four groups are Тип клетки: Атакующая, Тип клетки: Конвертер,
Тип клетки: Защитник and Тип клетки: Инжектор. Digestor is Конвертер.
Raw energy is сырая энергия; usable energy is доступная энергия. These remain
different energy stores. Defender modes retain Защита от атак and Защита от
инъекций. The normative glossary is unchanged.

The attacker cost describes an attack attempt; injector cost describes a
successful injection. Strength, radius, size protection and lineage protection
remain separate controls. Conversion transfers raw energy into usable energy;
it does not create energy. Conductivity scales transfer between connected cells
and digestors, with the canonical Energy conductivity property explicitly named.
Conversion preserves the canonical Energy conversion reference. Anti-attacker
help preserves division by (1 + value) per defender. Anti-injector help describes
increased injection cost, without promising prevention or new injector modes.

Existing source/documentation discrepancies are recorded rather than repaired:
InjectorProcessor currently uses `1 + numDefenders * 0.5f`, not the exposed
`defenderAntiInjectorStrength` value. DigestorProcessor computes its conversion
threshold using `1 - rawEnergyConductivity` and maxRawEnergyConversion; the
canonical help calls its scaled property Energy conversion. The translated
catalog preserves that canonical description. This localization does not claim
to validate or change these pre-existing model relationships.

Each catalog adds exactly 30 entries: 24 parameter name/description keys,
four group names and two numeric-matrix axis labels. EN/RU have identical sets
of 408 keys. Earlier entries are byte-for-byte equivalent as parsed values.
English parameter names and full descriptions exactly reproduce getSpec().
Absent external catalogs fall back to embedded English. The embedded English
LocalizationService object was explicitly removed and recompiled to avoid the
known Ninja/MSVC stale resource issue.

## Numeric food-chain matrix

The matrix remains BaseLayerParameter<ColorMatrix<float>>, 10 by 10, range
0..1, format %.2f, default all ones. Row means attacking customization color;
column means target customization color. Russian axes are Цвет атакующей клетки
and Цвет цели. The example retains row 2 and column 3. Zero means no energy
obtained; one means the mechanism's maximum; fractional efficiencies remain
numeric. The diagonal is editable, exactly as before.

InputFloatColorMatrixParameters receives optional label/localizationKey plus
axis strings with legacy defaults. InputFloatColorMatrix forwards them to the
existing ExpandedColorMatrixParameters<float>. SpecificationGuiService opts in
only after comparing the typed attackerFoodChainColorMatrix member pointer.
Other float member alternatives receive no label or localization prefix, their
original help, and the same original axes. Currently the canonical parameter
model declares only this one ColorMatrix<float>; generic float/int compatibility
is preserved for future and other callers. Int and bool wrappers are unchanged.

L2-B4b's existing shared path supplies translated Edit matrix, visible dialog
title, Adopt/Cancel and reset hint. ColorMatrixDialog itself is unchanged.
Its canonical window name remains the original English string. Its existing
setPresentation updates the open instance, and processContent resolves the title
and buttons each frame. Language changes do not reopen the dialog or replace its
matrix draft/callback. The label, expand-state key, edit button, dialog window,
child table and numeric slider ID construction all remain canonical. Numeric
slider IDs are tested using their original empty name/id scopes and color scope;
they are not assumed to match boolean checkbox IDs.

The direct numeric widget and actual layer specification dispatcher both receive
real Ctrl-click/text/Enter input for every one of the 100 cells. Asymmetric
[2][7] = 0.25 and [7][2] = 0.75 are checked independently, along with zero,
one and editable diagonal [9][9] = 1. All 100 adopted results are compared.
EN -> RU -> EN retains the popup instance, expansion/edit/cell IDs, original
source values and pending draft; adoption subsequently confirms draft retention.
A separate edit followed by Cancel preserves adopted data. Reset restores the
original matrix. The layer test preserves base values while editing an enabled
override. Synthetic parameter files round-trip base/layer matrices and the
other eleven parameters with the original serializer. No serializer code changes.

A complete source call-site review found one production InputFloatColorMatrix
caller: createWidgetsForFloatSpec. All its typed member alternatives are reviewed;
the opt-in predicate excludes every other float matrix. Python contracts compare
the entire original float wrapper/dispatch regions after removing only the
presentation extension, and preserve all remaining shared matrix handlers,
ColorMatrixDialog, bool/int paths and legacy defaults. Existing L2-B4b native
boolean matrix editing, disabled diagonal, tools and layout regressions pass.

## Search, controls and layout

The filter still searches canonical English and localized displayed text. It does
not search descriptions and introduces no stemming or synonyms. Tests cover all
four English/Russian group searches, mixed case, Food chain, Raw energy, Energy
cost, Attack radius and Injection radius, and their matching Russian forms.
Russian matching forms include пищев and сырой энергии. Exact nominative
Пищевая цепь and Сырая энергия do not match the inflected captions пищевой цепи
and сырой энергии under the preserved substring rule; native tests explicitly
record this. Empty-query restoration preserves declaration order and visibility.
Existing UTF-8 input, highlight-boundary and filter-focus regressions remain active.

Native mouse/keyboard tests edit/reset all eleven nonmatrix parameters, then expand
and independently edit color 2 of all nine color vectors, comparing every unaffected
neighbor. Language changes preserve input focus, color expansion and values.
All four group collapse/expansion states retain canonical IDs across languages.
Base/layer rendering and file round-trips retain enabled overrides. Python checks
independently assert every original range, format, logarithmic flag, type and
default, and compare unchanged model/serialization sources.

Headless production rendering checks widths 650/950 and scales 1.0/1.5/2.0.
Long captions remain after controls and expose full text on hover. Numeric matrix
layout renders target-axis text, Apply/Cancel and two-decimal numeric text in
narrow/wide dialogs at each scale; button bounds stay inside the dialog.
The original production help markers expose every complete description in both
languages at both widths and all three scales, including each paragraph of the
food-chain example. These are automated ImGui
layout checks, not human acceptance or an interactive GPU-rendered visual review.
The vertical attacking-axis appearance and practical readability remain on the
manual checklist below.

## Build and verification

Windows Release: `build-windows-ninja.bat agent`, successful, build-agent only.
No IDE build trees were configured, built into or deleted. clang-format 19.1.5
was applied only where the file was clean at HEAD. AlienGui.cpp has pre-existing
formatting differences, so only the four forwarding lines were inserted.
`git diff --check` passes.

| Suite | Passed | Failed | Disabled |
| --- | ---: | ---: | ---: |
| BaseTests | 41 | 0 | 0 |
| DataTests | 247 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 |
| PersisterTests | 142 | 0 | 1 |
| GuiLocalizationTests | 83 | 0 | 0 |
| Total | 593 | 0 | 1 |

Python: 45 checks pass (41 previous and four new substantive contracts). Nine new native tests supplement the previous 74 GUI tests; all 593 C++
tests pass with the one previous disabled Persister test. Logs/XML stay
under ignored temp/l2b4d. Early fixture failures were an invalid array assignment
and missing synthetic layer order metadata; they were corrected without changing
production behavior. GPU/EngineTests and stress tests are not run: no CUDA,
physics, cellular function or shared model layout changes require them.

## User data and isolated review

Tests use synthetic SimulationParameters and existing process-local
SettingsTestScope isolation. The review executable is separately linked from the
final production libraries with SettingsTestScope, Russian selected, autosave
and save-on-exit disabled. Its profile-only check exits successfully (0), also with scale 1.0 and 2.0.
The PowerShell launcher parses without errors.
It does not initialize a simulation during that check. Runtime resources and
DLLs are copied into its own directory; no real worlds, genomes or autosaves
are copied, opened or modified. No real-user registry write or restoration is
performed. Before/after real HKCU/Software/alien exports match:
SHA-256 `8A32BC6754E34A6C41397AD976CC8BFAB335B242AE2ABF24B619888B588F38E0`.
AGENTS.md retains SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`
and its original skip-worktree flag. README, dependencies, upstream, submodule,
normative terminology, model, CUDA, CLI, MCP and persistence code are unchanged.

Prepared launcher (ignored, local acceptance artifact):

```powershell
& 'E:\Projects\ALIEN\temp\l2b4d\review\Launch-L2B4d.ps1'
& 'E:\Projects\ALIEN\temp\l2b4d\review\Launch-L2B4d.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l2b4d\review\Launch-L2B4d.ps1' -Scale 2.0
```

The default scale is 1.5. No interactive launch is claimed; this package is ready
for user acceptance and does not ship as a tracked alternate application.

## GitNexus and limitations

Pre-edit impact analysis: adapter key has three resolved direct consumers
(label, description, alternativeLabels), LOW risk; the shared matrix path
resolves bool/int/float wrappers, LOW risk. Float dispatch resolves parameter
group rendering and its table/alternative consumers, LOW risk. Group label and
InputFloatColorMatrix have UNKNOWN/no resolved callers; explicit source searches
confirm their consumers. UNKNOWN is not used as evidence of safety. Test-file
lookups also lack graph anchors; CMake and Python discovery plus native execution
confirm those consumers. No HIGH or CRITICAL verdict was returned.

The index was refreshed successfully. Full-text/BM25 index creation emitted its
known failure; graph/embedding analysis completed. Receiver-resolution gaps and
bounded process enumeration mean absent flows do not prove absence of callers.
A fresh local backend runs final detect_changes(scope: all) after indexing releases
its lock: 19 files, 75 listed changed symbols, LOW risk, no indexed affected
processes, partial=false and truncated=false. The listed count matches the summary.
The process resource was inspected; its sampled inventory and missing GUI flows
remain limitations. Direct call-site review and passing native tests provide
additional evidence. Results are retained in temp/l2b4d/graph-final.json. No CodeQL
or submodule update was run.

## Changed tracked files

- `resources/locales/en.json`
- `resources/locales/ru.json`
- `source/Base/Tests/LocalizationServiceTests.cpp`
- `source/Gui/Impl/AlienGui.cpp`
- `source/Gui/Impl/AlienGui.h`
- `source/Gui/Impl/SimulationParametersPresentation.cpp`
- `source/Gui/Impl/SpecificationGuiService.cpp`
- `source/Gui/Impl/Tests/LocalizationGuiTests.cpp`
- `source/Gui/Impl/Tests/check_localization_l2a.py`
- `source/Gui/Impl/Tests/check_localization_l2b1.py`
- `source/Gui/Impl/Tests/check_localization_l2b2.py`
- `source/Gui/Impl/Tests/check_localization_l2b3a.py`
- `source/Gui/Impl/Tests/check_localization_l2b3b.py`
- `source/Gui/Impl/Tests/check_localization_l2b3c.py`
- `source/Gui/Impl/Tests/check_localization_l2b4a.py`
- `source/Gui/Impl/Tests/check_localization_l2b4b.py`
- `source/Gui/Impl/Tests/check_localization_l2b4c.py`
- `source/Gui/Impl/Tests/check_localization_l2b4d.py`
- `docs/localization-l2b4d-validation.md`

## Manual acceptance

1. Run the isolated launcher at 1.5. Create only a small disposable simulation.
   Open Parameters and inspect all four groups. Confirm Конвертер, сырая versus
   доступная энергия, attacker attempt cost versus successful injection cost,
   and both defender mode descriptions. Hover every help marker.
2. Search the English/Russian queries above, including mixed case, clear the
   filter and confirm full ordering. Use matching inflected forms/stems for
   food chain/raw energy, consistent with existing substring filtering.
3. Expand color-dependent controls, edit different colors and switch EN -> RU ->
   EN. Confirm unchanged values, input focus and expansion; exercise Reset.
4. Open the food-chain numeric dialog. Rows must be attacking colors, columns
   targets. Read both axes and the full row-2/column-3 example. Enter [2][7] =
   0.25 and [7][2] = 0.75, zero and one elsewhere, including the diagonal.
5. Switch EN -> RU -> EN with the dialog open before applying. Confirm live title,
   axes and button language, unchanged draft and two decimal places. Apply and
   verify both directions; reopen, change and cancel; confirm no source change.
   Reset and verify all ones. Repeat in an enabled layer override and confirm
   base values remain independent. Disable/re-enable the override as appropriate.
6. Save/reload only a synthetic parameter file, checking all matrix values and
   selected color controls. Recheck the boolean customization-transition matrix.
7. Repeat narrow/wide editor and dialog sizes at 1.0/1.5/2.0. Inspect vertical
   axis readability, clipped-caption full-text hover, numeric labels, and lack
   of overlap with buttons. Close without loading or saving real user worlds.


## User acceptance and integration review

Date: 2026-10-22 (Europe/Moscow).

The user explicitly accepts implementation commit
`f69224414674deaaccb47d1be479cd66a3bdf79c` and reports completing all six
items of the acceptance checklist supplied in the integration request:

1. The four groups and their descriptions.
2. Color-dependent parameters, language switching and reset.
3. The numeric food-chain matrix, directed row/column relationships and diagonal.
4. Apply, Cancel and language switching while the dialog is open.
5. Layer overrides, synthetic save/load and the boolean-matrix regression.
6. Layout under window-width and scale changes.

Result reported by the user: no remarks. This records those six items only;
it does not assert separate completion of all seven items in the earlier report
instructions or validation of real biological dynamics. The user authorizes
fast-forward integration into main, pushing main to origin and safe removal of
the completed feature branch locally and remotely. L2-B4e remains outside scope.

After fetching origin, main and origin/main both matched
`03a3131ceac4d2f762d7de5e0ad510c7cae6b5d0`; both feature refs matched the
user-reviewed implementation SHA. The index and working tree were clean, the
feature was ahead one / behind zero, and no additional worktree was registered.

Final review against main found no blocking issue. All four groups and twelve
typed pointer bindings have separate complete EN/RU names and descriptions.
Normative cell names and raw/usable energy distinctions are preserved, as are
attacker attempt cost and successful injection cost. The numeric matrix remains
10 by 10, row=attacker, column=target, floating coefficients 0..1, %.2f, default
ones, editable diagonal, independent base/layer values and original canonical
IDs. Open-dialog presentation updates preserve the draft, callback and instance;
Apply, Cancel and Reset handlers are unchanged. The boolean matrix and generic
matrix handlers retain their existing behavior. English fallback and prior
substantive native/Python regressions remain active. Model, engine, CUDA,
cellular algorithms, serialization, CLI, MCP, README, dependencies, upstream,
submodule and normative glossary have no diff against the baseline.

The injector's fixed `0.5f` coefficient in InjectorProcessor.cuh, instead of
`defenderAntiInjectorStrength`, remains a known defect of the original project,
unrelated to localization. The user has registered it separately and explicitly
deferred any fix to a later experimental fork. No injector algorithm, coefficient,
specification or related test is changed during integration. The existing Digestor
mechanism/description discrepancies above also remain unchanged and outside scope.

A fresh integration run confirms 593 passing C++ tests, zero failures and one
previously disabled Persister test: Base 41, Data 247, EngineInterface 2,
McpTools 40, Network 38, Persister 142 and GUI localization 83. All 45 Python
checks pass again. EN/RU retain exactly 408 keys with identical key sets; previous
catalog values and meaningful regression assertions are preserved. The production
code and catalogs are identical to the reviewed SHA; this follow-up changes only
this report. The successful Windows Release build remains valid. No rebuild or
embedded-English catalog regeneration is required; the rerun includes the native
missing-catalog English fallback checks. GPU/stress tests are not run.

Integration logs and graph results are retained under `temp/l2b4d/integration`.
User-data and final GitNexus verification details are recorded below before the
documentation commit and fast-forward integration.


The integration checks use synthetic parameters and the existing process-local
SettingsTestScope; no real worlds, genomes or autosaves are opened or modified.
The initial registry observation interval differed solely in one ImGui `Size`
line (1115,26 versus 1115,1108) while an independent alien.exe was running.
The exports alone cannot identify the writer; no registry restoration or user
setting edit was performed. A separate repeat around all 16 localization service
tests and all 83 GUI localization tests confirms byte-identical real profile
exports, SHA-256
`0A0E36DC9E206FF6095CD85305F5A48CE3930F0AE7662885F383A3D2A712780D`.
The independently running application is left untouched. AGENTS.md retains its
original bytes, SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`,
and its original skip-worktree flag. All 74 prior GUI test definitions and their
source are preserved; nine new tests supplement them.


The integration GitNexus index refresh succeeds, with the existing full-text/BM25
build warning and receiver/process coverage limitations. A fresh local backend
compares the branch against main: 19 files, 75 listed symbols, LOW risk, no indexed
affected processes, partial=false and truncated=false. The all-scope pre-commit
check covers only this documentation follow-up: one file, two listed Markdown
symbols, LOW risk, partial=false and truncated=false. Counts match their listings.
The missing caller anchors for InputFloatColorMatrix/group presentation are
supplemented by direct source inspection of SpecificationGuiService; absence
of indexed callers/processes is not evidence that those paths are unused.
`git diff --check` passes. This documentation commit is published on the feature
branch before the separately verified fast-forward of main; history is preserved.
