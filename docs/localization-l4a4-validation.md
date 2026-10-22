# L4-A4 validation: mutation editor

Date: 2026-10-22 (Europe/Moscow).
Branch: `feat/ru-localization-l4a4`.
Baseline: `ba2e7748a8b294875d5ac5fe384f46a37a82eea2` (`main`).

## Delivered scope

`MutationRatesWidget` and `MutationRatesDialog` resolve their display text every
frame through the existing localization service. All 19 mutation groups, the
paired rate panels, node/gene/genome probabilities, sigma, weight, bias,
activation-function and discrete-choice probabilities are covered. Both the
expanded active-rate list and compact count/tooltip summary are translated.
The count uses a complete neutral sentence (`Edit (active: {count})`) rather
than a partially translated noun phrase.

`MutationRatesPresentation.h` maps canonical captions to display keys and measures
parameter labels with the current font. `GenomeEditorPresentation` additionally
maps all 32 consumed mutation-related `EntityAttributeHelp` descriptions,
including the two genome summary/editor descriptions. English descriptions match
the canonical help table exactly; missing catalogs use embedded English. The
canonical `EntityAttributeHelp.cpp` table is unchanged. Donor and neural terms
follow the terminology document.

Both catalogs now contain **854 keys**: 51 new mutation captions/messages and
32 attribute descriptions, all under `mutation.*`. Existing entries are unchanged.
Adopt, Cancel, Copy, Paste, their tooltips and copy/paste notifications are covered.
The original dialog had no reset action. The added Reset action restores its
draft to the existing `MutationRatesDesc` defaults and shows a localized message;
Adopt applies that draft and Cancel discards it. Default values themselves are
unchanged.

## Identity and layout

Canonical `.name()`, `.id()`, section/child/table scopes, group order and repeated
rate identifiers are retained. Optional display labels and `DisplayButton` keep
the original button IDs. The compact summary retains its original dynamic ID
including the active count. Changing language does not reopen the dialog or
replace its draft, callback or clipboard.

Label and clipboard-button widths are measured from current translations.
Minimum table widths reserve room for the translated parameter labels and
sliders. Narrow content can scroll horizontally and vertically; clipboard actions
wrap to a second footer row when the translated actions do not fit together.
The footer reserves two rows. The stable modal title API is reused.

No mutation algorithm, numeric range, default, numeric format, probability
calculation, genome schema, persistence key or serialization implementation was
changed. `loadSettings`, `saveSettings`, `open` and `onAdopt` remain byte-identical
to the baseline. L4-A5 diagnostics, graph and signal-buffer work was not started.

## Automated validation

Windows Release passed with the repository-root command:

```text
build-windows-ninja.bat agent Release 16
```

Only the agent build tree was configured/built. Final logs, XML results and local
validation scripts are retained under `temp/l4a4/`.

| Suite | Passed | Other results |
| --- | ---: | --- |
| BaseTests | 41 | 0 failures |
| DataTests | 247 | 0 failures |
| EngineInterfaceTests | 2 | 0 failures |
| McpToolsTests | 40 | 0 failures |
| NetworkTests | 38 | 0 failures |
| PersisterTests | 142 | 1 existing disabled test |
| GuiLocalizationTests | 118 | 4 new A4 tests; 0 failures |
| EngineTests | 2196 | 2 existing runtime skips, 2 disabled tests |

Total: **2824 passing C++ tests**, 2 runtime skips and 3 disabled tests. The engine
skips are `provideEnergy_insufficientEnergy/2` and `/5`. No build or test was
cancelled. The initial BaseTests run detected its old 771-key assertion; that
assertion was updated to 854, the new consumer files were added to its reference
scan, and the complete BaseTests suite passed again. The complete GUI suite was
rerun after the final production/layout changes.

The four new production ImGui tests cover:

- All 32 descriptions in EN/RU/EN and exact English fallback without catalogs.
- Open modal, both summary presentations and all 19 active mutation types at
  100/150/200%, narrow/wide hosts, measured long-label bounds and unchanged genome
  values. A synthetic genome round-trip preserves the complete mutation data.
- An actual Ctrl-click probability edit to `0.375` across a language change,
  retained active ImGui ID, independent second rate (`0.875`), unchanged neuron
  parameters and serialization/reload of that edited synthetic genome.
- Actual Copy/Reset/Paste/Adopt/Cancel button clicks, language switching while a
  button is held, translated overlay messages, draft-only reset, independent
  paired values, single adoption and cancellation without adoption.

All **72 localization Python checks pass**, including five new A4 contracts for
catalog preservation/parity/signatures, complete exact-English help coverage,
canonical IDs/order/ranges/formats/settings, measured layout/stable actions and
unchanged data/engine/later-stage files. Historical catalog checks exclude only
the new `mutation.*` slice; historical mutation-editor freezes are superseded by
the A4 identity/data contracts. Their other stage boundaries remain enforced.
Both Windows Ninja dependency regression tests also pass, without skips.

