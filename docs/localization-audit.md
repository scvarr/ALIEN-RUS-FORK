# ALIEN: Russian localization audit

Audit date: 2026-10-19 (Europe/Moscow). Research only; no localization implementation.

## 1. Repository baseline and scope

| Item | Recorded state |
| --- | --- |
| Workspace | `E:\Projects\ALIEN`, initially empty, including hidden entries |
| Official source | <https://github.com/chrxh/alien>, branch `develop` |
| Audited source SHA | `d1365091ae51b091ab205407cb4142e61f5644fc` |
| Source commit | Merge PR #884, “Add MCP tools to edit and summarize genomes in bulk”, committed 2026-10-18 22:07:30 +0200 |
| Application version | `5.0.0-alpha.40`, `source/Base/Resources.cpp` |
| Local baseline branch | `develop`, pointing at the source SHA |
| Working branch | `research/ru-localization-audit` |
| Main repository remotes | `git remote -v`: empty |
| Local push protection | `push.default=nothing` |
| Submodule | `external/vcpkg`, clean at `af752f21c9d79ba3df9cb0250ce2233933f58486` |

Git was initialized in the existing directory, then populated using `git fetch https://github.com/chrxh/alien.git develop` and `git switch -c develop FETCH_HEAD`. Recursive submodule initialization completed. The requested working branch and push protection were created locally. A subsequent read-only `git ls-remote` confirmed the same official `develop` SHA during the audit. The submodule's own remote is retained; it is not a main-repository remote. No fork, push, merge, PR, dependency update, build, or application execution was performed.

The explicit user instruction to initialize submodules took precedence over the newly fetched `AGENTS.md` prohibition on `git submodule update`. No further submodule updates are needed. The report and commit message use English as required by `AGENTS.md`.

This is an independent local Git history with the original ancestry retained, not a GitHub fork. Future updates can be fetched by URL into `FETCH_HEAD` without adding a remote; inspect the new SHA and deliberately integrate it in a later task. `push.default=nothing` prevents an implicit default push, but does not prohibit an explicitly addressed push. No write operation to GitHub was used here.

## 2. Architecture and localization boundary

The source has 874 tracked files; 838 are C++/CUDA headers or implementations, of which 742 remain after excluding directories named `Tests` and `TestData`. These are source inventory counts, not translation counts.

The GUI application composes facade implementations in `source/Apps/Alien/Main.cpp`. `Gui/Impl/MainWindow.cpp` initializes GLFW, Dear ImGui, fonts, rendering, and controllers. `Gui/Impl` holds menus, dialogs, custom widgets, genome and neural network editors, inspection, documentation, browser/cloud interaction, statistics, and notifications. `AlienGui` is a large widget wrapper, and `AlienWindow` / `AlienDialog` / `ModalWindow` manage window identity and lifetime.

Important text is outside GUI code:

- `Data/SimulationParameters.cpp` defines parameter groups, names, descriptions, and alternatives consumed by `Gui/Impl/SpecificationGuiService.cpp`.
- `Data/CellTypeConstants.h` supplies shared English display arrays for cell/object types, activation functions, shapes, and operation modes.
- `Base/DocumentationReader.cpp` and `MarkdownParser.cpp` feed both GUI documentation and `McpTools/Impl/McpDocumentationTools.cpp`.
- `Persister/Impl/PersisterWorker.cpp` produces errors that are passed through `PersisterErrorInfo.message` into `GenericMessageDialog`.
- `Apps/Alien-cli`, `ConsoleUi`, and `Apps/Alien/HelpStrings.h` contain CLI help, console panels, and pre-GUI failure guidance.
- `Rendering/Impl/RenderSteps.cpp` draws cell-type text from a separately generated texture atlas.

Localization must remain a presentation concern. Do not translate serialized member names, simulation parameters' storage keys, enum values, MCP method/schema/property names, HTTP fields, CLI switches, shader source, paths, or CUDA identifiers. Do not change simulation state structures, CUDA kernels, numeric behavior, or physics. CPU-side text rendering and presentation adapters will need changes; this does not require changing the simulation algorithms.

## 3. Automatic inventory and interpretation

### 3.1 Method and limits

A temporary, uncommitted Python lexer scanned every production `.cpp`, `.h`, `.cu`, and `.cuh` under `source`, excluding `Tests` and `TestData`. It consumed comments and character literals, excluded include/pragma operands, recognized ordinary/prefixed/raw strings, and joined string literals separated only by whitespace. It did not preprocess C++, resolve macros, perform data-flow analysis, or decode escape sequences. Distinct counts therefore mean distinct source spellings. Icon macros and dynamic expressions can split one message into several groups. Inactive code can still be counted.

The direct-candidate heuristic selected literals immediately following `.name`, `.description`, `.tooltip`, `.hint`, `.text`, `.label`, selected ImGui/AlienGui/ImPlot display calls, and window/dialog constructors. It required two consecutive ASCII letters and excluded strings beginning with `##`, `http:`, or `https:`. This deliberately does not label every English-looking literal as translatable. The reproducible scanner is in section 10.

