# L2-B4a validation: cell life cycle, construction and lineage

Date: 2026-10-22 (Europe/Moscow).

## Baseline and scope

Base SHA: `eff787fc38247f338a7be3e07bde07968c43669f`.
An origin fetch confirmed HEAD, main and origin/main at that exact commit, with
a clean working tree, before creating `feat/ru-localization-l2b4a`.
The customized AGENTS.md and its skip-worktree flag are preserved.
The normative terminology and L2-B2, L2-B3a, L2-B3b and L2-B3c reports were read.

Three group captions and eight complete parameter names/descriptions are added
through the existing presentation adapter. Each catalog now contains 325 entries,
with identical EN/RU key sets; the previous 306 entries are unchanged.
No L2-B4b work or main integration is included. Human acceptance remains pending.

## Changed files

- `resources/locales/en.json`
- `resources/locales/ru.json`
- `source/Base/Tests/LocalizationServiceTests.cpp`
- `source/Gui/Impl/SimulationParametersPresentation.cpp`
- `source/Gui/Impl/Tests/LocalizationGuiTests.cpp`
- `source/Gui/Impl/Tests/check_localization_l2a.py`
- `source/Gui/Impl/Tests/check_localization_l2b1.py`
- `source/Gui/Impl/Tests/check_localization_l2b2.py`
- `source/Gui/Impl/Tests/check_localization_l2b3a.py`
- `source/Gui/Impl/Tests/check_localization_l2b3b.py`
- `source/Gui/Impl/Tests/check_localization_l2b3c.py`
- `source/Gui/Impl/Tests/check_localization_l2b4a.py`
- `docs/localization-l2b4a-validation.md`

Earlier Python checks and the Base catalog test change only their expected catalog
count. Earlier headless untranslated-group fixtures now use still-untranslated
Cell type: Reconnector and Cell type: Detonator. Their assertions remain active.

## Typed bindings and terminology

Every prefix below has independent `.name` and `.description` entries. Bindings
compare original typed member pointers, never English captions or translated names.
English catalog descriptions exactly reproduce the original specifications,
including Font Awesome markers, paragraph boundaries and ordering.

| Original name | Member | Type | Russian display | Catalog prefix |
| --- | --- | --- | --- | --- |
| Maximum age | `maxCellAge` | `IntSpec` | Максимальный возраст | `parameters.lifecycle.maxcellage` |
| Maximum free cell age | `freeCellMaxAge` | `IntSpec` | Максимальный возраст свободных клеток | `parameters.lifecycle.freecellmaxage` |
| Minimum energy | `minCellEnergy` | `FloatSpec` | Минимальная энергия | `parameters.lifecycle.mincellenergy` |
| Normal energy | `normalCellEnergy` | `FloatSpec` | Нормальный уровень энергии | `parameters.lifecycle.normalcellenergy` |
| Decay rate of dying cells | `cellDeathProbability` | `FloatSpec` | Вероятность распада умирающих клеток | `parameters.lifecycle.celldeathprobability` |
| Energy to cell free transformation | `particleTransformationAllowed` | `BoolSpec` | Образование свободных клеток | `parameters.lifecycle.particletransformationallowed` |
| Connection distance | `constructorConnectingCellDistance` | `FloatSpec` | Расстояние проверки соединений | `parameters.construction.constructorconnectingcelldistance` |
| New lineage threshold | `newLineageThreshold` | `FloatSpec` | Новое происхождение: порог | `parameters.mutations.newlineagethreshold` |

Group captions: Cell life cycle → Жизненный цикл клеток; Cell construction →
Строительство клеток; Mutations → Мутации.

The proposed lineage caption is reordered to retain the exact normative noun
Происхождение in the nominative case. The required search Происхождение cannot
match the genitive происхождения in the proposed Порог нового происхождения
through the existing substring filter. Новое происхождение: порог preserves the
meaning and makes that search work without changing filtering or adding stemming.
This is a working wording for acceptance, not a glossary amendment.

Both age descriptions explicitly count simulation steps. Ordinary cells enter
the dying state before eventual disintegration. Free cells arise from energy
particles and disintegrate back into energy particles after exceeding their
separate age limit. Minimum energy explicitly means usable energy.

Normal energy retains all four applications in order, with four chevrons and
four double-newline separators: surplus transfer to connected cells still building;
energy above normal going into depot storage; construction permitted only with
sufficient residual constructor energy; and the permitted transformation of an
energy particle into a free cell at the normal level.

