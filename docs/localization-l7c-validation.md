# L7-C Russian cell, energy and parameter reference validation

Date: 2026-10-26 (Europe/Moscow).
Base: `3bc6977fb3d5d62a3df7545191a74717beae717d`.
Branch: `feat/ru-localization-l7c`.

## Scope and delivery status

This implements the five requested L7-C chapters, accepted as recorded below.
The entire L7 stage remains open. L7-D is not started. English Markdown,
all original images, catalogs, production code, MCP, genomes, world parameters,
serialization, user data/settings and dependencies are unchanged.

## Acceptance and closure

Date: 2026-10-26 (Europe/Moscow).

The user accepted implementation
`ff501d8bf0a25909214702cda788f5ad9a667634` based on the automated checks,
review of this report and selective review across all five Russian chapters.
**L7-C is complete. The entire L7 stage remains open; L7-D remains open and
is not started.** This acceptance does not claim exhaustive manual visual review.

The complete C01–C20 register below remains unchanged: original assertions,
confirmed code behavior and proposed clarifications are retained. These
discrepancies are not fixed by acceptance. The separate L7-B observations about
velocity telemetry, constructor timing and the shapes image's node counts also
remain unchanged and unresolved.

Final image-content verification and image localization remain L7-D work,
including embedded English labels and the clipped text in `cell-injector.png`.
All recorded limitations on manual visual review, end-to-end Vulkan textures,
installer packaging and the absence of a new GPU run remain applicable.

This closure edits only this report and the localization roadmap. Production
code, tests and help resources are unchanged from the accepted implementation;
builds and tests are not rerun, and their recorded implementation evidence
remains applicable. The user authorizes a documentation commit and push,
fast-forward-only integration into `main`, pushing `main`, synchronization and
cleanliness verification, and deletion of the integrated feature branch locally
and remotely. `AGENTS.md`, its skip-worktree flag, English documentation, MCP,
user data/settings and dependencies are preserved. No L7-D work is authorized.

The translation follows the roadmap, L3 audit, normative terminology and the
L7-A/L7-B validation records. The existing Russian contents already has the
correct five captions in the canonical order; no contents change is necessary.
The seven L7-B files remain byte-identical to the stated base.

All substantive prose, headings, definitions, examples, lists, notes, table
headers/cells and alternative captions are translated. Canonical filenames,
fragments, channel indices, numbers, signs, shortcuts and technical `Free`,
`Tanh`, `Identity`, `MCP` and `.settings.json` designations are retained. The
glossary keeps the original entry order rather than sorting it by Russian words.

| File in resources/docs/ru | Characters | UTF-8 bytes | Headings | Images |
| --- | ---: | ---: | ---: | ---: |
| cell-types.md | 26,435 | 46,130 | 50 | 15 |
| energy.md | 8,331 | 15,127 | 10 | 1 |
| simulation-parameters.md | 16,400 | 28,672 | 23 | 0 |
| glossary.md | 5,359 | 9,259 | 1 | 0 |
| how-life-works.md | 7,255 | 12,596 | 8 | 3 |
| Total | 63,780 | 111,784 | 92 | 19 |

Counts include Markdown and whitespace. Word counts are not a completeness
criterion across languages. There are 92 headings and 19 image references.

## Terminology and cross-chapter review

The normative type names are Базовая, Накопитель, Сенсор, Генератор, Атакующая,
Инжектор, Мышца, Защитник, Соединитель, Детонатор, Конвертер, Память,
Коммуникатор and Донор. Solid remains Частица (Частицы for search/target modes),
with explanatory prose identifying solid matter. No new translation for the
internal `Free` mode is invented. A generator produces signals; a constructor
is additional cell functionality, explicitly excluding donors in its reference
section. The source's overbroad introductory summary is recorded below.

Energy chapters consistently distinguish Сырая энергия, Доступная энергия,
Запасённая энергия and Зарезервированная энергия. The latter two are respectively
depot storage and construction reserves, not interchangeable names for a pool.
The introductory two-form explanation is a simplification, not an exhaustive
inventory; the seven-row energy table supplies the full inventory.