| Production component | Literal groups | Files with literals | Direct visible candidates | Files with direct candidates |
| --- | ---: | ---: | ---: | ---: |
| Gui | 2,809 | 99 | 717 | 55 |
| Data | 457 | 7 | 238 | 1 |
| Apps | 98 | 7 | 0 | 0 |
| ConsoleUi | 46 | 3 | 0 | 0 |
| Base | 706 | 17 | 0 | 0 |
| Network | 357 | 7 | 0 | 0 |
| Persister | 534 | 10 | 0 | 0 |
| McpTools | 1,778 | 18 | 0 | 0 |
| Engine | 116 | 15 | 0 | 0 |
| Rendering | 300 | 90 | 0 | 0 |
| **Total** | **7,201** | **273** | **955** | **56** |

All literals have 4,461 distinct spellings. The direct candidates have **709 distinct spellings** and 4,723 alphabetic word tokens across occurrences. Zero direct candidates in a component means the narrow heuristic did not match its APIs, not that the component has no user-facing text. For example, CLI option descriptions are the third argument, and persister errors use aggregate initialization.

The largest direct-candidate sources are `Data/SimulationParameters.cpp` (238), `Gui/Impl/InspectionWindow.cpp` (110), `NodeEditorWidget.cpp` (74), `MutationRatesDialog.cpp` (47), and `MainLoopController.cpp` (43). These cover parameter descriptions, editors, menus, and toolbars.

### 3.2 Additional text and false positives

Targeted inspection found the following additional pools. Counts below are alphabetic literal groups using the same lexer/filter, **not final catalog entries**, and overlap the preceding table.

| Source | Groups / distinct spellings | Meaning |
| --- | ---: | --- |
| `Gui/Impl/HelpStrings.h` | 29 / 29 | Long tooltips, login/privacy guidance; icon macros split some paragraphs |
| `Gui/Impl/EntityAttributeHelp.cpp` | 208 / 206 | Attribute and cell-function help, including long multi-part tooltips |
| `Gui/Impl/GenomeIssueDescription.cpp` | 57 / 50 | Validation explanations and grammatical fragments |
| `Data/CellTypeConstants.h` | 64 / 59 | Shared display choices and technical function names |
| `Apps/Alien/HelpStrings.h` | 3 / 3 | One long startup guidance message split by dynamic filenames |
| `Apps/Alien-cli/CommandLineParser.cpp` | 18 / 18 | Option names plus help descriptions; switches stay unchanged |
| `ConsoleUi/ConsoleSimulationPanel.cpp` | 19 / 16 | Status labels, units, control hints |
| `Persister/Impl/PersisterWorker.cpp` | 45 / 35 | Mostly visible errors, with dynamic fragments and internal literals mixed in |
| `Gui/Impl/EvolutionDashboardWindow.cpp` | 63 / 45 | Metrics, plots, cards, settings and hidden identifiers mixed together |

For scale, GUI alone has 2,189 alphabetic groups / 1,466 distinct spellings before semantic exclusions. It includes at least 128 occurrences beginning with known settings prefixes (`windows.`, `settings.`, `dialogs.`, `gui.`), plus hidden IDs, popup names, filters, logs, and format tokens. These must not become translations. `McpTools` contributes 1,651 alphabetic groups / 1,005 distinct spellings, heavily dominated by schemas, protocol keys, descriptions, and technical error output; adding them blindly would inflate the estimate.

**Planning estimate:** approximately **1,400–2,000 contextual message keys across 100–130 first-party source files** for the desktop application, associated user-visible errors, and CLI. This is an engineering allowance derived from the direct set and additional pools, not an exact extraction result or a confidence interval. Deduplication reduces repeated words such as “Cancel”, while context, plural variants, and reconstruction of whole dynamic messages can increase key counts. Third-party UI, server emails, and optional localization of agent-facing descriptions are separate work. A reviewed call-site inventory is necessary before committing to a fixed translation budget.

The audit scanned the entire production C++/CUDA literal surface and traced representative consumers in each category. It does not claim that every one of the 7,201 groups has been manually adjudicated. In particular, errors crossing component boundaries, indirect arrays, third-party text, and dynamic strings require a second extraction/review pass during implementation.

### 3.3 Documentation inventory

`resources/docs` contains **19 Markdown files**, including `contents.md` and 18 chapters: **153,848 bytes**, **2,140 lines**, **224 ATX headings**, and **23,031 English word tokens**. The word pattern is `\b[A-Za-z]+(?:['-][A-Za-z]+)*\b`; it counts links, code, and identifiers too, so this is gross source volume rather than billable prose. These byte/line figures describe this Windows checkout; checkout newline policy can affect them elsewhere.

| File | English word tokens |
| --- | ---: |
| ai-agents.md | 930 |
| cell-types.md | 3,884 |
| contents.md | 114 |
| energy.md | 1,221 |
| evolution.md | 1,405 |
| files.md | 683 |
| first-creature.md | 1,102 |
| format-reference.md | 356 |
| generative-art.md | 1,351 |
| genomes.md | 1,908 |
| getting-started.md | 757 |
| glossary.md | 828 |
| how-life-works.md | 1,169 |
| keyboard-and-mouse.md | 308 |
| neural-networks.md | 1,298 |
| sandbox.md | 1,190 |
| simulation-parameters.md | 2,272 |
| troubleshooting.md | 693 |
| user-interface.md | 1,562 |

