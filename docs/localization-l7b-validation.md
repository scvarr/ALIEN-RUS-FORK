# L7-B Russian documentation package validation

Date: 2026-10-26 (Europe/Moscow).
Base: `9ab2171ef8e13d8b1d2dc56791152cf24a3fe0e7`.
Branch: `feat/ru-localization-l7b`. L7-C/D are not started.

## Translation inventory

The seven requested UTF-8 Markdown files are under `resources/docs/ru/`.
All text, explanations, examples, tables, notes, list steps and image captions are
translated. Technical filenames, fragments, URLs, the neural formula, channel
numbers, numeric values and keyboard shortcuts are retained. The untranslated
internal `Free` construction mode retains its technical name as required by the
terminology authority; its complete explanation is translated.

| File | Russian characters | UTF-8 bytes | Headings |
| --- | ---: | ---: | ---: |
| contents.md | 961 | 1,426 | 3 |
| getting-started.md | 5,286 | 9,146 | 4 |
| user-interface.md | 10,375 | 18,142 | 19 |
| keyboard-and-mouse.md | 2,552 | 4,214 | 7 |
| genomes.md | 11,922 | 21,062 | 13 |
| neural-networks.md | 8,217 | 14,360 | 11 |
| first-creature.md | 6,667 | 11,630 | 10 |
| Total | 45,980 | 79,980 | 67 |

Character counts include Markdown syntax and whitespace. There are 6,452 Russian
whitespace-delimited tokens versus 7,719 in the English originals; word counts are
language-dependent, not a completeness metric. The actual project parser verifies
the same block count, block kinds, heading levels/order, list nesting/numbering,
table dimensions/alignment and note structure for every translated file. Links
inside tables and notes and exact fenced code contents are checked separately.

## Terminology and practical instructions

Translation was reviewed against `docs/localization-terminology.md` and the actual
L1–L6 catalog `resources/locales/ru.json`. This preserves distinctions between raw,
usable, stored and reserved energy; cell types and sensor/muscle modes; neural
inputs, outputs and memory; construction and separation. The internal `Free`
mode is not assigned a new Russian UI label.

UI action names were checked against the catalog and their consumers, including
`GenomeEditorWidget`, `GenomeTabWidget`, `MutationRatesWidget`,
`NeuralNetEditorWidget`, `CellFunctionChannels`, and the world-tool presentation.
Examples use the actual Russian labels for creating an energized seed, adding a
node, construction, separation, disc-shaped networks, energy particles, mutation
probabilities and neural weights/biases. The browser tab is called `Миры`, and its
workspaces use the accepted Featured/Community/Private translations. Existing
English technical names in activation-function explanations identify the same
operations; their Russian labels match the editor.

The tutorial retains width 600, height 400, external energy 100000, six nodes,
sensor at node 5, muscles at nodes 0–4, construction at node 2, disc radius 4,
particle energy 30 and approximate placement distance 50. Neural examples retain
all signs, weights, biases, channel indices and the memory-gate threshold 0.1.
The gate is distinct from the four recurrent memory values and from the Memory
cell type. Signal direction remains last node toward node 0, with one cycle per
cell and six time steps per cycle.

Source inspection corroborated `NeuronProcessor::processCell` (16 inputs,
12 outputs, gate exclusion from memory sums), `calcTelemetryInput` (age saturation
at 100000), and `GenomeEditorWidget` (last-node connection weight zero).
`GenomeDesc.h` and `CellTypeConstants.h` establish constructor auto-trigger interval
100. `SimulationParameters.h` establishes constructor inflow 50, inflow threshold
0, unrestricted offspring inflow and normal cell energy 100. These defaults and
`ConstructorProcessor` support the tutorial's external-energy reproduction step.
`SensorProcessor` scans for matching detections using distance ordering, and the
channel descriptions agree with the editor. This is code/parameter verification;
the biological experiment was not manually run or newly tested on the GPU.

## Navigation and a narrow L7-A correction

The canonical 17 chapters keep their order and filenames; the GUI-only format
reference remains the eighteenth chapter. All three section names and the TOC
captions are translated in the Russian contents file. Six translated content
chapters load from the Russian directory without an English-fallback flag. The
remaining 12 GUI chapters load their English originals with that flag and the
existing notice; their English chapter titles remain recognizable.

Real mixed contents exposed an L7-A issue: section names were translated only for
chapters whose bodies had a translation. This split one canonical section into
alternating English/Russian groups. A small change to the Russian branch of
`DocumentationReader::getGuiChapters` maps canonical part names through the Russian
TOC independently of body availability, including the appended format reference.
It changes only GUI section metadata. English catalog loading, fallback chapter
titles, chapter order, body selection and the separate MCP reader are preserved.
All actual RU chapters are tested to belong to the three translated parts.