The glossary's four memory neurons are recurrent network outputs, distinct from
the Память cell type and its signal buffer. Its definitions agree with the
neural chapter's eight signal outputs, four memory outputs, sixteen inputs and
memory gate. The cycle/direction account agrees with the genome chapter and
first-creature tutorial: six time steps per cycle, signal copying towards node 0,
new cells inserted at the constructor and node 0 farthest away. Root-gene
construction plus separation produces offspring; mutation rates belong to the
genome. Повторения, Происхождение and Снимок мира match the existing chapters.

Parameter/action captions were compared with current `resources/locales/en.json`
and `ru.json`, `SimulationParametersGuiText.h`, `GenomeEditorPresentation.cpp`,
`NodeEditorWidget.cpp` and `CellFunctionChannels.cpp`. The contracts compare at
least 65 unambiguous parameter-table names to actual catalog values. This includes
Управляемая подача энергии, Передача энергии конструкторам/источникам,
Порог собственной энергии, Только первому потомку, Возврат энергии,
Предел внешнего запаса, all sigma groups, the six muscle modes and four memory
modes. Genome resistance uses the actual label Защита от инъекций; detonator
states use Готов, Активирован, Взорвался. Combined/shortened labels such as
source Radius/Size retain the reference table's grouping and are manually checked.

## Source verification

These are source-inspection results, not newly executed GPU behavior tests.
Paths below are relative to `source/Engine/Impl/Kernels/` unless stated otherwise.

