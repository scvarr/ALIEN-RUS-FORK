# L6-B validation: world and genome browser localization

Date: 2026-10-23 (Europe/Moscow).
Branch: `feat/ru-localization-l6b`.
Baseline: `f97d6d0373fe5592740401fc1a8e724d703da7f4` (`main`).

## Delivered components

| Component | Result |
| --- | --- |
| `BrowserWindow` | Browser title, world/genome tabs, Featured/Community/Private captions and help, toolbar/overflow actions, account invitation/menu, filter hint, reactions, delete confirmation and status totals |
| `BrowserGalleryWidget` | Sorting captions and display values, measured sorting width, card-size label, pagination, new badge, visibility, metadata/units, preview/loading/empty states and reaction hints |
| `BrowserTableWidget` | Native table captions with stable suffixes and unchanged sort IDs; folder resource counts, visibility/new badges, reaction/download hints and units; explicit empty state |
| `BrowserUserListWidget` | User/GPU/time/reaction columns, active-instance badge and help, hidden/not-shared GPU states and login/share invitations |
| `BrowserHelper` | Download hint |
| `BrowserLoginBannerWidget`, `BrowserLoginHintWidget` | Display-time invitations, private-workspace explanation, login/register and dismissal hints; stable button identities |
| `UploadSimulationDialog` | World upload title, privacy/folder/share explanations and name hint; the existing L4 genome translations and shared controls are reused |
| `EditSimulationDialog`, `ReplaceSimulationDialog` | Display-time titles, field hints, actions and complete resource-specific replacement questions; input validation message |
| `BrowserController`, `BrowserData`, `NetworkTransferController` | GUI-owned operation/error explanations, pending reaction-user caption, transfer overlays and new-version warning; raw diagnostics remain literal arguments |
| `BrowserPresentation` | GUI-only canonical-caption adapter, native column suffixes and Russian one/few/many count selection |
| `GenericMessageDialog`, `AlienGui` | Opt-in display-time localized Markdown retaining clickable links; action-button measurement ignores hidden ID suffixes |

Both catalogs contain **1,425 keys**, including **150 additive `browser.*`
entries**. All 1,275 previous entries are unchanged. The canonical browser
`HelpStrings` constants remain intact and their browser consumers translate
through the explicit adapter; account/security consumers remain outside this
stage. The terminology, audit and roadmap documents are unchanged.

L6-C account localization/Unicode search and L6-D MCP localization were not
implemented. Network/Persister/Data/Engine/MCP sources and APIs are unchanged.

## Identity, state and language policy

- Canonical resource/workspace enums, settings paths (`simulations`, `genomes`,
  `public`, `private`, `alien-project`), request fields, resource/reaction IDs,
  ownership/visibility conditions, access checks and callbacks are retained.
- Native resource/user tables retain their table IDs and numeric column/sort
  IDs; translated column captions append canonical suffixes. Their existing
  resize/reorder/hide/sort behavior remains native ImGui behavior.
- Workspace buttons use stable `###workspaceN` suffixes independent of caption
  and count. Tabs retain canonical identities, sorting/card controls retain
  canonical names, and the filter/name hints retain explicit canonical IDs.
- User names, resource/folder names, descriptions, server addresses, timestamps,
  version strings, paths, URLs and external diagnostic text are never translated.
  Named arguments are substituted once, without interpreting inserted braces.
- Captions, help, open confirmations/error dialogs and the linked new-version
  warning resolve at display time. An open dialog keeps its canonical title,
  instance, edited text and existing action callback during EN/RU/EN.
- Transfer overlays retain the established **event-time** policy: their language
  is chosen when emitted. A successful resource-name notification remains user
  data. No request is rescheduled merely because the language changes.
- Complete world/genome sentence variants avoid composing Russian prose from
  untranslated type names. Folder/status counts use Russian one/few/many rules,
  including 0, 1, 2, 5, 11, 12, 21, 22, 25 and 111. Time/file-size abbreviations
  are display-only; original numeric calculations and formatting are retained.
- Raw Persister/network failures remain diagnostic arguments beneath a localized
  operation explanation. The shared simulation-load fallback uses the L6-A
  presentation boundary; its machine-facing producer remains unchanged.

