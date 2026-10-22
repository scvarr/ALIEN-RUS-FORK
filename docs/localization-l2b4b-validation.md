# L2-B4b validation: property and cell-identity meta-mutations

Date: 2026-10-22 (Europe/Moscow).

## Baseline and scope

Base SHA: `a3d2ae563c4be86b2c307a3ec1d69ec07e1ed78d`.
Local main and origin/main matched this commit before and after fetching origin.
The working tree and index were clean. Work is isolated on
`feat/ru-localization-l2b4b`. The initial delivery did not merge into main.
The accepted integration review is recorded below; no L2-B4c work is included.
AGENTS.md, the terminology document, the L2-B3a/B3b/B3c/B4a reports, specifications,
presentation/dispatch code, matrix widgets/dialog and relevant mutation operations
were consulted. The normative glossary is unchanged. Initial interface wording was provisional;
user acceptance is now recorded in the integration review below.

This slice adds two group captions and ten complete names/descriptions through the
existing presentation adapter. Both catalogs contain 356 matching keys: the previous
325 entries are unchanged; 31 entries are added (20 parameter entries, two group
captions, nine matrix-control captions). Canonical specifications are unchanged.

## Typed bindings and translations

Each field is selected by a typed member pointer, never a caption. Every catalog
prefix has separate `.name` and `.description` keys. English descriptions exactly
reproduce `getSpec()`, including the shared `metaMutationDescription()` expansion.

| Original | Field | Type | Russian caption | Catalog prefix |
| --- | --- | --- | --- | --- |
| Neuron sigma | `neuronsMetaMutationsSigma` | FloatSpec | Нейроны: сигма | `parameters.metamproperties.neuronsmetamutationssigma` |
| Connection sigma | `connectionsMetaMutationsSigma` | FloatSpec | Соединения: сигма | `parameters.metamproperties.connectionsmetamutationssigma` |
| Cell type property sigma | `cellTypePropertiesMetaMutationsSigma` | FloatSpec | Свойства типов клеток: сигма | `parameters.metamproperties.celltypepropertiesmetamutationssigma` |
| Constructor sigma | `constructorMetaMutationsSigma` | FloatSpec | Конструктор: сигма | `parameters.metamproperties.constructormetamutationssigma` |
| Geometry sigma | `geometryMetaMutationsSigma` | FloatSpec | Геометрия: сигма | `parameters.metamproperties.geometrymetamutationssigma` |
| Cell type sigma | `cellTypeMetaMutationsSigma` | FloatSpec | Тип клетки: сигма | `parameters.metamidentity.celltypemetamutationssigma` |
| Cell type mode sigma | `cellTypeModeMetaMutationsSigma` | FloatSpec | Режим типа клетки: сигма | `parameters.metamidentity.celltypemodemetamutationssigma` |
| Void sigma | `voidMetaMutationsSigma` | FloatSpec | Донор: сигма | `parameters.metamidentity.voidmetamutationssigma` |
| Customization sigma | `customizationMetaMutationsSigma` | FloatSpec | Настройка: сигма | `parameters.metamidentity.customizationmetamutationssigma` |
| Customization transition matrix | `customizationTransitionMatrix` | BoolSpec / ColorMatrix<bool> | Матрица переходов настройки | `parameters.metamidentity.customizationtransitionmatrix` |

Groups: Meta-mutations: Properties → Метамутации: свойства;
Meta-mutations: Cell identity → Метамутации: идентичность клетки.

Sigma means the standard deviation of a Gaussian change to a mutation probability
stored in a genome. It is neither that probability itself nor the magnitude of a
neuron/constructor/geometry edit. All nine Russian descriptions retain the genome
condition `Apply meta-mutations` (with a Russian explanation) and once-per-offspring
application. None claims a guaranteed mutation at sigma 1 or per-timestep execution.
Zero, defaults, FloatSpec ranges 0…1, logarithmic mode and `%.5f` remain unchanged.
The headless tests enter 0, 0.00001 and 0.99999 through real Ctrl-click numeric-input
handlers for each of the nine production-dispatched controls, switch EN → RU → EN
with the numeric field still active, assert unchanged focus/ID/value, and click reset.

Void is resolved rather than guessed. `MutationProcessor::applyMutations_void`
reads `genome->mutationRates.voidMutation.nodeProbability` and switches eligible
nodes between `CellType_Void` and `chooseRandomCellTypeExceptVoid`. The separate
meta-mutation changes this stored probability using `voidMetaMutationsSigma`.
This is the Void cell type already called Донор in the normative glossary.
The description retains `(Void)` and explains the switch between donor and non-donor
types. It is distinct from ordinary cell-type changes, type-mode changes and
customization-color changes. No glossary amendment is made.