| Subject | Evidence and checked behavior |
| --- | --- |
| Scheduling and channels | `NeuronProcessor.cuh`, `CellProcessor.cuh`, `source/Data/CellTypeConstants.h`, `CellFunctionChannels.cpp`: eight signal channels; common manual trigger uses absolute channel 0 >= 0.1; readiness excludes UnderConstruction, BeingActivated and a nonzero activation time. Directional functions check initialized front direction. Constructor and memory exceptions are detailed below. |
| Base, depot, generator | `DepotProcessor.cuh`: signed channel 0, up to 2 units per cycle, only surplus over normal energy stored, storage capped by min(configured limit, 500). `GeneratorProcessor.cuh`: square/sawtooth output at channel 0, period/time offset, additive mode and final [-2,2] clamp; no energy creation. Base only runs its neural network. |
| Sensors | `SensorProcessor.cuh`: four modes, auto/manual scan, negative channel 0 relocation even in auto mode, initialized front direction, 64 rays, randomized sampling and nearest packed-distance selection, solid and nearby own-connection occlusion. Scan range is min(configured maximum, color radius); size/color/lineage filters apply to creature matching. A successful scan writes 0–3; failure only clears found channel 0. Creature mass is clamp(0.25*log2(cellCount/30)+0.5,0,1); distance is 1-min(1,distance/256). Tagging registers the match and up to three further nearby creatures, limited to the current detection cycle. |
| Attacks and conversion | `AttackerProcessor.cuh`: channel 0 trigger, raw energy <2, static exclusions, visible suitable targets, offspring exclusion, sensor tags for creature mode, color matrix, lineage/size/defender factors, costs, raw-energy loot and channel 2 clamp(loot/10,0,1). Attackable energy includes usable, depot storage and constructor reserve; raw energy itself is not loot. `CellProcessor::performEnergyFlow` and `DigestorProcessor.cuh`: raw energy goes to adjacent ready digestors below the receiving raw threshold 100; digestor-to-digestor balancing uses complementary conversion weights. Conversion per cycle is min(raw,(1-conductivity)*maxConversion*6). |
| Injection and defense | `InjectorProcessor.cuh`: foreign nonstatic constructor, genome resistance and visibility checks, genome clone and gene selection, restarted constructor, channel 1 success, cost/minimum-energy guard. `AttackerProcessor::countDefenderCells` includes the target itself for anti-attack mode and its direct defenders. Actual anti-injection cost differs from the advertised parameter (issue register). No unsupported InjectorMode selector is claimed. |
| Muscles | `MuscleProcessor.cuh`: all six modes and their channel access; first connection is pivot/length reference, permitted connection geometry, clamped activation, automatic alternating strokes versus sign-controlled strokes, angle steering and front updates, direct acceleration. The balance and deviation properties control distinct mechanisms; “manual” means signal control. Direct movement is signed; negative activation reverses the acceleration. Costs are signed in current code (issue register). |
| Memory and communication | `MemoryProcessor.cuh`: ring-buffer delay, recorder state machine, indexed storage with rounding/clamping, read-only overrides, selected-channel output mask and weighted smoothing `(1-w)*old+w*signal`. `CommunicatorProcessor.cuh`: only triggered senders act; all eight channels copied to other-creature receivers, receiver color/lineage filters, valid front direction on both sides, absolute-direction-preserving channel 1 conversion. One-way mode selects the opposite half-plane, not the forward half-plane. Buffer/range limits are also present in editor/data constraints. |
| Detonator and donor | `DetonatorProcessor.cuh`: latched countdown, static exclusions, blast/chain behavior and eight-radius shock-wave reach. `VoidProcessor.cuh`: ready donors share total energy among directly connected non-donor cells, then become InstantDying; without such neighbors deletion handles the energy. Genome validation excludes first/last donors and donor constructors. |
| Construction | `ConstructorProcessor.cuh`, `ConstructorHelper.cuh`: reserve before usable energy, host-normal-energy gate, energy requirement recomputed from actual construction data, external request threshold/eligibility, insertion order, activation propagation, branch/concatenation progress, root-gene offspring and separation. `Free` bypasses energy deductions, so is an explicit conservation exception. |
| Absorption, particles and release | `EnergyProcessor.cuh`: static exception for digestors, under-construction exclusion, cells receive raw energy but free cells receive object energy, fluids pass particles, reflection against solid connections, merge, stochastic split above threshold and optional normal-energy free-cell transformation. `ObjectProcessor::radiation`, cell decay and object-deletion paths release accounted energy through particles/backflow. |
| External pool | `SimulationKernels.cu::cudaNextTimestep_constructor_prepareExternalEnergyInflow` proportionally scales color-specific requests against a finite pool; infinite sentinel is not debited. Constructors receive reserves. `EnergyProcessor::provideExternalEnergyForSources` requests per-color inflow multiplied by each active source's relative strength; finite requests can receive only the remainder. Backflow is all-or-nothing per emission when the resulting pool would exceed its limit. |
| Parameters and defaults | `source/Data/SimulationParameters.h`, `.cpp` and `SimulationParametersTypes.h`: every published default, ten colors, base/layer/source membership, float/vector/matrix and enableable fields. Numerical defaults in the English tables match current initializers. Background is (0,0,0.106), source light white, coloring Customization, palette predefined, boolean/infinity defaults as translated. The chapter's “every parameter” completeness claim has exceptions below. |
| Layers and fields | `ParameterCalculator.cuh`, `ForceFieldKernels.cu`: scalar/vector/matrix values blend in increasing layer index order, later layers blend over earlier ones; opacity/fade apply to geometry. Circular/rectangular periodic-world distance and central/radial/linear/Perlin fields checked. Linear angle 0 up/90 right/180 down; central acceleration is delta*strength/(distanceSquared+50). Perlin interpolates successive seeded fields over its temporal scale and divides force by mass; static objects are skipped. Discrete color rules have a different layer selection policy below. |
| Mutation and color | `MutationProcessor.cuh`: meta changes are Gaussian additions to the genome mutation probabilities, clamped to [0,1], gated by applyMetaMutations and applied on offspring generation. Every published sigma defaults to zero. The customization matrix is boolean permission data, not a sigma; old color is the row, new color the column, one used color is replaced genome-wide. Lineage threshold compares accumulated normalized mutation changes. `CellProcessor::aging` implements timed object color changes and age reset, with the exceptions below. |

