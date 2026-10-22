# L4-A1 validation: genome shell and gene editor

Date: 2026-10-22 (Europe/Moscow).

Branch: `feat/ru-localization-l4a1`.
Baseline: `729ea59746f35c53ca9f48e7181d56e4a447933d` (`main`).

## Delivered scope

- `GenomeEditorWindow`: title, toolbar actions and overflow hint, unsaved chip,
  new-tab hint, injection counts and seed notification. Chip measurement uses
  the same translated text as rendering.
- `GenomeEditorWidget`: genome properties and name-generation hint, structure
  headings, gene/node identities, shape and cell-type display, commands, errors
  and root-gene/correction confirmations. Generated and user-authored names
  remain unchanged.
- `GeneEditorWidget`: selection and empty-state captions, base properties,
  shape choices and all five directly consumed attribute descriptions.
- `GenomeTabWidget`: Phenotype/Gene network captions, status counts and warning
  navigation hint. Numeric tab identities, view order and selections stay stable.
- New `GenomeEditorPresentation.h/.cpp`: ordered shape/cell-type labels, nine
  attribute descriptions and English/Russian count forms, looked up at render
  time. Unmapped attributes retain `EntityAttributeHelp::get`; its complete
  enum-ordered canonical table and static assertion are unchanged.
- `FileTransferController`: genome open/save captions, genome filter description
  and localized failure messages; extension syntax and serializer calls unchanged.
- `ChangeColorDialog`: title, filter/rule labels and buttons.
- `UploadSimulationDialog`: genome upload form captions, help and validation
  message. Simulation upload keeps its existing English presentation. Resource
  names, description values, folder paths and network requests are unchanged.
- `LocalizationGuiText.h` and the additive argument overload of
  `GenericMessageDialog::localizedInformation`: whole-message argument lookup
  on every frame, including an already open error modal. Existing three-argument
  callers remain supported. Substituted values are not interpreted as templates.

EN/RU catalogs each contain **567 entries**, including **134 new `genome.*`
keys**. All 433 previous entries are unchanged. Russian terminology follows
`localization-terminology.md`, including Донор, Конвертер, Большой леденец and
Малый леденец. No new glossary decisions are recorded here.

Canonical property `name` values and parent scopes remain intact; translated
captions use the existing `label` API. Tree rows retain `##gene` / `##node` and
their numeric scopes. View tabs explicitly retain their original canonical IDs.
Color/upload buttons use `DisplayButton` with canonical button names. Upload
name input retains its canonical hint ID. Language changes do not recreate tabs,
edit data, clipboard contents, tree selection or active text input.

Property labels have wider reserved space. Narrow property children expose
horizontal scrolling with minimum content width rather than negative input
widths. Wider children use their available width normally. Shared font and
tooltip infrastructure is unchanged.

There are no production changes to Data, Engine, persistence, network protocols,
serialization or genome file formats. Neural-network editing, node properties,
mutation editing, detailed issue descriptions and preview controls remain the
later L4 stages defined in the roadmap.

## Automated validation

Windows Release built successfully with:

```powershell
.\build-windows-ninja.bat agent Release 16
```

Only `build-agent` was configured/built. Neither IDE build tree was used.
No kernel structure/header changed, so a clean CUDA rebuild was not required.

| Executable | Passed | Other results |
| --- | ---: | --- |
| BaseTests | 41 | 0 failures |
| DataTests | 247 | 0 failures |
| EngineInterfaceTests | 2 | 0 failures |
| McpToolsTests | 40 | 0 failures |
| NetworkTests | 38 | 0 failures |
| PersisterTests | 142 | 1 previously disabled test |
| GuiLocalizationTests | 99 | Includes 7 new L4-A1 tests; 0 failures |
| EngineTests | 2196 | 2 runtime skips, 2 disabled; 0 failures; approximately 87 seconds |

