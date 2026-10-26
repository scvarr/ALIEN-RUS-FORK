# Roadmap: completing Russian localization after L2

Date: 2026-10-22 (Europe/Moscow).
Base: `7ccafa229d13cace74f7c9af49cd6ff609896fd8`.
Original L3 planning baseline; completion status updated on 2026-10-26.
L4-A1–A5, L5-A–C, L6-A–D and L7-A–D are complete. L4, L5, L6 and L7 are closed. L7 closure covers completed F1 text localization with an explicit graphics exception; it does not mean full visual localization. L8 is not started; L9 release acceptance remains pending.

Evidence and per-file inventory: [localization-l3-audit.md](localization-l3-audit.md).
Translation authority: [localization-terminology.md](localization-terminology.md).
New terms require separate agreement; this document approves no new translations.

## L4 completion record

L4-A1–A5 are complete. Stage evidence is retained in
[L4-A1](localization-l4a1-validation.md),
[L4-A2](localization-l4a2-validation.md),
[L4-A3](localization-l4a3-validation.md),
[L4-A4](localization-l4a4-validation.md) and
[L4-A5](localization-l4a5-validation.md).

The user accepted L4-A5 implementation
`231a9195b9819a10aa939ec7ff7ab270e89bb446` based on automated checks and
localization review, without an additional manual launch. No completed visual
acceptance is claimed for A5. Its validation report records the acceptance
basis and deferred editorial wording candidates; catalogs are unchanged by
closure. This L4 closure did not authorize L5; the separately accepted L5-A
implementation is recorded below.

## L5-A completion record

Date: 2026-10-23 (Europe/Moscow).

The user accepted L5-A implementation
`c766b7fc7525f8e4fb59d7ca50fb4df8526abd4d` based on automated tests and review
of the [L5-A validation report](localization-l5a-validation.md). L5-A is complete.
Manual visual acceptance was not performed. The inspector's separately drawn
title requires visual verification during subsequent use; this known limitation
remains recorded in the validation report.

Closure updates documentation only. Builds and tests are not rerun; the existing
validation evidence remains applicable. At L5-A closure, L5-B was not yet started
or authorized; its subsequent acceptance is recorded below.

## L5-B completion record

Date: 2026-10-23 (Europe/Moscow).

The user accepted L5-B implementation
`ddae7b6cbf869c3fe634708f74ccc245885727cc` based on automated checks and review
of the [L5-B validation report](localization-l5b-validation.md). L5-B is complete.
Manual visual acceptance was not performed. Automated ImGui/font/layout checks
cover 100/150/200%; narrow windows, hovered tooltips, vendor file-dialog controls
and final desktop composition retain the visual-verification limitations recorded
in the report. Acceptance does not remove those limitations or the existing L5-A
inspector-title limitation.

Closure changes documentation only. Builds and tests are not rerun; the recorded
implementation evidence remains applicable. The user authorizes a documentation
commit and push, fast-forward-only integration into `main`, pushing `main`,
synchronization verification and deletion of the integrated feature branch.
At L5-B closure, L5-C was not yet started or authorized; its subsequent acceptance
and the closure of L5 are recorded below.

## L5-C completion record and L5 closure

Date: 2026-10-23 (Europe/Moscow).

The user accepted L5-C implementation
`4d85183ce261a8648baae90ac2aa6cd6f75843b3` based on automated checks and review
of the [L5-C validation report](localization-l5c-validation.md). L5-C is complete.
Manual visual acceptance was not performed and is not claimed.

**L5-A, L5-B and L5-C are complete; the entire L5 stage is closed.** Evidence is
retained in the [L5-A](localization-l5a-validation.md),
[L5-B](localization-l5b-validation.md) and [L5-C](localization-l5c-validation.md)
validation reports.

Acceptance retains the recorded visual limitations: final desktop composition,
hovered tooltips and unusually narrow manually resized windows still need visual
verification during ordinary use. The L5-A inspector-title limitation and L5-B
vendor file-dialog limitations remain. Automated 100/150/200% layout and actual
ImPlot state checks are accepted evidence; they do not constitute manual visual
acceptance. The existing automatic axis-limit policy, emit-time notification
policy and GitNexus full-text index limitation remain documented in the reports.

