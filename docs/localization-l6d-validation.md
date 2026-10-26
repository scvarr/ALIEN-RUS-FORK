# L6-D validation: MCP desktop interface

Date: 2026-10-23 (Europe/Moscow).
Base `main`: `5f83ed9a4c6024f3cd325af8f930649ae8f3a7ea`.
Delivery branch: `feat/ru-localization-l6d`.

## Implemented scope

The English and Russian catalogs gain 30 additive `mcp.*` entries, bringing each
catalog to 1,504 entries. All earlier keys and values are preserved. The existing
CMake resource dependency regenerates the embedded English fallback.

| Component | Change |
| --- | --- |
| `McpWindow` | Server title, running/stopped badge, start/stop/settings/clear actions, overflow help, connection instructions and help, copy action/notification, command-log count and column headings |
| `McpSettingsDialog` | Stable localized title, port label, local connection/restart explanation, Apply and Cancel; larger initial dialog and action wrapping |
| `McpController` | Only the three GUI messages: running URL overlay, stopped overlay, unavailable-port dialog |
| `McpPresentation` | GUI-only captions for all six existing tool groups; unknown groups retain their literal names |
| EN/RU catalogs | Connection prose retains HTTP and Streamable HTTP; named placeholders preserve literal URL, port, count and tool-list arguments |
| Test/build registration | New `McpLocalizationTests` executable with a link-time server simulation; HIP device-link registration follows the existing test targets |
| Test-fixture access | Only `friend class McpLocalizationTests` in the three MCP headers, `AlienDialog`, `GenericMessageDialog`, `WindowController` and `OverlayController` |

Instructions wrap using the current content width. The URL and copy action occupy
a separate row, whose input width reserves room for the translated button.
Timestamp-column sizing considers both its canonical sample and translated label.
The existing tooltip renderer supplies wrapping for the longer connection help.

There is no JSON configuration block in the baseline MCP panel: the existing
copyable example is the server URL. No new configuration format, client-specific
configuration, capability or connection guarantee was introduced. User-facing
surrounding prose is localized; actual tool identifiers remain copyable literals.

## MCP compatibility boundary

`check_localization_l6d.py` compares every tracked file under `source/Network`,
`source/McpTools` and `resources/help` with the requested base using Git blob
identities. Windows checkout newline conversion is normalized by Git; the
protocol implementations, schemas, descriptions, response construction, help
examples and existing machine tests are otherwise byte-identical.

A second check compares the **entire** `McpController.cpp` against that base after
reversing exactly three specific GUI-message substitutions and their localization
include. This covers lifecycle, binding URL, port clamping and persistence,
thread queue, shutdown result, tool wrapping, exception conversion, arguments,
command serialization, timestamps, truncation, retention and technical logging.
The guard fails if any additional controller logic changes.

`showMessage` continues to display agent-supplied text literally. The machine
shutdown text remains `The MCP server has been stopped.`. Machine tool groups,
instructions, names, descriptions, JSON fields/types/values, endpoint and HTTP
behavior have no dependency on GUI language. Only the six group captions passed
to the tooltip are translated; the controller's metadata is unchanged.

The real wrapped synchronous and deferred handlers are exercised with synthetic
JSON containing mixed-case Cyrillic, braces, percent signs and `###` markers.
Tests compare serialized arguments, command strings, raw results, error flags,
image data and stored timestamps. Switching language while a deferred command
is pending neither completes nor reschedules it. Completion produces one literal
log entry and the original result. No localization is applied to either payload.

## Identity, state, layout and isolation

Actual ImGui components with production fonts/style are exercised at 100%, 150%
and 200%, including EN → RU → EN while the MCP window and settings dialog remain
open. Tests retain the modal/input IDs, active input/focus, edited port value,
window visibility and underlying controller port. The original title/settings
identity and toolbar index scopes are retained. Apply, Cancel and Copy use
canonical IDs through the existing `DisplayButton` helper. Table and child IDs
remain unchanged. A populated 100-row log retains its table object/ID, scroll
offset, resized column weight and exact contents across language switches.