There are **28 images** in `resources/docs/images`. Screenshots and explanatory diagrams must be visually reviewed for embedded English; counting image files does not establish which require replacement. Preserve shared artwork, technical examples, filenames, and links. README, release notes, server documentation, and other root documents are outside these 23,031 words and need a separately agreed publishing scope.

## 4. Technical findings

### 4.1 Fonts and UTF-8: several independent blockers

`Gui/Impl/StyleService.cpp:245–295` loads compressed DroidSans, DroidSansBold, Cousine, and Reef at multiple sizes. FontAwesome and AlienIconFont are separate icon assets. Text fonts are loaded without explicit glyph ranges. The pinned vcpkg port is Dear ImGui **1.91.9**, not simply the manifest's minimum `1.90`; its default range covers U+0020–U+00FF and excludes Cyrillic. FreeType being enabled does not add missing font coverage automatically. See the [pinned ImGui font implementation](https://github.com/ocornut/imgui/blob/v1.91.9/imgui_draw.cpp).

The actual four compressed text-font payloads were decompressed in memory using the matching ImGui/STB token format. Both decompressed length and Adler-32 checksum were verified; fontTools 4.67.0 inspected each TTF's Unicode cmap. The 66-character test set was U+0410–U+044F plus U+0401/U+0451 (including both forms of Yo).

| Embedded asset | Decompressed bytes | Russian letters present | U+2116 number sign | U+2014 em dash |
| --- | ---: | ---: | --- | --- |
| DroidSans | 190,044 | 66 / 66 | yes | yes |
| DroidSansBold | 42,480 | 0 / 66 | no | yes |
| Cousine-Regular | 43,912 | 0 / 66 | no | yes |
| Reef | 126,028 | 66 / 66 | yes | yes |

Thus adding Cyrillic ranges alone will not fix bold headings or monospace text. Supply licensed complete fonts or deliberate matching fallbacks for every text role; retain icon merges and sizes. Build the atlas for the union of English/Russian characters, punctuation, symbols, and supported user input, with ranges kept alive for atlas construction. Check license/redistribution terms before substituting assets. Test actual glyphs rather than allowing ImGui's replacement glyph to hide omissions.

`Rendering/Impl/RenderSteps.cpp:511–590` builds a separate cell-type atlas using `for (auto const& character : label)` followed by `FindGlyph(static_cast<ImWchar>(character))`. This is **byte iteration**, so Russian UTF-8 fails even with a correct font atlas. It also builds labels from shared English arrays and uses a fixed 512-pixel texture height. Decode code points, size using actual glyph advances, and invalidate/rebuild localized label textures when the language changes. Pass translated labels through the rendering facade/presentation boundary; keep enum indices and GPU simulation data unchanged.

Additional UTF-8 issues:

- `DocumentationWindow.cpp` lowercases with `std::tolower` per byte. `Base/StringHelper.cpp` uses byte-based `tolower`/`toupper` for case-insensitive filtering and matching. These do not implement Russian case folding. Parameter filtering in `Data/SpecificationFilterService.cpp` also searches original specification names; translated labels alone would leave English-only search.
- `MarkdownParser::toAnchor` preserves high bytes but lowercases per byte. Translating a heading changes its derived anchor; all cross-chapter fragments must be rebuilt/validated. Cyrillic uppercase/lowercase handling needs explicit rules.
- Documentation snippets normally cut at spaces, but `createSnippet` has a fallback `start + SnippetLength` byte cut for long tokens. Test and correct boundary handling there. `StringHelper::truncate` already skips UTF-8 continuation bytes and is a useful existing pattern, not proof that every path is safe.
- `Network/NetworkValidationService.cpp` tests individual `char` values with `std::isalnum` and an ASCII punctuation allowlist. Cyrillic account/resource metadata is not reliably accepted; passing negative signed-char values to ctype functions is also unsafe. A translated interface must not promise unrestricted Russian cloud input. Fixing validation is a separate, explicitly tested compatibility change, with server rules checked too.
- `SpecificationGuiService.cpp:399–422` edits fixed-size `Char64` fields using their byte capacity. Cyrillic consumes more bytes than ASCII; retain on-disk/GPU layouts, enforce byte-safe input limits, and explain them where necessary.
- `Base/Console.cpp` already sets Windows console output to UTF-8 and counts non-continuation bytes for visible length. CLI still needs testing in redirected/plain output and startup failure paths, including paths that precede console initialization.
- `Apps/Alien/HelpStrings.h` explicitly warns about non-English installation paths/usernames. Some paths use `.string()` and narrow streams. Russian UI support does **not** establish Unicode filesystem support; test save/open, autosave, screenshots, exports, and launch under Cyrillic paths independently.
- No explicit `/utf-8` or equivalent source/execution charset option was found in project CMake sources. Runtime UTF-8 catalogs reduce literal-encoding dependence, but translated source literals or generated embedded data still need deterministic encoding and Windows tests.

### 4.2 ImGui identity and retained state