Closure changes documentation only. Builds and tests are not rerun; the accepted
implementation evidence remains applicable. The user authorizes a documentation
commit and push, fast-forward-only integration into `main`, pushing `main`,
synchronization verification and deletion of the integrated feature branch.
At L5 closure, L6 had not been started or authorized; subsequent L6-A acceptance
is recorded below.

## L6-A completion record

Date: 2026-10-23 (Europe/Moscow).

The user accepted L6-A implementation
`a5748740d395c90d0c6198c0a0f4993ed39b1bcb` based on automated checks and review
of the [L6-A validation report](localization-l6a-validation.md). **L6-A is complete.**
Manual visual acceptance was not performed and is not claimed.

Acceptance retains the report's limitations: final desktop composition, hovered
tooltips, vendor controls and unusually narrow saved windows need ordinary-use
visual verification. Real asynchronous disk/network failures were not injected
through a running GUI. Shared picture/preview/MCP diagnostic text remains
canonical English; file chooser open-time and overlay emit-time language policies
and the GitNexus full-text/BM25 index limitation remain documented.

Closure changes documentation only. Builds and tests are not rerun; the recorded
implementation evidence remains applicable. The user authorizes a documentation
commit and push, fast-forward-only integration into `main`, pushing `main`,
synchronization/cleanliness verification and deletion of the integrated feature
branch locally and remotely. At L6-A closure, L6-B was not yet started or
authorized; subsequent acceptances and final L6 closure are recorded below.
At L6-A closure, L6-C and L6-D were pending and the entire L6 stage was still open.

## L6-B completion record

Date: 2026-10-23 (Europe/Moscow).

The user accepted L6-B implementation
`c1e86f2d9100ef8de0208a708c922d140848df72` based on automated checks and review
of the [L6-B validation report](localization-l6b-validation.md). **L6-B is complete.**
Manual visual acceptance and testing of an actual authenticated session were
not performed and are not claimed.

Acceptance retains the report's aggregate **HIGH** graph risk, isolated network
stub coverage and lack of real authenticated/server-side ownership validation.
Final desktop composition, hovered tooltips, narrow saved windows, vendor/native
controls and raw English diagnostics retain the recorded limitations. The known
GitNexus full-text/BM25 index failure was unresolved at L6-B closure; successful
graph analysis was separate evidence. The later successful index refresh and
remaining completeness limits are recorded in the L6-D closure below. At L6-B
closure, Unicode metadata search, including Cyrillic case folding and Yo handling,
was still deferred to L6-C.

Closure changes documentation only. Builds and tests are not rerun; the recorded
implementation evidence remains applicable. The user authorizes a documentation
commit and push, fast-forward-only integration into `main`, pushing `main`,
synchronization/cleanliness verification and deletion of the integrated feature
branch locally and remotely. At L6-B closure, the entire L6 stage was still open and
L6-C and L6-D remained pending; subsequent acceptance and final closure are
recorded below.

## L6-C completion record

Date: 2026-10-23 (Europe/Moscow).

The user accepted L6-C implementation
`122b1d22b4aad2414a117a4f4e65a27d48370c7f` based on automated checks and review
of the [L6-C validation report](localization-l6c-validation.md). **L6-C is complete.**
Manual visual acceptance and testing against a real server were not performed
and are not claimed.

Acceptance retains isolated authorization-response simulations, the absence of
real authenticated/server account checks, the recorded visual-verification
limitations and the GitNexus FTS/BM25 failure known at L6-C closure. The later
successful refresh and remaining completeness limits are recorded below. Browser
metadata search uses the existing UTF-8 adapter for case-insensitive Russian letters, including Ё/ё;
Е and Ё remain distinct. Full Unicode normalization is not provided, and MCP
resource search remains unchanged.

Closure changes documentation only. Production code, tests and catalogs are
unchanged; builds and tests are not rerun, and the recorded implementation
evidence remains applicable. The user authorizes a documentation commit and
push, fast-forward-only integration into `main`, pushing `main`, synchronization
and cleanliness verification, and deletion of the integrated feature branch
locally and remotely. At L6-C closure, the entire L6 stage was still open and
L6-D had not been started or authorized; its subsequent acceptance and final
L6 closure are recorded below.

