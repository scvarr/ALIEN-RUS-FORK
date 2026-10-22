# L3 audit: remaining Russian localization

Date: 2026-10-22 (Europe/Moscow). Documentation only; no L4 implementation.

## Baseline, evidence and interpretation

The audit branch is `research/ru-localization-l3`, created from fetched
`origin/main` at `7ccafa229d13cace74f7c9af49cd6ff609896fd8`.
Local `main` matched that commit. The working tree started clean and
`git ls-files -v AGENTS.md` returned `S AGENTS.md` (skip-worktree).
The authoritative translation decisions remain in
[localization-terminology.md](localization-terminology.md); this audit neither
changes that glossary nor approves new terminology.

Evidence consists of current source reads, a production C++ literal survey,
catalog comparison, documentation measurements and the L1/L2 validation reports.
All 237 immediate `.cpp`/`.h` files in `source/Gui/Impl/` were included in the
literal survey, including files with no direct display literals. Supporting
Data, Base, Network, Persister, Rendering, MCP, application and console sources
were surveyed for display text and shared dependencies. Tests and TestData
were excluded from workload counts. No build, tests or runtime UI validation
were performed; statements about behavior below describe source paths.

GitNexus query for localization/documentation/editor flows found the localization
service and documentation symbols. Its response reported an index two commits
behind HEAD. Upstream impact on `DocumentationReader` reported LOW, one direct
dependent, no indexed affected processes/modules. This class-level result is
limited evidence: current source explicitly has both GUI and MCP method callers.
The index was not regenerated during this documentation-only audit. No function,
class or method is edited, and the graph result must not be reused as clearance
for a later implementation. Re-run method-level impact on a fresh index then.

## Measured scope and current coverage

`resources/locales/en.json` and `ru.json` each contain **433 keys**, with no
missing RU keys relative to EN. This measures the existing catalog, not total
application coverage. The remaining editors do not yet request catalog keys.

The direct-call lexical survey found **710 English-looking literal occurrences
in 65 GUI files**, plus 141 in `Data/SimulationParameters.cpp` and 16 in five
McpTools files. These are review candidates, not 867 missing translations:
canonical names with localized labels, semantic keys, constructors with a title
key and technical IDs can all match. Conversely, indirect pools, dynamically
built sentences, later arguments and custom draw calls escape the direct-call
pattern. The source-linked appendix records the GUI matches without claiming
that each is untranslated. Domain findings below resolve the important cases.

Two additional substantial English pools are `EntityAttributeHelp.cpp`
(144 attribute descriptions) and `HelpStrings.h` (25 named strings). There are
also mode/type arrays, mutation summaries and genome validation sentences.
For planning, allow **roughly 800–1,200 additional desktop message keys** after
deduplication and whole-message extraction; this is an estimate, not a measured
unique-string count. CLI and a decision to localize MCP prose add scope.

F1 assets contain **19 Markdown files**, **25,606 whitespace-delimited words**
and **28 images**. `contents.md` lists **17 chapter entries**; `contents.md`
itself and `format-reference.md` account for the other two files. The latter is
linked in the contents introduction, but `DocumentationReader::parseChapters`
only collects list-item links. Consequently the GUI chapter lookup cannot open
that introductory format-reference link through its chapter list.

### Already implemented: preserve and extend

| Area | Source and actual state | Remaining boundary |
| --- | --- | --- |
| Catalog loading | `source/Base/LocalizationService.cpp`: semantic keys, strict UTF-8/type checks, EN embedded fallback, RU fallback, diagnostics | No general plural or argument formatter; callers currently replace placeholders themselves |
| Language selection | `LocalizationGui.cpp`, `MainLoopController.cpp:88`, `Base/GlobalSettings.*`: request applied at a frame boundary and persisted | New cached strings must follow that boundary |
| Main menus | `MainLoopController.cpp:426–710`: translated captions with canonical `###` suffixes | Startup failure at `setupEmptySimulation` still sends English text; menu coverage does not cover destination windows |
| Basic dialogs | `DisplaySettingsDialog.cpp`, `NewSimulationDialog.cpp`, `SavePictureDialog.cpp`, `ExitDialog.cpp` | These use title keys and translated display labels; their canonical constructor/name literals are not remaining UI work |
| Generic messages | `GenericMessageDialog.cpp` has translated common buttons and key-based entry points | Legacy `information("Error", ...)` callers still supply English payloads |
| Simulation parameters | `SimulationParametersPresentation.cpp`, `SpecificationGuiService.cpp`, `SimulationParametersGuiText.h`, base/layer/source/main widgets | L2 typed adapters translate captions/descriptions/options while preserving specification names and indices. Do not count canonical Data specification strings again as missing |
| Shared controls | `AlienGui.h/.cpp`: optional display labels, localized matrix/color/reset paths; `ColorMatrixDialog.cpp` | These are opt-in paths, not automatic translation of every caller |
| Unicode text | `TextFontService.cpp`: regular/bold/monospace Noto roles and Cyrillic glyph range; `Utf8Search.h` | Fonts are existing infrastructure. Cyrillic search is not universally adopted |

Unless stated otherwise, paths in the next sections are relative to
`source/Gui/Impl/`. Appendix locations refer to the frozen baseline.

## Domain inventory: editors first

| Component / source anchors | Untranslated display text and scope | State and risk |
| --- | --- | --- |
| `GenomeEditorWindow.cpp:92,156` | Window title; open/save/share/clone/copy/paste tools and explanations; tab actions; file dialog captions, errors and confirmations | English. Keep genome names, clipboard representation, file filters and tab identity separate from captions |
| `GenomeEditorWidget.cpp:92,141,208` | Genome/name/front-angle/injection-resistance/meta-mutation fields; structure tree, gene/node operations, context actions | English. Tree captions and dynamically constructed names may also identify items; never translate stored genome/gene names |
| `GeneEditorWidget.cpp:45,77` | Nothing selected; base properties; gene name, shape generator, connection distance, stiffness, homogeneous cell type | English. Shape options come from canonical Data arrays; preserve numeric order and generation behavior |
| `NodeEditorWidget.cpp:209,227,303` | Selected node banner, base/construction properties, every cell-type-specific field, trigger/target/mode selectors, energy and construction controls | English. Uses `EntityAttributeHelp` and `Const::*Strings`; node and gene indices must remain unchanged |
| `GenomeTabWidget.cpp:244`, `GeneGraphWidget.cpp` | Phenotype/Gene network tabs; graph tooltips and issue display reached through helpers | English display sources. Numeric selection and hidden `##GenomeView` are identity; warning badges are counts |
| `GenomeIssueDescription.cpp:47,74,91,123,174,184` | Warning title/problem/explanation/consequence/location; void boundaries, cut-off nodes, cycles, unreachable genes, removed references and separation limit | English, assembled with singular/plural fragments. Translate complete message variants with typed counts/lists, not fragments. Use approved Void terminology; do not change corrections or issue priorities |
| `NeuralNetEditorWidget.cpp:93,508,658,816,967,1037,1168` | Telemetry Energy/Attacked/Age/Velocity; INCOMING/MEMORY/TELEMETRY/OUTGOING rails; yes/no; Connection weights, Weight, Bias; dialog and Adopt/Cancel/Clear/Identity/Randomize/Copy/Paste | English, including manually drawn labels. Width calculations repeat English literals. Translate display and measurement together; preserve weights, biases, activation enum and edit/adopt/cancel behavior |
| `CellFunctionChannels.cpp:35,69,129` | trigger/found/angle/mass/distance, delayed/replayed/restored/smoothed, read/write, activation/success/transfer; abbreviated module names | English visible channel annotations. Build labels for the selected language instead of caching translated static vectors; channel numbers, masks and module logic remain technical |
| `CreaturePreviewWidget.cpp:265,293`, `PreviewWidget.cpp:213`, `PreviewSettingsDialog.cpp:10` | Neural activity editor, no cell selected, detailed simulation disabled; detailed-mode and signal editor hints; display-mode options, TPS caption | English. Shared by genome preview/live inspection; validate both embedded and modal neural editors |
| `SignalsBufferDialog.cpp:19,59`, `AlienGui.cpp:3082` | Signal buffer title, number of signals, edit signal, Adopt/Cancel and shared signal editors | English. Buffer order, channel indices and stored values are not labels |
| `MutationRatesWidget.cpp:52,124`, `MutationRatesDialog.cpp:28,57` | Active mutation summaries; connection/neuron/property/geometry/mode/type/customization/void/gene/node groups; probability/sigma/weight/bias/activation-function fields; dialog actions | English despite L2 translating simulation meta-mutation parameters. These edit genome mutation data through separate widgets. Repeated labels need their existing gene/type/id scope |

`source/Data/CellTypeConstants.h` supplies English `CellTypeStrings` (line 60),
`ObjectTypeStrings` (76), `ActivationFunctionStrings` (105), shapes/energy modes
(183–185), Generator (241), Sensor (307), Muscle (341), Attacker (363), Defender
(379), Injector (404), Reconnector (443), Memory (482) and Communicator (518).
Their **display consumers** need a presentation adapter. The underlying enum,
vector ordering and canonical values must remain stable. `ProvideEnergyGenomeStrings`
has only two options, whereas `ProvideEnergyStrings` also has `Free`; the glossary
explicitly leaves the latter unresolved. Injector mode labels existing in Data
are not evidence that a corresponding control is active. Do not create controls
to make glossary entries visible. Mathematical short names such as `tanh`, `abs`,
`mod` should retain recognizable notation; descriptive activation tooltips need
separate review.

`EntityAttributeHelp::get` returns an English entry from a compile-time complete,
enum-ordered array. Its 144 descriptions cover object/cell/genome/gene/node,
construction, modes and mutations and are shared by inspectors and editors.
Translate at presentation lookup time using stable attribute mapping; preserve
the `EntityAttribute::Count` completeness contract. A static translated table
initialized once would become stale after EN/RU switching.

The neural graph renders rotated labels by drawing complete strings and rotating
their vertices (`NeuralNetEditorWidget.cpp:addRotatedNodeLabel`), rather than
iterating UTF-8 bytes as separate characters. The remaining issue on that path is
label/font measurement, clipping and rail spacing; a new character renderer is
not justified by the source. The same file's `calcNetToolButtonsWidth`, inspector
label width and connection slider width all explicitly measure English captions.

## Domain inventory: remaining application

| Component / source anchors | Untranslated display families | State / dependencies and risk |
| --- | --- | --- |
| `InspectionWindow.cpp:55,401` and `InspectionController.cpp` | Properties titles; energy particle/object/cell fields, connection tables, construction/lifecycle state, cell modes, neural signals and genome actions | English, large surface (122 simple line-scan anchors before indirect text). Reuse editor attribute/mode adapters; dynamic object ID scopes and live values must survive language change |
| `EditorWidget.cpp:84`, `SelectionWidget.cpp:216`, `SimulationInteractionController.cpp:171` | Tool modes, selection HUD, forces, paste/copy/inspect, MCP shortcut, image conversion, exit-edit-mode hints | English; some labels contain `##hud...`. A `##` suffix alone does not make changing the visible prefix ID-safe |
| `CreatorWidget.cpp:50`, `MultiplierWidget.cpp:115` | Material, color, energy, pencil, shapes and dimensions; copy counts, horizontal/vertical, Build/Undo | English, with HelpStrings tooltips. Use approved object names and retain generation parameters |
| `ResizeWorldDialog.cpp:28`, `MassEditDialog.cpp:110`, `ChangeColorDialog.cpp:20` | Dimensions/scale-content, selection filters, color/energy randomization and mutation operations, transition rules and confirmations | English. Labels do not authorize changing world data or randomization semantics |
| `ImageToPatternDialog.cpp:37` | Open image, filter description, unreadable-image error | English; extension grammar and filenames remain technical/user content |
| `TemporalControlWindow.cpp:28`, `SpatialControlWindow.cpp:42`, `LocationWindow.cpp`, `LocationController.cpp` | Time steps, real-time/run/pause/slowdown, sync, world size, zoom, center, autotracking and tooltips | English. Keep units/format placeholders and settings paths stable; Location window also consumes shared parameter presentation |
| `EvolutionDashboardWindow.cpp:316,651,742,786,972,1028` | Dashboard metrics/cards, lineage filters/table, representative genome action, extinct-creature notice, timeline modes/steps/plot height/axes | English, including static metric tables and custom drawing. Card child ID concatenates label (`##card` + label); ImPlot series labels can identify selection state |
| `LiveStatisticsService.cpp`, `LiveStatisticsHistory.cpp` | Numeric data feeding dashboard | No catalog UI found here; presentation should stay in the dashboard, not statistics computation |
| `AutosaveWindow.cpp:27,79`, `AutosaveController.cpp:143,173,219` | Save-point operations/table/status/settings/errors; Creating/Deleting/Cleaning up overlays | English. Keep sender ID, directory, filename and persisted peak type distinct from translated status. Existing peak text is also stored in entries |
| `LogWindow.cpp:27,42`, `GuiLogger.cpp` | Log title, Timestamp/Message/Verbose controls and displayed diagnostics | English UI. Raw diagnostic contents are a separate exception; translating headers does not translate producers |
| `AboutDialog.cpp:11,23`, `NetworkSettingsDialog.cpp:18` | About title/OK; server address, active-user setting, Adopt/Cancel | English. Preserve ALIEN/product/author names, URLs and actual server address |
| `FileTransferController.cpp`, `MainLoopController.cpp:setupEmptySimulation`, `PictureGuiService.cpp`, `NewSimulationService.cpp` | Operation failures, version/startup notices, loading and saving overlays; file type display names | Mixed: file open/save labels already translated; many legacy payloads remain English. Inspect callbacks, not just dialog implementation |
| `HelpStrings.h` | 25 English constants: creator hints, login/security/privacy, uploads/folders/workspaces and related prose | Untranslated shared pool. Registry paths, HTTPS, SHA-256 and URLs are literal technical values inside translatable prose; security claims must not be strengthened |
| `BrowserWindow.cpp:177,188,283,332` | Account chip, toolbar, simulation/genome tabs, Featured/Community/Private workspace labels, context actions/confirmations and folder hints | English. Resource type strings also form settings paths; translate presentation only. Folder/resource/user names are user content |
| `BrowserGalleryWidget.cpp:83`, `BrowserTableWidget.cpp:205`, `BrowserUserListWidget.cpp:73`, `BrowserHelper.cpp:29` | Sort/order/card size/pagination; badges, reaction and download actions; resource/user table headings, units and privacy status | English. Keep sort IDs, reaction codes, version strings, resource IDs and pagination behavior intact |
| `BrowserLoginBannerWidget.cpp:24`, `BrowserLoginHintWidget.cpp:29` | Invitation headline/body/button and hide hint; private-workspace empty state | English static strings; no translation call despite names ending in `substr` elsewhere in files |
| `LoginDialog.cpp:28`, `CreateUserDialog.cpp:14`, `ActivateUserDialog.cpp:29`, `ResetPasswordDialog.cpp:10`, `NewPasswordDialog.cpp:28`, `DeleteUserDialog.cpp:13` | Account forms, validation, password/code/email hints, security/privacy prose, resend/delete actions | English. Never translate credentials or confirmation codes; preserve case sensitivity and existing validation rules |
| `UploadSimulationDialog.cpp:76`, `EditSimulationDialog.cpp:58`, `ReplaceSimulationDialog.cpp:21` | Upload/edit names and descriptions, folders, visibility, replacement confirmation | English, sentence fragments depend on resource type. Use full message variants; preserve user text |
| `LoginController.cpp:66`, `NetworkTransferController.cpp:31–138`, `BrowserController.cpp` | Async errors and Downloading/Uploading/Replacing/Deleting/Applying changes/Changing visibility status | English. Errors cross Persister/Network boundaries; map application-owned status/error identity at GUI consumption, preserve raw external details |
| `McpWindow.cpp:33,74,114`, `McpSettingsDialog.cpp:15`, `McpController.cpp` | Server controls, command log, connection instructions, settings/port/restart explanation | English. Preserve HTTP endpoint, tool names, methods and argument JSON shown in instructions/logs |
| `source/McpTools/Impl/McpDocumentationTools.cpp:22–61`, other `Mcp*Tools.cpp`, `source/Network/Mcp*.cpp` | Tool descriptions/schema explanations/errors/results, documentation text | English protocol-facing prose; separate decision from desktop UI. Do not make schemas/results depend implicitly on GUI locale |
| `GenericFileDialog.cpp:16–40`, `external/ImFileDialog/` | Application-supplied title/filter descriptions and dependency-owned file picker labels | Wrapper has no locale hook; caller title translation alone will leave dependency UI in English. Plan an explicit adapter/vendor strategy without touching pinned vcpkg |
| ImGui/ImPlot dependency UI | Color-edit context menus, plot context menus, automatic tooltips | Their strings do not originate in first-party catalogs. Inspect only actually exposed controls during implementation; library-owned English cannot be found by first-party scanning alone |
| `ConsoleModeController.cpp:69`, `source/ConsoleUi/ConsoleSimulationPanel.cpp`, `ConsoleWidgets.cpp`, `source/Apps/Alien-cli/CommandLineParser.cpp`, `ConsoleOutput.cpp`, `LoginSession.cpp`, `SimulationRunner.cpp`, `PeriodicUploader.cpp` | Console rendering-off instructions, statistics/status, CLI usage/options descriptions, account/upload errors | English; include as a separate client completion stage. Keep option spellings, terminal escapes, protocol fields and numeric output formats stable; define locale selection for CLI explicitly |
| `StartupCheckService.cpp:13`, `source/Apps/Alien/Main.cpp:73–81`, `source/Rendering/Impl/SimulationRenderer.cpp:90` | Resource/startup/GPU/rendering failures and exception detail | English diagnostics with user-visible consumers. Pre-initialization failures need fallback wording; do not add locale dependency to CUDA kernels |