Decay means probability per simulation step. All three dying-state conditions
remain: insufficient energy, excessive age and lost connection to the creature's
head cell. The final separate paragraph retains free-cell disintegration with
the same probability when energy is insufficient. No condition is described as
instant destruction. The transformation checkbox concerns energy particles.

Connection distance describes intersection checking near the first cell of a new
branch and postponed construction on intersection. It is distinct from Maximum
distance, the previously localized maximum connection length. The lineage threshold
assigns a new lineage ID and resets the accumulated-mutation counter; it does not
create a species, change mutation probabilities, trigger mutations or edit a genome.
Constructor, generator, free cells, creature cells, raw/usable energy, dying and
final disintegration retain their distinctions. The normative glossary is unchanged.

## Preserved behavior and energy constraints

There are no implementation changes to Data, Engine, CUDA, persistence, serialization,
MCP, CLI, SpecificationGuiService or AlienGui. SimulationParameters.cpp/.h,
SimulationParametersSpecification.h, ParametersValidationService and
SpecificationEvaluationService are byte-compared against the fixed baseline by
Python checks. Member pointers, declaration indices, numeric ranges/formats,
logarithmic/infinity flags, color dependence, defaults, reset behavior and canonical
ImGui ID stacks are unchanged. No physics or biological model is modified.

The normal-energy declaration still contains
`.greaterThan(&SimulationParameters::minCellEnergy)`. A C++ test inspects the typed
dependency and exercises the real validator with seven distinct per-color minimum
energies above the initial normal energy. The existing validator raises each normal
value to the corresponding minimum using `std::max`; it allows equality. The test
then renders and validates EN → RU → EN and compares the complete parameter objects.
That initial normalization is pre-existing validation, not a localization effect.

The unchanged L2-B3c IntSpec path supplies translated labels/descriptions to
SliderInt. Both ages retain 1…10,000,000, logarithmic mode and the original infinity
sentinel. Decay retains 0.000001…0.1, logarithmic mode and `%.6f`. Construction
retains 0.1…4.0. Lineage retains 0…10,000, logarithmic mode, infinity and `%.5f`.

The existing UTF-8 truncation/full-text-hover path handles long age captions.
Dedicated tests render the production age controls at column width 650 scaled
by 1.0/1.5/2.0, hover clipped captions, verify the full text is available, and
check that caption positions remain after the slider/reset controls. Separate
tests exercise all eight captions and full descriptions in narrow text areas.
Multiline tooltip log checks compare each paragraph line because ImGui logging
adds indentation; the displayed description itself remains intact.

## Automated verification

Windows Release succeeds with `build-windows-ninja.bat agent`, using build-agent
only. The initial build explicitly removes and recompiles the four affected
objects: LocalizationService (embedded English catalog),
SimulationParametersPresentation, LocalizationServiceTests and LocalizationGuiTests.
The build log confirms each compilation and relinking. Later test-only changes
are compiled with the same required script. No shared layout or CUDA header changed,
so an engine clean rebuild is unnecessary. IDE-owned build trees are untouched.

| Suite | Passed | Failed | Disabled |
| --- | ---: | ---: | ---: |
| BaseTests | 41 | 0 | 0 |
| DataTests | 247 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 |
| PersisterTests | 142 | 0 | 1 |
| GuiLocalizationTests | 65 | 0 | 0 |
| Total | 575 | 0 | 1 |

All 33 Python checks pass (29 previous plus four new). The nine new C++ headless
tests supplement the previous 56 GUI tests. Real ImGui mouse/keyboard handlers
edit/reset all five FloatSpec declarations and both IntSpec declarations. Color
expansion is clicked, individual rows are edited, unaffected neighbors are checked,
and EN → RU → EN retains active IDs, input focus, values and expanded state.
Both ages render localized infinity and reset correctly. The real checkbox toggles
and resets particle transformation, retaining the complete parameter state across
language switches. Lineage float infinity is rendered without changing its value.

Binding tests compare all names and full descriptions with catalog values,
check typed lookup after replacing canonical captions, and verify embedded English
fallback with absent external catalogs. Search tests cover Cell life cycle,
Жизненный цикл, Maximum age, Максимальный возраст, Normal energy, Нормальный,
Cell construction, Строительство, New lineage and Происхождение, mixed case,
UTF-8 match boundaries and unchanged declaration order after clearing. Existing
real UTF-8 input/highlight and filter-focus tests remain active, as do group
collapse-state tests extended with these three groups.

