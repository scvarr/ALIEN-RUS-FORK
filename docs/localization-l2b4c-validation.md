# L2-B4c validation: gene and genome structure meta-mutations

Date: 2026-10-22 (Europe/Moscow).

## Baseline and scope

Base SHA: `634970082bd0c1e628cba91f5c40cbca2d098879`.
After fetching origin, main and origin/main both matched this SHA. The index and
working tree were clean. Work is isolated on `feat/ru-localization-l2b4c`.
The initial delivery did not merge main or begin L2-B4d. The subsequent user
acceptance and integration review are recorded below.

AGENTS.md, the normative terminology document, both B4a/B4b reports, canonical
parameter declarations, presentation and GUI dispatch, MutationRatesWidget,
MutationProcessor and the ten corresponding structural mutation test files were
consulted. Production changes are confined to the presentation adapter and EN/RU
catalogs. Tests and this report complete the tracked changes.

## Typed bindings

Every row uses `references<FloatSpec>(spec, &SimulationParameters::<field>)`.
Each prefix has separate `.name` and `.description` entries. Names or interface
positions never identify parameters. The tests independently compare all ten
member pointers and change canonical captions to prove presentation identity.

| Original | SimulationParameters field | Russian caption | Catalog prefix |
| --- | --- | --- | --- |
| Add node sigma | `addNodeMetaMutationsSigma` | Добавление узла: сигма | `parameters.metamgene.addnodemetamutationssigma` |
| Delete node sigma | `deleteNodeMetaMutationsSigma` | Удаление узла: сигма | `parameters.metamgene.deletenodemetamutationssigma` |
| Extend gene sigma | `extendGeneMetaMutationsSigma` | Расширение гена: сигма | `parameters.metamgene.extendgenemetamutationssigma` |
| Trim gene sigma | `trimGeneMetaMutationsSigma` | Укорочение гена: сигма | `parameters.metamgene.trimgenemetamutationssigma` |
| Copy node section sigma | `copyNodeSectionMetaMutationsSigma` | Копирование участка узлов: сигма | `parameters.metamgene.copynodesectionmetamutationssigma` |
| Move node section sigma | `moveNodeSectionMetaMutationsSigma` | Перемещение участка узлов: сигма | `parameters.metamgene.movenodesectionmetamutationssigma` |
| Add gene sigma | `addGeneMetaMutationsSigma` | Добавление гена: сигма | `parameters.metamgenome.addgenemetamutationssigma` |
| Duplicate gene sigma | `duplicateGeneMetaMutationsSigma` | Дублирование гена: сигма | `parameters.metamgenome.duplicategenemetamutationssigma` |
| Delete gene sigma | `deleteGeneMetaMutationsSigma` | Удаление гена: сигма | `parameters.metamgenome.deletegenemetamutationssigma` |
| Swap gene sigma | `swapGeneMetaMutationsSigma` | Перестановка генов: сигма | `parameters.metamgenome.swapgenemetamutationssigma` |

Groups:

| Canonical group | Russian caption | Key |
| --- | --- | --- |
| Meta-mutations: Gene structure | Метамутации: структура гена | `parameters.metamgene.name` |
| Meta-mutations: Genome structure | Метамутации: структура генома | `parameters.metamgenome.name` |

Both catalogs contain 378 identical keys, adding exactly 22 entries to B4b's 356.
Every earlier catalog entry remains unchanged. English descriptions exactly
reproduce the canonical `metaMutationDescription()` expansion. Missing catalogs
fall back to embedded English for every new name, description and group.

## Terminology and verified operation meanings

Node, gene, genome, mutation, meta-mutation and sigma are rendered consistently
as узел, ген, геном, мутация, метамутация and сигма. The new captions and explanatory
sentences are working terminology decisions for user acceptance; the normative
glossary is unchanged.

The following is source/test review of the structural operations, not a claim
that a sigma parameter performs the operation directly:

| Operation | Meaning confirmed in MutationProcessor and existing tests |
| --- | --- |
| Add node | Insert a new node into any gene slot, including the first and last slots. Unlike Extend gene, internal slots are eligible. |
| Delete node | Delete eligible nodes while preserving at least one node in the gene. Unlike Trim gene, removal is not restricted to boundaries. |
| Extend gene | Add a node exclusively at the beginning or end of a gene. |
| Trim gene | Remove a node exclusively from the beginning or end, keeping at least one. |
| Copy node section | Copy a contiguous section into the same or another target gene; the original section remains. |
| Move node section | Remove a contiguous section at its original location and insert it into the same or another target gene, keeping the source gene nonempty. |
| Add gene | Append a new single-node gene and establish a constructor reference to it. |
| Duplicate gene | Append an independent copy of an existing gene referenced at least twice, repointing one chosen constructor/injector reference. |
| Delete gene | Remove an existing gene, retaining at least one gene and correcting references. |
| Swap gene | Exchange the content of two genes inside one genome; indices remain valid and existing references retain their indices. No exchange between organisms is implied. |

Reviewed tests include `addNodeMutation_insertsBeforeEveryNode`,
`deleteNodeMutation_deletesEveryNodeButOne`,
`extendGeneMutation_addsExactlyOneNodePerPass`,
`trimGeneMutation_removesExactlyOneNodePerPass`,
`copyNodeSectionMutation_insertsSectionIntoSingleGene`,
`moveNodeSectionMutation_singleGeneConservesNodeCount`,
`addGeneMutation_appendsSingleNodeGeneAndPointsToIt`,
`duplicateGeneMutation_duplicatesGeneReferencedAtLeastTwice`,
`deleteGeneMutation_keepsAtLeastOneGene` and
`swapGeneMutation_referenceKeepsItsGeneIndex`, plus their adjacent cases.
They were read, not executed as GPU tests in this presentation-only slice.

## Sigma and preserved invariants

All ten sigma values are standard deviations of Gaussian changes to their
corresponding mutation probabilities stored in a genome. They act only when
Apply meta-mutations is enabled and once per offspring. They do not directly
set a mutation probability, the size of a structural edit, a guarantee at 1,
or execution on every simulation timestep. The existing clamp of resulting
probabilities to 0…1 is unchanged. Russian descriptions preserve this shared
meaning and append a short explanation of the corresponding operation.

All ten remain FloatSpec, default 0, minimum 0, maximum 1, logarithmic, `%.5f`.
`SimulationParameters::getSpec()`, original member pointers, canonical English
specifications, values, default/revert behavior and all ImGui ID seeds/stacks
remain unchanged. The adapter returns presentation text only. No shared widgets
or dispatch handlers required changes.