Indirect labels deserve explicit attention beyond the direct-call queue:
`BrowserTableWidget::getColumns` supplies Name, Description, Reactions,
Timestamp, User name, Downloads, Width, Height, Objects/Cells, File size and
Version through designated initializers. `EvolutionDashboardWindow.cpp:71`
defines ten metric caption pairs: Creatures, Avg cells, Avg nodes, Avg generation,
Avg mut. rate/Avg mutation rate, Internal energy, Created /1K, Mutations /1K,
Avg attacks /1K and Avg muscles /1K. Retain the rate-per-1K-step meaning and
creature averaging of the last two metrics. Translation must not relabel these
as raw totals. Their static metadata is outside the direct-call scan rule.

## Identity, search, persistence and switching

| Contract | Source evidence | Required implementation constraint |
| --- | --- | --- |
| Window identity | `AlienWindow.cpp:process/drawTitle` uses canonical `_title` for `PushID`/`Begin`; `_titleKey` only for drawn title | Use `setTitleKey`, retain canonical title/settings node; a translated constructor argument changes identity |
| Modal identity | `AlienDialog.cpp`, `ModalWindow.cpp` separate title key from underlying modal state | Extend existing mechanism to new dialogs; test an open modal across switching |
| Control identity | `AlienGui.h` supplies optional `.label` for many parameter types; `.name`/`.id` retain scopes | Do not replace `.name("Weight")` blindly. Repeated fields need existing ancestors and indices. Buttons/tabs/toolbars without separate labels need a stable-ID display path |
| Hidden labels | `SelectionWidget.cpp` uses visible prefixes plus `##hud...`; dashboard constructs `##card` + label | ImGui hashes the visible prefix with `##`; `###` resets identity. Preserve original IDs deliberately, including parent scopes and ImPlot series |
| Specification identity | `SimulationParametersPresentation.cpp` maps typed members/options; `SpecificationGuiService.cpp` scopes canonical group names | Reuse L2 design; never translate Data specification definitions to fix GUI captions |
| Saved formats | `source/Persister/Interface/SerializerService.cpp`, JSON parser services, `Data/CellTypeConstants.h` | Numeric enums, field names, schema tokens, extensions and parser keywords are contracts. GUI adapters must not rewrite them |
| Parameter search | `SimulationParametersPresentation::filter`, `AlienGui::TextWithHighlightedSubstring`, `Utf8Search.h` | Existing canonical/localized matching and UTF-8 byte-range highlighting are a useful pattern |
| Browser search | `BrowserData.cpp:68` calls `NetworkResourceRawTO::matchWithFilter`; that uses `StringHelper::containsCaseInsensitive` | `StringHelper.cpp:240` lowercases bytes. Same-case Cyrillic substrings may match, but case-insensitive Russian matching is not implemented correctly; separate remediation from UI captions |
| UTF-8 search limits | `Utf8Search.h` decodes strictly, folds ASCII and Russian uppercase including Yo, returns original byte ranges | No full Unicode casefold/normalization, no stemming and no automatic Yo/e equivalence. Do not claim those features |
| Cached display values | Neural telemetry arrays, HelpStrings, attribute descriptions, metric tables, loaded docs, async messages | Static storage should keep canonical keys, with lookup per frame/use. Decide whether queued/open messages refresh or retain the language at creation; test and document that policy |
| EN/RU switching | `LocalizationService::requestLanguage/applyPendingLanguage`, `MainLoopController.cpp:88` | Apply at frame boundary; translated strings must remain alive through the UI call. Preserve active edits, tab/selection/focus and dialog state |
| Formatting and grammar | `LocalizationService::getText` returns strings; `SimulationParametersGuiText::named` replaces one `{name}` | No general pluralization system is present. Define whole-message variants for counts and resource-type sentences; preserve formatting argument signatures and numeric semantics |

Do not translate semantic catalog keys; C++ identifiers; `EntityAttribute`,
activation/cell/mode enums; channel numbers and masks; `PushID` strings;
settings/registry paths; JSON fields/MCP method and tool names; CLI switches;
filenames/extensions/Markdown destinations; URLs; user-created names/descriptions;
raw third-party exception details; mathematical symbols/units where conventional.
Translate their surrounding human explanation and retain the actual value.
Application-generated default names are a separate decision: translating a
default at creation is a data change, unlike drawing a translated caption.

## Russian F1 documentation architecture and search

### Current path

`Base/Resources.cpp:13` defines `DocsPath = ResourcePath / "docs"`.
`DocumentationReader::getChapters` reads `contents.md`; `parseChapters` extracts
part headings and list-item link/title pairs; `readChapter` uses the same global
directory. No locale parameter or locale-aware fallback is present.
`DocumentationWindow::initIntern` calls `loadChapters` once, parses Markdown,
and builds section text for search. It does not observe language changes.

`DocumentationWindow.cpp:35,83,287,314,353` also has English window/title,
Show after startup, Search, No matches found and missing-document messages.
`MarkdownRenderer.cpp:renderImage` supplies an English image-not-found message.
Previous/next buttons and navigation captions derive from English chapter titles.
The reader returns empty data for unreadable files; it does not implement
language fallback or explain partial translation sets.

The renderer resolves images against `Const::DocsPath`, caches textures by path
and tracks the document address. The GUI resolves links by exact chapter filename
and splits optional `#anchor`; external HTTP(S) links go to the browser. A mere
new Russian directory will therefore neither switch reader content nor image bases.

### Proposed architecture, not implemented

Keep the current English tree and canonical chapter filenames. Add a separate
Russian tree, for example `resources/docs/ru/`, with an explicit locale/root
resolver passed to reader and renderer. This path is a proposal, not an existing
contract. Model resolved content together with its actual base path so per-chapter
fallback cannot load English text with Russian image paths or vice versa.
Resolve contents/chapter availability consistently and expose an intentional
fallback indicator. A complete translated contents file must not falsely promise
missing chapters. Preserve existing English reader behavior for MCP by default.

On language change, reload chapters, clear/rebuild search and renderer caches,
release textures, and restore the active canonical filename/section where possible.
Do not retain dangling pointers to replaced Markdown documents or old result indices.
Preserve `windows.documentation.chapter` filenames and navigation settings.

`CMakeLists.txt:387–395` recursively copies `resources` to build/runtime output,
so nested localized docs fit the existing copy mechanism in principle. A later
packaging check must verify both trees and referenced images in the delivered
layout; this audit does not establish installer behavior.

The 28 images include screenshots/diagrams (`ui-layout.png`,
`genome-editor-layout.png`, `neural-network.png`, `signal-flow.png`,
`energy-flow.png`, cell images and illustrative photographs). Their existence
does not establish how much English is embedded. Review every image visually,
record reuse/localized replacement decisions and preserve attribution. Translate
alt/caption prose separately. New editor screenshots should follow editor acceptance.

### Cyrillic search and anchors

`DocumentationWindow.cpp:toLowerCase/splitIntoTerms/updateSearchResults` lowercases
each byte, splits on ASCII space, requires every term to occur in section title
or text, then ranks all-title matches first. Plain text includes paragraphs,
lists, image captions, code, notes and table cells. Searching Russian bytes in
the same case may work, but uppercase/lowercase Russian matching is not reliable.
The existing GUI `Utf8Search` helper is not used on this path.

`createSnippet` uses byte constants 30/100. Despite its comment about cutting at
spaces, fallback branches select `matchPos` or `start + SnippetLength`; a long
word without a suitable space can be cut in the middle of a UTF-8 character.
Reuse strict decoding and original byte offsets for term matching and snippet
boundaries; retain current AND/ranking behavior unless separately agreed.

`Base/MarkdownParser.cpp:toAnchor` preserves bytes >= 0x80 and uses byte
`tolower`, so Russian heading case is not folded. Duplicate headings receive
numeric suffixes. Translating a heading changes its generated fragment. Keep
filenames canonical, audit all fragment destinations, and either add stable
anchor aliases or update translated links consistently with parser behavior.
Explicit alias support is proposed work, not an existing Markdown feature.
Fix the format-reference navigation gap explicitly rather than silently dropping
the file from translation coverage.

`McpDocumentationTools.cpp:38,61` calls the same reader as F1.
Changing the reader's implicit global root would change `read_documentation`
for agents, including titles/parts/text. Keep default EN behavior and stable
`chapter`/`chapters`/`title`/`part` fields; any explicit language option is separate
API work with schema/compatibility checks. GUI locale must not silently govern
machine clients or worker threads.

### Documentation translation batches

| Batch | Files in `resources/docs/` | Dependency / risk |
| --- | --- | --- |
| Entry/navigation | `contents.md`, `getting-started.md`, `user-interface.md`, `keyboard-and-mouse.md` | Locale resolver, accepted menu/editor labels, keyboard tokens unchanged |
| Genome and neural reference | `genomes.md`, `neural-networks.md`, `first-creature.md` | Accepted editor terminology, screenshots and channel names |
| Cell/energy/parameters | `cell-types.md`, `energy.md`, `simulation-parameters.md`, `glossary.md`, `how-life-works.md` | Normative glossary and completed L2 names; distinguish raw/usable/stored/reserved energy |
| Workflows | `sandbox.md`, `evolution.md`, `generative-art.md` | Editing/statistics labels; generated examples remain data |
| Technical/support | `ai-agents.md`, `files.md`, `troubleshooting.md`, `format-reference.md` | Preserve CLI/MCP examples, filenames, JSON/code blocks and URLs; fix chapter reachability |

## Completion risks and audit limits

The highest implementation risks are language-dependent ImGui/ImPlot identity,
translated static caches, indirect shared descriptions, channel graph geometry,
Russian grammar in assembled sentences, byte-based search/snippets, and a shared
documentation reader used by MCP. Wider translated captions require measured
layout changes at normal/high DPI, including neural vertical rails and repeated
mutation columns. Glossary decisions take precedence over arbitrary shortening.

Use the glossary's approved object/cell/mode/shape/energy/signal terms in every
consumer and in documentation. Do not reinterpret Generator as an energy source,
Memory integrator as a different algorithm, or Manual muscle control as direct
user control. `Free` energy provision and additional terms such as mutation
operations/activation descriptions require separate terminology decisions.

The remaining workload is several implementation/review stages plus substantial
documentation translation, not a catalog parity fix. Server emails/admin text
under `source/Server/`, official cloud content and user content are outside a
desktop-language completion claim; localizing those requires a separate server
and content policy. Raw logs/vendor diagnostics should have a documented exception
policy, while application-owned explanatory messages should be translated.

See [localization-roadmap.md](localization-roadmap.md) for ordered implementation
and acceptance gates. No runtime completeness claim is made by this source audit.

## Appendix: direct GUI literal review queue

The following generated source anchors supplement the domain inventory. They
include every GUI match of the stated lexical rule, not just selected examples.
An entry means **review candidate**, not confirmed missing translation. In
already localized dialogs/parameter code it can be a canonical name/key; in
`AlienGui.cpp` it can be an internal widget ID. Consult the classification above
and the full surrounding call before extracting it. Text is excerpted to 140
characters; `...` indicates truncation, not another source message.

Rule: lex C++ strings outside comments/includes/character literals, require two
ASCII letters, exclude values starting `##`/HTTP(S), and select literals directly
following `.name/.text/.tooltip/.hint/.values/.buttonText`, first arguments of
display Text/Button/Menu/Selectable/Table/Plot calls, window/dialog constructors,
or overlay/message calls. A `.values({ ... })` match captures the first literal;
the whole vector still requires review. Count occurrences, not unique keys.
Indirect pools and computed/drawn text are inventoried separately above.