## L6-D completion record and L6 closure

Date: 2026-10-26 (Europe/Moscow).

The user accepted L6-D implementation
`edb337e07cac6a917d553a1e6b3aeefa0621dddd` based on automated checks and review
of the [L6-D validation report](localization-l6d-validation.md). **L6-D is complete.**
Manual visual acceptance and connection of real MCP agents were not performed
and are not claimed.

**L6-A, L6-B, L6-C and L6-D are complete; the entire L6 stage is closed.**
Evidence and stage-specific limitations remain in the
[L6-A](localization-l6a-validation.md), [L6-B](localization-l6b-validation.md),
[L6-C](localization-l6c-validation.md) and [L6-D](localization-l6d-validation.md)
validation reports.

Closure retains the deferred visual acceptance: final desktop composition,
hovered tooltips, vendor/native controls and unusually narrow saved windows need
ordinary-use visual verification. Automated ImGui/font/state/layout checks at
100/150/200% do not constitute manual visual acceptance. L6-A did not inject real
asynchronous disk/network failures through a running GUI. L6-B did not exercise
an actual authenticated owner/non-owner session, server-side ownership rules or
successful remote mutations. L6-C used isolated authorization/persister
simulations; real login, registration, email delivery, password changes, account
deletion and server-side account semantics remain unverified. L6-D simulated
port availability and server startup/restart: actual socket conflicts, real MCP
agents and concurrent live transport interoperability were not exercised by its
GUI tests. These limits are retained despite the passing existing network/MCP
suites. Raw technical diagnostics and command/protocol payloads remain literal.
Browser search still distinguishes Е and Ё and does not provide full Unicode
normalization; MCP resource search semantics are unchanged.

The latest recorded GitNexus index refresh completed successfully during L6-D;
keyword queries returned definitions without the earlier FTS/BM25 error. Earlier
stage reports retain that historical failure, rather than asserting it is the
current refresh result. Successful indexing does not prove complete analysis:
dynamic dispatch, UNKNOWN targets, missing candidate flows, traversal/candidate
budgets, cross-language field gaps, discarded cached facts and skipped large
files remain limitations. L6-B's aggregate HIGH risk and L6-D's shared-header
HIGH/CRITICAL warnings are retained; L6-D's complete change listing reported
aggregate LOW without superseding those warnings.

Closure updates documentation only. Production code, tests and catalogs remain
unchanged; builds and tests are not rerun, and the recorded implementation
validation remains applicable. The user authorizes a documentation commit and
push, fast-forward-only integration into `main`, pushing `main`, synchronization
and cleanliness verification, and deletion of the integrated feature branch
locally and remotely. `AGENTS.md`, its skip-worktree flag, user data/settings and
dependencies are preserved. At L6-D closure, L7 had not been started or authorized;
subsequent L7-A acceptance is recorded below.

## L7-A completion record

Date: 2026-10-26 (Europe/Moscow).

The user accepted L7-A implementation
`190592478405d1388852a82146542b0b0136cf98` based on automated checks, review of the
[L7-A validation report](localization-l7a-validation.md), and user verification
that F1 opens and works in the running application. **L7-A is complete.**
The user's launch confirmation does not constitute full manual visual acceptance
of every language, scale, navigation or image scenario.

**The entire L7 stage remains open.** No mass translation of real chapters was
performed in L7-A. Production chapter translation begins with L7-B and continues
in L7-C/D. At L7-A closure, L7-B had not been started or authorized; its subsequent
acceptance is recorded below.

Acceptance retains the report's limits: texture lifecycle tests use a rendering-
facade double, with no new end-to-end Vulkan allocation/presentation acceptance;
installer-specific packaging of both language trees remains unverified. The
unchanged English image assets still require translation/screenshot review in
L7-B–D. Automated ImGui checks at 100/150/200% and the confirmed F1 launch do not
replace complete manual visual review. External-browser dispatch and static
GitNexus analysis limits remain documented in the report.

Closure changes only documentation. Code, tests, catalogs and English documentation
are unchanged; builds and tests are not rerun, and the accepted implementation
evidence remains applicable. The user authorizes a documentation commit and push,
fast-forward-only integration into `main`, pushing `main`, synchronization and
cleanliness verification, and deletion of the integrated feature branch locally
and remotely. `AGENTS.md`, its skip-worktree flag, user data/settings and
dependencies are preserved.

