# L7-D complete Russian F1 documentation validation

Date: 2026-10-26 (Europe/Moscow).
Base and unchanged main: `27d9d82a2c4a5a506db5947c8464ab76f406b413`.
Branch: `feat/ru-localization-l7d`.

## Scope and status

The seven final translations are `sandbox.md`, `evolution.md`,
`generative-art.md`, `ai-agents.md`, `files.md`, `troubleshooting.md` and
`format-reference.md` under `resources/docs/ru/`. All nineteen English Markdown
filenames now have Russian counterparts, including contents. The GUI exposes
eighteen content chapters; contents supplies the navigation metadata. MCP still
exposes the original seventeen English chapters and excludes the format reference.
The original Russian contents bytes, chapter order and three part names are retained.

This is an implementation and validation delivery, **not closure of L7** or final
visual/release acceptance. L8 is not started. Production C++, parser, renderer,
locales, CLI, MCP schemas/handlers, engine and dependencies are unchanged. Existing
Russian chapter changes are restricted to image captions. Previous validation
reports and their evidence are retained unchanged.

All new user prose, table cells, instructions, warnings, example agent prompts,
captions and headings are translated. Machine tool identifiers, URLs, CLI flags,
commands, extensions, transport names, indices, probabilities and experiment
numbers remain unchanged. Natural-language text inside the format reference's
indented demonstration code is translated; executable commands remain exact.
Heading count, order and levels match the originals throughout all nineteen files.

Two explicit technical qualifications are added without changing English originals:
the evolution energy cycle is restricted to ordinary closed evolution with finite
pools and no Free construction or external edits (C03); the format reference
distinguishes ASCII lowercasing from preserved Cyrillic case and documents the
English aliases implemented by F1. These are reported clarifications, not silent
repairs of the original documents. No earlier C issue is declared resolved.

## Terminology and technical commands

The roadmap, normative terminology and L7-A/B/C reports were read. Materials keep
the distinction between a solid particle, fluid, free cell and creature cell.
Cell names include Конвертер and Донор; energy terms distinguish raw, usable,
stored and reserved energy. Происхождение and Снимок мира are retained. New UI
instructions were compared with current English/Russian catalogs; Python contracts
check selected actual captions, including all eight construction shapes through
the established terminology, world tools, energy inflows, mutation controls,
rendering modes, autosave, display and MCP controls.

`source/Apps/Alien-cli/CommandLineParser.cpp`, `main.cpp`, `SimulationRunner.cpp`
and `ConsoleOutput.cpp` supply the CLI behavior: input/output, optional step limit,
Q/Ctrl+C shutdown, login, paired upload options, minimum one-minute upload interval,
debug tracing and plain output. The documented command
`alien-cli -i input.sim -o output.sim -t 100000` is unchanged. No real user simulation
was opened or run and no cloud upload was performed. Docker publishing is supported
by the repository workflow, rather than a live registry availability claim.
CLI localization remains L8-A work.

Every backtick tool name in the AI chapter is checked against actual
`.name = ...` registrations in `source/McpTools/Impl/*.cpp`, and the complete
tool-name sequence is identical in EN/RU. Natural-language examples are goals for
an agent, not additional tools or guarantees of successful creature design.
`mass_edit_genomes` supports filters, scale edits and preview, while
`summarize_genomes` supports lineage summaries; these back the selective genome
examples. `take_screenshot`, view/inspection, creator, temporal and parameter
tools back the other prompt families. No live external agent was connected.

`source/Network/McpServer.cpp` binds `127.0.0.1`, serves `/mcp`, and validates
local origins/hosts. The port default is 8765. The Claude Code command and its
arguments are byte-identical; HTTP and Streamable HTTP are retained. These are
repository-contract checks, not a claim that every third-party MCP client version
has been exercised. There are no JSON examples in these seven English chapters;
no JSON structures or machine parameters were invented or translated.

Troubleshooting was checked against `SimulationCudaFacade.cu` (CUDA capability
gate), `VulkanContext.cpp` (device selection, optional ray tracing and cross-device
host copy), `Resources.cpp`, settings/serialization, autosave, browser/network and
MCP code. Windows AMD executable packaging is confirmed by
`.github/workflows/windows-nightly-deploy.yml`; the HIP path does not impose the
CUDA capability gate. RDNA family/driver support is not newly hardware-certified.
Existing recommendations that exceed what code proves are recorded below.