## Matrix identity, direction and editing

Only the BoolSpec bound to `customizationTransitionMatrix` receives the new optional
presentation fields. `CheckboxColorMatrixParameters` and
`ExpandedColorMatrixParameters<T>` now accept an optional display label and an
optional localization-key prefix. Defaults retain previous captions and axes for
other matrices; numeric matrix dispatch/rendering is unchanged.

The original `.name()` remains the seed for the outer widget, expansion state,
reset, table, rows/columns, checkboxes and dialog. `DisplayButton` computes the
legacy canonical button ID, uses it as an override seed for an empty-label native
ImGui button, and separately renders its display text. This preserves the exact
pre-localization ID rather than introducing a different `###` hash. Edit matrix,
Adopt, Cancel, Clear, Select all, Invert and Randomize retain their canonical IDs.
The localized reset tooltip uses the existing common translation.

The dialog's native window/popup title remains the canonical name, preserving its
exact window ID and child seeds. Its visible title is painted separately in the
title-bar clip rectangle, leaving the existing close button and drag behavior.
This avoids changing the native window hash through a translated title. The
presentation refresh updates labels and axes of an already-open dialog without
reopening it, copying source values, replacing the draft or invoking its callback.

Russian axes are Исходная настройка (rows) and Новая настройка (columns).
The engine still tests
`customizationTransitionMatrix.value[candidateSourceColor][color]`.
The unchanged table accesses `value[row - 1][col - 1]`; its color headers still
access the corresponding original row/column indices. Ten colors, their palette,
all 100 values, order, diagonal exclusion and default `i != j` are unchanged.
The adapter never transposes, sorts, regenerates or normalizes the matrix.

The production-widget/dialog test opens with the actual Edit matrix handler,
expands the read-only preview, activates cell [2][7] through the native ImGui
checkbox navigation handler, attempts [4][4], switches languages while the draft
is open, and applies it. It asserts every cell: only [2][7] changes, including an
explicit unchanged [7][2] and diagonal. A second open edits [7][2] and cancels;
the source remains unchanged. Reset restores the full original matrix. The test
also asserts the original popup hash, cell ID, 11 table columns including the
color-header column, edit-button ID and expanded preview across language changes.
The two selected color indices refer to different unchanged palette colors.

Localized dialog tools occupy their own footer rows and wrap at narrow widths;
legacy dialogs retain their original footer layout. Headless rendering checks
Russian control text and the column-axis text at scales 1.0/1.5/2.0, host widths
650/950 logical pixels and resized dialog widths 350/700. It asserts footer bounds
and unchanged values. Existing font tests cover Cyrillic glyphs, including the
vertical row-axis renderer. This is automated geometry/text verification; no
interactive GPU visual acceptance or evolution simulation is claimed.

## Search, fallback and regressions

All ten original and translated parameter names and both groups are exercised.
The explicit query set includes Meta-mutations, Метамутации, Neuron sigma,
Нейроны, Cell type, Тип клетки, Customization, Настройка, Matrix and Матрица.
The existing semantics are retained: canonical English plus the active display
language are searchable, so English queries work in EN and RU; Russian captions
are searchable in RU. In particular Matrix discovers the same matrix in either
language, and Матрица discovers its Russian caption. Matching is case-insensitive
through the unchanged UTF-8 helper. Filtering preserves declaration order/references;
canonical group/expansion IDs and existing highlight behavior remain unchanged.

Missing-catalog tests retain the embedded English fallback for all ten names and
descriptions. Earlier Python assertions retain their original hashes/baselines:
a normalization helper excludes only the exact B4b widget-extension sites before
applying old whole-file contracts. A new test separately proves every other byte
of AlienGui.cpp/.h and SpecificationGuiService.cpp matches the B4b base. This keeps
previous controls, dispatch paths and algorithms covered without freezing the
newly authorized presentation extension. Earlier catalog tests update only counts.

## Build and automated results

Release builds successfully with `build-windows-ninja.bat agent`, exclusively in
build-agent. No IDE build tree is configured, built or deleted. clang-format 19.1.5
is used only for changed files clean at HEAD; AlienGui.cpp has pre-existing format
violations and is not whole-file formatted.

The initial Base fallback test exposed the known Ninja/MSVC dependency issue:
DefaultLocale.h was regenerated but LocalizationService.cpp.obj remained stale.
Removing only that object from build-agent and rerunning the prescribed build
forces the embedded catalog to compile and all consumers to relink. The final
seven-suite run passes after this correction. The isolated runner is relinked
against these final libraries.