The numeric reference checked includes the 0/50/0/false/100/0/infinity external
group; layer/source dimensions 100, 100, 1 and 30 x 60; field strengths
0.001/0.05/0.0001/0.001 and Perlin sizes 100/10000; physics
1/0.8/0.1/0.1/0.001/0 and thresholds 2/0.8/0.3/3.6/0.1; radiation
1/false/1/0/0/0/500/50/disabled; life-cycle infinity/infinity/50/100/0.001/false;
construction 3.5; lineage 0.25; attacker 0/1/0.05/0/0/2; digestor 3/0.1;
defender 0.5/0.5; injector 250/3; muscle 0/1/1/1; sensor/reconnector/detonator
255/2/10/0.2. These are simulation units, time steps and dimensionless fractions,
not physical SI units. Numeric preservation is checked both chapter-wide and
per table cell, including all signs, channel indices and defaults.

## English-source discrepancies and proposed corrections

The Russian body deliberately preserves the original scope, numbers and
claims below; this register prevents treating translation as technical approval
of every original assertion. No algorithm, original English chapter or image is
silently repaired. Proposed source changes require separate review and coordinated
EN/RU updates. Editorial terminology normalization (for example, Energy reserve
to Зарезервированная энергия and “Maximum usable energy ... hold” to the depot's
storage limit) follows the approved glossary and the actual control.

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

The already recorded L7-B issues remain separate and unchanged: neural velocity
telemetry actually uses min(2,length(vel)*20); constructor timing has a minimum
cycle and creature phase; shapes paragraph counts are 12,16,16,19,20,27,19,16,
whereas embedded image labels are 8,16,16,19,16,25,13,20. Neither the shapes image
nor paragraph is repaired by L7-C.

## Markdown, navigation and real F1 resources

The extended `DocumentationLocalizationTests::checkShippedRussianPackage` loads
all twelve Russian Markdown files through the actual `DocumentationReader` and
parses both languages with ALIEN's `MarkdownParser`. It compares block count,
variant kinds, ordered heading levels, list depth/numbering, table columns/rows
and alignment, note structure and exact code blocks. Links in paragraphs, lists,
notes and table cells keep their target strings. Numeric contracts additionally
compare chapter-wide multisets and ordered numbers in each table cell.

The test navigates all shipped chapter links, including L7-B to L7-C, L7-C to
L7-B and L7-D fallback destinations, in EN/RU/EN. It resolves every canonical
English heading alias and native Russian anchor, including repeated headings
such as Сигналы, Свойства and Режимы. Eleven content chapters have Russian
bodies/titles without fallback; seven remaining GUI chapters, including the
format reference, still use English fallback. Contents is the separate twelfth
Russian Markdown resource. Canonical contents order and all three Russian part
names are preserved. No external website availability is asserted.

The existing search fixtures retain invalid-UTF8, case-folding, Ё/ё distinction
and snippets; real-resource searches additionally cover запасённая/ЗАПАСЁННАЯ,
конвертер/КОНВЕРТЕР and зарезервированная/ЗАРЕЗЕРВИРОВАННАЯ. Upper/lower forms
produce equal nonempty result counts with valid UTF-8 snippets. This does not
claim morphological search or equivalence of Е and Ё.

All five new chapters join the four existing long chapters in the actual
`DocumentationWindow` rendering test at 100/150/200%, in EN/RU/EN. Every alias
is requested and completed, scrolling remains stable and bounded, current
chapter, host identity and dimensions survive switches, and image metadata has
no missing paths. End-of-chapter clamping can leave an earlier top heading;
the test correctly permits this. Rendering uses the existing rendering-facade
double and native ImGui processing, not a live Vulkan screenshot inspection.

## Image decisions