Total: **2805 passing C++ tests**, 2 runtime skips and 3 disabled tests. The two
engine skips are the existing `provideEnergy_insufficientEnergy/2` and `/5`
cases. No build or test was cancelled. Engine tests completed before the final
GUI-only content-width adjustment; the complete GUI suite was rerun afterward.

New GUI coverage uses production editor widgets and handlers, synthetic genomes
and the existing process-local `SettingsTestScope`. It exercises multiple real
tabs, tab selection, unsaved data, clone/copy/paste, distinct tab IDs, lineage
markers, save/load through the production serializer, reset/revert, mouse gene
selection, stored node indices, Cyrillic input with the same active ImGui ID,
open confirmation identity, dynamic error arguments, EN/RU/EN switching,
1.0/1.5/2.0 scaling, narrow/wide property panels, metadata ordering and embedded
English fallback. The preview is paused and replaced by a test facade in shell
tests; this is not a preview simulation test. Real engine functionality is
covered separately by EngineTests.

```powershell
python -m unittest discover -s source/Gui/Impl/Tests -p 'check_localization*.py'
```

Result: **55 Python checks pass**, including 6 new L4-A1 contracts. These verify
catalog parity and safe keys, placeholder signatures, preservation of all old
entries, display consumers, canonical scopes, unchanged operation bodies,
serializer calls, attribute descriptions and untouched later-stage editors/data.
Historical L2 catalog-size assertions now count their original slice separately
from `genome.*`; their original translation and binding checks remain intact.
The Base catalog completeness test expects 567 and checks the new source files.

Formatting uses clang-format **19.1.5**. Only changed files clean at HEAD and
new C++ files were formatted. `FileTransferController.cpp` was not clean at HEAD;
its unrelated formatting was retained. `git diff --check` passes.

## Graph analysis

The index was refreshed with `node .gitnexus/run.cjs analyze --index-only` at the
baseline. Indexing completed; its full-text/BM25 build failed, while the graph
remained available. The MCP connection retained stale state and later closed;
CLI/local backend were used for fresh graph checks.

Method-level upstream results for shell rendering and commands were LOW.
The concrete `GenomeEditorWindow` header has MEDIUM impact, with nine direct
importers including EditorController, InspectionController/Window,
EvolutionDashboardWindow, MainLoopController, NetworkTransferController,
ReplaceSimulationDialog and UploadSimulationDialog. Widget headers were LOW.
Ambiguous forward declarations in `Definitions.h` were disambiguated to the
concrete definitions; no forward declaration was edited.

Constructors, singleton calls, virtual dialog methods and tests can return
UNKNOWN or no indexed target. These results were not treated as unused code:
actual call sites, macro-generated tests and unittest discovery were inspected.
The common message dialog was regression-tested by the complete GUI suite.

Final staged `detect_changes(scope="all")` through a fresh local backend reports
**34 files, 68 indexed changed symbols, 2 affected flows, MEDIUM risk**,
`partial=false`, `truncated=false`, with all 68 returned symbols listed. New
presentation files are additionally covered by source contracts and runtime
tests; this index was built at the baseline. The machine-readable result and
raw CLI output are retained under `temp/l4a1/`. The affected shell flows are
toolbar delayed execution and adding a
scheduled tab; the existing genome operation bodies are also checked verbatim.

## Isolated visual acceptance package

Launch from PowerShell:

```powershell
& 'E:\Projects\ALIEN\temp\l4a1\review\Launch-L4A1.ps1'
& 'E:\Projects\ALIEN\temp\l4a1\review\Launch-L4A1.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l4a1\review\Launch-L4A1.ps1' -Scale 2.0 -English
```

Default scale is 1.5 and startup language is RU. Executable:
`E:\Projects\ALIEN\temp\l4a1\review\alien-l4a1.exe`.