## Automated validation

Windows Release: `build-windows-ninja.bat agent Release 16` — **passed**.
Only `build-agent` was used. No CUDA struct/kernel/constant-memory header changed;
a clean CUDA rebuild was not required. Logs/XML/formatting/graph records are
retained locally under `temp/l6b/` and are not committed.

| Suite | Passed | Other results |
| --- | ---: | --- |
| BaseTests | 41 | 0 failures |
| DataTests | 247 | 0 failures |
| EngineInterfaceTests | 2 | 0 failures |
| McpToolsTests | 40 | 0 failures |
| NetworkTests | 38 | 0 failures |
| PersisterTests | 142 | 1 existing disabled test; 0 failures |
| EngineTests | 2,196 | 2 skipped, 2 existing disabled tests; 0 failures |
| GuiLocalizationTests | 147 | Full existing 142-test regression plus 5 browser tests; 0 failures |
| Python localization contracts | 99 | Existing 93 contracts plus 6 browser contracts; 0 failures |

The full GPU suite completed in approximately **88 seconds**, without
cancellation. Existing skips remain
`ConstructorTests_ProvideEnergy_Separation.provideEnergy_insufficientEnergy/2`
and `/5`.

The new browser runtime tests use actual production ImGui widgets and isolated
Rendering/Persister facade stubs. No real network request, login, upload,
replacement, move, deletion, reaction or user-resource modification is performed.
They exercise:

1. **EN/RU/EN at 100/150/200%** for both resource types and all three workspace
   presentations, gallery/table views, a 105-resource page boundary, native table
   instance/width/sort identity, gallery sorting, filter text, collapsed folders,
   selected resources and Cyrillic/long user metadata.
2. Empty lists, logged-out Private presentation, pending list/picture requests,
   and a simulated refresh/access failure. Switching language does not schedule
   another request; error details containing Cyrillic, `%`, braces and `###`
   survive exactly.
3. Open upload/edit/replace dialogs for worlds and genomes across all three
   scales. Canonical modal instances, edited names/descriptions
   and share toggle survive language switching; source contracts retain folder
   fields and request arguments. These tests seed dialog state
   directly, avoiding preview generation and account/network side effects.
4. Russian count forms and an open localized Markdown warning with its original
   project URL.
5. All six resource transfer operations against a stub: pending tasks survive
   EN/RU/EN; simulated failures produce open localized dialogs retaining literal
   diagnostics. No success-side mutation is performed against an external service.

Source contracts additionally compare settings/payload/sort/child/table/popup IDs,
request producers, ownership/access/reaction/filter/tree/picture algorithms and
callback arguments with the requested baseline. They verify old catalog entries,
key/argument parity, stable presentation IDs, unchanged shared diagnostics and
stage boundaries. Historical key-count checks exclude only the additive browser
namespace. The accepted L6-A scope check is pinned to its implementation commit;
its shared-widget normalization removes only the new hidden-suffix measurement
and opt-in Markdown sites. Current L6-B contracts cover those sites and the
previously untranslated GUI transfer consumer separately.

After the final shared toolbar-overflow hint was connected to the existing L4
key, the Release build, five browser runtime tests and six browser source
contracts were repeated successfully.

The first attempt to add tests to the existing large GUI test translation unit
hit MSVC C1128 (object section limit). Browser tests now compile in their own
`BrowserLocalizationTests.cpp`; the original GUI test source is unchanged and no
compiler-limit workaround was added. Test-only friend declarations expose state
without changing production interfaces or behavior.

`git diff --check` passes. clang-format **19.1.5** was used only on modified C++
files already clean at HEAD and on new files. `AlienGui.cpp`,
`EditSimulationDialog.cpp` and `NetworkTransferController.cpp` already have
formatting differences at HEAD; whole-file formatting was not applied to them.

## Graph analysis

