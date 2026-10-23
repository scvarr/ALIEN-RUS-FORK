# L5-C validation: simulation controls and evolution statistics

Date: 2026-10-23 (Europe/Moscow).
Branch: `feat/ru-localization-l5c`.
Baseline: `33bc2be134e6d1f2d3f075a8c325ffbc915da4db` (`main`).

## Delivered scope

| Component | Localized presentation |
| --- | --- |
| `TemporalControlWindow` | Title, TPS/total steps/real time, slowdown and rendering synchronization, toolbar/overflow actions, flashback help and emitted notifications |
| `SpatialControlWindow` | Title, world dimensions, zoom and center coordinates, selection tracking, zoom sensitivity and toolbar/overflow actions |
| `EvolutionDashboardWindow` | Title, four metric cards and their details, customization filter, lineage table and summaries, ten metric captions, history controls, cursor information, missing-creature notification, infinity and empty-plot explanations |
| `LocationWindow` / `LocationController` | Audited existing L2 integration; production files and their location widgets remain byte-identical. Existing translated titles, parameter labels and numeric window IDs are retained; user names remain data |

`ControlStatisticsPresentation.h` maps canonical captions to existing or new
catalog keys and measures display labels with the current font and scale.
Both catalogs contain **1,232 keys**, including **56 new `simulationui.*` keys**.
All 1,176 previous entries are preserved exactly. Existing menu actions, common
field labels and object vocabulary are reused. The terminology document is
unchanged. L6 was not started.

## Identity, state and data contracts

Window titles use the established `AlienWindow::setTitleKey` API, retaining the
original window identity and settings nodes. Fields retain canonical names and
use optional display labels. Toolbar and overflow entries retain index-based
IDs. Cards retain their original `##card` plus canonical-English-name IDs;
only their rendered captions change. Table, row, column-index header, plot,
lineage, line and shaded-item scopes remain canonical. History mode values stay
English internally; the switcher receives separate translated display values.

The runtime tests exposed an existing ImPlot behavior: registering items with
hidden labels forces their `Show` flag to true. Merely preserving IDs does not
preserve hidden-series state. The plot renderer now reads each existing line
and fill item's state and passes it through `HideNextItem` before registration.
`SetupLock` runs in the same lineage scope where the original first plot-item
call implicitly ran it, preserving the original ID stack and item hashes.
This narrow adapter uses the installed ImPlot internal header; dependencies and
vendor code were not modified.

The dashboard already applies computed axis limits with `ImGuiCond_Always` on
every frame. That existing policy is unchanged. Tests assert identical actual
X/Y ranges across language switches; this does not claim that arbitrary manual
pan/zoom ranges persist against the existing automatic-limit policy.

No statistics aggregation, rate calculation, samples, history/cache keys,
simulation data, CSV/export code, persistence, serialization, settings keys,
finite numeric formatting or timestamp format was changed. Full cursor messages
use named arguments and a dynamic string, avoiding truncation of longer UTF-8
captions in the former fixed buffer. The textual infinity marker is translated;
its threshold and numeric representation are unchanged.

Overlay notifications retain the existing emit-time language policy: an already
emitted message remains in its original language until replaced. Captions and
tooltips resolve at drawing time. User-authored location names and technical IDs
are never translated or used as localization keys.

## Layout and runtime evidence

Three new production ImGui/ImPlot tests cover EN/RU/EN at **100/150/200%**:

- All ten metric plots plus the time axis, both history modes, overall statistics
  and two selected lineages. The test checks actual ImPlot objects and item
  pointers, retained hidden/visible flags seeded on the actual ImPlot items,
  X/Y axis ranges, selected lineage IDs,
  color filter, sorting, time interval and history mode. It asserts zero ImGui
  frame errors; translation-key presence alone is insufficient.
- Empty timelines and long plot captions/help measured with real font atlases.
  Plot captions wrap within their label column. Existing numeric missing-value
  markers remain unchanged; empty plots additionally explain the missing data.
- Production temporal/spatial panels, persistent slowdown and viewport/tracking
  state, canonical card instances/bounds, the missing-representative notification
  and a live production `LocationWindow` with a bilingual user-authored name
  containing `%`, braces and `###`. The same location window retains its position,
  identity and open state across language changes.

Temporal sliders move below their toggles when needed. Labels reserve measured
text, toggle width and actual item spacing. Dashboard cards use two rows when
the window is narrow; omitted card details remain available in a tooltip.
History controls account for measured labels and separator spacing. Table headers
have full-caption tooltips; existing column resizing and horizontal scrolling
remain available without resetting widths on a language switch.

## Automated validation

Windows Release: `build-windows-ninja.bat agent Release 16` — passed.
Only `build-agent` was used. No CUDA/header change required a clean rebuild.
Build logs, GTest XML, Python results, formatting evidence and graph results are
retained locally under `temp/l5c/` and are not committed.