Mouse-driven Apply/Cancel checks prove that Cancel leaves the server port and
start count unchanged, while Apply restarts the running simulated server once
with the selected port. Lifecycle checks cover stopped/running, redundant start,
unchanged port, restart, unavailable port, lower/upper port bounds and exact URLs.
The startup-error modal retains its identity and port argument while its prose
resolves in the active language. Overlays retain the existing emit-time policy.
Empty and populated logs are rendered. Guide/copy and settings action bounds are
checked with the actual fonts and scaled layouts.

`McpServerStub.cpp` defines the complete server API for this executable without
an HTTP implementation or listener. It records requested ports and can return
startup success/failure. A synthetic tools facade supplies no world operations.
The production controller still performs its real restart and task-queue logic.
`SettingsTestScope` redirects settings away from the user's profile and ImGui INI
persistence is disabled in the fixture. No external agent, real MCP listener,
user-world operation or account operation is used by these new tests.

## Windows Release and automated results

Build command: `build-windows-ninja.bat agent` (MSVC/Ninja Multi-Config, Release).
Final build succeeded in `build-agent`; no kernel structure/header was changed,
so a clean CUDA rebuild was unnecessary. No IDE build tree was used.

| Suite | Result |
| --- | --- |
| `BaseTests` | 41 passed |
| `DataTests` | 247 passed |
| `EngineInterfaceTests` | 2 passed |
| `McpToolsTests` | 40 passed |
| `NetworkTests` | 38 passed |
| `PersisterTests` | 142 passed; 1 disabled |
| `EngineTests` | 2,196 passed; 2 skipped; 2 additional disabled; about 89 seconds |
| `GuiLocalizationTests` | 148 passed, including L4–L6-C regression |
| `AccountLocalizationTests` | 6 passed |
| `McpLocalizationTests` | 7 passed |
| Python localization checks | 111 passed, including seven new L6-D contract checks |

Historical catalog checks exclude only the additive `mcp.*` namespace. Historical
L6-A/B whole-file assertions for superseded MCP presentation are pinned to the
accepted L6-C baseline; the new L6-D full-controller check owns the current source.
Shared picture/simulation diagnostics continue to be compared with current source.
Documentation-preservation checks use this task's accepted base. The Base catalog
size assertion is updated from 1,474 to 1,504.

Build/test logs and XML results are retained locally in ignored `temp/l6d/`.
Formatting uses clang-format 19.1.5; modified C++ files were confirmed clean at
HEAD before whole-file formatting. New C++ files use the same configuration.
`git diff --check` passes.

## GitNexus evidence

Upstream analysis preceded production edits. Concrete MCP classes report LOW:
`McpWindow` has four direct consumers (EditorWidget, MainLoopController,
MainWindow and its implementation); settings has three (MainWindow, McpWindow
and its implementation); controller has four (MainWindow, McpWindow, settings
and its implementation). Toolbar/guide/log methods have their window caller;
`setServerRunning` is reached by the toolbar and `startServer` by initialization
and start control. No relevant execution processes were resolved by these walks.

Constructor and virtual-entry queries can return UNKNOWN. They were checked
against concrete class graphs, singleton registration and source call sites;
UNKNOWN was not treated as an all-clear. `MainLoopEntity` dispatch is a reported
lower-bound boundary with 24 implementations. Shared headers report HIGH
(`AlienDialog`: 23 direct consumers, `WindowController`: 13, `OverlayController`:
19) and CRITICAL (`GenericMessageDialog`: 31). These warnings were reported
before the fixture-only friendship additions; their runtime implementations
are unchanged and are guarded by exact source comparisons.

Index refresh uses `node .gitnexus/run.cjs analyze --index-only`. The first refresh
completed successfully with 730,717 nodes, 951,652 edges and 432 reported flows.
It explicitly reports missing candidate flows, trace/branching budgets,
callable-value candidate caps, cross-language field-resolution gaps and 46
oversized skipped files, mostly vendored content. An empty process list is not
proof of no runtime callers. Earlier reports' FTS/BM25 failure is historical;
this refresh completed and keyword queries returned definitions without that
error. This does not establish exhaustive keyword or process coverage.

The final refresh completed successfully in about 335 seconds, with 730,753
nodes, 951,850 edges and 432 reported flows. It additionally reported 11 dropped
malformed/over-bound cached sites in one file. No caller/process absence is
interpreted as exhaustive coverage.