`DocumentationLocalizationTests.shippedRussianPackageStructureFallbackLinksImagesAndSearch`
uses the real `MarkdownParser` and F1 `DocumentationWindow` on shipped files. It
visits every supported internal link in EN/RU/EN, including English-to-Russian and
Russian-to-English destinations, native Russian fragments and every canonical
English heading alias. Heading count, order and levels match, so all L7-A aliases
remain available. The Russian contents introduction retains its format-reference
link; it does not add an MCP chapter. External link targets are compared exactly
through parsed spans, including notes and tables. No live website availability or
external-browser launch is claimed.

Real searches for lower/upper-case Cyrillic, including `жёсткость`/`ЖЁСТКОСТЬ`
and `соединённых`/`СОЕДИНЁННЫХ`, return nonempty matching result counts and valid
UTF-8 snippets. Existing fixture tests additionally cover Ё/ё, Е/Ё distinction,
multi-term AND/ranking, invalid UTF-8 and long-token snippets. There is no claim of
morphological search or Е/Ё equivalence.

`shippedLongChaptersRenderAndNavigateAtEveryScale` renders the actual interface,
genome, neural and tutorial chapters through the native `AlienWindow` process at
100/150/200%, in EN/RU/EN. It visits every heading alias, checks accepted/completed
scroll requests, stable bounded scrolling, chapter preservation, window identity
and size, and image metadata without missing paths. At the end of a chapter the
top visible heading can precede the requested one because scrolling is clamped;
the test does not incorrectly require every heading to be the topmost heading.
Existing tests retain section/language restoration, UI IDs, settings, buttons,
texture lifecycle and missing/partial-tree regressions.

## Image decisions

All eight original PNGs were individually opened and inspected. They are
illustrations/schematics, including simplified UI layouts, rather than new
captures of the current Russian application. All contain English embedded text.
The package reuses the original bytes via `../images/...` and translates every
alternative caption and surrounding explanation. No fictional application
screenshot, AI replacement or altered signal arrow was introduced. Existing
repository provenance, any embedded attribution/metadata and original assets are
preserved. No editable source for these eight illustrations was found in the
repository; exact localized artwork is deferred rather than claiming an
unverified redraw is faithful.

| Image | Decision and reason |
| --- | --- |
| particles-to-ecosystems.png | Original reused; separate Russian typography update required. English hierarchy labels and descriptions remain; the translated introductory paragraph and caption explain all four levels. No UI controls are depicted. |
| ui-layout.png | Original reused; separate localized layout illustration required. English menus, window names and yellow callouts remain. The diagram has placeholder buttons, not a reliable current screenshot; the chapter documents the actual Russian controls. |
| genome-structure.png | Original reused; separate label localization required. English genome/gene/node, cell-type and constructor-reference labels remain. It illustrates structural references; its leading sensor is not the tutorial's default signal-processing chain. |
| shapes.png | Original reused; separate source reconciliation and localization required. English shape names and counts remain; numeric discrepancies with the source paragraph are recorded below. Correcting the figure would require choosing a new verified baseline. |
| cell-constructor.png | Original reused; separate label localization required. Construction order, insertion at the constructor and direction arrows agree with the text; English node/constructor/growth labels remain. The translated explanation preserves that node 0 ends farthest away. |
| neural-network.png | Original reused; separate precise label localization required. All channel indices, 16 input/12 output layout, connections and formula are retained. English input/output/memory/telemetry labels remain. The memory gate is explained in prose; it is not explicitly drawn. |
| genome-editor-layout.png | Original reused; separate Russian layout update required. English toolbar/property/column labels remain, with simplified placeholder controls. Current Russian labels are supplied by the translated tutorial, not inferred from this illustration. |
| tutorial-hunter.png | Original reused; separate label localization required. The five muscles, sensor at node 5, food and signal/motion arrows are consistent with the tutorial. English annotations remain; translated prose/caption describe channels #0/#1 and both directions. |

The image-language limitation is explicit: the text package is translated, but
these figures are not Russian artwork. Readability/faithfulness of future image
translations requires a separate artwork pass and acceptance. All eight paths
and PNG dimensions are checked by the real renderer; the existing rendering-facade
double receives actual encoded bytes. No new end-to-end Vulkan texture allocation
or manual visual acceptance is claimed.

## Verification

Windows Release built successfully with `build-windows-ninja.bat agent Release 16`.
Only `build-agent` was used. After the final resource edit, `alien.exe` was
relinked through that script so the existing CMake POST_BUILD copy ran. The
ordinary no-op build alone does not refresh Markdown resources. SHA-256 checks
confirm all seven translations and all 28 PNGs match source bytes under both
`build-agent/resources` and `build-agent/Release/resources`. The 11 documentation
tests also pass when launched with the Release directory as the working directory.
No separate manual-acceptance build was prepared.