The Python checks byte-compare model, validation, mutation/CUDA, serializer,
glossary, production dispatch, MutationRatesWidget, shared scalar/matrix widgets
and matrix dialog with the base. The final diff contains no Data, Engine,
Persister, CLI, MCP, network, dependencies, README, upstream or submodule changes.
AGENTS.md retains SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`
and its original `S` skip-worktree flag.

## Automated validation and build

Release succeeds with `build-windows-ninja.bat agent`, using build-agent only.
The first missing-catalog Base check detected the known Ninja/MSVC stale embedded
catalog issue. Removing only
`build-agent/source/Base/CMakeFiles/Base.dir/Release/LocalizationService.cpp.obj`
and rebuilding regenerated the final executable libraries and English fallback.
All results below are from the corrected build. The review executable was linked
after this correction. clang-format 19.1.5 was applied only to modified C++ files
verified clean at HEAD.

| Suite | Passed | Failed | Disabled |
| --- | ---: | ---: | ---: |
| BaseTests | 41 | 0 | 0 |
| DataTests | 247 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 |
| PersisterTests | 142 | 0 | 1 |
| GuiLocalizationTests | 74 | 0 | 0 |
| Total C++ | 584 | 0 | 1 |
| Python source/catalog checks | 41 | 0 | 0 |

The five new headless GUI tests cover:

- All ten controls through `SpecificationGuiService` production dispatch: real
  Ctrl-click text input of 0, 0.00001 and 0.99999, EN → RU → EN with active input,
  identical canonical slider IDs, preserved editing focus/value, and actual undo
  button clicks restoring defaults.
- Ten independent typed bindings, complete descriptions, renamed-caption
  independence, English fallback, synthetic nonzero values and save/read of all
  ten via the production SerializerService. Full parameter snapshots prove
  language changes and rendering do not modify data.
- Both group captions and every parameter name in English and Russian; the
  requested Add/Extend/Delete/Trim/Copy/Move/Swap queries in mixed case; filter
  clearing and unchanged declaration order. Existing semantics remain: canonical
  English is searchable in either language, Russian display names in RU.
  UTF-8 match boundaries and native highlighted draw output are checked.
- All ten production-dispatched Russian labels at scales 1.0/1.5/2.0 in a
  650-logical-pixel host. Their label area remains separate from numeric and undo
  controls; clipped labels expose full text on real hover. This is headless
  text/geometry validation, not interactive GPU visual acceptance.
- Real group header clicks close/reopen both groups. Canonical header IDs and
  closed/open state persist through EN → RU → EN, along with parameter data.
  This uses the production tree handler and production parameter dispatcher;
  the full application window is reserved for user acceptance.

All earlier B4a/B4b GUI tests pass unchanged, including the directed 10×10 matrix,
diagonal exclusion, popup/checkbox/button IDs, draft editing, apply/cancel/reset,
layout and persistence. Earlier Python checks change only their expected catalog
count; their previous semantic and byte-comparison assertions remain intact.

Commands and logs:

```powershell
.\build-windows-ninja.bat agent
python -m unittest discover -s source/Gui/Impl/Tests -p 'check_localization_*.py'
# Run the seven lightweight executables in build-agent/Release with XML output.
```

Logs/XML, build logs and the graph output are retained under ignored `temp/l2b4c`.
No heavy GPU/stress tests were needed or run. No mutation execution, evolution
performance or user visual acceptance is claimed.

## Isolation and review package

Tests use synthetic parameters and SettingsTestScope. The separately linked
review executable redirects its own HKCU access to a disposable profile before
any application setup, starts in Russian, disables autosave/save-on-exit, and
uses its own resources and working directory. It loads no real user data.
`--review-check-profile` exits 0 without setting up a simulation. Immediate real
profile exports around this self-check and all five new GUI tests are identical:
SHA-256 `4CDB8C1D982B928920C8714F42B6CBDFF49172E381CEA0CCF8A336BD13F81A74`.
The real profile was not restored or overwritten.

The local, ignored review package contains runtime DLLs, fonts and final catalogs.
The launcher defaults to scale 1.5 and supports 1.0 and 2.0:

```powershell
& 'E:\Projects\ALIEN\temp\l2b4c\review\Launch-L2B4c.ps1'
& 'E:\Projects\ALIEN\temp\l2b4c\review\Launch-L2B4c.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l2b4c\review\Launch-L2B4c.ps1' -Scale 2.0
```

User acceptance procedure:

1. Start the isolated runner and create a disposable empty world. Do not load a
   real world, genome, autosave or profile.
2. Find both groups by English and Russian captions. Review every name and full
   description, distinguishing Add node/Extend gene and Delete node/Trim gene.
   Confirm sigma changes a genome's stored probability and Swap gene refers to
   two genes in the same genome.
3. Enter 0, 0.00001 and 0.99999, switch EN → RU → EN during entry, then reset with
   the undo buttons. Verify focus, displayed precision and unchanged values.
4. Close/reopen groups, switch languages, search using every requested term in
   mixed case, inspect Cyrillic highlighting, and clear the filter. Check group
   state and the initial parameter order remain intact.
5. Repeat at scales 1.0/1.5/2.0, resize the parameter window and hover long captions
   to see their complete text. Verify labels do not cover numeric/reset controls.
6. Save/read only disposable synthetic settings. Optionally revisit the B4b
   customization matrix, then close this isolated instance.

User acceptance was subsequently completed, as recorded below. The normative
glossary is unchanged.

## Graph review

Before production edits, upstream impact analysis resolved the adapter's `key`
function to three direct consumers: parameter label, description and alternative
labels, with LOW risk and no listed processes. Group-label impact is UNKNOWN
with no resolved callers. Text inspection confirmed its actual consumers in the
adapter filter and SpecificationGuiService. These zero/LOW results are not
treated as proof of complete graph coverage: receiver typing limitations omit
some calls, and generated process sampling is bounded.

The index was refreshed using `node .gitnexus/run.cjs analyze --index-only`.
The known full-text/BM25 index-build failure remains; graph generation completed
with receiver/callable-resolution and process-sampling coverage warnings. Native
GUI tests and unchanged-model comparisons supplement these limitations.
No CodeQL or submodule update was run.

A fresh local backend ran `detect_changes({scope: "all"})` against the staged
15-file delivery: 21 indexed changed symbols, LOW risk, zero listed affected
flows, `partial: false`, `truncated: false`, with all 21 symbols returned.
This is a complete response for the indexed graph, not complete C++/GUI call
coverage. It also marks some unchanged neighboring adapter functions as touched;
the reviewed production diff is exactly ten typed branches and two group-caption
branches. Source review and the native interaction tests cover this discrepancy.

## User acceptance and final integration review

On 2026-10-22 the user confirmed that all six manual acceptance scenarios were
completed without remarks at reviewed SHA
`d16dfa815c92be9117e8f183576afe2b66306564` and explicitly authorized integration
into main. Names, complete tooltips, search, editing, reset and language switching
are visually accepted. This acceptance did not include actual mutation execution
in a GPU simulation. The normative terminology document remains unchanged.

An origin fetch confirmed both feature refs at the reviewed SHA and both main
refs at base `634970082bd0c1e628cba91f5c40cbca2d098879`: ahead one, behind zero.
The index and working tree were clean before this documentation-only follow-up.
The complete 15-file feature diff was reviewed; no blocking issue was found.

The review confirms both group captions, all ten independent FloatSpec member
bindings and full English/Russian descriptions. Add node allows any insertion
slot, whereas Extend gene targets boundaries; Delete node differs from boundary
trimming. Copy preserves the source section and Move removes it while keeping the
source gene nonempty. Swap exchanges the contents of two genes within one genome.
Every sigma describes the standard deviation of a change to a mutation
probability stored in a genome, conditional on Apply meta-mutations and applied
once per offspring. None of these controls directly specifies a structural edit.

Canonical specifications and English descriptions, zero defaults, ranges 0…1,
logarithmic scales, `%.5f`, ID seeds and reset handlers remain unchanged.
The production adapter diff contains only the ten typed branches and two group
caption branches. Existing catalog entries and all Data, Engine, CUDA, mutation,
serialization, CLI and MCP sources are unchanged. Shared widgets and dispatch
are unchanged as well. This follow-up changes only this report, preserving all
production code from the user-reviewed SHA.

The integration Release build succeeds with `build-windows-ninja.bat agent`.
The seven lightweight suites pass again: 584 successful C++ tests, zero failures
or errors, and one pre-existing disabled Persister test. The full 74-test GUI
suite includes B4a/B4b regressions and the directed 10×10 matrix checks.
All 41 Python checks pass. EN and RU each retain 378 matching keys, and the
embedded English missing-catalog tests pass; no forced recompilation is needed
in this integration run. Logs and XML are retained in `temp/l2b4c/integration`.

Tests used their existing isolated settings scopes and synthetic parameters.
No user worlds, genomes, real profile or autosaves were opened or modified.
No heavy GPU/stress tests were run. AGENTS.md retains its original SHA-256 and
`S` skip-worktree flag. README, dependencies, upstream and submodule are unchanged.
L2-B4d is outside this integration.

The integration graph was refreshed to the user-reviewed SHA with
`analyze --index-only`. A fresh backend's comparison against main reports 47
indexed changed symbols across 15 files, LOW risk, no listed affected flows,
`partial: false` and `truncated: false`; all 47 symbols are returned.
Upstream analysis again resolves three direct `key` consumers (label, description
and alternative labels), LOW risk. Group-label analysis remains UNKNOWN; its
production filter/dispatch call sites were confirmed by text inspection and
covered by the passing GUI tests. The pre-commit `scope: all` response for this
documentation-only follow-up is also untruncated and non-partial. The known FTS
failure, bounded process sampling and imperfect symbol/hunk attribution remain
coverage limitations, not evidence that unlisted GUI paths are unaffected.