The complete LocalBackend `detect_changes({scope: "all"})` result uses an expanded
listing limit: **43 changed files, 111 changed symbols, all 111 listed, aggregate
LOW risk**, with neither `partial` nor `truncated` set. No affected processes were
resolved. This aggregate does not remove the shared-header HIGH/CRITICAL
warnings or the dispatch/traversal limitations above. Structured evidence is
retained in `temp/l6d/graph-complete.json`; repeated upstream class results are
in `temp/l6d/impact-final.json`.

## Limits and preservation

These are automated ImGui/font/state/layout checks and baseline source-contract
comparisons, not manual Vulkan desktop or screenshot acceptance. Final desktop
composition, hovered tooltip appearance and unusually narrow saved windows remain
unverified. Port availability is simulated: OS socket conflicts, external-agent
interoperability and live concurrent transport behavior were not exercised by the
new GUI tests. Existing unchanged NetworkTests and MCP suites also passed.
Linux/HIP execution was not performed. No separate manual-acceptance build was
prepared.

At implementation delivery, `main` remained at the requested base and L7 was
not started. Acceptance and final L6 closure are recorded below. The audit and
terminology documents remain unchanged; closure updates this report and the roadmap.

`AGENTS.md` retains skip-worktree and SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`.
User worlds, genomes, autosaves, settings and dependencies are preserved. The
pinned vcpkg submodule is unchanged. No CodeQL or submodule update was run.

## Acceptance and L6-D closure; entire L6 stage closed

Date: 2026-10-26 (Europe/Moscow).

The user accepted L6-D implementation
`edb337e07cac6a917d553a1e6b3aeefa0621dddd` based on the recorded automated
checks and review of this report. **L6-D is complete.** Manual visual acceptance
and connection of real MCP agents were not performed and are not claimed.

**L6-A, L6-B, L6-C and L6-D are complete; the entire L6 stage is closed.**
The [roadmap](localization-roadmap.md) records final closure; the
[L6-A](localization-l6a-validation.md), [L6-B](localization-l6b-validation.md) and
[L6-C](localization-l6c-validation.md) reports retain their accepted evidence and
stage-specific limitations.

Acceptance preserves all limitations above. Automated 100/150/200% ImGui checks
are accepted evidence, not manual Vulkan desktop or screenshot acceptance.
Final desktop composition, hovered tooltips, vendor/native controls and narrow
saved windows still require ordinary-use visual verification. L6-A did not
inject real asynchronous disk/network failures through a running GUI. Browser
and account coverage in L6-B/C used isolated network/persister responses; actual
authenticated ownership, successful remote mutations, real login/registration,
email delivery, password changes/deletion and server-side account semantics
remain unverified. L6-D's isolated server simulation does not validate actual
socket conflicts, real MCP-client interoperability or concurrent live transport
behavior. Raw diagnostics, machine descriptions, commands and JSON remain
literal. Browser search preserves the documented Е/Ё distinction and Unicode
limits; MCP search remains unchanged. Linux/HIP execution remains unperformed.

The latest recorded GitNexus refresh succeeded during L6-D, and keyword queries
returned definitions without the earlier FTS/BM25 failure. This records the
latest successful refresh rather than claiming exhaustive analysis or erasing
the earlier reports' historical failures. The dispatch/UNKNOWN, candidate-flow,
traversal, cross-language, cached-fact and skipped-file limits documented above
remain applicable. L6-B's aggregate HIGH risk and the shared-header HIGH/CRITICAL
warnings remain part of the accepted evidence despite L6-D's aggregate LOW result.
No index refresh is needed for this documentation-only closure, and no new
runtime-coverage claim is made.

Closure changes only this report and the roadmap. Production code, tests and
catalogs are unchanged; builds and tests are not rerun. Existing implementation
validation remains applicable. The user authorizes a documentation commit and
push, fast-forward-only integration into `main`, pushing `main`, verification of
synchronization and cleanliness, and deletion of the integrated feature branch
locally and remotely. `AGENTS.md` retains its recorded bytes and skip-worktree;
user worlds, genomes, autosaves, settings, dependencies and pinned vcpkg are
preserved. **L7 is not started or authorized by this closure.**