Synthetic state tests render base and enabled layer overrides, compare complete
SimulationParameters objects across languages, and round-trip disposable parameter
files containing all eight declarations, per-color ages/energies/decay/construction,
layer values, infinity and the transformation flag. Decay values use floating-point
tolerance across the unchanged serializer's decimal conversion. Disabled overrides
retain the existing widget normalization rules; the invariance fixture enables
the layer overrides it intends to preserve.

Initial test failures were confined to fixture expectations: ImGui log indentation
for multiline text, absent layer metadata in the defaults object, and the nominative
lineage search discussed above. The final checks pass without production behavior
changes beyond the new presentation bindings and catalog entries.

clang-format 19.1.5 is applied only to the three modified C++ files after confirming
they are clean at HEAD. `git diff --check` passes. Logs, XML, registry exports and
the isolated review package remain ignored under `temp/l2b4a`.
GPU/EngineTests and stress tests are not run; this is presentation/CPU validation.

## User data and isolated review

Tests use synthetic parameters and SettingsTestScope. The isolated review executable
links the final production libraries, redirects its process to a disposable registry
profile, selects Russian, disables autosave/save-on-exit and runs in its own directory.
Its profile-only self-check succeeds without initializing a simulation. The real
HKCU/Software/alien exports before/after tests and profile checking are byte-identical,
SHA-256 `7501B7C9102AD6BAAFF45F846CABB9FB9AF204140FF115119430DCF582809B79`.
No user registry restoration is performed. User worlds and autosaves are not copied
or opened. Runtime assets/DLLs and earlier synthetic empty samples are packaged.

```powershell
& 'E:\Projects\ALIEN\temp\l2b4a\review\Launch-L2B4a.ps1'
& 'E:\Projects\ALIEN\temp\l2b4a\review\Launch-L2B4a.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l2b4a\review\Launch-L2B4a.ps1' -Scale 2.0
```

Default scale is 1.5. The package is prepared for user review; no interactive GPU
session or human visual acceptance is claimed by these headless tests.

## Manual acceptance

1. Launch the isolated runner at 1.5. Create a small disposable world or use a
   synthetic sample; open Parameters and inspect all three groups.
2. Read every full tooltip, especially dying versus final disintegration, energy
   particles versus cells, all four normal-energy applications, construction
   intersection checks and lineage-ID assignment without genome changes.
3. Expand both ages, minimum/normal energy, decay and construction by color.
   Enter distinct values, switch EN → RU → EN, confirm expanded state and values,
   then reset through the original buttons. Try finite and infinite ages.
4. Raise minimum energy above normal energy and observe the original validation
   adjustment. After it stabilizes, switch languages and check for no extra changes.
5. Toggle and reset free-cell formation. Edit lineage threshold, including infinity,
   and confirm that switching language changes only presentation.
6. Search every English/Russian query listed above, including mixed case. Verify
   complete Cyrillic highlighting, retained query, group state and full list/order
   restoration after clearing. Recheck previously localized groups.
7. Resize the editor, hover clipped age captions to read their full text, and
   repeat at 1.0 and 2.0. Save/reload only a disposable parameter file if desired.
   Close without loading real user simulations.

## GitNexus

The pre-edit upstream impact of adapter `key` is LOW: three direct consumers,
label, description and alternativeLabels, with no indexed affected process.
Group label and test-file lookups return UNKNOWN/no resolved callers; source
searches explicitly confirm SpecificationGuiService's group/control consumers
and CMake's test registration. UNKNOWN is not treated as an all-clear. The graph
also reports unresolved receiver types and bounded process enumeration, so absent
processes do not imply absent execution paths. The stale baseline index was
refreshed successfully. The final index refresh reports a failed full-text/BM25
index build; this does not prevent symbol/diff-based change analysis. No full-text
search result is used as evidence of absence. A fresh local backend performs the
final `detect_changes(scope: all)` after indexing releases its database lock:
13 changed files, 44 listed changed symbols, LOW risk, zero indexed affected
processes, `partial: false` and `truncated: false`. The listed-symbol count matches
the summary. The process inventory was inspected; its limited GUI coverage and
enumeration bounds remain limitations, supplemented by source checks and real
headless widget tests. Results are saved in `temp/l2b4a/graph-final.json`.