## L7-B completion record

Date: 2026-10-26 (Europe/Moscow).

The user accepted L7-B implementation
`34f261efdee6d34e32da7479142453fe6163e0ee` based on automated checks and review
of the [L7-B validation report](localization-l7b-validation.md), the seven Russian
documents and the `DocumentationReader` correction for translated section names
in mixed English/Russian contents. **L7-B is complete. The entire L7 stage remains
open.** L7-C is not started or authorized by this closure.

Acceptance retains all known limitations: eight reused images have English
embedded text; the shapes image and text disagree on node counts; the simplified
velocity-telemetry description omits the `* 20` scaling; constructor auto-trigger
timing depends on the minimum cell-function cycle interval and creature phase.
Manual visual acceptance and a repeat full GPU run were not performed and are
not claimed. The report retains the exact discrepancies, individual image
decisions, automated evidence and other validation boundaries.

**Image localization and final verification of image contents remain L7-D work**,
including reconciliation of the shapes image/text mismatch and review of all
28 images. Completion of L7-B does not constitute image-localization acceptance.

Closure changes only documentation. Production code, tests, catalogs, English
documentation, MCP and the accepted Russian help package remain unchanged;
builds and tests are not rerun. The user authorizes a documentation commit and
push, fast-forward-only integration into `main`, pushing `main`, synchronization
and cleanliness verification, and deletion of the integrated feature branch
locally and remotely. `AGENTS.md`, its skip-worktree flag, user data/settings and
dependencies are preserved.

## Scope and sequencing

**The first implementation stages are the genome editors and neural network
editor.** Shared label/identity changes needed for those screens belong inside
those small stages; they should not turn into a prerequisite rewrite of all UI.
Complete and review each stage in EN and RU before starting its dependents.

The existing 433-key catalogs, font roles, deferred language switch, stable-title
APIs, optional widget labels and L2 parameter presentation are the foundation.
Do not repeat L1/L2 or translate canonical Data definitions to bypass adapters.

Planning scale: approximately 800–1,200 additional desktop message keys,
19 English Markdown files (~25,600 whitespace words) and 28 images to review.
This key range is an estimate pending extraction/deduplication, not a coverage
measurement. CLI, vendor UI and protocol-facing MCP prose are separate work
packages. Complexity below describes change/validation difficulty, not elapsed
days; translation review effort can exceed code effort.

## Ordered implementation stages