The generative-art account describes rendering effects, not physical organs or
light affecting simulation energy. `SimulationRenderer.cpp`, `VulkanContext.cpp`
and the ray-tracing shaders implement the optional tissue/gas/light presentation;
`ParameterCalculator.cuh` and `ForceFieldKernels.cu` implement actual fields.
Palette modes, linear/radial/Perlin fields, circular/rectangular layers, shapes and
source-light controls retain their respective meanings. No rendering/physics code
was changed to make the prose true.

## Complete F1 checks

The extended `DocumentationLocalizationTests` uses the actual `MarkdownParser`,
`DocumentationReader`, `DocumentationWindow`, native `AlienWindow` processing and
Dear ImGui. It does not substitute a Python Markdown parser. All nineteen pairs
are compared for block variant/count, ordered heading levels, list nesting and
numbering, table dimensions/alignment, note structure, links and fenced code.
Only the translated natural-language code demonstration has a narrow exception;
the command examples must match exactly.

All supported internal links in both trees are opened through F1 in EN/RU/EN.
Every English heading alias and native Russian anchor is resolved, including
duplicate headings and suffixes. All eighteen RU content chapters load from
the Russian root without English fallback. The format reference remains reachable
through contents, navigation, search and chapter buttons without expanding MCP.

Every shipped Russian section title is searched and must return that section in
that chapter. Additional real-resource checks compare upper/lower Russian queries
including Ё/ё, with valid UTF-8 snippets; existing tests retain Е/Ё distinction,
multi-term AND, ranking and invalid-UTF8 cases. Search is retained during language
switches on each shipped chapter, and all resulting indices/snippets remain valid.

All eighteen content chapters are rendered and navigated at 100/150/200%, in
EN/RU/EN. Each anchor scroll request completes and scrolling stays bounded/stable;
window identity and dimensions survive switches. The format reference exercises
actual lists, nested list code, aligned tables, notes, images, links and code blocks.
Existing fixture tests cover window/search/navigation IDs, previous/next mouse
input, section restoration, navigation width and startup settings.

A new test makes every one of the twenty-nine unique PNG/JPEG/logo resources
visible individually in both languages. It passes actual encoded bytes through
the production renderer, checks metadata, observes one texture load, resets and
requires balanced texture deletion after each image. The existing tests cover
cache reuse, negative-cache invalidation, translated/shared roots and hide/reopen.
The rendering facade is a double: this establishes path, encoded-byte and lifetime
contracts, **not real Vulkan decoding/allocation/presentation**. Automated ImGui
layout is not exhaustive manual visual acceptance or proof of legibility of every
word. A separate manual Vulkan/typography pass remains an L9 acceptance requirement.

The English MCP baseline test compares exact contents, description, schema and
all seventeen chapter response hashes through EN/RU/EN, including fragment
requests. Protected-path hashes additionally compare the English assets and
machine implementations with the specified base, using Git checkout normalization.
English documents, original image bytes and MCP contracts are unchanged.

## Image inventory and decision policy

The complete machine-readable inventory is
[`localization-l7d-images.json`](localization-l7d-images.json). It records original
paths, decoded dimensions, actual format, SHA-256, every EN/RU chapter reference,
embedded-language status, decision and repository provenance. All resources were
visually inspected in readable contact sheets; crowded/clipped cases were also
inspected at original resolution. No generated image or replacement screenshot is
presented as current UI evidence.

**Correction to earlier inventory totals:** `resources/docs/images` has twenty-five
PNG and three JPEG files, twenty-eight files in total, not twenty-eight PNG files.
The additional `resources/images/logo.png` makes twenty-nine unique used resources
(twenty-six PNG and three JPEG). Thus JPEG and logo inspection is included, not
assumed from the old PNG count.

All twenty-five diagrams have embedded English text. Four resources (three JPEG
simulation views and the logo) have no embedded language and are justified reuse.
**Zero diagrams are claimed as localized artwork.** No verified editable diagram
source was found in the repository. Flat raster labels, crowded text, clipped text,
and disputed counts/mechanisms prevent a certified typography-only redraw in this
delivery. A generative reconstruction cannot establish exact preservation of every
node, arrow and signal connection. Each retained diagram now has a detailed Russian
caption explaining the semantic relationships and its specific limitations; original
English bytes, numbers, attribution and embedded metadata remain intact.

This is documented retention, not a claim that label localization is complete.
L9 should obtain/recover source artwork or approve a separately verified redraw,
starting with injector, shapes, memory, energy and creature-module semantics.
The table below assesses current conceptual correspondence; it does not certify
that schematic placeholder controls are current application screenshots.