All 19 referenced PNGs were opened for visual inspection in readable contact
sheets. All have embedded English text. Original PNG bytes, metadata and paths
are retained; RU links use `../images/...`, and every alt caption and surrounding
explanation is translated. No redraw, new illustration or fabricated screenshot
was created. There is no verified editable source in this package. Each row is a
retain-for-L7-C decision; label localization/final artwork acceptance is deferred
to L7-D, with specific issues noted rather than claiming current Russian artwork.

| Image | Inspection and decision |
| --- | --- |
| cell-base.png | Signal arrows go sensor -> base -> muscle; retain, translate English type/relay labels later. |
| cell-depot.png | Store/release arrows and 2-per-cycle labels match the mechanism; retain. Embedded “energy reserve” must become depot storage terminology, distinct from constructor reserve. |
| cell-sensor.png | Shows rays, occlusion, front and channels 0–3; retain as schematic. Example angle/mass/distance are illustrative, not measured output from the small drawn creature. |
| cell-generator.png | Square/sawtooth graphs clearly represent channel 0 signals; retain, no energy-source interpretation. Discrete phase sampling is simplified. |
| cell-attacker.png | Sensor tagging, loot arrow to attacker, adjacent digestor agree; retain. Digestor label/caption crowd the bottom, requiring typography review. |
| cell-injector.png | Genome arrow targets a foreign constructor; retain. The right explanatory label is clipped in the original image; recover from verified source or redesign after review in L7-D. |
| cell-muscle.png | Three mechanical families, pivot and channels 0/1 agree; retain. It does not attempt to draw all six control modes. |
| cell-defender.png | Formula uses one (1+strength) divisor per defender; retain. Label crowding near attacker needs layout review; it illustrates anti-attack only. |
| cell-reconnector.png | Positive connect/negative release and solid mesh agree; retain. Embedded “solid” will use Частица/Частицы; own-creature connections are retained on release. |
| cell-detonator.png | Countdown, blast and outward shock wave shown; retain as schematic, not a scale drawing of the eight-radius reach. Exact timing caveat C14 applies. |
| cell-digestor.png | Raw -> digestor -> usable flow and direct-neighbor requirement agree; retain with approved Конвертер/Сырая/Доступная labels later. |
| cell-memory.png | Four signal mechanisms shown; retain. Embedded >0/<0 recorder shorthand omits the strict threshold, and “running average” is weighted exponential smoothing, not an arithmetic window mean. |
| cell-communicator.png | All-eight-channel arrow and channel 1 reference-frame conversion agree; retain. Range circle is illustrative; one-way gating/filtering are described in prose. |
| cell-void.png | Temporary nodes vanish and energy goes to neighbors; retain. English Void will use Донор; this is not energy destruction. |
| cell-constructor.png | Insertion order and node 0 farthest away agree; retain. Constructor is a module, not a fifteenth cell type. Same asset was reviewed in L7-B. |
| energy-flow.png | Raw/usable/released/particle/external arrows form the intended cycle; retain. Stored/reserved energy, finite/infinite and Free exceptions are omitted by this overview, so it is not an exhaustive conservation proof. |
| creature-schematic.png | Cells, bonds and signals shown; retain. “Constructor” appears like a separate type and lacks the stated growing cells; prose supplies module semantics. Its network is an illustration, not the default tutorial genome. |
| signal-flow.png | Node 4 to node 0, one connection per cycle, agrees with default copy direction; retain. It is a separate five-node example, not the six-node first-creature tutorial. |
| genome-structure.png | Genes/nodes and constructor-reference arrows agree; retain. Leading sensor is not an example of default copying towards node 0; same limitation as L7-B. |

All paths, PNG signatures and copied bytes are checked by Python; encoded image
metadata/dimensions are checked by the actual F1 renderer with its facade double.
This is not proof of full Vulkan decoding/texture allocation/presentation.

## Build and automated verification