| Order / proposed stage | Deliverable and bounded files | Complexity | Dependencies / exit evidence |
| --- | --- | --- | --- |
| 1 / **Completed: L4-A1** Genome shell and gene editor | `GenomeEditorWindow`, `GenomeEditorWidget`, `GeneEditorWidget`, `GenomeTabWidget`; title, toolbar, tabs, properties, shape display adapter and directly used attribute help | Medium | Use existing title/label APIs; preserve names and gene/node IDs. Both languages: multiple open/renamed tabs, tree selection, clone/copy/paste, file captions, open dialog during switch; no genome data changes |
| 2 / **Completed: L4-A2** Neural network editor | `NeuralNetEditorWidget`, `CellFunctionChannels`, relevant `CreaturePreviewWidget`/`PreviewWidget` labels | High | Follows shell; translate telemetry/rails/channel prose, modal actions and activation descriptions. Update all paired measurements. Verify graph/connection selection, reset, Clear/Identity/Randomize, copy/paste and Adopt/Cancel in embedded and modal modes with memory/telemetry visible |
| 3 / **Completed: L4-A3** Node properties and modes | `NodeEditorWidget`, shared display adapters for `CellTypeConstants`, relevant `EntityAttributeHelp`; constructor and per-cell panels | High | Reuse A1/A2 adapters; review approved modes and object names. All actual cell panels/modes and nested constructors work; enum order, channel masks, hidden controls and values unchanged. Do not add inactive injector controls; resolve `Free` separately where exposed |
| 4 / **Completed: L4-A4** Mutation editor | `MutationRatesWidget`, `MutationRatesDialog`, remaining mutation attribute descriptions | Medium–high | A3 terminology and stable repeated-field scopes. Every mutation group, active summary, sigma/probability, reset and dialog action localized; values/limits/number formats unchanged |
| 5 / **Completed: L4-A5** Genome diagnostics and preview completion | `GenomeIssueDescription`, `GeneGraphWidget`, preview/settings, `SignalsBufferDialog`, shared signal editor | High | A1–A4; full-sentence variants and agreed count/list formatting. Exercise each warning family and 0/1/2/5/21 counts, issue locations/follow-ups; ensure correction algorithm unaffected; preview switches retain state |
| 6 / **Completed: L5-A** Object inspector | `InspectionWindow`, remaining object/cell `EntityAttributeHelp` | High | Shared modes/help from L4. Particle, solid, fluid, free cell and organism-cell inspectors, lifecycle/connections and neural panels; dynamic IDs and live data retained on switch |
| 7 / **Completed: L5-B** World/selection tools | `EditorWidget`, `SelectionWidget`, `CreatorWidget`, `MultiplierWidget`, `ResizeWorldDialog`, `MassEditDialog`, `ChangeColorDialog`, image conversion and interaction hints | Medium–high | L4 display adapters and L5-A names. HUD focus and repeated controls stable; create/copy/undo/filter/world resize semantics unchanged; no translation of user names or file filters' syntax |
| 8 / **Completed: L5-C** Controls and statistics | Temporal/spatial/location windows; `EvolutionDashboardWindow` metric labels, table and plots | High | Stable ImPlot series/card IDs and L5 terminology. Preserve selected lineage, plot visibility/ranges and history mode; long captions at multiple DPI levels; computation/CSV contracts stay canonical |
| 9 / **Completed: L6-A** Remaining local dialogs and messages | About/network settings, autosave/log, file/preview errors and startup/application notices; remaining HelpStrings | Medium | L4–L5 shared controls. Trace callback producers to generic messages; open/queued message language policy documented; test autosave status without changing directory/peak identifiers |
| 10 / **Completed: L6-B** Browser presentation | BrowserWindow/gallery/table/user list/login invitation, edit/upload/replace dialogs and related hints | High | L6-A message conventions. Use full simulation/genome sentence variants; preserve workspace/resource/folder IDs, sorting/reactions and user text. Test private/public/featured views, ownership, empty states and pagination |
| 11 / **Completed: L6-C** Account flows and browser Unicode search | Login/create/activate/reset/new-password/delete dialogs, LoginController; existing NetworkTransferController presentation retained; BrowserData and gallery metadata filtering | High | Accepted automated checks and report review; isolated authorization responses, stable form state and IDs, case-insensitive Russian letters with distinct Е/Ё. No full Unicode normalization or MCP search change; manual visual and real-server acceptance not performed |
| 12 / **Completed: L6-D** MCP desktop UI | McpWindow, McpSettingsDialog, GUI connection instructions/status | Medium | Accepted automated checks and report review; EN/RU state and 100/150/200% layout, isolated server states and exact baseline contract checks. Commands/endpoint/schema keys and log payloads remain literal; manual visual acceptance and real-agent interoperability remain unverified |
| 13 / **Completed: L7-A** F1 locale resolution and Unicode navigation | Explicit reader locale/root, GUI reload/cache lifecycle, reader/renderer path pairing, search/snippet and anchor policy, format-reference reachability | High | Accepted automated checks and report review; user confirmed F1 opens and works. EN default and canonical MCP filenames retained; fallback, language switching, links/anchors/images and Unicode search covered. Full visual, end-to-end texture, translated-image and installer-packaging limitations remain in the L7-A report |
| 14 / **Completed: L7-B** First translated help batch | Seven Russian contents/entry/UI/keyboard/genomes/neural/first-creature files; mixed-language GUI section metadata correction | High editorial, medium code | Accepted automated checks and report/document/DocumentationReader review. Real-resource links, anchors, Unicode search, EN/RU/EN and 100/150/200% checks passed. Eight English-labelled images and source-content discrepancies remain recorded; the L7-D inventory retains English diagram labels under the graphics exception, with final review assigned to L9. No manual visual acceptance or repeat full GPU run |
| 15 / **Completed: L7-C** | cell-types, energy, simulation-parameters, glossary, how-life-works | High editorial | Accepted automated checks, report review and selective review across all five chapters. C01–C20, separate L7-B source issues and per-image decisions remain unresolved as recorded. L7-D image inventory is complete; Russian diagram artwork is not localized, and final diagram review belongs to L9; visual/Vulkan/installer/new-GPU-run limitations remain |
| 16 / **Completed: L7-D** Workflow and support help | Seven final translations; all 19 Markdown files including contents, 18 GUI chapters; full 29-resource image inventory | Medium–high | Accepted implementation `5045f629e67ad91c76884aca8d5f8ebf808f3c70`: automated checks, report review and selective Russian-document review. L7 text localization is closed with a graphics exception: 25 diagrams remain English-labelled and are not graphically localized; three JPEG and the logo have no English labels and are reused. English docs, MCP contracts, commands and formats preserved; open C/B/D/I register and L9 acceptance obligations retained |
| 17 / L8-A Console and CLI | ConsoleModeController, ConsoleUi, Alien-cli human help/status/error output and explicit locale selection policy | Medium–high | Reuse catalogs only where appropriate; retain option names, machine output and numeric formats. Test console entry/exit and missing-resource errors; avoid introducing GUI dependency into CLI |
| 18 / L8-B Vendor and diagnostic closure | ImFileDialog integration; exposed ImGui/ImPlot menus; user-facing Base/Network/Persister/Rendering/startup errors | High | Trace exposed consumers; choose supported adapter or narrowly maintained patch. No pinned-vcpkg modification. Catalog-owned explanations translated, raw vendor diagnostics documented; validate early startup before locale initialization |
| 19 / L9 Release qualification | Complete bilingual inventory, final review of 25 English-labelled diagrams, manual visual/Vulkan review of every F1 chapter, installer-package completeness, ID/search/DPI regression and source coverage gate | High integration | Pending: inherited L7 graphics exception and outstanding manual/Vulkan/installer acceptance. All desktop stages; explicit disposition for CLI/MCP/vendor exceptions. EN/RU catalogs/embedded fallback agree, packaged resources complete, runtime language persistence and layout verified |