Upstream analysis covered the browser window/widgets/helpers/data/controller,
resource dialogs, GUI transfer controller, shared message/action-button controls
and test fixtures. BrowserWindow reported **HIGH** (15 direct consumers).
GenericMessageDialog reported **CRITICAL** (30 direct consumers); the opt-in
Markdown change was announced before implementation. AlienDialog reported
**HIGH** (23 direct consumers); its change is a test-only friend declaration.
Resolved helper/controller/dialog classes otherwise reported LOW/MEDIUM.
UNKNOWN/ambiguous constructors and virtual entries were checked against concrete
class targets, registrations, source consumers and actual test execution, rather
than interpreted as evidence of non-use. Indexed execution-flow search also
returns unrelated vendored browser symbols, so scoped graph/source review was
used to identify the actual GUI consumers.

The graph was refreshed with `analyze --index-only`. Full-text/BM25 creation
still fails (missing `File.file_fts` and `Typedef.typedef_fts`), as recorded in
preceding stages; graph indexing completed successfully. This is a search-index
limitation, not a clean keyword-search result.

The CLI LocalBackend all-scope check uses an expanded limit and its complete
structured result is retained in `temp/l6b/graph-complete.json`: **55 changed
files, 229 listed changed symbols, 10 affected processes, aggregate HIGH risk**.
Neither `partial` nor `truncated` is set. The changed-symbol list includes the new
fixture/stub methods. Affected GUI flows include workspace/footer selection,
login-required presentation, toolbar refresh/edit/replace/link actions, upload
presentation and browser-list processing. The aggregate HIGH result was reported
before commit; the source contracts and runtime/regression evidence above bound
the presentation changes without modifying the engine or network protocol.

## Limitations and acceptance evidence

This is automated ImGui/font/state execution and source/callback review,
not manual Vulkan desktop or screenshot-based visual acceptance. Final desktop
composition, hovered tooltips, unusually narrow saved windows and vendor/native
ImGui context-menu prose retain ordinary-use visual verification needs. No
separate manual-acceptance build was prepared.

The runtime suite checks logged-out/non-owner presentation and isolated transfer
failures. Authenticated owner/non-owner permissions, visibility predicates and
publication/reaction handlers are checked for source/argument preservation; an
actual logged-in session and server-side ownership rules were not exercised.
Server diagnostics, vendor controls and stored log text can remain English.
The existing byte-oriented case-insensitive metadata search is unchanged;
Cyrillic case folding/Yo handling remains L6-C work. Language switching does not
change the existing policy that an actual filter/sort/refresh/tree change may
clear selection; the tests settle those actions before testing language switches.

At delivery, only the feature branch is committed/pushed; `main` remains at the
requested baseline. `AGENTS.md` retains skip-worktree and SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`.
User data/settings/dependencies and pinned vcpkg are unchanged. No CodeQL,
submodule update or IDE-tree build was run. Manual visual acceptance and closure
of the entire L6 stage are not claimed.

## Acceptance and L6-B closure

Date: 2026-10-23 (Europe/Moscow).

The user accepted L6-B implementation
`c1e86f2d9100ef8de0208a708c922d140848df72` based on the recorded automated
checks and review of this validation report. **L6-B is complete.** Manual visual
acceptance and testing of an actual authenticated session were not performed
and are not claimed.

Acceptance retains the implementation's aggregate **HIGH** graph risk and all
recorded limitations. Runtime network coverage uses isolated facade stubs;
authenticated owner/non-owner permissions, server-side ownership rules and
successful remote mutations were not exercised against a real session. The
source-preservation checks and simulated failures do not replace that coverage.
Final desktop composition, hovered tooltips, unusually narrow saved windows,
vendor/native controls and English raw diagnostics retain their recorded
limitations. The known GitNexus full-text/BM25 index failure remains unresolved;
successful graph analysis is separate evidence. Unicode metadata search,
including Cyrillic case folding and Yo handling, remains deferred to L6-C.

Closure changes only this report and the roadmap. Production code, tests and
catalogs are unchanged; builds and tests are not rerun, and the existing
implementation validation evidence remains applicable. The user authorizes a
documentation commit and push, fast-forward-only integration into `main`,
pushing `main`, synchronization/cleanliness verification and deletion of the
integrated feature branch locally and remotely.

`AGENTS.md`, its skip-worktree flag, user worlds/genomes/autosaves/settings and
dependencies are preserved. **The entire L6 stage remains open.** L6-C and L6-D
remain pending; this closure does not start L6-C.