The separately linked executable uses the final production libraries with an
entry-point wrapper that redirects HKCU only inside that process. It enables the
genome editor, disables autosave/save-on-exit and supplies two disposable sample
genomes under `samples/`. It never opens a real user profile. The package contains
the required DLLs, current EN/RU catalogs, licensed fonts and tracked runtime
resources. No user autosave directory was copied. The launcher parses correctly;
`--review-check-profile` returns 0. Review files, sample genomes and logs are
ignored local artifacts, not an alternate tracked application.

Manual acceptance remains for the user; automated ImGui rendering checks are
not a claim of visual acceptance or live cloud-upload verification.

1. Start at 1.5. Open `samples/synthetic-a.genome` and `synthetic-b.genome` in
   separate tabs. Rename genome and gene fields in Cyrillic; confirm the names
   stay unchanged when switching EN/RU. Switch while a name input is active.
2. Select different genes and nodes, collapse a gene and revisit tabs. Clone,
   copy/paste, create a save point, edit and revert. Confirm the correct tab,
   gene index, node index, names and unsaved marker are retained.
3. Inspect all genome/gene property captions and help. Cycle through all eight
   shapes, including both lollipops, and verify the tree captions and values.
   Test root-gene deletion/swap confirmations in both languages; switch language
   with a confirmation open and cancel before applying any undesired change.
4. Change colors in a synthetic genome. Save only to `samples/edited.genome`,
   reopen it and compare names, shape, properties and nodes. Select an invalid
   disposable file to inspect the localized open error. Inspect the genome
   upload form if already logged in; no real upload is needed for acceptance.
5. Repeat narrow/wide windows at 1.0 and 2.0. Check text, tooltip wrapping,
   horizontal property scrolling, toolbar overflow, status counts and tabs.

### Explicit remaining boundaries

The existing third-party `ImFileDialog` is preserved. Application-supplied
open/save titles and filter descriptions are localized at opening. Its own
buttons, navigation and overwrite prompts remain English, and an already open
file dialog retains its opening title/filter until reopened; it has no public
live-title localization API. Switching language does not recreate or clear that
dialog's selection. General login/network dialogs, detailed genome diagnostics,
node/neural/mutation editors and preview labels retain their planned later-stage
coverage. These exceptions must not be interpreted as full L4 completion.

## Preservation

`AGENTS.md` retains skip-worktree and SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`.
Real `HKCU\Software\alien` exports around validation/profile checks match:
SHA-256 `CD0E9506A95CFB0E4C5989BCBCAB994EEDA36E2F4DC01480E391FB56FE1720E1`.
No registry restoration was needed. User worlds/genomes/autosaves and dependency
sources were not edited; `external/vcpkg` is unchanged. No CodeQL or submodule
update ran. The feature branch is committed and pushed without merging main.

## User acceptance and integration review

Date: 2026-10-22 (Europe/Moscow).

The user confirms successful visual acceptance of implementation commit
`06fe4e95f2d69a11b99988e5663da99cc1a2ab14`. Genome editor behavior, gene
operations, saving/loading synthetic genomes, EN/RU switching and interface
scaling were checked successfully. This records the user's visual verification;
the automated validation results above remain unchanged.

The English elements of the third-party file dialog remain the documented known
limitation and do not block integration. The user authorizes closing L4-A1,
fast-forward integration into main, pushing main to origin and deleting the
integrated feature branch locally and remotely. L4-A2 is outside this task.

After fetching origin, local main and origin/main both match
`729ea59746f35c53ca9f48e7181d56e4a447933d`; the feature branch and its origin
tracking ref both match the accepted implementation SHA. The working tree and
index are clean, and main is an ancestor of the feature, allowing fast-forward.

This follow-up changes only this report. No build or test is rerun, as explicitly
requested by the user. No application is launched and no user data, settings,
dependencies or submodules are modified. AGENTS.md retains its recorded bytes
and skip-worktree flag. This documentation commit is published on the feature
branch before the authorized fast-forward integration.