L4-A1/A2 are deliberately first even though A3/A4/A5 complete the same editor
family. During A1, translate only the attribute descriptions it actually consumes;
expand the enum-to-key map incrementally without replacing the canonical enum or
weakening its compile-time completeness checks. A2's shared channel labels then
support node editing and live inspection. Each row may be split further if its
review diff becomes too large; preserve this priority order.

## L7-D acceptance and entire L7 closure

Date: 2026-10-26 (Europe/Moscow).
Accepted L7-D implementation: `5045f629e67ad91c76884aca8d5f8ebf808f3c70`.
The user accepted the automated checks, report review and selective review of
Russian documents. **L7-A, L7-B, L7-C and L7-D are complete; the entire L7 stage
is closed as completed text localization of the built-in F1 help, with an
explicit exception for graphical resources and pending L9 release acceptance.**
Earlier completion records describe their historical scope; this record supplies
the current overall L7 status. L7-C implementation
`ff501d8bf0a25909214702cda788f5ad9a667634` was accepted on 2026-10-26 through
automated checks, report review and selective review of its five chapters; its
acceptance and evidence remain in
[L7-C validation](localization-l7c-validation.md); final text/inventory/checks and
the unified open register are in [L7-D validation](localization-l7d-validation.md).

All nineteen Markdown files, including contents, have Russian translations;
eighteen content chapters are available in GUI F1. English documentation,
MCP contracts, technical commands and formats are preserved. All recorded
automated implementation checks passed within their documented scope.

**Twenty-five diagrams still contain English labels. Russian graphical
localization of those diagrams has not been performed.** The three JPEG images
and the logo contain no English labels and are justifiably reused. Original image
data and all decisions remain unchanged in
[localization-l7d-images.json](localization-l7d-images.json). Closing L7 with this
graphics exception does not constitute full visual localization of the help.

L9 explicitly inherits final review of all twenty-five English-labelled diagrams,
manual visual/Vulkan review of every chapter at 100/150/200%, and completeness
checks of the installer package and both language trees. Manual visual/Vulkan
acceptance and installer-package verification have not been performed. Existing
GitNexus limits and external-agent, CLI and network-interaction verification
limits remain applicable; automated checks do not remove them.