| Suite | Passed | Other results |
| --- | ---: | --- |
| BaseTests | 41 | 0 failures |
| DataTests | 247 | 0 failures |
| EngineInterfaceTests | 2 | 0 failures |
| McpToolsTests | 40 | 0 failures |
| NetworkTests | 38 | 0 failures |
| PersisterTests | 142 | 1 existing disabled test; 0 failures |
| EngineTests | 2,196 | 2 skipped, 2 existing disabled tests; 0 failures |
| GuiLocalizationTests | 138 | Full L1–L5-B regression and three new L5-C tests; 0 failures |
| Python localization contracts | 87 | All previous stage checks and five new L5-C contracts; 0 failures |

The GPU suite completed in approximately **86 seconds** without cancellation.
The skipped cases are
`ConstructorTests_ProvideEnergy_Separation.provideEnergy_insufficientEnergy/2`
and `/5`, matching the preceding stage.

The first BaseTests run found its expected catalog size still set to 1,176; the
test was updated to 1,232 and its source-reference inventory extended. The full
suite then passed. Historical Python checks exclude the additive namespace from
their original-stage counts while retaining checks on previous entries and
behavior. L5-B's obsolete whole-file freeze on the three L5-C windows is replaced
by explicit L5-C computation, identity, location and scope contracts. The L2/L4
updates are limited to their additive-namespace exclusions.

`git diff --check` passes. clang-format **19.1.5** was used only for eligible
modified files clean at HEAD and for the new header. `SpatialControlWindow.cpp`
already has an unrelated include-order formatting difference at HEAD; its existing
formatting was preserved. No whole-repository formatting was performed.

## Graph analysis

Upstream impact covered the modified production methods and test fixtures.
Resolved panel methods report LOW; `formatMetricValue` reports MEDIUM with five
direct display consumers. Callers include `processIntern`, `processHeader`,
`processTimelineSection`, `processTimelinePlots` and `processTimelinePlot`.
Relevant dashboard flows include `ProcessIntern -> FindLineageEntry`,
`FindRateReferenceIndex`, `CalcRate`, `Scale` and `Max`.

UNKNOWN constructor/virtual-entry results were checked against window registration,
main-loop/menu consumers and source call sites. UNKNOWN GTest/unittest fixture
results were checked against actual test registration and execution. No empty
caller set was treated as an all-clear.

The index was refreshed with `node .gitnexus/run.cjs analyze --index-only`.
Graph analysis succeeded; full-text index creation failed, leaving BM25 search
degraded. The refreshed graph was queried through the CLI backend because the
running MCP server retained old metadata. Final all-scope analysis covers all
**31 changed files**, all **75 listed changed symbols** and **two affected
processes**, with aggregate **MEDIUM** risk. Neither `partial` nor `truncated`
is set. The complete structured result is retained locally in
`temp/l5c/graph-complete.json`.

## Limitations and preservation

Layout evidence is automated ImGui/font/ImPlot execution. No interactive Vulkan
desktop session or screenshot-based visual acceptance was performed. Final
desktop composition, hovered-tooltip appearance and unusually narrow manually
resized windows still need visual acceptance during ordinary use. No separate
manual-acceptance build was prepared, as requested. Vendor menus remain L8-B;
the existing L5-A inspector-title limitation is unchanged.

At implementation delivery, only the feature branch was committed and pushed;
`main` remained at the requested baseline. `AGENTS.md` retains skip-worktree and SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`.
User worlds, genomes, autosaves, dependencies and pinned vcpkg are unchanged.
Tests use isolated settings. No CodeQL, submodule update or IDE-tree build was run.

## Acceptance and L5 closure

Date: 2026-10-23 (Europe/Moscow).

The user accepted L5-C implementation
`4d85183ce261a8648baae90ac2aa6cd6f75843b3` based on the recorded automated
checks and review of this report. L5-C is complete. Together with the previously
accepted L5-A and L5-B implementations, this closes the entire L5 stage, as
recorded in the [roadmap](localization-roadmap.md).

Manual visual acceptance was not performed and is not claimed. The automated
100/150/200% layout checks and actual ImPlot identity, visibility and axis-range
checks remain the acceptance evidence. Final desktop composition, hovered-tooltip
appearance and unusually narrow manually resized windows still require visual
verification during ordinary use. The L5-A inspector-title limitation and L5-B
vendor file-dialog limitations also remain; acceptance does not resolve them.

The existing automatic axis-limit policy, emit-time overlay policy, ImPlot
internal-header dependency and GitNexus full-text index limitation documented
above remain unchanged. Vendor UI retains its planned L8-B scope.

This closure updates only this report and the roadmap. Builds and tests are not
rerun; the accepted implementation evidence remains applicable. The user
authorizes a documentation commit and push, fast-forward-only integration into
`main`, pushing `main`, synchronization verification and deletion of the
integrated feature branch. `AGENTS.md`, skip-worktree flags, user data and
dependencies are preserved. L6 is not started or authorized.