Windows Release: `build-windows-ninja.bat agent Release 16`, using only
`build-agent`. After the final resource edit, the generated `build-agent/Release/alien.exe`
is relinked through the same script so the existing CMake POST_BUILD resource copy
runs. A no-op build alone is insufficient for Markdown refresh. No separate
manual-acceptance build is prepared. SHA-256 contracts compare all twelve Russian
files and all 28 original PNGs in both build-agent resource trees to source.

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
| Release-working-directory documentation tests | 11 passed |
| check_localization_l7b.py -v | 5 passed; original seven-file checks retained, package membership extended |
| check_localization_l7c.py -v | 5 passed |
| clang-format 19.1.5 | Modified C++ test clean; HEAD checked clean before formatting |
| git diff --check | Passed |

No production code changes require engine regression. No full GPU suite was run,
as permitted for documentation changes. Existing unit/serialization and MCP tests
were run to support the preservation claims. The MCP regression checks exact
English contents/chapter bytes and machine schema through language switches.
The new Python preservation contract compares 489 tracked paths to base Git blobs
(with Git checkout normalization), covering all previous docs, help, catalogs,
Data, Engine, MCP, Network and Persister. Main is checked at the stated base.
User worlds, genomes, autosaves and settings were not opened or edited.

`AGENTS.md` retains SHA-256
`090c381a6f2b1f62597108de236e53131559ffef796146011a5796dbb7ef3368`
and `S AGENTS.md` skip-worktree. No submodule update, dependency edit or CodeQL
scan was performed.

## GitNexus

Pre-edit upstream impacts for the Markdown resource, test fixture and Python
contract returned UNKNOWN/unresolved with zero resolved callers. This was not
treated as unused/safe: F1 `DocumentationReader`/`DocumentationWindow` consumers,
separate MCP reader, `TEST_F` registrations, CMake GuiLocalizationTests membership
and Python invocation/report references were confirmed in text and source.
No production symbol was edited. Queries found the documentation parser/reader
and Radiation -> GetRandomNumber execution flow. No HIGH/CRITICAL impact was
reported. The index initially lagged the base by two commits.

`node .gitnexus/run.cjs analyze --index-only` completed successfully in 326.6s:
731,094 nodes, 952,675 relationships and 432 enumerated flows. The updated index
contains all 92 new Russian heading symbols and the changed test methods.
Final staged MCP `detect_changes(scope="all")` and CLI
`detect-changes --scope all --repo .` report 10 changed files, 111 recognized
changed symbols, zero enumerated affected processes and LOW risk. The structured
MCP response contains all 111 entries, with no partial/truncated flag; the human
CLI display abbreviates its listing. The new validation report was written after
the index scan and has no enumerated section symbols in that result; its entire
diff was reviewed directly. This does not hide a production-code change.

MCP context still advertises a two-commit staleness hint after the successful
index-only refresh; the log and new RU/test symbols establish that the changed
resource/test contents were indexed. This metadata hint is retained as a tool
limitation rather than a claim of a perfectly current graph. Resource consumption
and test registration are independently established by source and executed tests.

Analyzer limitations remain: large vendor files are excluded; callable candidate
sets can exceed caps; unresolved receiver/dispatch/cross-language references and
bounded flow enumeration prevent interpreting zero enumerated processes as no
GUI dependency. The refresh logs also report malformed/over-bound stored-site
facts dropped for a file; this limitation must remain visible in the result.

## Remaining acceptance and L7-D boundary

Manual visual acceptance of every table, typography, wrapping and diagram at
100/150/200% is not claimed. Automated scrolling/layout checks do not establish
subjective readability or absence of every clipped word. No end-to-end Vulkan
image acceptance, installer packaging check, external-browser dispatch or new
GPU execution of the biological examples is claimed.

L7-D remains sandbox.md, evolution.md, generative-art.md, ai-agents.md, files.md,
troubleshooting.md and format-reference.md, plus final localization/content
review or justified retention of the entire 28-image collection. It must carry
forward the L7-B shapes node-count mismatch, current English labels, injector
image clipping, source issues above and exact signal/energy semantics. This
branch does not translate those chapters, repair algorithms or begin that work.