| Original path | Actual format / pixels | Chapters (EN and RU) | English labels | Current correspondence and decision |
| --- | --- | --- | --- | --- |
| `resources/docs/images/art.jpg` | JPEG 1440 x 600 | generative-art.md | No | No labels; artwork simulation view, not a UI instruction. No arrows/count claims; retain unchanged. |
| `resources/docs/images/cell-attacker.png` | PNG 1440 x 540 | cell-types.md | Yes | Tagging, loot direction and neighboring digestor agree; bottom labels overlap. Exact typography repair needs source, so retain with detailed RU sequence. |
| `resources/docs/images/cell-base.png` | PNG 1440 x 420 | cell-types.md | Yes | Sensor to base to muscle signal arrows agree. Default neighbor-copy behavior. Retain flat raster with RU label/relationship explanation. |
| `resources/docs/images/cell-communicator.png` | PNG 1440 x 520 | cell-types.md | Yes | All-eight-channel arrow and direction reference-frame conversion agree; range circle illustrative, filters/one-way gating omitted. Preserve exact geometry; RU explanation supplied. |
| `resources/docs/images/cell-constructor.png` | PNG 1440 x 540 | cell-types.md, genomes.md | Yes | New node inserted next to constructor; node zero farthest away. Module, not extra cell type. Exact order/arrows retained; RU explanation supplies labels pending editable source. |
| `resources/docs/images/cell-defender.png` | PNG 1440 x 460 | cell-types.md | Yes | Per-defender divisor agrees with anti-attack mechanism. Labels overlap near attacker; does not describe anti-injection. Retain until separate typography repair. |
| `resources/docs/images/cell-depot.png` | PNG 1440 x 480 | cell-types.md | Yes | Store/release directions and two units per cycle agree. Embedded reserve means depot storage, not construction reserve. Retain; RU caption distinguishes pools. |
| `resources/docs/images/cell-detonator.png` | PNG 1440 x 540 | cell-types.md | Yes | Countdown/blast/outward wave schematic; not scale-accurate radius and exact zero timing is simplified (C14). Preserve numbers pending source revision. |
| `resources/docs/images/cell-digestor.png` | PNG 1440 x 460 | cell-types.md | Yes | Raw to digestor to usable, direct-neighbor connection agrees. RU explains Конвертер and energy forms; retain topology pending editable source. |
| `resources/docs/images/cell-generator.png` | PNG 1440 x 480 | cell-types.md | Yes | Square and sawtooth channel-zero signals, period/min/max; no energy generation. Discrete phase simplified. Retain and explain in RU. |
| `resources/docs/images/cell-injector.png` | PNG 1440 x 500 | cell-types.md | Yes | I02: foreign-constructor genome arrow agrees with InjectorProcessor; right sentence clipped. Do not invent missing typography. Retain with code-backed RU explanation. |
| `resources/docs/images/cell-memory.png` | PNG 1440 x 500 | cell-types.md | Yes | I03/C01: four mechanisms, sign-only recorder labels omit strict thresholds; running average is weighted exponential smoothing. RU explains both; literal label replacement would preserve a disputed simplification. |
| `resources/docs/images/cell-muscle.png` | PNG 1440 x 540 | cell-types.md | Yes | Three mechanical families/pivot and strength/direction agree; does not depict all six control modes. Retain original geometry and RU explanation. |
| `resources/docs/images/cell-reconnector.png` | PNG 1440 x 460 | cell-types.md | Yes | Positive connect/negative release and solid mesh agree; own-creature links retained on release. English solid means particles. Preserve topology, explain in RU. |
| `resources/docs/images/cell-sensor.png` | PNG 1440 x 600 | cell-types.md | Yes | Rays/front/angle/occlusion and channels zero to three agree. Sample mass/distance values are illustrative, not measurements of drawn organism. Preserve rays and values pending editable source. |
| `resources/docs/images/cell-void.png` | PNG 1440 x 480 | cell-types.md | Yes | Temporary scaffold vanishes and transfers energy to neighbors; not energy destruction. RU names Донор. Preserve topology pending source. |
| `resources/docs/images/creature-schematic.png` | PNG 1440 x 760 | how-life-works.md | Yes | I05: signals/bonds/genome relationship agree, but constructor appears as a separate type and no growing cells drawn. RU clarifies module semantics. Requires semantic layout review, not silent label replacement. |
| `resources/docs/images/energy-flow.png` | PNG 1440 x 864 | energy.md | Yes | I04/C03/C05/C06: conceptual cycle only; storage/reserve, infinite/Free/editor exceptions and backflow constraints missing. Full repair requires new topology and source approval; RU caption explicitly qualifies it. |
| `resources/docs/images/evolution.jpg` | JPEG 1440 x 640 | evolution.md | No | No labels; illustrative evolved population, not proof of preset values or repeatable outcomes; retain unchanged. |
| `resources/docs/images/genome-editor-layout.png` | PNG 1440 x 800 | first-creature.md | Yes | Toolbar/genome/tree/node/preview relationships agree; full-resolution inspection confirms six listed and six preview nodes. Placeholder values/control layout are schematic. Retain English source with detailed RU column explanation. |
| `resources/docs/images/genome-structure.png` | PNG 1440 x 760 | genomes.md, how-life-works.md | Yes | Genome/gene/node hierarchy, constructor reference arrows and root offspring agree. Leading sensor is not the default tutorial copying order. Exact arrow topology needs editable source; retain and explain. |
| `resources/docs/images/neural-network.png` | PNG 1440 x 840 | neural-networks.md | Yes | Sixteen inputs/twelve outputs, memory and telemetry layout and formula retained. Memory gate omitted; velocity description lacks scaling (B02). Dense connection topology must not be regenerated without a verified source. |
| `resources/docs/images/particles-to-ecosystems.png` | PNG 1440 x 560 | getting-started.md | Yes | Hierarchy particles/cells/creatures/populations agrees conceptually; not a literal scale sequence. Flat raster, no verified source; retain English diagram with RU relationship explanation. |
| `resources/docs/images/sandbox.jpg` | JPEG 1440 x 576 | sandbox.md | No | No labels; physics scene of matter/fluid/energy. No controls or quantitative claims; retain unchanged. |
| `resources/docs/images/shapes.png` | PNG 1440 x 912 | genomes.md | Yes | I01/B01: text and raster node counts differ. Shape order retained; do not choose new counts without exact genome reproduction. Localized redraw needs reconciled source. |
| `resources/docs/images/signal-flow.png` | PNG 1440 x 500 | how-life-works.md | Yes | Node four towards zero, one connection per cycle, agrees with default copying. Separate five-node example, not the tutorial creature. Retain indices/arrows and explain in RU. |
| `resources/docs/images/tutorial-hunter.png` | PNG 1440 x 652 | first-creature.md | Yes | Five muscles, sensor at node five, food/signal/motion arrows agree. Retain all indices/directions; detailed RU caption pending editable source. |
| `resources/docs/images/ui-layout.png` | PNG 1440 x 840 | user-interface.md | Yes | Menu/view/windows/edit-button/tools callouts are conceptual. Placeholder controls are not a current UI capture; retain with RU callout explanation until a verified localized layout is available. |
| `resources/images/logo.png` | PNG 399 x 399 | format-reference.md | No | No labels; original ALIEN mark. No UI/mechanism claim; retain unchanged. |

