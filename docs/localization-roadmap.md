# Roadmap: completing Russian localization after L2

Date: 2026-10-22 (Europe/Moscow).
Base: `7ccafa229d13cace74f7c9af49cd6ff609896fd8`.
Original L3 planning baseline; completion status updated on 2026-10-23.
L4-A1–A5, L5-A–C and L6-A–C are complete. L4 and L5 are closed; L6 remains open. L6-D and later stages remain pending.

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
authorized; its subsequent acceptance is recorded below. L6-C and L6-D remain
pending. The entire L6 stage is not yet closed.

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
GitNexus full-text/BM25 index failure remains unresolved; successful graph
analysis is separate evidence. Unicode metadata search, including Cyrillic case
folding and Yo handling, remains deferred to L6-C.

Closure changes documentation only. Builds and tests are not rerun; the recorded
implementation evidence remains applicable. The user authorizes a documentation
commit and push, fast-forward-only integration into `main`, pushing `main`,
synchronization/cleanliness verification and deletion of the integrated feature
branch locally and remotely. **The entire L6 stage remains open.** At L6-B
closure, L6-C and L6-D remained pending; subsequent L6-C acceptance is recorded
below.

## L6-C completion record

Date: 2026-10-23 (Europe/Moscow).

The user accepted L6-C implementation
`122b1d22b4aad2414a117a4f4e65a27d48370c7f` based on automated checks and review
of the [L6-C validation report](localization-l6c-validation.md). **L6-C is complete.**
Manual visual acceptance and testing against a real server were not performed
and are not claimed.

Acceptance retains isolated authorization-response simulations, the absence of
real authenticated/server account checks, the recorded visual-verification
limitations and the known GitNexus FTS/BM25 failure. Browser metadata search uses
the existing UTF-8 adapter for case-insensitive Russian letters, including Ё/ё;
Е and Ё remain distinct. Full Unicode normalization is not provided, and MCP
resource search remains unchanged.

Closure changes documentation only. Production code, tests and catalogs are
unchanged; builds and tests are not rerun, and the recorded implementation
evidence remains applicable. The user authorizes a documentation commit and
push, fast-forward-only integration into `main`, pushing `main`, synchronization
and cleanliness verification, and deletion of the integrated feature branch
locally and remotely. **The entire L6 stage remains open. L6-D is pending and
has not been started or authorized.**

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
| 12 / L6-D MCP desktop UI | McpWindow, McpSettingsDialog, GUI connection instructions/status | Medium | L6-A errors. Commands/endpoint/schema keys remain literal; GUI translation does not change network tool contracts or command log payloads |
| 13 / L7-A F1 locale resolution and Unicode navigation | Explicit reader locale/root, GUI reload/cache lifecycle, reader/renderer path pairing, search/snippet and anchor policy, format-reference reachability | High | L4 accepted vocabulary; inspect shared GUI/MCP callers before edits. EN default retained for MCP, canonical filenames stable. Validate missing chapter/tree fallback, active chapter across switch, links/anchors/images, mixed-case Russian, long UTF-8 tokens and duplicate headings |
| 14 / L7-B First translated help batch | contents/entry/UI/keyboard chapters; genomes/neural/first-creature chapters and associated images | High editorial, medium code | L7-A plus accepted L4 UI. Chapter/title/search coverage, correct links and screenshots in both languages; no untranslated navigation/control captions |
| 15 / L7-C Cell, energy and parameter reference | cell-types, energy, simulation-parameters, glossary, how-life-works | High editorial | L2/L4 terminology accepted. Distinguish energy forms and signal control accurately; preserve technical values/code and validate all internal links |
| 16 / L7-D Workflow and support help | sandbox/evolution/generative-art, ai-agents/files/troubleshooting/format-reference; finish 28-image review | Medium–high | Accepted tool/browser/statistics vocabulary and L7-B/C. All 19 files accounted for; English embedded image text explicitly replaced or justified; MCP/CLI examples remain runnable contracts |
| 17 / L8-A Console and CLI | ConsoleModeController, ConsoleUi, Alien-cli human help/status/error output and explicit locale selection policy | Medium–high | Reuse catalogs only where appropriate; retain option names, machine output and numeric formats. Test console entry/exit and missing-resource errors; avoid introducing GUI dependency into CLI |
| 18 / L8-B Vendor and diagnostic closure | ImFileDialog integration; exposed ImGui/ImPlot menus; user-facing Base/Network/Persister/Rendering/startup errors | High | Trace exposed consumers; choose supported adapter or narrowly maintained patch. No pinned-vcpkg modification. Catalog-owned explanations translated, raw vendor diagnostics documented; validate early startup before locale initialization |
| 19 / L9 Release qualification | Complete bilingual inventory, packaging, ID/search/DPI regression and source coverage gate | High integration | All desktop stages; explicit disposition for CLI/MCP/vendor exceptions. EN/RU catalogs/embedded fallback agree, packaged resources complete, runtime language persistence and layout verified |

L4-A1/A2 are deliberately first even though A3/A4/A5 complete the same editor
family. During A1, translate only the attribute descriptions it actually consumes;
expand the enum-to-key map incrementally without replacing the canonical enum or
weakening its compile-time completeness checks. A2's shared channel labels then
support node editing and live inspection. Each row may be split further if its
review diff becomes too large; preserve this priority order.

L7-A can be investigated after L4 and developed separately from later browser
work once explicitly authorized, but translated help must use accepted UI terms.
The numbered order is the recommended delivery sequence, not an instruction to
start parallel branches. No source edits are authorized by this audit itself.

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