C01–C20, B01–B04, D01–D06 and I01–I05 remain an open register for later
consideration, retaining the original evidence and without declaring any issue
fixed. Possible functional engine defects C02, C12 and C13 require independent
investigation outside localization. No simulation algorithm or diagram is changed
by this documentation closure.

Only the roadmap and L7-D validation report change. Production code, tests, help
resources, image inventory, English documents, MCP, user worlds/genomes/autosaves/
settings and dependencies remain unchanged. AGENTS.md bytes and skip-worktree
are preserved. Builds and tests are not rerun; accepted implementation evidence
is retained. The user authorizes the documentation acceptance commit and branch
push, fast-forward-only integration into main, pushing main, synchronization and
cleanliness checks, and deletion of the integrated feature branch locally and
remotely. L8 is not started or authorized. The numbered order remains the
recommended delivery sequence, not an instruction to start later-stage work.

## Cross-stage contracts

1. Before source edits, refresh/check GitNexus and run upstream impact for the
   exact symbols. Report callers/processes/risk; warn on HIGH/CRITICAL. UNKNOWN
   requires confirming text/call-site investigation. Run complete graph change
   analysis before each implementation commit.
2. Keep ImGui identity, canonical specification names, enum values, JSON and
   serialization fields, CLI switches and MCP schemas independent of language.
   Preserve original parent scopes, not just a new child `###` suffix.
3. Translate whole messages. Define argument signatures and Russian count forms
   where needed; never concatenate an English noun with a translated fragment.
   New terminology is reviewed against the normative glossary before cataloging.
4. Use keys/static canonical metadata with lookup at display time. Invalidate
   locale-dependent document/search/layout caches at the existing frame boundary.
   A language switch must preserve unsaved edits, selection, focus and open modals.
5. Use decoded UTF-8 boundaries for matching, highlighting and truncation. Existing
   `Utf8Search` covers Russian case/Yo but does not imply normalization, stemming
   or Yo/e equivalence. Keep byte offsets tied to original rendered strings.
6. Do not translate user data, URLs, technical paths, tokens or raw foreign
   diagnostics. Generated defaults, persisted peak labels and protocol prose need
   deliberate policy rather than silent replacement.

## Validation planned for implementation

L3 itself requires only document/source checks and a documentation commit.
For later stages, use small catalog/static checks plus focused GUI interaction
coverage. Reuse and extend `source/Gui/Impl/Tests/LocalizationGuiTests.cpp` and
the existing localization checker patterns where they establish useful contracts;
do not infer runtime completeness from key parity or a lexical scan.

GUI-only changes must still build with `build-windows-ninja.bat agent`.
Choose runtime/tests appropriate to shared-code impact under repository rules;
Base/Network/McpTools/Persister changes need their relevant suites. Localization
should not alter kernel structs, CUDA algorithms or simulation semantics. Full
release qualification should run the applicable integration suites on the required
GPU and compare failures to the English baseline; never cancel long-running tests.

Each stage records exact touched files/keys, unresolved terminology, reviewed
screens and remaining exceptions. Check EN/RU startup and persistence, missing
catalog fallback, switch with a modal open, repeated/dynamic IDs, 100/150/200%
scaling, tooltip wrap, Cyrillic inputs and unchanged saved data/protocols.
Documentation stages additionally verify every chapter link/fragment/image,
search ranking/snippet safety, missing files and actual runtime resource paths.

## Completion and explicit boundaries

Desktop completion means every first-party owned menu/window/control/tooltip,
editor/inspector/statistics caption, notification and explanatory error has RU
coverage, with EN fully supported and stable data/identity contracts. All 19 help
files are accounted for, Cyrillic search works by the declared matching policy,
and all 28 images have a recorded disposition. Vendor strings and raw diagnostics
need an explicit tested solution or named exception rather than unexplained English.

MCP tool descriptions/results can remain a stable English machine-facing API
while the desktop MCP panel is Russian. An explicit MCP locale extension, server
emails/admin localization, translated bundled/cloud worlds and user-authored
content are separate product/API/content decisions. They are not prerequisites
for documenting desktop completion, and desktop completion must not be described
as localization of the entire ALIEN ecosystem.