### source/Gui/Impl/AboutDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [11](../source/Gui/Impl/AboutDialog.cpp#L11) | About |
| [17](../source/Gui/Impl/AboutDialog.cpp#L17) | Artificial Life Environment, version %s\n\nis an open source project initiated and maintained by\nChristian Heinemann. |
| [23](../source/Gui/Impl/AboutDialog.cpp#L23) | OK |


### source/Gui/Impl/ActivateUserDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [29](../source/Gui/Impl/ActivateUserDialog.cpp#L29) | Activate user |
| [34](../source/Gui/Impl/ActivateUserDialog.cpp#L34) | Please enter the confirmation code sent to your email address. |
| [39](../source/Gui/Impl/ActivateUserDialog.cpp#L39) | Code (case sensitive) |
| [44](../source/Gui/Impl/ActivateUserDialog.cpp#L44) | OK |
| [55](../source/Gui/Impl/ActivateUserDialog.cpp#L55) | Resend |
| [60](../source/Gui/Impl/ActivateUserDialog.cpp#L60) | Resend to other email address |
| [69](../source/Gui/Impl/ActivateUserDialog.cpp#L69) | Cancel |
| [82](../source/Gui/Impl/ActivateUserDialog.cpp#L82) | Error |
| [85](../source/Gui/Impl/ActivateUserDialog.cpp#L85) | Information |


### source/Gui/Impl/AlienGui.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [1964](../source/Gui/Impl/AlienGui.cpp#L1964) | Open in a separate window |
| [2082](../source/Gui/Impl/AlienGui.cpp#L2082) | None |
| [3082](../source/Gui/Impl/AlienGui.cpp#L3082) | Number of signals |
| [3095](../source/Gui/Impl/AlienGui.cpp#L3095) | Edit signal |


### source/Gui/Impl/AutosaveController.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [143](../source/Gui/Impl/AutosaveController.cpp#L143) | Creating save point ... |
| [173](../source/Gui/Impl/AutosaveController.cpp#L173) | Deleting save point ... |
| [219](../source/Gui/Impl/AutosaveController.cpp#L219) | Cleaning up save points ... |
| [259](../source/Gui/Impl/AutosaveController.cpp#L259) | Error |


### source/Gui/Impl/AutosaveWindow.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [27](../source/Gui/Impl/AutosaveWindow.cpp#L27) | Autosave |
| [68](../source/Gui/Impl/AutosaveWindow.cpp#L68) | Error |
| [79](../source/Gui/Impl/AutosaveWindow.cpp#L79) | Create save point |
| [83](../source/Gui/Impl/AutosaveWindow.cpp#L83) | Delete save point |
| [89](../source/Gui/Impl/AutosaveWindow.cpp#L89) | Delete all save points |
| [92](../source/Gui/Impl/AutosaveWindow.cpp#L92) | Delete |
| [105](../source/Gui/Impl/AutosaveWindow.cpp#L105) | Error: Savepoint files could not be read or created in the specified directory. |
| [112](../source/Gui/Impl/AutosaveWindow.cpp#L112) | Simulation |
| [113](../source/Gui/Impl/AutosaveWindow.cpp#L113) | Timestamp |
| [114](../source/Gui/Impl/AutosaveWindow.cpp#L114) | Time step |
| [133](../source/Gui/Impl/AutosaveWindow.cpp#L133) | In queue |
| [137](../source/Gui/Impl/AutosaveWindow.cpp#L137) | In progress |
| [141](../source/Gui/Impl/AutosaveWindow.cpp#L141) | Load save point |
| [149](../source/Gui/Impl/AutosaveWindow.cpp#L149) | Error |
| [200](../source/Gui/Impl/AutosaveWindow.cpp#L200) | Settings |
| [206](../source/Gui/Impl/AutosaveWindow.cpp#L206) | Autosave interval (min) |
| [231](../source/Gui/Impl/AutosaveWindow.cpp#L231) | Directory |
| [235](../source/Gui/Impl/AutosaveWindow.cpp#L235) | The directory where the savepoints are stored can be chosen here. This allows the savepoints to be created in a separate  |
| [245](../source/Gui/Impl/AutosaveWindow.cpp#L245) | Mode |
| [246](../source/Gui/Impl/AutosaveWindow.cpp#L246) | Limited save files |
| [256](../source/Gui/Impl/AutosaveWindow.cpp#L256) | Number of files |


### source/Gui/Impl/BrowserController.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [88](../source/Gui/Impl/BrowserController.cpp#L88) | Error |


### source/Gui/Impl/BrowserData.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [158](../source/Gui/Impl/BrowserData.cpp#L158) | Error |
| [218](../source/Gui/Impl/BrowserData.cpp#L218) | Error |


### source/Gui/Impl/BrowserGalleryWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [83](../source/Gui/Impl/BrowserGalleryWidget.cpp#L83) | Sort by |
| [98](../source/Gui/Impl/BrowserGalleryWidget.cpp#L98) | Card size |
| [169](../source/Gui/Impl/BrowserGalleryWidget.cpp#L169) | First page |
| [173](../source/Gui/Impl/BrowserGalleryWidget.cpp#L173) | Previous page |
| [183](../source/Gui/Impl/BrowserGalleryWidget.cpp#L183) | Next page |
| [187](../source/Gui/Impl/BrowserGalleryWidget.cpp#L187) | Last page |
| [533](../source/Gui/Impl/BrowserGalleryWidget.cpp#L533) | Add a reaction |


### source/Gui/Impl/BrowserHelper.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [29](../source/Gui/Impl/BrowserHelper.cpp#L29) | Download |


### source/Gui/Impl/BrowserLoginBannerWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [73](../source/Gui/Impl/BrowserLoginBannerWidget.cpp#L73) | Hide the invitation until the next program start |


### source/Gui/Impl/BrowserTableWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [215](../source/Gui/Impl/BrowserTableWidget.cpp#L215) | NEW |
| [326](../source/Gui/Impl/BrowserTableWidget.cpp#L326) | Add a reaction |


### source/Gui/Impl/BrowserUserListWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [73](../source/Gui/Impl/BrowserUserListWidget.cpp#L73) | Simulators |
| [113](../source/Gui/Impl/BrowserUserListWidget.cpp#L113) | not shared |
| [228](../source/Gui/Impl/BrowserUserListWidget.cpp#L228) | Name |
| [232](../source/Gui/Impl/BrowserUserListWidget.cpp#L232) | Time spent |
| [234](../source/Gui/Impl/BrowserUserListWidget.cpp#L234) | Reactions received |
| [237](../source/Gui/Impl/BrowserUserListWidget.cpp#L237) | Reactions given |


### source/Gui/Impl/BrowserWindow.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [51](../source/Gui/Impl/BrowserWindow.cpp#L51) | Browser |
| [188](../source/Gui/Impl/BrowserWindow.cpp#L188) | Refresh |
| [195](../source/Gui/Impl/BrowserWindow.cpp#L195) | Upload  |
| [197](../source/Gui/Impl/BrowserWindow.cpp#L197) | Upload your current  |
| [216](../source/Gui/Impl/BrowserWindow.cpp#L216) | Change name or description |
| [221](../source/Gui/Impl/BrowserWindow.cpp#L221) | Replace  |
| [223](../source/Gui/Impl/BrowserWindow.cpp#L223) | Replace the selected  |
| [230](../source/Gui/Impl/BrowserWindow.cpp#L230) | Change visibility |
| [231](../source/Gui/Impl/BrowserWindow.cpp#L231) | Change visibility: Community  |
| [235](../source/Gui/Impl/BrowserWindow.cpp#L235) | Delete selected  |
| [240](../source/Gui/Impl/BrowserWindow.cpp#L240) | Expand all folders |
| [244](../source/Gui/Impl/BrowserWindow.cpp#L244) | Collapse all folders |
| [250](../source/Gui/Impl/BrowserWindow.cpp#L250) | Gallery view |
| [251](../source/Gui/Impl/BrowserWindow.cpp#L251) | Show the  |
| [256](../source/Gui/Impl/BrowserWindow.cpp#L256) | Table view |
| [257](../source/Gui/Impl/BrowserWindow.cpp#L257) | Show the  |
| [263](../source/Gui/Impl/BrowserWindow.cpp#L263) | Open ALIEN Discord server |
| [283](../source/Gui/Impl/BrowserWindow.cpp#L283) | Simulations |
| [283](../source/Gui/Impl/BrowserWindow.cpp#L283) | Genomes |
| [454](../source/Gui/Impl/BrowserWindow.cpp#L454) | Choose a reaction |
| [482](../source/Gui/Impl/BrowserWindow.cpp#L482) | More |
| [599](../source/Gui/Impl/BrowserWindow.cpp#L599) | Delete |


### source/Gui/Impl/ChangeColorDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [20](../source/Gui/Impl/ChangeColorDialog.cpp#L20) | Change color |
| [25](../source/Gui/Impl/ChangeColorDialog.cpp#L25) | Filter |
| [28](../source/Gui/Impl/ChangeColorDialog.cpp#L28) | Restrict to selected gene |
| [30](../source/Gui/Impl/ChangeColorDialog.cpp#L30) | Color transition rule |
| [66](../source/Gui/Impl/ChangeColorDialog.cpp#L66) | OK |
| [72](../source/Gui/Impl/ChangeColorDialog.cpp#L72) | Cancel |


### source/Gui/Impl/CreateUserDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [14](../source/Gui/Impl/CreateUserDialog.cpp#L14) | Create user |
| [28](../source/Gui/Impl/CreateUserDialog.cpp#L28) | Security information |
| [31](../source/Gui/Impl/CreateUserDialog.cpp#L31) | Data privacy policy |
| [36](../source/Gui/Impl/CreateUserDialog.cpp#L36) | Please enter your email address to receive the\nconfirmation code for the new user. |
| [38](../source/Gui/Impl/CreateUserDialog.cpp#L38) | Email |
| [43](../source/Gui/Impl/CreateUserDialog.cpp#L43) | Create user |
| [52](../source/Gui/Impl/CreateUserDialog.cpp#L52) | Cancel |
| [63](../source/Gui/Impl/CreateUserDialog.cpp#L63) | Error |


### source/Gui/Impl/CreatorWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [50](../source/Gui/Impl/CreatorWidget.cpp#L50) | Color |
| [61](../source/Gui/Impl/CreatorWidget.cpp#L61) | Pencil radius |
| [74](../source/Gui/Impl/CreatorWidget.cpp#L74) | Material |
| [76](../source/Gui/Impl/CreatorWidget.cpp#L76) | Solid |
| [80](../source/Gui/Impl/CreatorWidget.cpp#L80) | Energy |
| [84](../source/Gui/Impl/CreatorWidget.cpp#L84) | Glow |
| [95](../source/Gui/Impl/CreatorWidget.cpp#L95) | Stiffness |
| [109](../source/Gui/Impl/CreatorWidget.cpp#L109) | Horizontal objects |
| [112](../source/Gui/Impl/CreatorWidget.cpp#L112) | Vertical objects |
| [117](../source/Gui/Impl/CreatorWidget.cpp#L117) | Layers |
| [121](../source/Gui/Impl/CreatorWidget.cpp#L121) | Outer radius |
| [124](../source/Gui/Impl/CreatorWidget.cpp#L124) | Inner radius |
| [132](../source/Gui/Impl/CreatorWidget.cpp#L132) | Object distance |
| [139](../source/Gui/Impl/CreatorWidget.cpp#L139) | Sticky |
| [147](../source/Gui/Impl/CreatorWidget.cpp#L147) | Static |
| [158](../source/Gui/Impl/CreatorWidget.cpp#L158) | Finish |
| [165](../source/Gui/Impl/CreatorWidget.cpp#L165) | Abort |


### source/Gui/Impl/CreaturePreviewWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [265](../source/Gui/Impl/CreaturePreviewWidget.cpp#L265) | Hide the neural activity editor |
| [293](../source/Gui/Impl/CreaturePreviewWidget.cpp#L293) | Neural activity editor |
| [360](../source/Gui/Impl/CreaturePreviewWidget.cpp#L360) | Detailed simulation mode disabled |
| [362](../source/Gui/Impl/CreaturePreviewWidget.cpp#L362) | No cell selected |


### source/Gui/Impl/DeleteUserDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [13](../source/Gui/Impl/DeleteUserDialog.cpp#L13) | Delete user |
| [19](../source/Gui/Impl/DeleteUserDialog.cpp#L19) | Warning: All the data of the user ' |
| [23](../source/Gui/Impl/DeleteUserDialog.cpp#L23) | Re-enter password |
| [27](../source/Gui/Impl/DeleteUserDialog.cpp#L27) | Delete |
| [32](../source/Gui/Impl/DeleteUserDialog.cpp#L32) | Error |
| [40](../source/Gui/Impl/DeleteUserDialog.cpp#L40) | Cancel |
| [51](../source/Gui/Impl/DeleteUserDialog.cpp#L51) | Information |
| [53](../source/Gui/Impl/DeleteUserDialog.cpp#L53) | Error |


### source/Gui/Impl/DisplaySettingsDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [25](../source/Gui/Impl/DisplaySettingsDialog.cpp#L25) | Display settings |
| [38](../source/Gui/Impl/DisplaySettingsDialog.cpp#L38) | Full screen |
| [46](../source/Gui/Impl/DisplaySettingsDialog.cpp#L46) | Resolution |
| [57](../source/Gui/Impl/DisplaySettingsDialog.cpp#L57) | Frames per second |
| [68](../source/Gui/Impl/DisplaySettingsDialog.cpp#L68) | Adopt scaling from OS |
| [78](../source/Gui/Impl/DisplaySettingsDialog.cpp#L78) | Content scaling |


### source/Gui/Impl/DocumentationWindow.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [35](../source/Gui/Impl/DocumentationWindow.cpp#L35) | Documentation |
| [83](../source/Gui/Impl/DocumentationWindow.cpp#L83) | Show after startup |
| [253](../source/Gui/Impl/DocumentationWindow.cpp#L253) | Search |
| [314](../source/Gui/Impl/DocumentationWindow.cpp#L314) | No matches found. |
| [353](../source/Gui/Impl/DocumentationWindow.cpp#L353) | No documentation found in  |


### source/Gui/Impl/EditorController.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [101](../source/Gui/Impl/EditorController.cpp#L101) | Selection copied |
| [115](../source/Gui/Impl/EditorController.cpp#L115) | Selection pasted |
| [127](../source/Gui/Impl/EditorController.cpp#L127) | Selection deleted |
| [179](../source/Gui/Impl/EditorController.cpp#L179) | Selection glued |


### source/Gui/Impl/EditorWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [84](../source/Gui/Impl/EditorWidget.cpp#L84) | Apply forces (available while the simulation is running) |
| [92](../source/Gui/Impl/EditorWidget.cpp#L92) | MCP server |
| [93](../source/Gui/Impl/EditorWidget.cpp#L93) | Connect your AI agent to ALIEN |
| [97](../source/Gui/Impl/EditorWidget.cpp#L97) | Pattern from image |
| [97](../source/Gui/Impl/EditorWidget.cpp#L97) | Create a pattern from an image |
| [102](../source/Gui/Impl/EditorWidget.cpp#L102) | Paste |
| [103](../source/Gui/Impl/EditorWidget.cpp#L103) | Paste the copied selection (CTRL+V) |
| [110](../source/Gui/Impl/EditorWidget.cpp#L110) | Apply to selected objects |
| [111](../source/Gui/Impl/EditorWidget.cpp#L111) | Apply to selected objects\n\nEdits only affect the selected objects. When they are moved, their connections to unselected  |
| [118](../source/Gui/Impl/EditorWidget.cpp#L118) | Apply to entire networks |
| [119](../source/Gui/Impl/EditorWidget.cpp#L119) | Apply to entire networks\n\nEdits affect the whole object networks of the selected objects.\nHold SHIFT to switch to selected  |
| [126](../source/Gui/Impl/EditorWidget.cpp#L126) | Glue on contact |
| [127](../source/Gui/Impl/EditorWidget.cpp#L127) | Glue on contact\n\nIf enabled, moved or rotated objects connect to the objects they touch. |
| [182](../source/Gui/Impl/EditorWidget.cpp#L182) | Only in selection |
| [184](../source/Gui/Impl/EditorWidget.cpp#L184) | If enabled, only connections between selected objects are cut. |


### source/Gui/Impl/EditSimulationDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [58](../source/Gui/Impl/EditSimulationDialog.cpp#L58) | Name |
| [64](../source/Gui/Impl/EditSimulationDialog.cpp#L64) | Desc (optional) |
| [71](../source/Gui/Impl/EditSimulationDialog.cpp#L71) | OK |
| [77](../source/Gui/Impl/EditSimulationDialog.cpp#L77) | Error |
| [84](../source/Gui/Impl/EditSimulationDialog.cpp#L84) | Cancel |
| [92](../source/Gui/Impl/EditSimulationDialog.cpp#L92) | Folder name |
| [99](../source/Gui/Impl/EditSimulationDialog.cpp#L99) | OK |
| [111](../source/Gui/Impl/EditSimulationDialog.cpp#L111) | Error |
| [118](../source/Gui/Impl/EditSimulationDialog.cpp#L118) | Cancel |


### source/Gui/Impl/EvolutionDashboardWindow.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [316](../source/Gui/Impl/EvolutionDashboardWindow.cpp#L316) | Evolution dashboard |
| [742](../source/Gui/Impl/EvolutionDashboardWindow.cpp#L742) | Filter by customizations |
| [786](../source/Gui/Impl/EvolutionDashboardWindow.cpp#L786) | Lineage |
| [787](../source/Gui/Impl/EvolutionDashboardWindow.cpp#L787) | Customizations |
| [850](../source/Gui/Impl/EvolutionDashboardWindow.cpp#L850) | All filtered ( |
| [881](../source/Gui/Impl/EvolutionDashboardWindow.cpp#L881) | Open the genome of a representative creature (highest generation) in the genome editor |
| [891](../source/Gui/Impl/EvolutionDashboardWindow.cpp#L891) | Lineage # |
| [933](../source/Gui/Impl/EvolutionDashboardWindow.cpp#L933) | The creature no longer exists |
| [973](../source/Gui/Impl/EvolutionDashboardWindow.cpp#L973) | Mode |
| [984](../source/Gui/Impl/EvolutionDashboardWindow.cpp#L984) | Steps |
| [995](../source/Gui/Impl/EvolutionDashboardWindow.cpp#L995) | Plot height |


### source/Gui/Impl/ExitDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [11](../source/Gui/Impl/ExitDialog.cpp#L11) | Exit |


### source/Gui/Impl/FileTransferController.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [46](../source/Gui/Impl/FileTransferController.cpp#L46) | Loading ... |
| [57](../source/Gui/Impl/FileTransferController.cpp#L57) | Error |
| [61](../source/Gui/Impl/FileTransferController.cpp#L61) | Error |
| [69](../source/Gui/Impl/FileTransferController.cpp#L69) | Saving ... |
| [77](../source/Gui/Impl/FileTransferController.cpp#L77) | Error |
| [92](../source/Gui/Impl/FileTransferController.cpp#L92) | Open genome |
| [106](../source/Gui/Impl/FileTransferController.cpp#L106) | Save genome |


### source/Gui/Impl/GeneEditorWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [45](../source/Gui/Impl/GeneEditorWidget.cpp#L45) | Nothing selected |
| [77](../source/Gui/Impl/GeneEditorWidget.cpp#L77) | Base properties |
| [81](../source/Gui/Impl/GeneEditorWidget.cpp#L81) | Gene name |
| [87](../source/Gui/Impl/GeneEditorWidget.cpp#L87) | Shape generator |
| [103](../source/Gui/Impl/GeneEditorWidget.cpp#L103) | Connection distance |
| [113](../source/Gui/Impl/GeneEditorWidget.cpp#L113) | Stiffness |
| [123](../source/Gui/Impl/GeneEditorWidget.cpp#L123) | Homogeneous cell type |


### source/Gui/Impl/GenericMessageDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [91](../source/Gui/Impl/GenericMessageDialog.cpp#L91) | Message |


### source/Gui/Impl/GenomeEditorWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [92](../source/Gui/Impl/GenomeEditorWidget.cpp#L92) | Genome |
| [102](../source/Gui/Impl/GenomeEditorWidget.cpp#L102) | Genome name |
| [110](../source/Gui/Impl/GenomeEditorWidget.cpp#L110) | Front angle |
| [120](../source/Gui/Impl/GenomeEditorWidget.cpp#L120) | Resistance to injection |
| [127](../source/Gui/Impl/GenomeEditorWidget.cpp#L127) | Apply meta-mutations |
| [141](../source/Gui/Impl/GenomeEditorWidget.cpp#L141) | Structure |
| [156](../source/Gui/Impl/GenomeEditorWidget.cpp#L156) | Name |
| [157](../source/Gui/Impl/GenomeEditorWidget.cpp#L157) | Type |
| [158](../source/Gui/Impl/GenomeEditorWidget.cpp#L158) | References |
| [159](../source/Gui/Impl/GenomeEditorWidget.cpp#L159) | Referenced by |
| [343](../source/Gui/Impl/GenomeEditorWidget.cpp#L343) | Add gene |
| [349](../source/Gui/Impl/GenomeEditorWidget.cpp#L349) | Add node to the selected gene |
| [410](../source/Gui/Impl/GenomeEditorWidget.cpp#L410) | Fix all warnings shown in the gene network |
| [503](../source/Gui/Impl/GenomeEditorWidget.cpp#L503) | Error |
| [517](../source/Gui/Impl/GenomeEditorWidget.cpp#L517) | Error |
| [531](../source/Gui/Impl/GenomeEditorWidget.cpp#L531) | Error |
| [536](../source/Gui/Impl/GenomeEditorWidget.cpp#L536) | Delete root gene |
| [547](../source/Gui/Impl/GenomeEditorWidget.cpp#L547) | Swap root gene |
| [556](../source/Gui/Impl/GenomeEditorWidget.cpp#L556) | Swap root gene |
| [565](../source/Gui/Impl/GenomeEditorWidget.cpp#L565) | Fix all warnings |


### source/Gui/Impl/GenomeEditorWindow.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [92](../source/Gui/Impl/GenomeEditorWindow.cpp#L92) | Genome editor |
| [156](../source/Gui/Impl/GenomeEditorWindow.cpp#L156) | Open genome |
| [156](../source/Gui/Impl/GenomeEditorWindow.cpp#L156) | Open genome from file |
| [158](../source/Gui/Impl/GenomeEditorWindow.cpp#L158) | Save genome |
| [158](../source/Gui/Impl/GenomeEditorWindow.cpp#L158) | Save genome to file |
| [162](../source/Gui/Impl/GenomeEditorWindow.cpp#L162) | Share genome |
| [163](../source/Gui/Impl/GenomeEditorWindow.cpp#L163) | Share your genome with other users:\nYour current genome will be uploaded to the server and made visible in the browser. |
| [167](../source/Gui/Impl/GenomeEditorWindow.cpp#L167) | Clone genome |
| [167](../source/Gui/Impl/GenomeEditorWindow.cpp#L167) | Open a copy of the current genome in a new tab |
| [171](../source/Gui/Impl/GenomeEditorWindow.cpp#L171) | Copy genome |
| [171](../source/Gui/Impl/GenomeEditorWindow.cpp#L171) | Copy genome to clipboard |
| [174](../source/Gui/Impl/GenomeEditorWindow.cpp#L174) | Paste genome |
| [175](../source/Gui/Impl/GenomeEditorWindow.cpp#L175) | Paste genome from clipboard |
| [180](../source/Gui/Impl/GenomeEditorWindow.cpp#L180) | Close other tabs |
| [181](../source/Gui/Impl/GenomeEditorWindow.cpp#L181) | Close all tabs except the current one |
| [187](../source/Gui/Impl/GenomeEditorWindow.cpp#L187) | Create save point |
| [188](../source/Gui/Impl/GenomeEditorWindow.cpp#L188) | Create save point in this tab |
| [193](../source/Gui/Impl/GenomeEditorWindow.cpp#L193) | Revert to save point |
| [194](../source/Gui/Impl/GenomeEditorWindow.cpp#L194) | Revert genome to save point |
| [199](../source/Gui/Impl/GenomeEditorWindow.cpp#L199) | Change colors |
| [199](../source/Gui/Impl/GenomeEditorWindow.cpp#L199) | Change the color of all nodes with a certain color |
| [205](../source/Gui/Impl/GenomeEditorWindow.cpp#L205) | Inject genome |
| [206](../source/Gui/Impl/GenomeEditorWindow.cpp#L206) | Inject the current genome into the selected creatures in the simulation |
| [211](../source/Gui/Impl/GenomeEditorWindow.cpp#L211) | Create seed |
| [212](../source/Gui/Impl/GenomeEditorWindow.cpp#L212) | Create a seed with the current genome. It needs energy to build its first offspring. |
| [218](../source/Gui/Impl/GenomeEditorWindow.cpp#L218) | Create seed with energy |
| [219](../source/Gui/Impl/GenomeEditorWindow.cpp#L219) | Create a seed with the current genome. It builds its first offspring without energy cost. |
| [229](../source/Gui/Impl/GenomeEditorWindow.cpp#L229) | New genome |
| [334](../source/Gui/Impl/GenomeEditorWindow.cpp#L334) | Genome injected to  |
| [350](../source/Gui/Impl/GenomeEditorWindow.cpp#L350) | Seed created |
| [370](../source/Gui/Impl/GenomeEditorWindow.cpp#L370) | Gene 0 |


### source/Gui/Impl/GenomeTabWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [244](../source/Gui/Impl/GenomeTabWidget.cpp#L244) | Phenotype |
| [245](../source/Gui/Impl/GenomeTabWidget.cpp#L245) | Gene network |


### source/Gui/Impl/ImageToPatternDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [44](../source/Gui/Impl/ImageToPatternDialog.cpp#L44) | Error |


### source/Gui/Impl/InspectionController.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [59](../source/Gui/Impl/InspectionController.cpp#L59) | Inspection not possible |
| [107](../source/Gui/Impl/InspectionController.cpp#L107) | Inspection not possible |
| [171](../source/Gui/Impl/InspectionController.cpp#L171) | Inspection not possible |


### source/Gui/Impl/InspectionWindow.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [55](../source/Gui/Impl/InspectionWindow.cpp#L55) | Gene index |
| [67](../source/Gui/Impl/InspectionWindow.cpp#L67) | Gene index |
| [73](../source/Gui/Impl/InspectionWindow.cpp#L73) | Properties## |
| [401](../source/Gui/Impl/InspectionWindow.cpp#L401) | Energy particle |
| [406](../source/Gui/Impl/InspectionWindow.cpp#L406) | Position |
| [414](../source/Gui/Impl/InspectionWindow.cpp#L414) | Velocity |
| [421](../source/Gui/Impl/InspectionWindow.cpp#L421) | Energy |
| [427](../source/Gui/Impl/InspectionWindow.cpp#L427) | Color |
| [444](../source/Gui/Impl/InspectionWindow.cpp#L444) | Object |
| [449](../source/Gui/Impl/InspectionWindow.cpp#L449) | Position |
| [457](../source/Gui/Impl/InspectionWindow.cpp#L457) | Velocity |
| [465](../source/Gui/Impl/InspectionWindow.cpp#L465) | Stiffness |
| [474](../source/Gui/Impl/InspectionWindow.cpp#L474) | Color |
| [479](../source/Gui/Impl/InspectionWindow.cpp#L479) | Static |
| [482](../source/Gui/Impl/InspectionWindow.cpp#L482) | Sticky |
| [486](../source/Gui/Impl/InspectionWindow.cpp#L486) | Object type |
| [493](../source/Gui/Impl/InspectionWindow.cpp#L493) | Connections |
| [498](../source/Gui/Impl/InspectionWindow.cpp#L498) | Connection # |
| [502](../source/Gui/Impl/InspectionWindow.cpp#L502) | Distance |
| [510](../source/Gui/Impl/InspectionWindow.cpp#L510) | Ref angle |
| [528](../source/Gui/Impl/InspectionWindow.cpp#L528) | Solid |
| [532](../source/Gui/Impl/InspectionWindow.cpp#L532) | Energy |
| [542](../source/Gui/Impl/InspectionWindow.cpp#L542) | Fluid |
| [546](../source/Gui/Impl/InspectionWindow.cpp#L546) | Energy |
| [550](../source/Gui/Impl/InspectionWindow.cpp#L550) | Glow |
| [560](../source/Gui/Impl/InspectionWindow.cpp#L560) | Free cell |
| [564](../source/Gui/Impl/InspectionWindow.cpp#L564) | Energy |
| [568](../source/Gui/Impl/InspectionWindow.cpp#L568) | Age |
| [576](../source/Gui/Impl/InspectionWindow.cpp#L576) | Cell |
| [581](../source/Gui/Impl/InspectionWindow.cpp#L581) | Usable energy |
| [588](../source/Gui/Impl/InspectionWindow.cpp#L588) | Raw energy |
| [595](../source/Gui/Impl/InspectionWindow.cpp#L595) | Front angle |
| [601](../source/Gui/Impl/InspectionWindow.cpp#L601) | Age |
| [604](../source/Gui/Impl/InspectionWindow.cpp#L604) | Cell state |
| [607](../source/Gui/Impl/InspectionWindow.cpp#L607) | Node index |
| [610](../source/Gui/Impl/InspectionWindow.cpp#L610) | Gene index |
| [614](../source/Gui/Impl/InspectionWindow.cpp#L614) | Concatenation index |
| [619](../source/Gui/Impl/InspectionWindow.cpp#L619) | Branch index |
| [623](../source/Gui/Impl/InspectionWindow.cpp#L623) | Construction id |
| [629](../source/Gui/Impl/InspectionWindow.cpp#L629) | Activation time |
| [634](../source/Gui/Impl/InspectionWindow.cpp#L634) | Head cell |
| [639](../source/Gui/Impl/InspectionWindow.cpp#L639) | Cell type |
| [652](../source/Gui/Impl/InspectionWindow.cpp#L652) | Has constructor |
| [680](../source/Gui/Impl/InspectionWindow.cpp#L680) | Constructor |
| [683](../source/Gui/Impl/InspectionWindow.cpp#L683) | Auto trigger interval |
| [689](../source/Gui/Impl/InspectionWindow.cpp#L689) | Construction angle |
| [697](../source/Gui/Impl/InspectionWindow.cpp#L697) | Provide energy |
| [706](../source/Gui/Impl/InspectionWindow.cpp#L706) | Reserved energy |
| [712](../source/Gui/Impl/InspectionWindow.cpp#L712) | Separation |
| [720](../source/Gui/Impl/InspectionWindow.cpp#L720) | Number of branches |
| [731](../source/Gui/Impl/InspectionWindow.cpp#L731) | Concatenations |
| [749](../source/Gui/Impl/InspectionWindow.cpp#L749) | Lineage id |
| [758](../source/Gui/Impl/InspectionWindow.cpp#L758) | Edit |
| [759](../source/Gui/Impl/InspectionWindow.cpp#L759) | Edit genome |
| [768](../source/Gui/Impl/InspectionWindow.cpp#L768) | Creature |
| [776](../source/Gui/Impl/InspectionWindow.cpp#L776) | Neural activity |
| [784](../source/Gui/Impl/InspectionWindow.cpp#L784) | Signal # |
| [800](../source/Gui/Impl/InspectionWindow.cpp#L800) | Mem # |
| [816](../source/Gui/Impl/InspectionWindow.cpp#L816) | Neural network |
| [850](../source/Gui/Impl/InspectionWindow.cpp#L850) | Channel mask bit 0-3 |
| [859](../source/Gui/Impl/InspectionWindow.cpp#L859) | Channel mask bit 4-7 |
| [886](../source/Gui/Impl/InspectionWindow.cpp#L886) | Storage limit |
| [893](../source/Gui/Impl/InspectionWindow.cpp#L893) | Stored usable energy |
| [901](../source/Gui/Impl/InspectionWindow.cpp#L901) | Auto trigger |
| [905](../source/Gui/Impl/InspectionWindow.cpp#L905) | Tag for attackers |
| [911](../source/Gui/Impl/InspectionWindow.cpp#L911) | Mode |
| [919](../source/Gui/Impl/InspectionWindow.cpp#L919) | Min density |
| [929](../source/Gui/Impl/InspectionWindow.cpp#L929) | Min density |
| [938](../source/Gui/Impl/InspectionWindow.cpp#L938) | Restrict to colors |
| [946](../source/Gui/Impl/InspectionWindow.cpp#L946) | Min num cells |
| [952](../source/Gui/Impl/InspectionWindow.cpp#L952) | Max num cells |
| [959](../source/Gui/Impl/InspectionWindow.cpp#L959) | Restrict to colors |
| [964](../source/Gui/Impl/InspectionWindow.cpp#L964) | Restrict to lineage |
| [966](../source/Gui/Impl/InspectionWindow.cpp#L966) | No |
| [972](../source/Gui/Impl/InspectionWindow.cpp#L972) | Min range |
| [980](../source/Gui/Impl/InspectionWindow.cpp#L980) | Max range |
| [989](../source/Gui/Impl/InspectionWindow.cpp#L989) | Additive |
| [993](../source/Gui/Impl/InspectionWindow.cpp#L993) | Min value |
| [1001](../source/Gui/Impl/InspectionWindow.cpp#L1001) | Max value |
| [1008](../source/Gui/Impl/InspectionWindow.cpp#L1008) | Time offset |
| [1012](../source/Gui/Impl/InspectionWindow.cpp#L1012) | Mode |
| [1019](../source/Gui/Impl/InspectionWindow.cpp#L1019) | Period |
| [1024](../source/Gui/Impl/InspectionWindow.cpp#L1024) | Period |
| [1031](../source/Gui/Impl/InspectionWindow.cpp#L1031) | Mode |
| [1040](../source/Gui/Impl/InspectionWindow.cpp#L1040) | Restrict to colors |
| [1052](../source/Gui/Impl/InspectionWindow.cpp#L1052) | Mode |
| [1057](../source/Gui/Impl/InspectionWindow.cpp#L1057) | Max angle deviation |
| [1064](../source/Gui/Impl/InspectionWindow.cpp#L1064) | Max distance deviation |
| [1071](../source/Gui/Impl/InspectionWindow.cpp#L1071) | Forward backward ratio |
| [1090](../source/Gui/Impl/InspectionWindow.cpp#L1090) | Attraction repulsion ratio |
| [1109](../source/Gui/Impl/InspectionWindow.cpp#L1109) | Mode |
| [1117](../source/Gui/Impl/InspectionWindow.cpp#L1117) | Mode |
| [1129](../source/Gui/Impl/InspectionWindow.cpp#L1129) | Restrict to colors |
| [1137](../source/Gui/Impl/InspectionWindow.cpp#L1137) | Min num cells |
| [1143](../source/Gui/Impl/InspectionWindow.cpp#L1143) | Max num cells |
| [1150](../source/Gui/Impl/InspectionWindow.cpp#L1150) | Restrict to colors |
| [1155](../source/Gui/Impl/InspectionWindow.cpp#L1155) | Restrict to lineage |
| [1157](../source/Gui/Impl/InspectionWindow.cpp#L1157) | No |
| [1166](../source/Gui/Impl/InspectionWindow.cpp#L1166) | State |
| [1171](../source/Gui/Impl/InspectionWindow.cpp#L1171) | Countdown |
| [1177](../source/Gui/Impl/InspectionWindow.cpp#L1177) | Energy conductivity |
| [1186](../source/Gui/Impl/InspectionWindow.cpp#L1186) | Energy conversion |
| [1197](../source/Gui/Impl/InspectionWindow.cpp#L1197) | Mode |
| [1207](../source/Gui/Impl/InspectionWindow.cpp#L1207) | Delay |
| [1212](../source/Gui/Impl/InspectionWindow.cpp#L1212) | Read only |
| [1217](../source/Gui/Impl/InspectionWindow.cpp#L1217) | Read only |
| [1223](../source/Gui/Impl/InspectionWindow.cpp#L1223) | New signal weight |
| [1232](../source/Gui/Impl/InspectionWindow.cpp#L1232) | Edit |
| [1233](../source/Gui/Impl/InspectionWindow.cpp#L1233) | Signal buffer |
| [1243](../source/Gui/Impl/InspectionWindow.cpp#L1243) | Mode |
| [1253](../source/Gui/Impl/InspectionWindow.cpp#L1253) | Range |
| [1257](../source/Gui/Impl/InspectionWindow.cpp#L1257) | One-way |
| [1264](../source/Gui/Impl/InspectionWindow.cpp#L1264) | Restrict to colors |
| [1269](../source/Gui/Impl/InspectionWindow.cpp#L1269) | Restrict to lineage |
| [1271](../source/Gui/Impl/InspectionWindow.cpp#L1271) | No |


### source/Gui/Impl/LocalizationGui.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [12](../source/Gui/Impl/LocalizationGui.cpp#L12) | settings.language |


### source/Gui/Impl/LoginController.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [66](../source/Gui/Impl/LoginController.cpp#L66) | Error |


### source/Gui/Impl/LoginDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [28](../source/Gui/Impl/LoginDialog.cpp#L28) | Login or register |
| [33](../source/Gui/Impl/LoginDialog.cpp#L33) | How to create a new user? |
| [36](../source/Gui/Impl/LoginDialog.cpp#L36) | Forgot your password? |
| [39](../source/Gui/Impl/LoginDialog.cpp#L39) | Security information |
| [46](../source/Gui/Impl/LoginDialog.cpp#L46) | User name |
| [50](../source/Gui/Impl/LoginDialog.cpp#L50) | Password |
| [57](../source/Gui/Impl/LoginDialog.cpp#L57) | Remember |
| [63](../source/Gui/Impl/LoginDialog.cpp#L63) | Share GPU model info |
| [72](../source/Gui/Impl/LoginDialog.cpp#L72) | Login |
| [84](../source/Gui/Impl/LoginDialog.cpp#L84) | Create user |
| [92](../source/Gui/Impl/LoginDialog.cpp#L92) | Reset password |
| [102](../source/Gui/Impl/LoginDialog.cpp#L102) | Cancel |


### source/Gui/Impl/LogWindow.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [27](../source/Gui/Impl/LogWindow.cpp#L27) | Log |
| [42](../source/Gui/Impl/LogWindow.cpp#L42) | Timestamp |
| [43](../source/Gui/Impl/LogWindow.cpp#L43) | Message |
| [78](../source/Gui/Impl/LogWindow.cpp#L78) | Verbose |


### source/Gui/Impl/MainLoopController.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [253](../source/Gui/Impl/MainLoopController.cpp#L253) | Error |
| [329](../source/Gui/Impl/MainLoopController.cpp#L329) | Saving on exit ... |


### source/Gui/Impl/MassEditDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [110](../source/Gui/Impl/MassEditDialog.cpp#L110) | Filter |
| [113](../source/Gui/Impl/MassEditDialog.cpp#L113) | Restrict to selection |
| [115](../source/Gui/Impl/MassEditDialog.cpp#L115) | Operations |
| [121](../source/Gui/Impl/MassEditDialog.cpp#L121) | Randomize object colors |
| [138](../source/Gui/Impl/MassEditDialog.cpp#L138) | Randomize genome colors |
| [155](../source/Gui/Impl/MassEditDialog.cpp#L155) | Randomize energies |
| [160](../source/Gui/Impl/MassEditDialog.cpp#L160) | Minimum energy |
| [162](../source/Gui/Impl/MassEditDialog.cpp#L162) | Maximum energy |
| [167](../source/Gui/Impl/MassEditDialog.cpp#L167) | Randomize cell ages |
| [172](../source/Gui/Impl/MassEditDialog.cpp#L172) | Minimum age |
| [174](../source/Gui/Impl/MassEditDialog.cpp#L174) | Maximum age |
| [179](../source/Gui/Impl/MassEditDialog.cpp#L179) | Randomize detonation countdown |
| [184](../source/Gui/Impl/MassEditDialog.cpp#L184) | Minimum value |
| [186](../source/Gui/Impl/MassEditDialog.cpp#L186) | Maximum value |
| [191](../source/Gui/Impl/MassEditDialog.cpp#L191) | Randomize mutants |
| [194](../source/Gui/Impl/MassEditDialog.cpp#L194) | Randomize lineage ids |
| [198](../source/Gui/Impl/MassEditDialog.cpp#L198) | Randomize fluid glow |
| [203](../source/Gui/Impl/MassEditDialog.cpp#L203) | Minimum glow |
| [205](../source/Gui/Impl/MassEditDialog.cpp#L205) | Maximum glow |
| [210](../source/Gui/Impl/MassEditDialog.cpp#L210) | Mutation rates |
| [225](../source/Gui/Impl/MassEditDialog.cpp#L225) | OK |
| [233](../source/Gui/Impl/MassEditDialog.cpp#L233) | Cancel |
| [241](../source/Gui/Impl/MassEditDialog.cpp#L241) | Mass operations |


### source/Gui/Impl/McpController.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [59](../source/Gui/Impl/McpController.cpp#L59) | MCP server running at  |
| [63](../source/Gui/Impl/McpController.cpp#L63) | MCP server stopped |
| [220](../source/Gui/Impl/McpController.cpp#L220) | MCP server |


### source/Gui/Impl/McpSettingsDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [15](../source/Gui/Impl/McpSettingsDialog.cpp#L15) | MCP server settings |
| [24](../source/Gui/Impl/McpSettingsDialog.cpp#L24) | Port |
| [27](../source/Gui/Impl/McpSettingsDialog.cpp#L27) | Local port on which the MCP server accepts connections. A running server is restarted on the new port. |
| [33](../source/Gui/Impl/McpSettingsDialog.cpp#L33) | Adopt |
| [40](../source/Gui/Impl/McpSettingsDialog.cpp#L40) | Cancel |


### source/Gui/Impl/McpWindow.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [33](../source/Gui/Impl/McpWindow.cpp#L33) | MCP server |
| [74](../source/Gui/Impl/McpWindow.cpp#L74) | Start server |
| [76](../source/Gui/Impl/McpWindow.cpp#L76) | Stop server |
| [79](../source/Gui/Impl/McpWindow.cpp#L79) | Clear command log |
| [84](../source/Gui/Impl/McpWindow.cpp#L84) | Settings |
| [93](../source/Gui/Impl/McpWindow.cpp#L93) | Connect your AI agent |
| [114](../source/Gui/Impl/McpWindow.cpp#L114) | In your AI agent, add an MCP server of type HTTP. |
| [117](../source/Gui/Impl/McpWindow.cpp#L117) | Use this URL: |
| [137](../source/Gui/Impl/McpWindow.cpp#L137) | Copied to clipboard |
| [170](../source/Gui/Impl/McpWindow.cpp#L170) | Timestamp |
| [171](../source/Gui/Impl/McpWindow.cpp#L171) | Command |
| [172](../source/Gui/Impl/McpWindow.cpp#L172) | Result |


### source/Gui/Impl/MultiplierWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [115](../source/Gui/Impl/MultiplierWidget.cpp#L115) | Build |
| [122](../source/Gui/Impl/MultiplierWidget.cpp#L122) | Undo |
| [181](../source/Gui/Impl/MultiplierWidget.cpp#L181) | Number of copies |
| [182](../source/Gui/Impl/MultiplierWidget.cpp#L182) | Min angle |
| [183](../source/Gui/Impl/MultiplierWidget.cpp#L183) | Max angle |
| [185](../source/Gui/Impl/MultiplierWidget.cpp#L185) | Min velocity X |
| [187](../source/Gui/Impl/MultiplierWidget.cpp#L187) | Max velocity X |
| [189](../source/Gui/Impl/MultiplierWidget.cpp#L189) | Min velocity Y |
| [191](../source/Gui/Impl/MultiplierWidget.cpp#L191) | Max velocity Y |
| [193](../source/Gui/Impl/MultiplierWidget.cpp#L193) | Min angular velocity |
| [195](../source/Gui/Impl/MultiplierWidget.cpp#L195) | Max angular velocity |
| [196](../source/Gui/Impl/MultiplierWidget.cpp#L196) | Overlapping check |
| [262](../source/Gui/Impl/MultiplierWidget.cpp#L262) | Random multiplication |


### source/Gui/Impl/MutationRatesDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [28](../source/Gui/Impl/MutationRatesDialog.cpp#L28) | Node probability |
| [39](../source/Gui/Impl/MutationRatesDialog.cpp#L39) | Value change sigma |
| [57](../source/Gui/Impl/MutationRatesDialog.cpp#L57) | Node probability |
| [68](../source/Gui/Impl/MutationRatesDialog.cpp#L68) | Weight change sigma |
| [79](../source/Gui/Impl/MutationRatesDialog.cpp#L79) | Bias change sigma |
| [90](../source/Gui/Impl/MutationRatesDialog.cpp#L90) | ActFn change probability |
| [108](../source/Gui/Impl/MutationRatesDialog.cpp#L108) | Node probability |
| [119](../source/Gui/Impl/MutationRatesDialog.cpp#L119) | Value change sigma |
| [130](../source/Gui/Impl/MutationRatesDialog.cpp#L130) | Enum change probability |
| [148](../source/Gui/Impl/MutationRatesDialog.cpp#L148) | Gene probability |
| [159](../source/Gui/Impl/MutationRatesDialog.cpp#L159) | Value change sigma |
| [170](../source/Gui/Impl/MutationRatesDialog.cpp#L170) | Enum change probability |
| [188](../source/Gui/Impl/MutationRatesDialog.cpp#L188) | Node probability |
| [206](../source/Gui/Impl/MutationRatesDialog.cpp#L206) | Node probability |
| [224](../source/Gui/Impl/MutationRatesDialog.cpp#L224) | Genome probability |
| [242](../source/Gui/Impl/MutationRatesDialog.cpp#L242) | Node probability |
| [266](../source/Gui/Impl/MutationRatesDialog.cpp#L266) | Gene probability |
| [290](../source/Gui/Impl/MutationRatesDialog.cpp#L290) | Node probability |
| [308](../source/Gui/Impl/MutationRatesDialog.cpp#L308) | Node probability |
| [319](../source/Gui/Impl/MutationRatesDialog.cpp#L319) | Value change sigma |
| [330](../source/Gui/Impl/MutationRatesDialog.cpp#L330) | Enum change probability |
| [341](../source/Gui/Impl/MutationRatesDialog.cpp#L341) | Constructor toggle probability |
| [536](../source/Gui/Impl/MutationRatesDialog.cpp#L536) | Connection weight mutations |
| [547](../source/Gui/Impl/MutationRatesDialog.cpp#L547) | Neuron mutations |
| [558](../source/Gui/Impl/MutationRatesDialog.cpp#L558) | Cell type property mutations |
| [569](../source/Gui/Impl/MutationRatesDialog.cpp#L569) | Geometry mutations |
| [580](../source/Gui/Impl/MutationRatesDialog.cpp#L580) | Cell type mode mutations |
| [589](../source/Gui/Impl/MutationRatesDialog.cpp#L589) | Cell type mutations |
| [598](../source/Gui/Impl/MutationRatesDialog.cpp#L598) | Customization mutations |
| [607](../source/Gui/Impl/MutationRatesDialog.cpp#L607) | Void mutations |
| [616](../source/Gui/Impl/MutationRatesDialog.cpp#L616) | Extend gene mutations |
| [626](../source/Gui/Impl/MutationRatesDialog.cpp#L626) | Add node mutations |
| [636](../source/Gui/Impl/MutationRatesDialog.cpp#L636) | Trim gene mutations |
| [646](../source/Gui/Impl/MutationRatesDialog.cpp#L646) | Delete node mutations |
| [656](../source/Gui/Impl/MutationRatesDialog.cpp#L656) | Add gene mutations |
| [666](../source/Gui/Impl/MutationRatesDialog.cpp#L666) | Duplicate gene mutations |
| [676](../source/Gui/Impl/MutationRatesDialog.cpp#L676) | Delete gene mutations |
| [686](../source/Gui/Impl/MutationRatesDialog.cpp#L686) | Swap gene mutations |
| [696](../source/Gui/Impl/MutationRatesDialog.cpp#L696) | Copy node section mutations |
| [706](../source/Gui/Impl/MutationRatesDialog.cpp#L706) | Move node section mutations |
| [716](../source/Gui/Impl/MutationRatesDialog.cpp#L716) | Constructor mutations |
| [739](../source/Gui/Impl/MutationRatesDialog.cpp#L739) | Adopt |
| [746](../source/Gui/Impl/MutationRatesDialog.cpp#L746) | Cancel |
| [755](../source/Gui/Impl/MutationRatesDialog.cpp#L755) | Copy |
| [757](../source/Gui/Impl/MutationRatesDialog.cpp#L757) | Mutation rates copied |
| [759](../source/Gui/Impl/MutationRatesDialog.cpp#L759) | Copy all mutation rates to the clipboard |
| [763](../source/Gui/Impl/MutationRatesDialog.cpp#L763) | Paste |
| [765](../source/Gui/Impl/MutationRatesDialog.cpp#L765) | Mutation rates pasted |
| [768](../source/Gui/Impl/MutationRatesDialog.cpp#L768) | Paste all mutation rates from the clipboard |


### source/Gui/Impl/MutationRatesWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [124](../source/Gui/Impl/MutationRatesWidget.cpp#L124) | Edit |
| [125](../source/Gui/Impl/MutationRatesWidget.cpp#L125) | Click to edit |
| [156](../source/Gui/Impl/MutationRatesWidget.cpp#L156) | Edit ( |
| [157](../source/Gui/Impl/MutationRatesWidget.cpp#L157) | Mutation rates |


### source/Gui/Impl/NetworkSettingsDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [18](../source/Gui/Impl/NetworkSettingsDialog.cpp#L18) | Network settings |
| [24](../source/Gui/Impl/NetworkSettingsDialog.cpp#L24) | Blocks |
| [24](../source/Gui/Impl/NetworkSettingsDialog.cpp#L24) | Server address |
| [26](../source/Gui/Impl/NetworkSettingsDialog.cpp#L26) | Count me as active |
| [31](../source/Gui/Impl/NetworkSettingsDialog.cpp#L31) | Adopt |
| [38](../source/Gui/Impl/NetworkSettingsDialog.cpp#L38) | Cancel |


### source/Gui/Impl/NetworkTransferController.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [31](../source/Gui/Impl/NetworkTransferController.cpp#L31) | Downloading ... |
| [43](../source/Gui/Impl/NetworkTransferController.cpp#L43) | Error |
| [59](../source/Gui/Impl/NetworkTransferController.cpp#L59) | Error |
| [64](../source/Gui/Impl/NetworkTransferController.cpp#L64) | Uploading ... |
| [75](../source/Gui/Impl/NetworkTransferController.cpp#L75) | Error |
| [80](../source/Gui/Impl/NetworkTransferController.cpp#L80) | Replacing ... |
| [92](../source/Gui/Impl/NetworkTransferController.cpp#L92) | Error |
| [97](../source/Gui/Impl/NetworkTransferController.cpp#L97) | Deleting ... |
| [108](../source/Gui/Impl/NetworkTransferController.cpp#L108) | Error |
| [113](../source/Gui/Impl/NetworkTransferController.cpp#L113) | Applying changes ... |
| [123](../source/Gui/Impl/NetworkTransferController.cpp#L123) | Error |
| [128](../source/Gui/Impl/NetworkTransferController.cpp#L128) | Changing visibility ... |
| [138](../source/Gui/Impl/NetworkTransferController.cpp#L138) | Error |


### source/Gui/Impl/NeuralNetEditorWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [508](../source/Gui/Impl/NeuralNetEditorWidget.cpp#L508) | Connection weights |
| [983](../source/Gui/Impl/NeuralNetEditorWidget.cpp#L983) | Weight |
| [994](../source/Gui/Impl/NeuralNetEditorWidget.cpp#L994) | Bias |
| [1048](../source/Gui/Impl/NeuralNetEditorWidget.cpp#L1048) | Adopt |
| [1054](../source/Gui/Impl/NeuralNetEditorWidget.cpp#L1054) | Cancel |
| [1071](../source/Gui/Impl/NeuralNetEditorWidget.cpp#L1071) | Clear |
| [1081](../source/Gui/Impl/NeuralNetEditorWidget.cpp#L1081) | Identity |
| [1091](../source/Gui/Impl/NeuralNetEditorWidget.cpp#L1091) | Randomize |
| [1105](../source/Gui/Impl/NeuralNetEditorWidget.cpp#L1105) | Copy |
| [1111](../source/Gui/Impl/NeuralNetEditorWidget.cpp#L1111) | Paste |


### source/Gui/Impl/NewPasswordDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [28](../source/Gui/Impl/NewPasswordDialog.cpp#L28) | New password |
| [33](../source/Gui/Impl/NewPasswordDialog.cpp#L33) | Security information |
| [40](../source/Gui/Impl/NewPasswordDialog.cpp#L40) | Please enter a new password and the confirmation code\nsent to your email address. |
| [42](../source/Gui/Impl/NewPasswordDialog.cpp#L42) | New password |
| [43](../source/Gui/Impl/NewPasswordDialog.cpp#L43) | Code (case sensitive) |
| [48](../source/Gui/Impl/NewPasswordDialog.cpp#L48) | OK |
| [56](../source/Gui/Impl/NewPasswordDialog.cpp#L56) | Cancel |
| [70](../source/Gui/Impl/NewPasswordDialog.cpp#L70) | Error |
| [73](../source/Gui/Impl/NewPasswordDialog.cpp#L73) | Information |


### source/Gui/Impl/NewSimulationDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [35](../source/Gui/Impl/NewSimulationDialog.cpp#L35) | New simulation |
| [44](../source/Gui/Impl/NewSimulationDialog.cpp#L44) | Project name |
| [51](../source/Gui/Impl/NewSimulationDialog.cpp#L51) | Width |
| [52](../source/Gui/Impl/NewSimulationDialog.cpp#L52) | Height |
| [54](../source/Gui/Impl/NewSimulationDialog.cpp#L54) | Energy |
| [57](../source/Gui/Impl/NewSimulationDialog.cpp#L57) | Adopt parameters |


### source/Gui/Impl/NodeEditorWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [209](../source/Gui/Impl/NodeEditorWidget.cpp#L209) | Selected: Node  |
| [227](../source/Gui/Impl/NodeEditorWidget.cpp#L227) | Base properties |
| [234](../source/Gui/Impl/NodeEditorWidget.cpp#L234) | Angle |
| [246](../source/Gui/Impl/NodeEditorWidget.cpp#L246) | Customization |
| [255](../source/Gui/Impl/NodeEditorWidget.cpp#L255) | Type |
| [261](../source/Gui/Impl/NodeEditorWidget.cpp#L261) | Error |
| [263](../source/Gui/Impl/NodeEditorWidget.cpp#L263) | Error |
| [288](../source/Gui/Impl/NodeEditorWidget.cpp#L288) | Construction |
| [303](../source/Gui/Impl/NodeEditorWidget.cpp#L303) | Construction properties |
| [313](../source/Gui/Impl/NodeEditorWidget.cpp#L313) | Auto trigger interval |
| [321](../source/Gui/Impl/NodeEditorWidget.cpp#L321) | Construction angle |
| [331](../source/Gui/Impl/NodeEditorWidget.cpp#L331) | Provide energy |
| [342](../source/Gui/Impl/NodeEditorWidget.cpp#L342) | Separation |
| [353](../source/Gui/Impl/NodeEditorWidget.cpp#L353) | Number of branches |
| [365](../source/Gui/Impl/NodeEditorWidget.cpp#L365) | Concatenations |
| [378](../source/Gui/Impl/NodeEditorWidget.cpp#L378) | Type-specific properties |
| [385](../source/Gui/Impl/NodeEditorWidget.cpp#L385) | Max energy for storage |
| [398](../source/Gui/Impl/NodeEditorWidget.cpp#L398) | Auto trigger |
| [404](../source/Gui/Impl/NodeEditorWidget.cpp#L404) | Tag for attackers |
| [413](../source/Gui/Impl/NodeEditorWidget.cpp#L413) | Mode |
| [429](../source/Gui/Impl/NodeEditorWidget.cpp#L429) | Min density |
| [441](../source/Gui/Impl/NodeEditorWidget.cpp#L441) | Min density |
| [450](../source/Gui/Impl/NodeEditorWidget.cpp#L450) | Restrict to colors |
| [460](../source/Gui/Impl/NodeEditorWidget.cpp#L460) | Min creature cells |
| [466](../source/Gui/Impl/NodeEditorWidget.cpp#L466) | Max creature cells |
| [473](../source/Gui/Impl/NodeEditorWidget.cpp#L473) | Restrict to colors |
| [479](../source/Gui/Impl/NodeEditorWidget.cpp#L479) | Restrict to lineage |
| [480](../source/Gui/Impl/NodeEditorWidget.cpp#L480) | No |
| [490](../source/Gui/Impl/NodeEditorWidget.cpp#L490) | Min range |
| [500](../source/Gui/Impl/NodeEditorWidget.cpp#L500) | Max range |
| [516](../source/Gui/Impl/NodeEditorWidget.cpp#L516) | Additive |
| [524](../source/Gui/Impl/NodeEditorWidget.cpp#L524) | Min value |
| [532](../source/Gui/Impl/NodeEditorWidget.cpp#L532) | Max value |
| [542](../source/Gui/Impl/NodeEditorWidget.cpp#L542) | Time offset |
| [551](../source/Gui/Impl/NodeEditorWidget.cpp#L551) | Mode |
| [565](../source/Gui/Impl/NodeEditorWidget.cpp#L565) | Period |
| [577](../source/Gui/Impl/NodeEditorWidget.cpp#L577) | Period |
| [591](../source/Gui/Impl/NodeEditorWidget.cpp#L591) | Mode |
| [606](../source/Gui/Impl/NodeEditorWidget.cpp#L606) | Restrict to colors |
| [619](../source/Gui/Impl/NodeEditorWidget.cpp#L619) | Gene index |
| [631](../source/Gui/Impl/NodeEditorWidget.cpp#L631) | Mode |
| [646](../source/Gui/Impl/NodeEditorWidget.cpp#L646) | Max angle deviation |
| [657](../source/Gui/Impl/NodeEditorWidget.cpp#L657) | Forward backward ratio |
| [674](../source/Gui/Impl/NodeEditorWidget.cpp#L674) | Max angle deviation |
| [685](../source/Gui/Impl/NodeEditorWidget.cpp#L685) | Forward backward ratio |
| [702](../source/Gui/Impl/NodeEditorWidget.cpp#L702) | Max angle deviation |
| [713](../source/Gui/Impl/NodeEditorWidget.cpp#L713) | Attraction repulsion ratio |
| [729](../source/Gui/Impl/NodeEditorWidget.cpp#L729) | Max distance deviation |
| [740](../source/Gui/Impl/NodeEditorWidget.cpp#L740) | Forward backward ratio |
| [757](../source/Gui/Impl/NodeEditorWidget.cpp#L757) | Max distance deviation |
| [768](../source/Gui/Impl/NodeEditorWidget.cpp#L768) | Forward backward ratio |
| [787](../source/Gui/Impl/NodeEditorWidget.cpp#L787) | Mode |
| [800](../source/Gui/Impl/NodeEditorWidget.cpp#L800) | Mode |
| [817](../source/Gui/Impl/NodeEditorWidget.cpp#L817) | Restrict to colors |
| [827](../source/Gui/Impl/NodeEditorWidget.cpp#L827) | Min creature cells |
| [833](../source/Gui/Impl/NodeEditorWidget.cpp#L833) | Max creature cells |
| [840](../source/Gui/Impl/NodeEditorWidget.cpp#L840) | Restrict to colors |
| [846](../source/Gui/Impl/NodeEditorWidget.cpp#L846) | Restrict to lineage |
| [847](../source/Gui/Impl/NodeEditorWidget.cpp#L847) | No |
| [860](../source/Gui/Impl/NodeEditorWidget.cpp#L860) | Countdown |
| [869](../source/Gui/Impl/NodeEditorWidget.cpp#L869) | Energy conductivity |
| [878](../source/Gui/Impl/NodeEditorWidget.cpp#L878) | Energy conversion |
| [892](../source/Gui/Impl/NodeEditorWidget.cpp#L892) | Mode |
| [908](../source/Gui/Impl/NodeEditorWidget.cpp#L908) | Delay |
| [916](../source/Gui/Impl/NodeEditorWidget.cpp#L916) | Read only |
| [921](../source/Gui/Impl/NodeEditorWidget.cpp#L921) | Edit |
| [922](../source/Gui/Impl/NodeEditorWidget.cpp#L922) | Signal buffer |
| [934](../source/Gui/Impl/NodeEditorWidget.cpp#L934) | Read only |
| [939](../source/Gui/Impl/NodeEditorWidget.cpp#L939) | Edit |
| [940](../source/Gui/Impl/NodeEditorWidget.cpp#L940) | Signal buffer |
| [952](../source/Gui/Impl/NodeEditorWidget.cpp#L952) | New signal weight |
| [967](../source/Gui/Impl/NodeEditorWidget.cpp#L967) | Channel mask bit 0-3 |
| [976](../source/Gui/Impl/NodeEditorWidget.cpp#L976) | Channel mask bit 4-7 |
| [997](../source/Gui/Impl/NodeEditorWidget.cpp#L997) | Mode |
| [1011](../source/Gui/Impl/NodeEditorWidget.cpp#L1011) | Range |
| [1019](../source/Gui/Impl/NodeEditorWidget.cpp#L1019) | One-way |
| [1030](../source/Gui/Impl/NodeEditorWidget.cpp#L1030) | Restrict to colors |
| [1036](../source/Gui/Impl/NodeEditorWidget.cpp#L1036) | Restrict to lineage |
| [1037](../source/Gui/Impl/NodeEditorWidget.cpp#L1037) | No |
| [1055](../source/Gui/Impl/NodeEditorWidget.cpp#L1055) | Nothing selected |
| [1072](../source/Gui/Impl/NodeEditorWidget.cpp#L1072) | Neural network |


### source/Gui/Impl/PreviewSettingsDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [10](../source/Gui/Impl/PreviewSettingsDialog.cpp#L10) | Preview settings |
| [24](../source/Gui/Impl/PreviewSettingsDialog.cpp#L24) | Display mode |
| [24](../source/Gui/Impl/PreviewSettingsDialog.cpp#L24) | Node index |
| [32](../source/Gui/Impl/PreviewSettingsDialog.cpp#L32) | Adopt |
| [40](../source/Gui/Impl/PreviewSettingsDialog.cpp#L40) | Cancel |


### source/Gui/Impl/PreviewWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [213](../source/Gui/Impl/PreviewWidget.cpp#L213) | Activates a more detailed simulation including signals and muscles |
| [221](../source/Gui/Impl/PreviewWidget.cpp#L221) | Shows the neural activity editor in the preview |
| [230](../source/Gui/Impl/PreviewWidget.cpp#L230) | Preview settings |
| [284](../source/Gui/Impl/PreviewWidget.cpp#L284) | TPS:  |


### source/Gui/Impl/ReplaceSimulationDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [21](../source/Gui/Impl/ReplaceSimulationDialog.cpp#L21) | Replace |
| [42](../source/Gui/Impl/ReplaceSimulationDialog.cpp#L42) | Do you really want to replace the following %s with the currently opened %s? |
| [52](../source/Gui/Impl/ReplaceSimulationDialog.cpp#L52) | Yes |
| [57](../source/Gui/Impl/ReplaceSimulationDialog.cpp#L57) | No |


### source/Gui/Impl/ResetPasswordDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [10](../source/Gui/Impl/ResetPasswordDialog.cpp#L10) | Reset password |
| [23](../source/Gui/Impl/ResetPasswordDialog.cpp#L23) | Security information |
| [26](../source/Gui/Impl/ResetPasswordDialog.cpp#L26) | Data privacy policy |
| [31](../source/Gui/Impl/ResetPasswordDialog.cpp#L31) | Please enter your email address to receive the\nconfirmation code to reset the password. |
| [33](../source/Gui/Impl/ResetPasswordDialog.cpp#L33) | Email |
| [38](../source/Gui/Impl/ResetPasswordDialog.cpp#L38) | OK |
| [47](../source/Gui/Impl/ResetPasswordDialog.cpp#L47) | Cancel |
| [58](../source/Gui/Impl/ResetPasswordDialog.cpp#L58) | Error |


### source/Gui/Impl/ResizeWorldDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [28](../source/Gui/Impl/ResizeWorldDialog.cpp#L28) | Resize world |
| [44](../source/Gui/Impl/ResizeWorldDialog.cpp#L44) | Width |
| [54](../source/Gui/Impl/ResizeWorldDialog.cpp#L54) | Height |
| [58](../source/Gui/Impl/ResizeWorldDialog.cpp#L58) | Scale content |
| [62](../source/Gui/Impl/ResizeWorldDialog.cpp#L62) | OK |
| [69](../source/Gui/Impl/ResizeWorldDialog.cpp#L69) | Cancel |


### source/Gui/Impl/SavePictureDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [29](../source/Gui/Impl/SavePictureDialog.cpp#L29) | Save picture |
| [59](../source/Gui/Impl/SavePictureDialog.cpp#L59) | Width |
| [60](../source/Gui/Impl/SavePictureDialog.cpp#L60) | Height |


### source/Gui/Impl/SelectionWidget.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [296](../source/Gui/Impl/SelectionWidget.cpp#L296) | Make sticky |
| [299](../source/Gui/Impl/SelectionWidget.cpp#L299) | Make unsticky |
| [310](../source/Gui/Impl/SelectionWidget.cpp#L310) | Make static |
| [313](../source/Gui/Impl/SelectionWidget.cpp#L313) | Make non-static |
| [335](../source/Gui/Impl/SelectionWidget.cpp#L335) | Objects (ALT+N) |
| [338](../source/Gui/Impl/SelectionWidget.cpp#L338) | Genomes (ALT+F) |
| [341](../source/Gui/Impl/SelectionWidget.cpp#L341) | Creatures (ALT+P) |
| [365](../source/Gui/Impl/SelectionWidget.cpp#L365) | Center position and velocity |
| [367](../source/Gui/Impl/SelectionWidget.cpp#L367) | Position X |
| [368](../source/Gui/Impl/SelectionWidget.cpp#L368) | Position Y |
| [374](../source/Gui/Impl/SelectionWidget.cpp#L374) | Velocity X |
| [375](../source/Gui/Impl/SelectionWidget.cpp#L375) | Velocity Y |
| [380](../source/Gui/Impl/SelectionWidget.cpp#L380) | Center rotation |
| [383](../source/Gui/Impl/SelectionWidget.cpp#L383) | Angle |
| [389](../source/Gui/Impl/SelectionWidget.cpp#L389) | Angular velocity |


### source/Gui/Impl/SignalsBufferDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [19](../source/Gui/Impl/SignalsBufferDialog.cpp#L19) | Signal buffer |
| [59](../source/Gui/Impl/SignalsBufferDialog.cpp#L59) | Number of signals |
| [70](../source/Gui/Impl/SignalsBufferDialog.cpp#L70) | Edit signal |
| [88](../source/Gui/Impl/SignalsBufferDialog.cpp#L88) | Adopt |
| [96](../source/Gui/Impl/SignalsBufferDialog.cpp#L96) | Cancel |


### source/Gui/Impl/SimulationInteractionController.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [171](../source/Gui/Impl/SimulationInteractionController.cpp#L171) | Leave edit mode (ALT+E) |


### source/Gui/Impl/SimulationParametersMainWindow.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [45](../source/Gui/Impl/SimulationParametersMainWindow.cpp#L45) | Simulation parameters |
| [204](../source/Gui/Impl/SimulationParametersMainWindow.cpp#L204) | Overview |
| [307](../source/Gui/Impl/SimulationParametersMainWindow.cpp#L307) | Expert settings |


### source/Gui/Impl/SpatialControlWindow.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [42](../source/Gui/Impl/SpatialControlWindow.cpp#L42) | Spatial control |
| [64](../source/Gui/Impl/SpatialControlWindow.cpp#L64) | World size |
| [72](../source/Gui/Impl/SpatialControlWindow.cpp#L72) | Zoom factor |
| [79](../source/Gui/Impl/SpatialControlWindow.cpp#L79) | Center position |
| [87](../source/Gui/Impl/SpatialControlWindow.cpp#L87) | Autotracking on selection |
| [91](../source/Gui/Impl/SpatialControlWindow.cpp#L91) | Zoom sensitivity |
| [107](../source/Gui/Impl/SpatialControlWindow.cpp#L107) | Zoom in |
| [110](../source/Gui/Impl/SpatialControlWindow.cpp#L110) | Zoom out |
| [113](../source/Gui/Impl/SpatialControlWindow.cpp#L113) | Center |
| [120](../source/Gui/Impl/SpatialControlWindow.cpp#L120) | Resize |


### source/Gui/Impl/TemporalControlWindow.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [28](../source/Gui/Impl/TemporalControlWindow.cpp#L28) | Temporal control |
| [49](../source/Gui/Impl/TemporalControlWindow.cpp#L49) | Time steps per second |
| [60](../source/Gui/Impl/TemporalControlWindow.cpp#L60) | Total time steps |
| [71](../source/Gui/Impl/TemporalControlWindow.cpp#L71) | Real-time |
| [86](../source/Gui/Impl/TemporalControlWindow.cpp#L86) | Slow down |
| [99](../source/Gui/Impl/TemporalControlWindow.cpp#L99) | Sync with rendering |
| [119](../source/Gui/Impl/TemporalControlWindow.cpp#L119) | Run |
| [122](../source/Gui/Impl/TemporalControlWindow.cpp#L122) | Run |
| [124](../source/Gui/Impl/TemporalControlWindow.cpp#L124) | Pause |
| [126](../source/Gui/Impl/TemporalControlWindow.cpp#L126) | Pause |
| [131](../source/Gui/Impl/TemporalControlWindow.cpp#L131) | Load previous time step |
| [135](../source/Gui/Impl/TemporalControlWindow.cpp#L135) | Loading previous time step ... |
| [138](../source/Gui/Impl/TemporalControlWindow.cpp#L138) | Process single time step |
| [144](../source/Gui/Impl/TemporalControlWindow.cpp#L144) | Create flashback |
| [145](../source/Gui/Impl/TemporalControlWindow.cpp#L145) | Creating in-memory flashback: It saves the content of the current world to the memory. |
| [149](../source/Gui/Impl/TemporalControlWindow.cpp#L149) | Creating flashback ... |
| [154](../source/Gui/Impl/TemporalControlWindow.cpp#L154) | Load flashback |
| [155](../source/Gui/Impl/TemporalControlWindow.cpp#L155) | Loading in-memory flashback: It loads the saved world from the memory. Static simulation parameters will not be changed.  |
| [161](../source/Gui/Impl/TemporalControlWindow.cpp#L161) | Loading flashback ... |


### source/Gui/Impl/UploadSimulationDialog.cpp

| Line | Literal excerpt / review candidate |
| --- | --- |
| [76](../source/Gui/Impl/UploadSimulationDialog.cpp#L76) | Data privacy policy |
| [80](../source/Gui/Impl/UploadSimulationDialog.cpp#L80) | How to use or create folders? |
| [91](../source/Gui/Impl/UploadSimulationDialog.cpp#L91) | The following folder has been selected in the browser\nand will used for the upload:\n\n |
| [108](../source/Gui/Impl/UploadSimulationDialog.cpp#L108) | Desc (optional) |
| [116](../source/Gui/Impl/UploadSimulationDialog.cpp#L116) | Share with the community |
| [118](../source/Gui/Impl/UploadSimulationDialog.cpp#L118) | If true, the  |
| [125](../source/Gui/Impl/UploadSimulationDialog.cpp#L125) | OK |
| [131](../source/Gui/Impl/UploadSimulationDialog.cpp#L131) | Error |
| [140](../source/Gui/Impl/UploadSimulationDialog.cpp#L140) | Cancel |

## Appendix: indirect description pool index

Each row is an untranslated English description at the baseline. The enum or constant name is a technical lookup identity, not a translation target. Excerpts identify the entry; extract the complete concatenated description, including later literals, during implementation.


### EntityAttributeHelp.cpp

| Stable entry identity | English description opening |
| --- | --- |
| [ObjectId](../source/Gui/Impl/EntityAttributeHelp.cpp#L19) | Unique 64 bit id of the object. It is assigned by the simulation and cannot be changed. |
| [Position](../source/Gui/Impl/EntityAttributeHelp.cpp#L20) | Position of the object in world coordinates. |
| [Velocity](../source/Gui/Impl/EntityAttributeHelp.cpp#L21) | Velocity of the object in world units per time step. |
| [Stiffness](../source/Gui/Impl/EntityAttributeHelp.cpp#L22) | Resistance of the object against deformation of its connections. |
| [Color](../source/Gui/Impl/EntityAttributeHelp.cpp#L23) | Customization color of the object. Many simulation parameters can be defined per color. |
| [Static](../source/Gui/Impl/EntityAttributeHelp.cpp#L24) | A static object is immortal and not affected by forces. It does not age and keeps its velocity, so it sta... |
| [Sticky](../source/Gui/Impl/EntityAttributeHelp.cpp#L27) | If enabled, the object can spontaneously form new connections on contact. It is sufficient if one of the ... |
| [ObjectType](../source/Gui/Impl/EntityAttributeHelp.cpp#L29) | Type of the object. |
| [ConnectedId](../source/Gui/Impl/EntityAttributeHelp.cpp#L35) | Id of the object that is attached by this connection. |
| [ConnectionDistance](../source/Gui/Impl/EntityAttributeHelp.cpp#L36) | Reference distance of the connection. A deviation of the actual distance generates restoring forces. |
| [ConnectionRefAngle](../source/Gui/Impl/EntityAttributeHelp.cpp#L37) | Reference angle between this connection and the previous one. A deviation generates restoring forces. |
| [ParticleId](../source/Gui/Impl/EntityAttributeHelp.cpp#L40) | Unique 64 bit id of the energy particle. |
| [ParticleEnergy](../source/Gui/Impl/EntityAttributeHelp.cpp#L41) | Energy of the particle. Cells can absorb it. |
| [SolidEnergy](../source/Gui/Impl/EntityAttributeHelp.cpp#L44) | Energy stored in the solid particle. |
| [FluidEnergy](../source/Gui/Impl/EntityAttributeHelp.cpp#L45) | Energy stored in the fluid particle. |
| [FluidGlow](../source/Gui/Impl/EntityAttributeHelp.cpp#L46) | Additional brightness of the fluid particle during rendering. |
| [FreeCellEnergy](../source/Gui/Impl/EntityAttributeHelp.cpp#L47) | Energy of the free cell. Below the minimum cell energy the cell decays. |
| [FreeCellAge](../source/Gui/Impl/EntityAttributeHelp.cpp#L49) | Age of the free cell in time steps. |
| [CellUsableEnergy](../source/Gui/Impl/EntityAttributeHelp.cpp#L52) | Energy that keeps the cell alive and can be used for constructing new cells. Below the minimum cell energ... |
| [CellRawEnergy](../source/Gui/Impl/EntityAttributeHelp.cpp#L54) | Unprocessed energy. Cells gain raw energy by absorbing energy particles and by attacks. Digestor cells co... |
| [CellFrontAngle](../source/Gui/Impl/EntityAttributeHelp.cpp#L56) | Angle between the first connection and the front direction of the creature. It orients muscle, sensor and... |
| [CellAge](../source/Gui/Impl/EntityAttributeHelp.cpp#L58) | Age of the cell in time steps. |
| [CellState](../source/Gui/Impl/EntityAttributeHelp.cpp#L59) | State of the cell in its life cycle. |
| [CellNodeIndex](../source/Gui/Impl/EntityAttributeHelp.cpp#L66) | Index of the genome node from which this cell was built. |
| [CellGeneIndex](../source/Gui/Impl/EntityAttributeHelp.cpp#L67) | Index of the gene from which this cell was built. |
| [CellConcatenationIndex](../source/Gui/Impl/EntityAttributeHelp.cpp#L68) | Index of the concatenation in which this cell was built. |
| [CellBranchIndex](../source/Gui/Impl/EntityAttributeHelp.cpp#L69) | Index of the branch in which this cell was built. |
| [CellConstructionId](../source/Gui/Impl/EntityAttributeHelp.cpp#L70) | Identifies the branch construction in which this cell was built. Cells of the same creature with the same... |
| [CellActivationTime](../source/Gui/Impl/EntityAttributeHelp.cpp#L72) | Remaining time steps until the cell starts executing its function. As long as the value is greater than 0... |
| [CellHeadCell](../source/Gui/Impl/EntityAttributeHelp.cpp#L74) | Marks the head cell of the creature. Cells that lose connection to head cells die off. |
| [CellType](../source/Gui/Impl/EntityAttributeHelp.cpp#L76) | Function that the cell performs after its neural network has been executed. |
| [CellHasConstructor](../source/Gui/Impl/EntityAttributeHelp.cpp#L94) | If enabled, the cell can build other cells specified by a gene. |
| [GenomeName](../source/Gui/Impl/EntityAttributeHelp.cpp#L97) | Name of the genome. Every creature that is built from it inherits the name. |
| [GenomeFrontAngle](../source/Gui/Impl/EntityAttributeHelp.cpp#L98) | Angle between the first connection of the head cell and the front direction of the creature. From the hea... |
| [GenomeResistanceToInjection](../source/Gui/Impl/EntityAttributeHelp.cpp#L101) | If enabled, injector cells cannot overwrite the genome of creatures built from it. |
| [GenomeApplyMetaMutations](../source/Gui/Impl/EntityAttributeHelp.cpp#L102) | If enabled, the mutation rates of this genome are themselves mutated. The step sizes come from the 'Meta-... |
| [GenomeMutationRatesEdit](../source/Gui/Impl/EntityAttributeHelp.cpp#L105) | Opens the editor for all mutation rates. The rows below only list the rates that are currently active. |
| [GenomeMutationRatesSummary](../source/Gui/Impl/EntityAttributeHelp.cpp#L106) | Opens the editor for all mutation rates. The number counts the mutation types with a probability above ze... |
| [GeneName](../source/Gui/Impl/EntityAttributeHelp.cpp#L110) | Name of the gene. It is only used for the display in the structure list. |
| [GeneShape](../source/Gui/Impl/EntityAttributeHelp.cpp#L111) | Determines the angles and the additional connections with which the nodes of this gene are assembled. The... |
| [GeneConnectionDistance](../source/Gui/Impl/EntityAttributeHelp.cpp#L114) | Reference distance between two cells that are constructed one after another from this gene. |
| [GeneStiffness](../source/Gui/Impl/EntityAttributeHelp.cpp#L115) | Stiffness of all cells constructed from this gene. It determines how strongly they resist a deformation o... |
| [GeneHomogeneousCellType](../source/Gui/Impl/EntityAttributeHelp.cpp#L117) | If enabled, every constructed cell of this gene uses the cell type and its properties of the first node. |
| [MutationConnectionProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L120) | Probability per node and connection that the weight of that connection in the neural network is changed. |
| [MutationConnectionValueChangeSigma](../source/Gui/Impl/EntityAttributeHelp.cpp#L122) | Standard deviation of the Gaussian change of the connection weight. The result is limited to [-2, 2]. |
| [MutationNeuronProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L124) | Probability per node and neuron that the neuron is changed. |
| [MutationNeuronWeightChangeSigma](../source/Gui/Impl/EntityAttributeHelp.cpp#L125) | Standard deviation of the Gaussian change of every input weight of the neuron. The result is limited to [... |
| [MutationNeuronBiasChangeSigma](../source/Gui/Impl/EntityAttributeHelp.cpp#L127) | Standard deviation of the Gaussian change of the bias of the neuron. The result is limited to [-2, 2]. |
| [MutationNeuronActfnChangeProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L129) | Probability that the activation function of the neuron is replaced by a randomly chosen one, which can al... |
| [MutationCellTypePropertiesProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L131) | Probability per node that the properties of its cell type are changed. |
| [MutationCellTypePropertiesValueChangeSigma](../source/Gui/Impl/EntityAttributeHelp.cpp#L132) | Standard deviation of the Gaussian change of the numeric properties, relative to the value range of each ... |
| [MutationCellTypePropertiesEnumChangeProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L134) | Probability that a switch or a selection of the cell type properties is changed, such as a color or linea... |
| [MutationGeometryProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L137) | Probability per gene that its geometry is changed. |
| [MutationGeometryValueChangeSigma](../source/Gui/Impl/EntityAttributeHelp.cpp#L138) | Standard deviation of the Gaussian change of the stiffness, relative to its value range. The connection d... |
| [MutationGeometryEnumChangeProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L140) | Probability that the shape generator of the gene is replaced by a different one. |
| [MutationCellTypeModeProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L141) | Probability per node that the mode of its cell type is switched to a different one. The mode-specific pro... |
| [MutationCellTypeProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L144) | Probability per node that its cell type is replaced by a different one, with all properties reset to thei... |
| [MutationCustomizationProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L147) | Probability per genome that one used customization color is replaced everywhere by another one. Which rep... |
| [MutationVoidProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L150) | Probability per node that it is toggled between void and a random other cell type. The first and the last... |
| [MutationExtendGeneProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L152) | Probability per gene that a new node is appended at its beginning or at its end. |
| [MutationAddNodeProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L153) | Probability per insertion position that a new node is inserted there. |
| [MutationTrimGeneProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L154) | Probability per gene that its first or its last node is removed. The last remaining node of a gene is alw... |
| [MutationDeleteNodeProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L156) | Probability per node that it is removed. The last remaining node of a gene is always kept. |
| [MutationAddGeneProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L157) | Probability per gene that a new gene consisting of a single node is created. A randomly chosen node of th... |
| [MutationDuplicateGeneProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L160) | Probability per gene that it is duplicated. Only genes that are referenced at least twice are duplicated;... |
| [MutationDeleteGeneProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L163) | Probability per gene that it is deleted. At least one gene is kept. A reference to a deleted gene switche... |
| [MutationSwapGeneProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L166) | Probability per gene that it is swapped with another randomly chosen gene. The gene indices stay the same... |
| [MutationCopyNodeSectionProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L169) | Probability per gene that a contiguous section of its nodes is copied into a randomly chosen gene, which ... |
| [MutationMoveNodeSectionProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L171) | Probability per gene that a contiguous section of its nodes is moved into a randomly chosen gene. At leas... |
| [MutationConstructorProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L173) | Probability per node that the constructor properties of the node are changed. |
| [MutationConstructorValueChangeSigma](../source/Gui/Impl/EntityAttributeHelp.cpp#L174) | Standard deviation of the Gaussian change of the numeric constructor properties, relative to the value ra... |
| [MutationConstructorEnumChangeProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L176) | Probability that a switch or a selection of the constructor is changed. |
| [MutationConstructorToggleProbability](../source/Gui/Impl/EntityAttributeHelp.cpp#L177) | Probability that the constructor of the node is switched on or off. |
| [NodeAngle](../source/Gui/Impl/EntityAttributeHelp.cpp#L180) | Angle of this cell relative to the connection to the previous cell. It is only evaluated for the first an... |
| [NodeCustomization](../source/Gui/Impl/EntityAttributeHelp.cpp#L183) | Customization color of the cell. Many simulation parameters are defined per color. |
| [NodeConstructionGene](../source/Gui/Impl/EntityAttributeHelp.cpp#L184) | Gene that this cell builds as a constructor. 'None' means that the cell has no constructor. |
| [ConstructorAutoTriggerInterval](../source/Gui/Impl/EntityAttributeHelp.cpp#L187) | If set, the constructor triggers itself every n time steps, with a phase that differs per creature. Witho... |
| [ConstructorConstructionAngle](../source/Gui/Impl/EntityAttributeHelp.cpp#L190) | Angle by which the first constructed cell deviates from the middle of the largest gap between the connect... |
| [ConstructorProvideEnergy](../source/Gui/Impl/EntityAttributeHelp.cpp#L193) | Determines how much energy the constructor cell raises for a construction. |
| [ConstructorReservedEnergy](../source/Gui/Impl/EntityAttributeHelp.cpp#L202) | Energy reserve of the constructor cell. It is used up for the construction before the usable energy of th... |
| [ConstructorSeparation](../source/Gui/Impl/EntityAttributeHelp.cpp#L205) | If enabled, the offspring detaches from the constructor cell after the last cell has been built. |
| [ConstructorNumBranches](../source/Gui/Impl/EntityAttributeHelp.cpp#L206) | Number of cell networks that the constructor builds next to each other from the same gene. |
| [ConstructorNumConcatenations](../source/Gui/Impl/EntityAttributeHelp.cpp#L207) | How often the gene is built in a row, each attached to the previous one. |
| [ConstructorGeneIndex](../source/Gui/Impl/EntityAttributeHelp.cpp#L208) | Index of the gene that the constructor builds. |
| [DepotStorageLimit](../source/Gui/Impl/EntityAttributeHelp.cpp#L211) | Maximum usable energy that the depot can store. |
| [DepotStoredEnergy](../source/Gui/Impl/EntityAttributeHelp.cpp#L212) | Usable energy that is currently stored in the depot. |
| [SensorAutoTrigger](../source/Gui/Impl/EntityAttributeHelp.cpp#L215) | If enabled, the sensor scans in every cycle. Otherwise it only scans when channel #0 exceeds the trigger ... |
| [SensorTagForAttackers](../source/Gui/Impl/EntityAttributeHelp.cpp#L217) | If enabled, attacker cells of the same creature in 'Creature' mode may attack the creatures this sensor d... |
| [SensorMode](../source/Gui/Impl/EntityAttributeHelp.cpp#L220) | Selects what the sensor scans for. |
| [SensorEnergyMinDensity](../source/Gui/Impl/EntityAttributeHelp.cpp#L226) | Minimum density of energy particles that is required for a match. The value lies between 0 and 1. |
| [SensorFreeCellMinDensity](../source/Gui/Impl/EntityAttributeHelp.cpp#L227) | Minimum density of free cells that is required for a match. The value lies between 0 and 1. |
| [SensorMinRange](../source/Gui/Impl/EntityAttributeHelp.cpp#L228) | Objects that are closer than this distance are not detected. |
| [SensorMaxRange](../source/Gui/Impl/EntityAttributeHelp.cpp#L229) | Objects that are farther away than this distance are not detected. The simulation parameter 'Radius' of t... |
| [SensorRestrictToColors](../source/Gui/Impl/EntityAttributeHelp.cpp#L232) | Only objects with one of the selected customization colors are detected. |
| [SensorRestrictToLineage](../source/Gui/Impl/EntityAttributeHelp.cpp#L233) | Restricts the detection to the relationship with the own creature. |
| [SensorMinNumCells](../source/Gui/Impl/EntityAttributeHelp.cpp#L238) | Only creatures with at least this number of cells are detected. Without a value there is no lower limit. |
| [SensorMaxNumCells](../source/Gui/Impl/EntityAttributeHelp.cpp#L239) | Only creatures with at most this number of cells are detected. Without a value there is no upper limit. |
| [GeneratorAdditive](../source/Gui/Impl/EntityAttributeHelp.cpp#L242) | If enabled, the generated value is added to channel #0 instead of overwriting it. |
| [GeneratorMinValue](../source/Gui/Impl/EntityAttributeHelp.cpp#L243) | Lower level of the generated signal. |
| [GeneratorMaxValue](../source/Gui/Impl/EntityAttributeHelp.cpp#L244) | Upper level of the generated signal. |
| [GeneratorTimeOffset](../source/Gui/Impl/EntityAttributeHelp.cpp#L245) | Shifts the phase of the signal by this number of time steps. |
| [GeneratorMode](../source/Gui/Impl/EntityAttributeHelp.cpp#L246) | Shape of the generated signal. |
| [GeneratorPeriod](../source/Gui/Impl/EntityAttributeHelp.cpp#L250) | Length of one period in time steps. |
| [AttackerMode](../source/Gui/Impl/EntityAttributeHelp.cpp#L253) | Selects the targets of the attack. The stolen energy is added to the own raw energy. |
| [AttackerRestrictToColors](../source/Gui/Impl/EntityAttributeHelp.cpp#L258) | Only free cells with one of the selected customization colors are attacked. |
| [InjectorGeneIndex](../source/Gui/Impl/EntityAttributeHelp.cpp#L261) | Gene index that is set in the constructor of the injected cell. It determines which gene is built there. |
| [MuscleMode](../source/Gui/Impl/EntityAttributeHelp.cpp#L264) | Selects how the muscle generates movement. Bending and crawling act on the first connection. |
| [MuscleMaxAngleDeviation](../source/Gui/Impl/EntityAttributeHelp.cpp#L272) | Maximum deviation from the initial angle, as a fraction of the available angular range. |
| [MuscleForwardBackwardRatio](../source/Gui/Impl/EntityAttributeHelp.cpp#L273) | Distributes the speed between the forward and the backward stroke. Since the faster stroke generates more... |
| [MuscleAttractionRepulsionRatio](../source/Gui/Impl/EntityAttributeHelp.cpp#L276) | Distributes the speed between the attracting and the repelling bending direction. |
| [MuscleMaxDistanceDeviation](../source/Gui/Impl/EntityAttributeHelp.cpp#L277) | Maximum change of the connection length, relative to its initial value. |
| [DefenderMode](../source/Gui/Impl/EntityAttributeHelp.cpp#L280) | Selects against which foreign cells the creature is protected. Only the affected cell itself and its dire... |
| [ReconnectorMode](../source/Gui/Impl/EntityAttributeHelp.cpp#L286) | Selects the objects with which connections are established. A positive value in channel #0 creates a conn... |
| [ReconnectorRestrictToColors](../source/Gui/Impl/EntityAttributeHelp.cpp#L292) | Connections are only established with objects of the selected customization colors. |
| [ReconnectorRestrictToLineage](../source/Gui/Impl/EntityAttributeHelp.cpp#L293) | Restricts the connections to the relationship with the own creature. |
| [ReconnectorMinNumCells](../source/Gui/Impl/EntityAttributeHelp.cpp#L298) | Connections are only established with creatures that have at least this number of cells. Without a value ... |
| [ReconnectorMaxNumCells](../source/Gui/Impl/EntityAttributeHelp.cpp#L300) | Connections are only established with creatures that have at most this number of cells. Without a value t... |
| [DetonatorState](../source/Gui/Impl/EntityAttributeHelp.cpp#L304) | State of the detonator. |
| [DetonatorCountdown](../source/Gui/Impl/EntityAttributeHelp.cpp#L309) | Number of cycles until the explosion after the trigger. One cycle comprises 6 time steps. |
| [DigestorEnergyConductivity](../source/Gui/Impl/EntityAttributeHelp.cpp#L312) | Determines how much raw energy the cell can take in from connected cells and pass on to connected digesto... |
| [DigestorEnergyConversion](../source/Gui/Impl/EntityAttributeHelp.cpp#L315) | Determines how much raw energy the cell converts into usable energy per time step. It is scaled by the si... |
| [MemoryMode](../source/Gui/Impl/EntityAttributeHelp.cpp#L320) | Selects how the signal is stored. |
| [MemoryDelay](../source/Gui/Impl/EntityAttributeHelp.cpp#L326) | Number of cycles by which the signal is delayed. It also determines the size of the ring buffer. |
| [MemoryReadOnly](../source/Gui/Impl/EntityAttributeHelp.cpp#L327) | If enabled, the cell only reads from the signal buffer and never overwrites it. |
| [MemoryNewSignalWeight](../source/Gui/Impl/EntityAttributeHelp.cpp#L328) | Weight of the current signal in the running average. At 0 the stored value is frozen, at 1 there is no sm... |
| [MemoryChannelMask](../source/Gui/Impl/EntityAttributeHelp.cpp#L330) | Selects the channels that are overwritten with the stored values. The remaining channels are passed throu... |
| [MemorySignalBuffer](../source/Gui/Impl/EntityAttributeHelp.cpp#L332) | Opens the editor for the stored signal entries. |
| [CommunicatorMode](../source/Gui/Impl/EntityAttributeHelp.cpp#L335) | Selects the role in the communication. It only takes place between different creatures and requires a val... |
| [CommunicatorRange](../source/Gui/Impl/EntityAttributeHelp.cpp#L339) | Radius within which receiver cells are addressed. |
| [CommunicatorOneway](../source/Gui/Impl/EntityAttributeHelp.cpp#L340) | If enabled, only the receivers on one side are addressed, namely those opposite to the direction that is ... |
| [CommunicatorRestrictToColors](../source/Gui/Impl/EntityAttributeHelp.cpp#L342) | Only signals from senders with one of the selected customization colors are accepted. |
| [CommunicatorRestrictToLineage](../source/Gui/Impl/EntityAttributeHelp.cpp#L343) | Restricts the reception to the relationship with the own creature. |
| [NeuralSignal](../source/Gui/Impl/EntityAttributeHelp.cpp#L350) | Value of the signal channel. It is the output of the neural network and the input for the connected cells... |
| [NeuralMemory](../source/Gui/Impl/EntityAttributeHelp.cpp#L351) | Value of the memory neuron. It is retained across cycles and is only recalculated when the gate input exc... |
| [CreatureId](../source/Gui/Impl/EntityAttributeHelp.cpp#L355) | Unique 64 bit id of the creature. |
| [CreatureGeneration](../source/Gui/Impl/EntityAttributeHelp.cpp#L356) | Number of times the genome has been passed on to an offspring. |
| [CreatureNumCells](../source/Gui/Impl/EntityAttributeHelp.cpp#L357) | Number of cells that currently belong to the creature. |
| [CreatureLineageId](../source/Gui/Impl/EntityAttributeHelp.cpp#L358) | Id of the lineage. Creatures with the same id count as related. A new id is assigned as soon as the accum... |
| [CreatureMutationsInLineage](../source/Gui/Impl/EntityAttributeHelp.cpp#L361) | Accumulated mutations since the current lineage was formed. |
| [CreatureMutationsTotal](../source/Gui/Impl/EntityAttributeHelp.cpp#L362) | Accumulated mutations over the entire ancestry. This value is never reset. |
| [CreatureGenomeName](../source/Gui/Impl/EntityAttributeHelp.cpp#L363) | Name of the genome from which the creature is built. |
| [CreatureResistanceToInjection](../source/Gui/Impl/EntityAttributeHelp.cpp#L364) | If active, injector cells cannot overwrite the genome of this creature. |
| [CreatureApplyMetaMutations](../source/Gui/Impl/EntityAttributeHelp.cpp#L365) | If active, the mutation rates stored in the genome are themselves subject to mutation. |
| [CreatureEditGenome](../source/Gui/Impl/EntityAttributeHelp.cpp#L366) | Opens the genome of the creature in the genome editor. |


### HelpStrings.h

| Stable entry identity | English description opening |
| --- | --- |
| [NotAllowedCharacters](../source/Gui/Impl/HelpStrings.h#L9) | Your input contains not allowed characters. |
| [CreatorEnergyTooltip](../source/Gui/Impl/HelpStrings.h#L11) | Energy of each created object or energy particle. |
| [CreatorPencilRadiusTooltip](../source/Gui/Impl/HelpStrings.h#L13) | The radius of the pencil in number of objects. |
| [CreatorMaterialTooltip](../source/Gui/Impl/HelpStrings.h#L15) | Material of the objects to be created.\n |
| [CreatorRectangleWidthTooltip](../source/Gui/Impl/HelpStrings.h#L21) | The number of objects in the horizontal direction. |
| [CreatorRectangleHeightTooltip](../source/Gui/Impl/HelpStrings.h#L23) | The number of objects in the vertical direction. |
| [CreatorHexagonLayersTooltip](../source/Gui/Impl/HelpStrings.h#L25) | The number of object layers, counted from the center. |
| [CreatorDiscOuterRadiusTooltip](../source/Gui/Impl/HelpStrings.h#L27) | The outer radius of the disc. |
| [CreatorDiscInnerRadiusTooltip](../source/Gui/Impl/HelpStrings.h#L29) | The inner radius of the disc. Objects are only created between the inner and the outer radius. |
| [CreatorDistanceTooltip](../source/Gui/Impl/HelpStrings.h#L31) | The distance between two neighboring objects. |
| [LoginHowToCreateNewUseTooltip](../source/Gui/Impl/HelpStrings.h#L33) | Please enter the desired user name and password and proceed by clicking the 'Create user' button. |
| [LoginForgotYourPasswordTooltip](../source/Gui/Impl/HelpStrings.h#L35) | Please enter the user name and proceed by clicking the 'Reset password' button. |
| [LoginSecurityInformationTooltip](../source/Gui/Impl/HelpStrings.h#L37) | The data transfer to the server is encrypted via https. On the server side, the password is not stored in... |
| [LoginRememberTooltip](../source/Gui/Impl/HelpStrings.h#L43) | If the toggle 'Remember' is activated, the password will be stored in the Windows registry under the path... |
| [LoginShareGpuInfoTooltip1](../source/Gui/Impl/HelpStrings.h#L47) | If this option is enabled, other users will be able to see in the browser window that you have the follow... |
| [LoginShareGpuInfoTooltip2](../source/Gui/Impl/HelpStrings.h#L49) | As a result, you will be able to see the GPU information of other registered users who have shared it. |
| [BrowserActiveSimulatorsTooltip](../source/Gui/Impl/HelpStrings.h#L51) | Number of ALIEN instances that fetched the browser list in the last 25 minutes, including those without a... |
| [NetworkCountAsActiveTooltip](../source/Gui/Impl/HelpStrings.h#L57) | If this option is enabled, this instance is included in the number of active simulators shown in the brow... |
| [BrowserFeaturedWorkspaceTooltip](../source/Gui/Impl/HelpStrings.h#L61) | This workspace is curated by the alien-project and contains the simulations that come along with the rele... |
| [BrowserCommunityWorkspaceTooltip](../source/Gui/Impl/HelpStrings.h#L65) | All logged-in users can share their simulations and genomes here. The files stored in this workspace are ... |
| [BrowserPrivateWorkspaceTooltip](../source/Gui/Impl/HelpStrings.h#L68) | Each user account has its own private space. The simulations and genomes are only visible to the logged-i... |
| [BrowserLoginChipTooltip](../source/Gui/Impl/HelpStrings.h#L71) | Log in or create a new account to upload your own simulations and genomes and to react to those of others... |
| [BrowserGpuHiddenLoggedOutTooltip](../source/Gui/Impl/HelpStrings.h#L73) | The GPU models are only visible to logged-in users who share their own GPU model. |
| [BrowserGpuHiddenNotSharedTooltip](../source/Gui/Impl/HelpStrings.h#L74) | The GPU models are only visible if you share your own GPU model. |
| [BrowserShareGpuTooltip](../source/Gui/Impl/HelpStrings.h#L75) | Other users will be able to see in the browser window that you have the following graphics card:  |