clang-format **19.1.5** was used on modified C++ files clean at HEAD and on the new
header. `MutationRatesDialog.cpp` was already not format-clean and received local
edits only. `git diff --check` passes.

## Graph analysis

Upstream impact reports are retained in `temp/l4a4/impact-fresh.json`. Dialog
rendering helpers report LOW risk; their direct consumer is the section table
flow (`SectionTable -> Get`). Button processing and width calculation lead back
to `processContent` and `process`. The dialog class is consumed by the mutation
widget and mass editor.

Widget entry points, the namespace help adapter, constructors and test entry
points have unresolved graph callers (UNKNOWN). They were not treated as unused:
source inspection confirmed the genome editor and mass-editor consumers, the
template helper calls and unittest/gtest registration. Existing shared help
adapter consumers remain covered by the full GUI suite. Template ambiguity was
also checked against the Function candidates, which report LOW risk.

GitNexus refresh and complete pre-commit diff analysis are retained under
`temp/l4a4/`. Full-text/BM25 index construction fails in this environment;
graph traversal remains available. The final change-analysis result is recorded
in `temp/l4a4/graph-final.json`: **83 changed indexed symbols, 26 changed files,
one affected flow (`ProcessContent -> Max`), MEDIUM risk**. All 83 symbols are
returned; neither partial nor truncated output is reported. The affected flow is
the dialog layout calculation and is covered by the production GUI tests.
The refreshed index contains 730195 nodes, 949826 edges and 432 reported flows.
Global index diagnostics report bounded process enumeration and unresolved call
sites; absence from a reported flow is not evidence that code has no consumers.

## Isolated visual acceptance build

The final Release libraries are separately linked into
`E:\Projects\ALIEN\temp\l4a4\review\alien-l4a4.exe`.
The wrapper uses `SettingsTestScope` to redirect HKCU for this process, disables
autosave/save-on-exit and enables the genome editor. Only tracked runtime assets,
catalogs, licensed fonts and required DLLs are packaged. Disposable
`samples/synthetic-a.genome` and `synthetic-b.genome` include all 19 active mutation
types, distinct paired probabilities and multiple genes/constructors. No user
world or genome was copied. Profile-isolation and sample-generation probes both
return 0.

Run from PowerShell:

```powershell
& 'E:\Projects\ALIEN\temp\l4a4\review\Launch-L4A4.ps1' -Scale 1.0
& 'E:\Projects\ALIEN\temp\l4a4\review\Launch-L4A4.ps1' -Scale 1.5
& 'E:\Projects\ALIEN\temp\l4a4\review\Launch-L4A4.ps1' -Scale 2.0 -English
```

Default: Russian, 150%. Each run has a disposable profile; save review edits only
inside this package's `samples/` directory.

**Visual acceptance is pending.** The agent attempted native window capture using
the [Computer Use skill](C:/Users/infarh/.codex/plugins/cache/openai-bundled/computer-use/26.1016.51333/skills/computer-use/SKILL.md).
The app window was discovered, but capture repeatedly returned
`window capture timed out: timed out waiting on channel` (also
`FrameArrived timed out`). No pixel-level visual review is claimed. The headless
production-widget and font-measurement tests establish interaction/layout
contracts, not human visual acceptance.

Manual acceptance checklist:

1. Open `samples/synthetic-a.genome`; open mutation editing. Inspect all 19
   groups and both paired panels, every label and tooltip, sigma/activation
   terminology, compact count and expanded active summaries in EN and RU.
2. Edit distinct probabilities and sigma values. Switch language during numeric
   editing and with the modal open. Verify focus, open groups and both paired
   values survive; inspect Copy/Paste and their notifications.
3. Check Reset then Cancel leaves the original genome intact; Reset then Adopt
   applies existing defaults. Verify a second copy/paste restores the draft.
4. Save the edited synthetic genome under a new review filename, reload it,
   compare values and confirm user gene names are unchanged.
5. Repeat at 100/150/200%, narrow/wide windows. Inspect long Russian headings,
   footer wrapping, slider room, tooltip wrapping and both scroll directions.

## Preservation

`main` remains at the requested baseline; this feature branch is committed and
pushed without integration. `AGENTS.md` retains skip-worktree and SHA-256
`090C381A6F2B1F62597108DE236E53131559FFEF796146011A5796DBB7EF3368`.
Real HKCU profile exports before/after validation match SHA-256
`1104383BB644A4FFE3E2CA667F50D764657233CB4A697AD009D8C1767503319A`.
User worlds, genomes, autosaves and dependency sources are preserved. No CodeQL,
submodule update, dependency edit or IDE-tree build ran.