| Check | Result |
| --- | --- |
| BaseTests | 44 passed |
| DataTests | 247 passed |
| EngineInterfaceTests | 2 passed |
| McpToolsTests | 40 passed |
| NetworkTests | 38 passed |
| PersisterTests | 142 passed; 1 existing disabled test |
| GuiLocalizationTests | 161 passed, including 11 documentation tests |
| AccountLocalizationTests | 6 passed |
| McpLocalizationTests | 7 passed |
| check_localization_l7b.py -v | 5 passed |
| clang-format 19.1.5, modified C++ files | Passed |
| git diff --check | Passed |

The suites were rerun after the part-name correction. The complete GUI tests,
Base reader regressions, EN/RU/EN exact MCP baseline and catalog/ID tests cover the
small production change. No engine/kernel/shared data structure or simulation
behavior changed, so another full CUDA EngineTests run was not warranted.

The Python contracts check all numeric tokens, channel indices and shortcuts
against the originals; the project-parser tests check Markdown and anchors.
The 124 protected paths under English docs, help, MCP tools, network and locales
match base Git blobs, respecting checkout line-ending normalization. In particular,
all 47 original documentation assets (19 Markdown and 28 PNG) are unchanged.
The MCP runtime test checks the original schema, tool description, exact serialized
17-chapter contents and hashes of all returned English chapters in EN/RU/EN,
including fragment requests and continued exclusion of format-reference.md.

`AGENTS.md` retains skip-worktree and SHA-256
`090c381a6f2b1f62597108de236e53131559ffef796146011a5796dbb7ef3368`.
User data, settings and dependencies are not edited. No submodule update or
CodeQL scan was run. Main remains at the stated base; only the feature branch is
committed/pushed.

## GitNexus

Pre-edit upstream impact was run for the documentation resources, reader and test
fixture. The Markdown file-path target was not resolved (UNKNOWN), so GUI/MCP
resource consumers were confirmed by text and code inspection; change detection
does enumerate Markdown section symbols. Reader class
impact was LOW; the specific getGuiChapters declaration was LOW with four
reachable GUI methods: loadChapters, initIntern, reloadLanguage, processBackground,
and no enumerated processes. Its implementation and the test fixture returned
UNKNOWN; these were not treated as proof of no callers. The fixture's TEST_F
registrations and CMake target, and GUI reader call sites, were checked directly.

The index was refreshed with `node .gitnexus/run.cjs analyze --index-only`.
Analyzer limits include excluded large vendor files, callable candidate caps,
receiver/dispatch and cross-language gaps, and bounded execution-flow enumeration.
These limits remain applicable even if change analysis reports no affected flows.
Final staged `detect_changes(scope="all")` and the CLI fallback both report 11
changed files, 83 changed symbols, zero enumerated affected processes and LOW risk.
The structured MCP result contains all 83 symbols, including getGuiChapters,
the real-resource test helpers and Russian section symbols, without partial or
truncated flags. The human CLI display abbreviates its symbol listing; completeness
was checked against the structured response. Zero enumerated processes does not
remove the explicitly confirmed GUI dependencies or analyzer limits above.

## Source issues and remaining work

1. The shapes paragraph lists 12, 16, 16, 19, 20, 27, 19, 16 nodes, while the
   embedded image labels read 8, 16, 16, 19, 16, 25, 13, 20. The Russian paragraph
   faithfully retains the original values; neither English text nor image is
   silently corrected. This is an existing illustration/document mismatch.
2. The neural velocity input is described simply as speed limited to 2. In
   `NeuronProcessor::calcTelemetryInput` it is actually `min(2, length(vel) * 20)`.
   The translation preserves the original level of explanation; a future source
   clarification should document the scaling in both languages.
3. Constructor auto-trigger documentation describes every n steps; the actual
   `isAutoTriggered` interval is at least `TIMESTEPS_PER_CELL_FUNCTION`, with
   creature-dependent phase and cycle scheduling. The original description is
   retained; exact timing beyond cycle resolution should not be inferred.
4. Simplified UI drawings contain placeholder controls and English labels. They
   must not be used as evidence that a current Russian control has the same
   literal text. All eight artwork decisions above remain visible limitations.

L7-C remains five full chapters: cell-types.md, energy.md,
simulation-parameters.md, glossary.md and how-life-works.md, with their terminology,
energy/signal semantics and illustrations. L7-D remains sandbox.md, evolution.md,
generative-art.md, ai-agents.md, files.md, troubleshooting.md and format-reference.md,
plus final review/localization or justified retention of the 28-image collection.
No content from those stages is translated in this branch. Installer-specific
packaging, external-browser dispatch, manual visual acceptance and a new GPU
execution of the tutorial remain outside this validation.
