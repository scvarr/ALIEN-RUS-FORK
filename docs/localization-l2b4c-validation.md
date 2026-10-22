# L2-B4c validation: gene and genome structure meta-mutations

Date: 2026-10-22 (Europe/Moscow).

## Baseline and scope

Base SHA: `634970082bd0c1e628cba91f5c40cbca2d098879`.
After fetching origin, main and origin/main both matched this SHA. The index and
working tree were clean. Work is isolated on `feat/ru-localization-l2b4c`.
This delivery does not merge main or begin L2-B4d.

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

User acceptance remains pending. The normative glossary is unchanged.

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