## Nineteen-file audit

In the table, links/anchors are checked by the real parser and F1, not inferred
from filenames. Image counts are references per chapter in either language;
shared assets count again where used. “Captioned EN” means English raster labels
remain and the Russian explanation explicitly states the limitation.

| Canonical file | EN / RU | Translation | Links / anchors | Images | Image language decision | Remaining content issues |
| --- | --- | --- | --- | --- | --- | --- |
| ai-agents.md | Present / Present | Complete text | Passed, both trees | 0 | No images | Agent design outcomes/client compatibility not guaranteed. |
| cell-types.md | Present / Present | Complete text | Passed, both trees | 15 | Captioned EN | C01/C02/C04/C11–C16/C20; I02/I03 and raster typography. |
| contents.md | Present / Present | Complete text | Passed, both trees | 0 | No images | TOC unchanged; GUI-only format link retained. |
| energy.md | Present / Present | Complete text | Passed, both trees | 1 | Captioned EN | C03–C06/C09/C10/C12; I04. |
| evolution.md | Present / Present | Complete text | Passed, both trees | 1 | Reuse: no embedded text | C03 locally qualified; C15/C19 and D06 remain open. |
| files.md | Present / Present | Complete text | Passed, both trees | 0 | No images | No CLI localization; no live registry/upload run. |
| first-creature.md | Present / Present | Complete text | Passed, both trees | 2 | Captioned EN | B03 scheduling; B04 schematic UI. |
| format-reference.md | Present / Present | Complete text | Passed, both trees | 1 | Reuse: no embedded text | D05 locally qualified; EN source unchanged. |
| generative-art.md | Present / Present | Complete text | Passed, both trees | 1 | Reuse: no embedded text | C18 color/age semantics; visual effects do not assert new physics. |
| genomes.md | Present / Present | Complete text | Passed, both trees | 3 | Captioned EN | B01/I01 shapes; C15/B03 scheduling. |
| getting-started.md | Present / Present | Complete text | Passed, both trees | 1 | Captioned EN | B04 image typography. |
| glossary.md | Present / Present | Complete text | Passed, both trees | 0 | No images | C16 donor summary. |
| how-life-works.md | Present / Present | Complete text | Passed, both trees | 3 | Captioned EN | C15/C16; I05. |
| keyboard-and-mouse.md | Present / Present | Complete text | Passed, both trees | 0 | No images | No new substantive issue found. |
| neural-networks.md | Present / Present | Complete text | Passed, both trees | 1 | Captioned EN | B02 speed scaling; gate omitted from diagram. |
| sandbox.md | Present / Present | Complete text | Passed, both trees | 1 | Reuse: no embedded text | D01 static velocity; general physics prose simplified. |
| simulation-parameters.md | Present / Present | Complete text | Passed, both trees | 0 | No images | C02/C05–C08/C10/C11/C17–C19. |
| troubleshooting.md | Present / Present | Complete text | Passed, both trees | 0 | No images | C13/C15; D02–D04 recommendations need qualification. |
| user-interface.md | Present / Present | Complete text | Passed, both trees | 1 | Captioned EN | B04 schematic UI; no current screenshot claim. |