| Suite | Passed | Failed | Disabled |
| --- | ---: | ---: | ---: |
| BaseTests | 41 | 0 | 0 |
| DataTests | 247 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 |
| PersisterTests | 142 | 0 | 1 |
| GuiLocalizationTests | 69 | 0 | 0 |
| Total C++ | 579 | 0 | 1 |
| Python source/catalog checks | 37 | 0 | 0 |

All new GUI tests pass in the full suite and in the isolated-profile verification.
Synthetic serialization covers all nine nonzero sigma values and an asymmetric
100-element matrix. Saving/reading preserves every sigma and the full matrix.
The original Data/Engine/CUDA/mutation, validation and serializer sources are
byte-compared with the base. No CLI, MCP, physics, dependencies, upstream, README
or submodule changes are present. Heavy GPU/stress tests have no new technical
reason and are not run. Logs, XML, graph output and the review package live under
ignored `temp/l2b4b`.

## Data isolation

Tests use synthetic parameters and SettingsTestScope. No real worlds, genomes or
autosaves are opened. The review executable redirects its own HKCU access into a
throwaway registry profile, selects Russian, disables autosave/save-on-exit, and
runs from its own directory. `--review-check-profile` exits 0 without simulation.
No real registry restoration or overwrite is performed.

The long-run registry snapshots differ only in geometry of Simulation parameters
and Evolution dashboard. A separate user-installed process
`E:\Program files\Alien\Release\alien.exe` was running independently during the
work; it was neither controlled nor stopped. The geometry changes are consistent
with that independent GUI, and the long-run snapshots cannot establish byte identity.
A separate immediate before/after export around the four new tests plus the final
review profile self-check is byte-identical, SHA-256
`C80F439260634F4A0DA32CA3EE0E7C6D27590F55DF1CF8D1B5061B2FC5B1A448`.
AGENTS.md retains SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`
and its original `S` skip-worktree flag.

## Isolated user review

The prepared runner is separate from the user's installed application and defaults
to scale 1.5. Each launch starts with a disposable profile. Runtime assets and DLLs
are packaged locally; only synthetic samples may be used.

```powershell
& 'E:\Projects\ALIEN\temp\l2b4b\review\Launch-L2B4b.ps1'
& 'E:\Projects\ALIEN\temp\l2b4b\review\Launch-L2B4b.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l2b4b\review\Launch-L2B4b.ps1' -Scale 2.0
```

1. Create a disposable empty world; do not load a user world/genome/autosave.
2. Find both meta-mutation groups by their English and Russian captions. Review
   all ten names and full tooltips, especially sigma and donor/Void semantics.
3. Enter 0, 0.00001 and 0.99999; switch EN → RU → EN during numeric entry;
   verify focus, value, formatting and reset.
4. Expand the matrix. Verify ten source/target colors in their original order.
   Open the editor, toggle source color 2 → target color 7, and confirm 7 → 2 is
   independent. The diagonal must remain unavailable.
5. Switch languages while the editor is open; apply, reopen/edit/cancel, then
   reset. Confirm all other cells, preview expansion and index order remain intact.
6. At scales 1.0, 1.5 and 2.0 resize both host and dialog, review long captions,
   source/target axes, button wrapping, title and close button. Review clipped
   captions through their full-text hover where applicable.
7. Save/read only the disposable synthetic settings. Close the review instance.

The headless tests above do not establish user acceptance. The subsequent explicit
user acceptance is recorded separately below; the normative glossary is unchanged.

## Graph review

Before edits, upstream impact was run for the adapter key/group label, bool dispatch,
shared matrix input, bool wrapper, dialog class and affected dialog methods.
The concrete BasicInputColorMatrix implementation has three direct callers:
CheckboxColorMatrix, InputIntColorMatrix and InputFloatColorMatrix (LOW).
The concrete bool dispatcher has one direct group-dispatch caller and two indirect
GUI consumers (LOW). The concrete dialog header has three direct importer files
and eight total listed dependents (LOW). Ambiguous class-name candidates were
narrowed to the actual header; broad Definitions.h matches were not used as the
implementation's verdict. UNKNOWN results for template methods, wrapper and group
labels were supplemented by source-call-site inspection. Empty caller/process
sets are not treated as evidence that the GUI path is unused.

The index was refreshed with `node .gitnexus/run.cjs analyze --index-only`.
A fresh local backend then ran `detect_changes({scope: "all"})` on the staged
19-file change: 54 changed symbols, one affected flow, MEDIUM risk, no partial or
truncated flags, and all 54 symbols listed. The affected flow is
`ProcessToolButtons → Get`, entering the dialog tool-button captions/translation
lookup. Source review and passing native headless interactions cover that path.

The indexer still reports the known full-text/BM25 index-build failure. Its graph
and embeddings build completes; bounded entry-point/process sampling and some
receiver/callable-resolution warnings remain. These are graph coverage limitations,
not an assertion that unlisted GUI consumers are unaffected. Direct model-byte
comparisons and native GUI tests supplement the graph. No CodeQL is run.


## User acceptance and final integration review

On 2026-10-22 the user explicitly reported completion of all six L2-B4b manual
acceptance scenarios with no remarks, and authorized integration into main.
The two localized meta-mutation groups and the customization transition matrix
are visually accepted at reviewed SHA
`d917886dc059bfedd3400d74e45723460f920518`.
The original model investigation confirms Void → Донор: the Void mutation
switches eligible nodes between CellType_Void and a non-Void type. The accepted
caption remains Донор: сигма. The normative terminology document was not changed.
This acceptance covers the requested interface scenarios; it is not a claim that
the user tested evolution algorithms under load. No GPU/stress run is attributed
to the user or performed during this integration review.

An origin fetch confirms feature HEAD and origin/feat/ru-localization-l2b4b at the
reviewed SHA, with main and origin/main at base
`a3d2ae563c4be86b2c307a3ec1d69ec07e1ed78d`: one commit ahead, zero behind.
The working tree and index were clean before this documentation follow-up.
The entire 19-file feature diff was reviewed, including all six shared GUI files,
the catalogs and test changes. No blocking issue was found.

The review confirms all ten typed member bindings, nine unchanged zero-default
FloatSpecs with range 0…1, logarithmic sliders and %.5f, full descriptions and
English fallback. Matrix layout/rendering still uses the same row/column indices,
ten-color order, defaults, diagonal guard and native checkbox IDs. The native
popup title/ID, button IDs, edit callback, draft, cancel and reset paths remain
canonical. Language refresh copies presentation fields only.

All three BasicInputColorMatrix callers were examined: CheckboxColorMatrix,
InputIntColorMatrix and InputFloatColorMatrix. Numeric wrappers and numeric spec
dispatch remain byte-identical to the base and use empty localizationKey/nullopt
label defaults. Their axes, numeric range/format, default/disabled matrices,
callback and footer branch retain legacy behavior. The display-button helper
preserves each original GetID result and native button sizing when labels are
unchanged. Existing regression checks compare all code outside the explicitly
authorized presentation extension regions; the full 69-test headless suite also
passes, including directed bool-cell editing, diagonal exclusion, apply/cancel,
reset, expansion, EN/RU/EN IDs/focus, layouts and synthetic persistence. This is
source review plus the actual available test coverage, not an assertion that the
suite exhaustively automates every legacy numeric-matrix interaction.

The integration Release build succeeds again with build-windows-ninja.bat agent.
The already-correct embedded English catalog passes its missing-catalog tests;
no further forced recompilation is needed. All seven lightweight test executables
pass again: 579 successful C++ tests, zero failures and one pre-existing disabled
Persister test. All 37 Python checks pass. EN and RU each contain 356 keys, with
identical key sets and unchanged earlier entries. Production code is unchanged
from the user-reviewed SHA; this follow-up changes this report only.
Logs/XML and the graph review are retained under temp/l2b4b/integration.

Tests use only synthetic parameters and process-local SettingsTestScope profiles.
The real HKCU/Software/alien exports immediately before and after this integration
check are byte-identical, SHA-256
`AADA608579A48DC36E1751395D18395A7CFE37C044743A46E563FE64981893CA`.
No user world, genome or autosave is opened, copied or modified; no registry
restoration is performed. The earlier independent application's geometry changes
remain documented above without attribution to tests. AGENTS.md retains its
original bytes, SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`,
and S skip-worktree flag. README, dependencies, upstream, submodule, source
specifications, mutations, physics, CUDA, serialization, MCP and CLI are unchanged.

The refreshed integration graph comparison against main reports MEDIUM risk:
19 changed files, 63 listed symbols and the same ProcessToolButtons → Get flow,
with neither partial nor truncated results. The documentation-only pre-commit
all-scope check reports one file, three listed Markdown symbols and LOW risk,
also without partial/truncated flags. The three matrix-wrapper callers were
resolved again; the adapter key's receiver-resolution gap and bounded process
coverage remain explicitly documented limitations. The known full-text/BM25
index-build warning persists; graph construction completes successfully. The
review therefore uses call-site/source comparisons and native regression results
alongside the graph rather than treating absent processes as a safety guarantee.