Changing visible strings in place is unsafe. Dear ImGui IDs incorporate labels and the current ID stack; `##` hides a suffix while retaining the whole label in the hash, whereas `###` separates the visible prefix from stable identity. See the [versioned ImGui ID FAQ](https://github.com/ocornut/imgui/blob/v1.91.9/docs/FAQ.md).

| Observed location | Coupling and failure risk | Required separation |
| --- | --- | --- |
| `AlienWindow::process` | `_title` goes to both `PushID` and `Begin`; language changes replace window/widget identity | Stable window ID and stable parent scope; translated title separately |
| `ModalWindow::process` | `OpenPopup`, `BeginPopupModal`, and `PushID` use `_title`; an open modal can disappear when the title changes | Same stable popup ID at open/begin; refresh title without changing parent scope |
| `AlienGui.cpp`, e.g. 153, 228, 365–374, 545–547, 683–689 | Names and hints feed `PushID` and strings such as `"##" + name`; slider/input focus and transient state can reset | Explicit widget ID independent of display name/hint |
| `AlienGui::Button`, `MenuItem`, tree widgets | Visible labels generate IDs; same Russian translations can collide | Stable contextual key/entity ID, even where English text is repeated |
| `SpecificationGuiService.cpp:55–63` | Group name, including “Expert settings: ”, feeds `PushID` and `_visibilityById` | Stable group identity, translated presentation and search adapter |
| Documentation navigation | `Selectable(chapter.title)` / section title participates in identity despite outer numeric scope | Chapter filename/section anchor as identity; text separate |
| Genome tabs, browser rows, dynamic titles, ImPlot series | Entity labels, names, counts or selected metric can affect identity/legend state | Stable entity/metric IDs and independent formatted captions |

`GlobalSettings::loadImGuiSettings` sets `IniFilename=nullptr`; ImGui's serialized state lives under `gui.imgui settings`. Stable `_settingsNode` values in `AlienWindow` preserve some custom position/state, but do not solve ImGui identity changes. Plan a one-time mapping from old English window IDs to new IDs, or document an intentional initial layout reset. Language switches thereafter must preserve state.

For labels, use a helper producing `translatedCaption + "###" + stableId` where that widget supports this convention. `"##stableId"` appended to a translated caption is insufficient. `PushID(stableId)` followed by `Button(translatedCaption)` is also insufficient because the caption remains part of the child hash. For custom widgets, draw translated text separately and operate a constant hidden control under a stable scope. Remove translated ancestor `PushID` values too. Repeated controls need entity identifiers; never use row text as identity. Handle literal `##`/`###` in user-supplied captions separately from trusted catalog text.

### 4.3 Settings and switching

`Base/GlobalSettings.cpp` supports strings and other types. Windows stores values under **HKCU\\SOFTWARE\\alien** via WinReg and UTF-8/UTF-16 conversion helpers. Other platforms use `$XDG_CONFIG_HOME/alien/settings.json`, falling back to `~/.config/alien/settings.json`. ImGui settings are persisted through the same service. The local copy therefore shares the installed ALIEN settings namespace unless a future task deliberately introduces separation.

No application localization service, language selection, or persistent locale setting was found. `MainLoopController.cpp` provides Settings menu actions, and `DisplaySettingsDialog.cpp` handles display/scaling preferences; neither implements language choice.

Add a stable new key such as `settings.language` with values `en` and `ru`, defaulting to `en`; unknown/invalid values resolve to `en`. Load the setting before presentation caches and window titles are built. A language selector should display “English” and the Russian autonym. Apply at a frame boundary and rebuild translated text, documentation search, label atlases, menu/title caches, and parameter filter caches. Do not rebuild/restart the simulation. Persistent modal state and active text edits must survive switching. Registry conversion helpers and settings round-trips need tests, even though locale codes themselves are ASCII.

### 4.4 Dynamic text, grammar and technical notation

Current code mixes printf-style strings (`%s`, `%d`, `%.2f`), `std::format` expressions, icon macros, newline escapes, concatenation with `std::to_string`, and plural fragments. `GenomeIssueDescription.cpp` distinguishes “gene ”/“genes ” and “node ”/“nodes ”; `StringHelper::formatEnumeration` inserts English “ and ”. `AutosaveWindow.cpp` appends time and savepoint counts, `InspectionWindow.cpp` appends “ properties”, and authentication dialogs splice usernames into sentences.

Extract **whole sentences with typed named arguments**, not grammatical fragments. Russian needs one/few/many/other handling; test 0, 1, 2, 5, 11, 21, 22, 25, 101 and fractional values. Context matters: translate a cell “constructor” consistently without confusing it with a C++ constructor. Create an English/Russian glossary for cells, genomes, genes, nodes, energy, radiation, layers, sources, lineages and mutations as part of the catalog work.

Keep `ALIEN`, CUDA, Vulkan, GPU models, enum/protocol values, filenames/extensions, keyboard shortcuts, mathematical notation and machine-readable numeric formats stable. Localize surrounding explanations and display units only where meaningful. Keep locale-sensitive numbers out of serialization, CSV schemas and CLI protocol output; do not switch the process-wide C locale as a shortcut. Render translated text through `TextUnformatted` or `Text("%s", text)`, never treat catalog text as a printf format string. Preserve hyperlinks, escapes, and icons explicitly.

### 4.5 Dependencies and other surfaces

- vcpkg pins `imgui` 1.91.9 and `implot` 0.17 at this baseline; GLFW, FreeType, Boost.JSON/property_tree, CLI11, md4c, Vulkan and other dependencies are declared in `vcpkg.json`. Minimum versions in the manifest are not exact installed versions. No dependency build was performed.
- Vendored `external/ImFileDialog/ImFileDialog.cpp` has visible English: table headings at 1100–1102, new file/directory and delete actions at 1207–1211, confirmation buttons, “File name:” at 1370, Save/Open/Cancel at 1388–1399, plus dates. `GenericFileDialog` can localize its title/filter captions but cannot translate these internals with the current public Open/Save API. A narrow catalog callback/text configuration patch or an alternative component must be evaluated later. Preserve its filesystem behavior and stable key `GenericFileDialog`.
- Dear ImGui built-in input context menus and ImPlot built-in plot/legend context menus require an explicit English-text review. Hiding them without equivalent functionality would not meet full localization. Audit the pinned versions before choosing a small adapter/patch; do not assume translating first-party calls covers library-owned UI.
- CLI11 generates help and parse errors in addition to ALIEN's descriptions. Retain switch spellings, exit codes and plain-output contracts; assess CLI11's supported customization rather than translating its exception strings ad hoc.
- `source/Server/main.py:426–434` generates English activation-email subjects and bodies. Client-only localization cannot change emails produced by the official server. Server HTML/admin surfaces are a separate deployment scope; no production server changes are part of this audit.
- User-created simulation/genome names, community descriptions, downloaded resources, driver/compiler diagnostics and external web content are not automatically translatable. Localize the application-owned wrapper around diagnostics; retain original technical detail for support. Inventory bundled resource descriptions separately if included in the release promise.
- MCP tool names, schemas and JSON fields must remain stable. Human-readable tool descriptions/errors may be localized separately if desired, but should default to English for automation compatibility. GUI language must not silently change MCP responses or the shared documentation API.

## 5. Windows and RTX 5080

The project currently uses CMake **3.31+**, C++23, CUDA C++20, vcpkg manifest mode, and the Ninja Multi-Config presets. `build-windows-ninja.bat` finds Visual Studio with the C++ toolset, calls `vcvars64`, and selects bundled CMake/Ninja when available. Future agent builds must use `build-windows-ninja.bat agent`, writing to `build-agent`, in accordance with `AGENTS.md`; `build-ninja` belongs to the IDE. The alternative HIP backend is irrelevant to NVIDIA testing.

`CMakeLists.txt:45–50` already defaults `CMAKE_CUDA_ARCHITECTURES` to **75 and 120** unless overridden. Its comment explicitly describes `120` as a diagnostic change investigating a hanging constructor kernel on Blackwell. This is evidence of native targeting, **not evidence that all RTX 5080 failures have been resolved**. Existing CMake caches may override these defaults.

NVIDIA lists [GeForce RTX 5080 as compute capability 12.0](https://developer.nvidia.com/cuda/gpus). [CUDA 12.8 release notes](https://docs.nvidia.com/cuda/archive/12.8.0/cuda-toolkit-release-notes/index.html) document compiler support for `SM_120`; the repository accordingly requires CUDA 12.8 or newer for this target. The [Blackwell compatibility guide](https://docs.nvidia.com/cuda/archive/12.8.2/blackwell-compatibility-guide/index.html) distinguishes native cubins from PTX forward compatibility. Select a toolkit/MSVC pair supported together and a driver supporting both the GPU and chosen toolkit; a generic CUDA 12.x minimum-driver number alone does not establish RTX 5080 support.

Read-only host checks observed **NVIDIA GeForce RTX 5080**, driver **617.42**, reported compute capability **12.0**, and `nvcc` **13.2 / V13.2.51**. `cmake.exe` is on PATH; Ninja was not found on the current PATH, though the build script may locate the VS-bundled binary. No claim is made about a successful configure, installed MSVC compatibility, or application execution.

`Rendering/Impl/VulkanContext.cpp` requests Vulkan **1.3**, detects optional hardware ray tracing, and uses external memory sharing with the engine GPU. Runtime packaging in CMake copies resources, CUDA runtime DLLs and MSVC redistributables. Fonts/catalogs/localized docs must join that packaging. Test the default interop path and `--no-interop`, ray tracing where available, first-run startup, constructor-heavy simulations and engine tests on the actual card. Do not alter kernel launches, architecture flags, physics or timings as a localization workaround. Record any pre-existing GPU failure against the English baseline before attributing it to localization.

## 6. Proposed localization design

### 6.1 Catalog choice

| Option | Benefits | Costs for this project | Decision |
| --- | --- | --- | --- |
| UTF-8 JSON | Existing Boost.JSON dependency, familiar tooling, easy key/argument validation, translators edit independently | No comments; reject duplicate keys and validate schema explicitly | **Recommended** |
| TOML | Comments and multiline strings are convenient | New parser/tooling and syntax rules without a decisive benefit | Not initially needed |
| C++ catalogs | Compiler-visible keys, no runtime file dependency | Rebuild for edits; escaping, encoding and source churn; harder translator workflow | Use only generated embedded English fallback |
| gettext PO/MO | Mature translator tools, contexts, plural conventions | New runtime/toolchain integration and migration work; msgid-based updates can churn without stable contexts | Viable if an established translation team prefers it |
| ICU/message-format catalogs | Rich grammar and broad locale support | Significant integration/validation footprint for two initial languages | Revisit if message grammar grows beyond a small explicit plural model |

Use `resources/locales/en.json` and `ru.json`, organized by semantic domains in flat stable keys, e.g. `menu.file.open`, `dialog.display.title`, `parameter.radiation.strength.help`, `genome.issue.missing_gene`. Never use an array position, current English sentence, source line, translated text, or ImGui hash as a catalog key. Share keys only when meaning and grammar are identical. Catalog keys and UI IDs are distinct concepts; one translated action may appear in several independently identified controls.

Keep English canonical and complete. Use Boost.JSON for strict loading; reject malformed UTF-8, duplicate keys, invalid message/argument definitions and unsafe control characters. Add a schema version. Store source context and argument metadata in catalog records rather than JSON comments. English changes should mark translations for review using a source-text hash, not force a new key when the semantic identity remains the same.

For robustness, generate a compiled English fallback from the same canonical JSON at build time, without hand-maintaining two English catalogs. Missing Russian entries, unknown locale, absent catalog files, and malformed Russian catalogs fall back predictably to canonical English; log diagnostics once. A missing canonical key is a development error and must fail validation. Release builds should show a diagnostic fallback rather than crash, but no unknown key is acceptable at release qualification.

### 6.2 Runtime placement and contracts

Implement a small CPU-only localization service in a presentation/common module accessible to GUI, CLI and rendering adapters. It has no CUDA includes, static initialization that reads settings, or mutable simulation data. Initialize explicitly from `GlobalSettings` before presentation setup. Publish immutable catalog snapshots; keep formatted strings owned for their required lifetimes. Do not cache bare `const char*` values across locale changes. Workers should return stable error codes/arguments where feasible and format at the UI boundary, leaving raw diagnostics available.

Initially, known legacy English errors can be mapped to catalog keys in presentation adapters while preserving original details; classify unknown diagnostics rather than blindly translating every exception. Longer-term typed error metadata must preserve existing APIs and serialization behavior. No translated text should cross into simulation control decisions.

For parameter specs and enum display arrays, use GUI-side mappings from stable original fields/enums to catalog keys. Do not mutate `SimulationParameters::getSpec()` according to global language: shared consumers include filters, MCP and rendering. Add a localized presentation/filter layer and include a locale-generation value in presentation cache invalidation. Keep API names and original canonical descriptions available to MCP. Supply translated cell labels to the CPU text-atlas builder through a narrow presentation API.

Support a constrained, validated named-argument grammar such as `{count}` and `{filename}`, with types/format policies declared in metadata. Reordering arguments is allowed; adding/removing/changing argument names or types is not. Use explicit plural variants (`one`, `few`, `many`, `other`) selected by locale rules. Do not assume `std::format` directly implements named arguments or Russian plural rules: the formatter adapter must supply those contracts. Avoid introducing a general expression language or arbitrary translator-controlled printf specifications.

### 6.3 Documentation and language switching

Preserve existing English files under `resources/docs` to reduce upstream conflicts. Add Russian chapters under a parallel path such as `resources/locales/ru/docs`, with stable chapter filenames and localized contents. The resolver should accept an explicit locale, fall back per chapter to English, and return the actual document directory for relative images. Shared images need explicit resolution to the original image directory; simply changing `DocsPath` would break links and images. Missing Russian chapters should be clearly marked as English fallback during development and fail the full-Russian release gate.

Keep GUI selection locale-aware, while the existing MCP documentation endpoint defaults explicitly to English. Validate the entire resolved link/anchor graph, including links from fallback English chapters into localized chapters; retain English anchor aliases or stable explicit anchors where necessary. Persist chapter identity by filename and preserve the selected section using stable anchor mappings, not translated heading text. Rebuild search using Unicode case folding and safe snippets.

Apply language changes between frames. Refresh window/modal captions, menus, tooltips, documentation navigation/search, parameter search, and rendering text atlases in a coordinated generation change. Prefer preloading both alphabets so switching does not require font GPU resource replacement. If font replacement is unavoidable, synchronize only presentation resources and rebuild dependent text textures. Preserve open dialogs, selected tabs, plot visibility, edits, viewport, simulation progress and saved UI state.

### 6.4 Upstream maintenance and validation

Keep catalogs and translated docs additive. Make narrow patches to widget wrappers and presentation call sites, grouped by screen/domain. Avoid mass formatting and avoid mechanically replacing all literals. Preserve original APIs through adapters where practical. Isolate any vendor localization patch with its upstream version and rationale; do not modify vcpkg pins simply to obtain localization features.

Future validation should enforce:

1. Every referenced key exists in canonical English; reject duplicate keys and orphan entries unless deliberately retained for compatibility.
2. Russian covers all required keys and plural variants. Fallback is a runtime resilience feature, not a substitute for release completeness.
3. Argument names, types, allowed format policies, escapes and plural-selector signatures match English. Reject malformed braces, unsafe format directives, and accidental `##`/`###` in catalog captions.
4. Stable keys survive English copy edits; semantic changes receive review. Track source hashes to catch stale Russian even when the key still exists.
5. A reviewed exception list accounts for remaining visible English: brands, symbols, user content and raw diagnostics. A lexical scan alone cannot prove complete localization.
6. Pseudolocalization expands text and adds non-ASCII characters; targeted UI checks verify no clipped buttons, tooltips, menus, fixed-width parameter columns or tables. `SpecificationGuiService` currently hardcodes column widths, so this is a concrete risk.
7. ID tests compare the same controls in English/Russian, including ancestor scopes, duplicate translated labels, dynamic entities, open modals, and save/reload of ImGui state.
8. Docs validation checks chapter coverage, links, fragment anchors, image resolution, screenshots, Unicode search and fallback behavior.
9. English/Russian startup, persistence, corrupted/missing catalogs and format failures have deterministic behavior. Runtime diagnostics do not flood logs each frame.

## 7. Risks and mitigation priorities

| Risk | Priority | Mitigation / evidence needed |
| --- | --- | --- |
| Missing Cyrillic glyphs, especially bold/monospace | Blocking | Font cmap gate for every role plus visual atlas verification |
| Byte-based cell label rendering | Blocking | Decode UTF-8 and rebuild presentation atlas; no simulation changes |
| Language-dependent ImGui IDs | Blocking | Explicit IDs throughout parent and child scopes; modal/layout regression checks |
| English-only filtering and documentation anchors | High | Localized search adapter, Unicode case folding and validated anchor aliases |
| Fragmented grammar and plural counts | High | Whole-message extraction, typed named arguments and Russian plural forms |
| Visible errors outside GUI and vendor menus | High | Trace error consumers; explicit third-party localization strategy |
| Russian cloud metadata / filesystem paths | High | Separate compatibility tests and validation work; do not conflate with translated labels |
| Upstream churn in alpha development | High | Additive catalogs/docs, narrow adapters, source-hash review after each URL fetch |
| RTX 5080 constructor hangs or toolchain mismatch | High | Establish English baseline and record GPU/driver/toolchain; investigate independently |
| Shared settings with official installation | Medium | Test new key defaults and backups; separate namespace only by explicit later decision |
| Longer text, text texture capacity and DPI | Medium | Pseudolocalization and 100/150/200% scale review; size from glyph metrics |
| Server-owned emails remain English | Scope constraint | Requires independently authorized server localization/deployment; client cannot guarantee it |

## 8. Implementation plan with verifiable exits

No stage below was implemented by this audit.

1. **Freeze and classify the message inventory.** Review direct/indirect string pools, dependencies and error flows; map each visible call site to a semantic key or justified nontranslation category. Agree terminology, desktop/CLI scope and server/content boundaries. Exit: reviewed inventory, measured final key/word count, no unassigned first-party display sites.
2. **Prove text infrastructure on the English baseline.** Add catalog schema/loading, canonical English generation, fallback, typed formatting and persistent locale code. Exit: missing/broken catalog tests pass, English behavior unchanged, no dependency from CUDA/physics to locale state.
3. **Separate identity before translating screens.** Extend wrappers/windows/modals to take stable IDs; migrate affected calls and persisted IDs deliberately. Exit: switching synthetic captions preserves focus, open popups, tabs, plot selection and saved layout, including repeated labels.
4. **Resolve Unicode blockers.** Provide licensed fonts with verified coverage, explicit ranges, UTF-8 text-atlas decoding, Unicode search and safe truncation. Validate Cyrillic paths/input limits separately. Exit: all text roles and cell overlays render the test alphabet, Yo, punctuation and mixed Latin/Cyrillic; searchable Russian terms work across cases.
5. **Extract and translate by domain.** Menus/settings and common dialogs; creation/inspection; parameters/mutations; genome/neural editors; browser/network/autosave; statistics/plots; user-visible errors; CLI/startup guidance. Add third-party strings through the chosen adapter. Exit per domain: complete catalog coverage, correct formatting/grammar and reviewed screen behavior in both languages.
6. **Localize documentation.** Translate contents and all 18 chapters, verify glossary consistency and review all 28 images. Implement locale resolution without changing MCP defaults. Exit: all chapters open, cross-links/anchors/images resolve, search works and no unexpected English fallback remains.
7. **Qualify Windows packaging and GPU behavior.** Build through the agent preset with the supported toolchain, verify shipped resources/fallbacks and clean-machine launch. Run appropriate Base/Data/EngineInterface/McpTools/Network/Persister tests and the GPU EngineTests suite; exercise RTX 5080 constructor-heavy simulations with interop and fallback rendering. Exit: failures triaged against English baseline, no localization-induced simulation changes, packaged release passes bilingual and DPI review.
8. **Rehearse an official update.** In a later authorized task, fetch official `develop` by URL, inspect/reconcile changes and regenerate coverage/staleness reports. Exit: new upstream UI strings are detected, keys/IDs remain stable, catalogs and docs merge without unrelated source churn.

## 9. Definition of complete Russian localization

- Every first-party desktop menu, dialog, control, parameter, tooltip, notification, genome/neural editor, graph caption and application-owned error is Russian when `ru` is selected; supported CLI help/status/error surfaces are covered too.
- Required JSON keys, argument signatures, plural forms and reviewed source hashes pass validation at 100%; English fallback works but is not invoked for normal supported Russian flows.
- All 19 documentation Markdown files are localized, all internal links/anchors and images work, and the 28-image review has no unresolved English instructional content in the declared scope.
- All text roles render the complete Russian alphabet, Yo, digits, necessary symbols and punctuation without replacement glyphs; input, copy/paste, search and truncation preserve UTF-8.
- Locale selection persists across restart; `en` remains a complete supported language. Switching language preserves simulation state, layout, focus, active dialogs, edited data and plot selections.
- ImGui/ImPlot identifiers, settings keys, save formats, protocols, CLI switches and simulation/CUDA/physics behavior are independent of language. Existing English files remain compatible.
- Third-party UI strings have an explicit tested solution. Raw external diagnostics, user content and brands are identified exceptions, not unexplained missing translations.
- Windows packaging includes all required assets and fallback data; RTX 5080 tests pass or any pre-existing engine defect is explicitly reported and prevents an unconditional compatibility claim.
- A claim that the **entire ecosystem** is Russian additionally requires server email/admin localization and deployment plus an agreed bundled/cloud-content policy. Client completion alone must not imply that the official server has been localized.

Audit validation: source/submodule state was checked, the entire production literal scan and documentation counts were run automatically, font payload checksums/cmaps were verified, source evidence was reviewed, and the official branch SHA was rechecked read-only. Only this new report is intended for the local audit commit. Build/runtime and translation-completeness tests belong to implementation and were not run for this document-only change.

## 10. Reproducing the source-string measurements

Run the following with Python from the audited repository root. It prints the table in section 3 without writing repository files. This is a lexical inventory, with the limitations described above; it is not a proposed production extractor.

```python
import pathlib, re

lex = re.compile(
    r'//[^\n]*|/\*.*?\*/|(?:u8|u|U|L)?R"(?P<d>[^ ()\\\t\r\n]{0,16})'
    r'\(.*?\)(?P=d)"|(?:u8|u|U|L)?"(?:\\.|[^"\\])*"|'
    r"'(?:\\.|[^'\\])*'", re.S)
rows = []
for p in pathlib.Path('source').rglob('*'):
    if p.suffix not in {'.h', '.cpp', '.cu', '.cuh'}:
        continue
    if any(x in p.parts for x in ('Tests', 'TestData')):
        continue
    s = p.read_text(encoding='utf-8-sig')
    groups, end = [], -1
    for m in lex.finditer(s):
        t = m.group()
        if t.startswith(('//', '/*', "'")):
            continue
        line = s[s.rfind('\n', 0, m.start()) + 1:m.start()]
        if re.match(r'\s*#\s*(include|pragma)', line):
            continue
        v = t[t.find('"') + 1:-1]
        if 'R"' in t[:4]:
            v = v[v.find('(') + 1:]
            v = v[:v.rfind(')')]
        if groups and re.fullmatch(r'\s*', s[end:m.start()]):
            groups[-1]['text'] += v
            end = m.end()
            continue
        ctx = s[max(0, m.start() - 180):m.start()]
        visible = bool(re.search(
            r'\.(?:name|description|tooltip|hint|text|label)\(\s*$|'
            r'(?:ImGui|AlienGui|ImPlot)::(?:Text\w*|Button|MenuItem|BeginMenu|'
            r'Begin|Selectable|TableSetupColumn|BeginTabItem|Plot\w*|BeginPlot|'
            r'SetupAxis\w*)\(\s*$|(?:AlienWindow|AlienDialog|ModalWindow)\(\s*$', ctx))
        groups.append(dict(file=p.as_posix(), text=v, visible=visible))
        end = m.end()
    rows += groups
candidates = [x for x in rows if x['visible'] and
              re.search('[A-Za-z]{2}', x['text']) and
              not x['text'].startswith(('##', 'http:', 'https:'))]
for mod in ['Gui', 'Data', 'Apps', 'ConsoleUi', 'Base', 'Network',
            'Persister', 'McpTools', 'Engine', 'Rendering']:
    rr = [x for x in rows if x['file'].split('/')[1] == mod]
    cc = [x for x in candidates if x['file'].split('/')[1] == mod]
    print(mod, len(rr), len({x['file'] for x in rr}),
          len(cc), len({x['file'] for x in cc}))
print('all', len(rows), 'distinct', len({x['text'] for x in rows}))
print('candidates', len(candidates),
      'distinct', len({x['text'] for x in candidates}))
for p in sorted(pathlib.Path('resources/docs').glob('*.md')):
    s = p.read_text(encoding='utf-8-sig')
    print(p.name, p.stat().st_size, len(s.splitlines()),
          len(re.findall(r"\b[A-Za-z]+(?:['-][A-Za-z]+)*\b", s)),
          len(re.findall(r'^#{1,6} ', s, re.M)))
```