## Unified unresolved register

The following C01–C20 rows are copied from [L7-C](localization-l7c-validation.md#english-source-discrepancies-and-proposed-corrections) without weakening their original
assertions, code evidence or proposed corrections. All remain open. Classifications
separate translation defects from original-document issues, possible engine defects
and image limits. Some issues span more than one class; classification is not a
diagnosis that the engine must change.

- Translation errors: none demonstrated by the executed contracts/review; this is
  not a guarantee against every linguistic defect. Any future correction must use
  the normative terminology and actual control captions.
- English-source issues: C01–C20 and B01–B03 below, plus D01–D06. The RU evolution
  qualification mitigates C03 locally, but EN, energy chapter and schema remain open.
- Possible engine issues requiring separate adjudication: C02 (unused configurable
  anti-injection strength), C12 (source/target low-energy condition), C13 (signed
  muscle cost). No engine fix or new GPU behavioral proof is claimed.
- Image issues: all twenty-five English diagrams, B01/B04 and I01–I05 below. They
  are preserved limitations, not resolved by translated captions.

| ID | Original assertion | Confirmed code behavior | Proposed source correction |
| --- | --- | --- | --- |
| C01 | Memory recorder starts at “at least 0.1” / “at most -0.1”; read-only playback also says at least. | `MemoryProcessor::processSignalRecorder` uses strict >0.1 and <-0.1, including read-only absolute comparison. Other manual triggers use >=. | Specify strict inequalities specifically for recorder initiation; recording/playback then follows its state machine. |
| C02 | Anti-injector strength parameter increases injection costs per defender. | `InjectorProcessor::processCell` uses baseCost*(1+defenderCount*0.5), not defenderAntiInjectorStrength. The configurable value exists but is not consumed here. Equality with minimum energy is allowed, contrary to “stays above”. | Document fixed current behavior and >= minimum; decide separately whether to wire the parameter into the engine. |
| C03 | “The energy inside the world is conserved” with external pool as the only exception; total internal+external remains constant with a pool/backflow. | Ordinary transfers account for energy, but Free constructors create cells without debiting energy. Infinite external stock is a sentinel and cannot form a finite conserved total. Editor creation/removal and parameter changes also change the accounting boundary. | Limit conservation to finite pools and ordinary closed evolution with no free construction or external edits; distinguish object/biological energy from kinetic energy. |
| C04 | All absorbed energy “becomes raw energy”, including a free cell. | `EnergyProcessor::absorbIntoObject`: cell.rawEnergy versus freeCell.energy; free cells do not have a raw-energy field. UnderConstruction cells are excluded. | Distinguish object energy for free cells and mention construction-state exclusion. |
| C05 | Backflow occurs as long as the pool holds less than the limit. | `createEnergyParticle` rejects the entire proposed return when oldPool+return > limit. It does not fill just the remaining capacity. | State the resulting-balance check and all-or-nothing return. |
| C06 | Source inflow is described as energy per step without its color/share qualification. | Each active source requests inflow[color]*sourceRelativeStrength. Sources disabled by their center coverage are omitted; omitted shares are not renormalized. Finite source requests are atomic and may receive only remaining energy, unlike proportional constructor allocation. | Describe per-color rates, source weights, inactive shares and finite depletion separately. |
| C07 | “If layers overlap, the later layer ... takes precedence” appears unqualified; all non-overridden parameters “keep the base value”. | Numeric blending is sequential over enabled layers. A disabled override preserves the accumulated earlier value, not necessarily base. Color transitions use `getFirstMatchingLayerOrBase`, selecting the first fully weighted layer. | Qualify later-layer precedence for interpolated values, preserve earlier accumulated values and explicitly document the discrete first-match rule. |
| C08 | Disable radiation sources switches off “all radiation sources inside the layer”; fade-out radius names a literal fading width. | `calcActiveSources` checks the source center; `isCoveredByLayers` requires calcWeight < NEAR_ZERO, effectively full-opacity core coverage. Fade denominator is configuredFadeout+1. | Document center/full-coverage semantics and effective fade formula rather than any overlap. |
| C09 | Energy particles “fly in a straight line” until absorption/reflection. | `cudaApplyForceFields` also accelerates energy particles. Reflection is tested against connections between solids. | Add the force-field exception and identify the reflecting boundaries. |
| C10 | Radiation fractions are emitted “per time step”; type I applies when older than minimum. | `ObjectProcessor::radiation` samples with probability 0.03 and rescales by random*2/0.03, capped to leave at least 1 in each energy component. Type I checks age > minimum; type II tests usable/raw separately. Free cells also radiate; static objects do not. | Explain expected-rate/stochastic semantics, strict age, components and exclusions. |
| C11 | Digestor “Max raw energy conductivity” is an upper limit for intake/forwarding. | `performEnergyFlow` scales each participating conductivity by maxConductivity*3, combining both digestors' contributions; it selects one connection per object per step and caps receiving raw energy eligibility at <100. Conversion itself scales maxConversion by six per cycle. | Specify units, selected-connection schedule and actual scaling; do not infer that 3 is an absolute per-cycle intake cap. |
| C12 | Usable energy flows from rich to poor; surplus goes to constructors. | `performEnergyFlow` limits each transfer to 2; current `connectedObjectLowEnergy = lowEnergy(object)` tests the source rather than the named connected object. | Document the exact limited transfer policy after separately reviewing whether this source/target condition is intended. |
| C13 | Muscle costs are activation-scaled; automatic bending “needs a positive signal”. | `radiate` charges only when cost*activation*6/3 >0; negative activation is not charged. Auto bending accepts signed clamped activation and uses it in stroke reversal; positive steady drive is a practical recommendation. | State signed-cost behavior and distinguish recommended input from a hard activation gate. Negative direct movement also reverses direction. |
| C14 | Detonator explodes “at zero” and nearby detonators “explode as well”. | Countdown decrements when >=0 and detonates on -1; a chained detonator is activated with countdown 1. | Describe trigger/decrement timing and chain delay precisely. Eight-radius shock-wave reach agrees with code. |
| C15 | Constructor auto-trigger “every n time steps”; all built cells stay inactive “until the last cell is built”. | Interval is max(6,n), with creature-ID phase and cycle scheduling. Last-node completion initiates BeingActivated propagation before readiness. | Retain L7-B's timing caveat; describe activation as propagation after completion, not simultaneous execution. |
| C16 | How-life summary says “Any cell can additionally carry a constructor”; glossary donor only says placeholder. | Reference section, genome validation and normative glossary exclude donor constructors; donors give energy to neighbors before dying. | Qualify the introductory “any” with the donor exclusion and expand the short glossary definition. The detailed Russian reference already explains both. |
| C17 | Parameter chapter promises “every parameter”; blank defaults for names/relative source strength are left unspecified. | Current spec also exposes Inner friction (0.6); object coloring also has Lineage + Customization. Initial names have placeholders, new source strength is 0. | Call this a selected reference or add omitted rows/defaults in a separately coordinated structural revision. No new rows were inserted into the translation. |
| C18 | Color retained “for the duration” then changes; static objects are described as protected from aging. | `aging` skips static/solid/fluid objects, increments age first, and changes color only for duration >0 and age >duration, then resets age to 0. | Specify the strict comparison and static exclusions. This is the same age used by lifetime checks. |
| C19 | All meta-mutation parameters are zero by default. | All sigmas are zero; the customization transition matrix in the same table is a predefined boolean permission matrix, not a zero sigma. | Limit “all” to the sigma values and identify the matrix separately. |
| C20 | Overview table has empty Reads cells for attacker/injector/detonator. | Their trigger consumes channel 0, as their detailed sections and `CellFunctionChannels` show. | Populate trigger reads in a coordinated EN/RU source table revision. Existing cell count/column structure is retained here. |

[L7-B observations](localization-l7b-validation.md#source-issues-and-remaining-work), numbered B01–B04 in the same order, preserved verbatim:

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

| ID | Class | Evidence and open decision |
| --- | --- | --- |
| D01 | English source | Sandbox “Static objects stay in place” is too broad. `ObjectProcessor::verletPositionUpdate` still adds velocity for static objects; `verletVelocityUpdate` skips acceleration. `inspection.attribute.static` correctly conditions remaining in place on zero velocity. RU preserves the original claim; separately qualify it in both languages. |
| D02 | English source | Troubleshooting requires the primary monitor on the compute GPU and says compute/render must use the same GPU. `VulkanContext::selectPhysicalDevice` prefers a matching UUID but supports a different rendering device with host-memory transfer. Treat this as a diagnostic/performance suggestion, not a universal launch requirement. |
| D03 | English source / environment limit | Blanket prohibition on non-English installation paths/usernames is not established by a full Unicode-path audit. The actual image loader handles a Cyrillic path in tests. Other I/O paths are not thereby certified; retain recommendation and separately audit affected loaders before removing it. No new system requirement is invented. |
| D04 | English source / environment limit | AMD RDNA2+ and latest-driver guidance are retained from the source and workflow targets. This CUDA validation does not certify every AMD GPU/driver or the installed nightly package. Latest live driver/registry/Discord availability is not tested. |
| D05 | English source | Format reference's unqualified GitHub/lowercase anchor rule does not describe Cyrillic: `MarkdownParser::toAnchor` lowercases ASCII bytes but preserves UTF-8 Cyrillic. RU now explains native case and GUI English aliases; English source remains unchanged/open. |
| D06 | English source / verification limit | Evolution preset values and reported evolved rate ranges are source examples, not newly reproduced long-running experiment results. All values and preset names are preserved. The current repo does not supply a controlled provenance/reproduction run for each remote preset version. |
| I01 | Images | Shapes: paragraph counts 12,16,16,19,20,27,19,16 versus image labels 8,16,16,19,16,25,13,20. Neither is silently chosen as truth. Reproduce each exact genome/shape before revising EN/RU text and diagram together. |
| I02 | Images | Injector right-hand sentence is cropped in the raster. `InjectorProcessor::processCell` confirms foreign-constructor genome replacement/restart, which RU explains, but cannot recover the missing original typography. Obtain source or approve a distinct redesigned figure. |
| I03 | Images / English source | Memory uses sign-only recorder shorthand and “running average”. `MemoryProcessor::processSignalRecorder` and `processIntegrator` establish strict thresholds and weighted smoothing. RU caption explains them; revise labels separately after source approval. |
| I04 | Images / English source | Energy flow omits stored/reserved pools and finite/infinite/Free/editor exceptions. Constructor, external-inflow, release/backflow paths are cited in C03/C05/C06. A complete replacement needs new nodes/arrows; preserve it as a simplified diagram, not an energy-conservation proof. |
| I05 | Images | Creature schematic places Constructor as though a cell type and does not draw its growing cells. `CellDesc`/genome data and constructor processing establish an attached module; RU caption says so. Separate redesign must make module/type distinction visible. |

## Build, tests and preservation

Windows Release uses `build-windows-ninja.bat agent Release 16`, exclusively
`build-agent`. The first Python resource check correctly detected stale copied
resources: the no-op/application-unlinked build did not update the new Markdown.
The generated `build-agent/Release/alien.exe` was removed and relinked using the
same script, invoking the existing CMake POST_BUILD copy. Source/output SHA-256
is checked for all thirty-eight Markdown files and all twenty-nine used image
resources under both `build-agent/resources` and `build-agent/Release/resources`.
No IDE tree was configured, built into or deleted.

| Check | Result |
| --- | --- |
| Windows agent Release build and forced application relink | Passed |
| BaseTests | 44 passed |
| DataTests | 247 passed |
| EngineInterfaceTests | 2 passed |
| McpToolsTests | 40 passed |
| NetworkTests | 38 passed |
| PersisterTests | 142 passed, one existing disabled test |
| GuiLocalizationTests | 162 passed, including 12 documentation tests |
| AccountLocalizationTests | 6 passed |
| McpLocalizationTests | 7 passed |
| Release-working-directory documentation tests | 12 passed |
| check_localization_l7b.py -v | 5 passed |
| check_localization_l7c.py -v | 5 passed |
| check_localization_l7d.py -v | 4 passed |
| alien-cli.exe --help | Passed; real documented flags present |
| clang-format 19.1.5, modified C++ test | Passed |
| git diff --check | Passed |

Only caption/UI wording and report edits followed the full-suite pass. The final
resource-copy and documentation/Python checks are repeated after those edits;
no implementation change followed the full regression. Logs remain under the
ignored `build-agent/l7d-validation` directory, not in the release resource tree.

The only initial Python content failure was its overly broad exact-code rule for
the translated “Code inside a list item” demonstration. It now permits precisely
that translated sample, while the CLI, Claude and EngineTests commands remain exact.
There was no parser or application failure behind that contract adjustment.

No engine/shared production dependency changed, so the full GPU EngineTests suite
was not repeated. The previous L7-A GPU evidence is historical, not a new L7-D run.
No live simulation, user genome, autosave, account or settings was opened/modified.
GUI tests use process-isolated temporary registry keys (`SettingsTestScope`) and
temporary document trees. `AGENTS.md` remains byte-identical, SHA-256
`090c381a6f2b1f62597108de236e53131559ffef796146011a5796dbb7ef3368`,
with `S AGENTS.md` skip-worktree. Dependencies and pinned vcpkg are unchanged.
No submodule update or CodeQL scan was run. Main stays at the stated base.

## GitNexus analysis

Upstream impacts before editing: DocumentationWindow LOW (five impacted symbols,
three direct), MarkdownParser LOW (two direct); no enumerated processes for either.
The documentation test fixture, Python contract classes and Markdown resource paths
returned UNKNOWN/unresolved, not a safe-zero conclusion. `TEST_F` registrations,
`source/Gui/Impl/Tests/CMakeLists.txt`, reader/window loading and report/Python
entry points were confirmed directly. No production symbol changed.

The initial index refresh completed in 336.3 seconds with 731,106 nodes, 952,690
relationships and 432 enumerated flows. One concurrent impact encountered a locked
database and was retried successfully after the refresh. Analyzer limits remain:
large vendor files excluded, callable candidate caps, receiver/cross-language gaps
and bounded entry/flow enumeration. Zero enumerated flows cannot establish absence
of GUI/resource consumers. The server also retains the historical two-commit
staleness hint after index-only refreshes; actual indexed RU/test symbols and CLI
analysis supplement it.

Final index refresh completed in 332.6 seconds: 731,214 nodes, 952,897
relationships, 6,468 clusters and 432 enumerated flows. The log also records
11 malformed/over-bound stored sites dropped for one file; those facts are not
claimed available. Final staged MCP `detect_changes(scope="all")` and CLI
`detect-changes --scope all --repo .` both report **21 files, 127 changed symbols,
zero enumerated affected processes, LOW risk**. The complete MCP JSON contains
all 127 entries, without partial/truncated flags; the human CLI listing abbreviates
its display. The existing analyzer process-enumeration limits still apply. Minor
caption punctuation/UI wording and this results paragraph do not introduce any
production-code change. The final diff is directly reviewed in addition to graph
analysis.

## L9 acceptance recommendations

1. Review every chapter in a real Vulkan application at 100/150/200%, including
   long image captions, widest parameter/tool tables, code blocks and notes;
   approve typography, contrast, clipping and practical window sizes manually.
2. Confirm all JPEG and logo textures visually, language switching with search and
   scroll, duplicate headings and external-browser dispatch on the release package.
3. Obtain editable diagram sources or approve accurate separately verified redraws.
   Retain original provenance. Do not mark the twenty-five English-labelled diagrams
   localized until labels, arrows, numeric values and meaningful geometry are reviewed.
4. Adjudicate C/B/D source issues and potential engine defects separately; coordinate
   English/Russian corrections and targeted engine tests where behavior changes.
5. Validate installer/archive packaging of both language trees and all images on a
   clean installation. Agent Release copying is verified; installer acceptance is not.

Final L7 acceptance/closure requires a separate decision. This delivery does not
merge into main, close the overall stage or begin L8.
