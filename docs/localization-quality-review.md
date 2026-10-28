# Russian localization quality review

Date: 2026-10-28.

## Scope and status

This is a partial quality review, not certification of all translations. The branch starts at `f86aa220863167b520ded9a5162c072b309bf836`, which includes the final-status documentation commit above the requested baseline. The abandoned L8-A branch was already absent; main is unchanged.

The normative terminology and the L4-A5 deferred review were read. The C01–C20, B01–B04 and D01–D06 register was inspected. Full sentence-by-sentence semantic review of all 1513 pairs and all 19 chapters remains outstanding. Structural regression checks cover the complete catalogs and documentation resources.

## Catalog corrections

| Key | Previous Russian | New Russian | Reason |
| --- | --- | --- | --- |
| `preview.primary.trimmed.one` | Основной: {count} клетка (усечён), индексы генов: {genes} | Основной (усечён): {count} клетка, индексы генов: {genes} | Moves the masculine qualifier next to the preview title, avoiding apparent disagreement with the cell count. |
| `preview.primary.trimmed.few` | Основной: {count} клетки (усечён), индексы генов: {genes} | Основной (усечён): {count} клетки, индексы генов: {genes} | Moves the masculine qualifier next to the preview title, avoiding apparent disagreement with the cell count. |
| `preview.primary.trimmed.many` | Основной: {count} клеток (усечён), индексы генов: {genes} | Основной (усечён): {count} клеток, индексы генов: {genes} | Moves the masculine qualifier next to the preview title, avoiding apparent disagreement with the cell count. |
| `preview.secondary.trimmed.one` | Дополнительный: {count} клетка (усечён), индексы генов: {genes} | Дополнительный (усечён): {count} клетка, индексы генов: {genes} | Moves the masculine qualifier next to the preview title, avoiding apparent disagreement with the cell count. |
| `preview.secondary.trimmed.few` | Дополнительный: {count} клетки (усечён), индексы генов: {genes} | Дополнительный (усечён): {count} клетки, индексы генов: {genes} | Moves the masculine qualifier next to the preview title, avoiding apparent disagreement with the cell count. |
| `preview.secondary.trimmed.many` | Дополнительный: {count} клеток (усечён), индексы генов: {genes} | Дополнительный (усечён): {count} клеток, индексы генов: {genes} | Moves the masculine qualifier next to the preview title, avoiding apparent disagreement with the cell count. |
| `preview.front` | Перед | Вперёд | Names the forward direction; the previous preposition was ambiguous. |

Consumers confirmed directly: `GenomeIssueDescription.cpp` formats node counts and ranges; `CreaturePreviewWidget::processTitle` selects primary/secondary and trimmed variants and wraps the title; `PreviewDescView.cpp` measures and centers the front-direction marker. No IDs, code or argument signatures changed. These changes address the preview-related deferred L4-A5 editorial candidates; final desktop visual acceptance remains open.

## Documentation register disposition

- Confirmed Russian wording errors: the seven catalog entries above.
- Inherited English inaccuracies: C01, C03–C11, C14–C20, B01–B03, D01–D02. No blanket closure is claimed.
- Potential implementation defects: C02, C12 and C13 remain for separate engine investigation.
- Ambiguous or environment-dependent: B04, D03–D04 and D06 retain their original limitations; B01 needs exact shape reproduction.
- D05 already documents native Cyrillic anchors and English aliases in Russian; the English source issue remains open.

B02 was confirmed in `NeuronProcessor::calcTelemetryInput`: speed is scaled by 20 and capped at 2. No documentation correction is included in this partial pass. C16 likewise remains open: the introductory constructor statement and donor glossary need alignment with the detailed reference. No English original, diagram or engine algorithm was changed. The full original register remains in [L7-D validation](localization-l7d-validation.md).

## Validation

- Windows Release: `build-windows-ninja.bat agent Release 16` passed. A final application relink refreshed POST_BUILD resources; both copied EN/RU catalogs match source bytes.
- BaseTests: 44 passed; GuiLocalizationTests: 162 passed; AccountLocalizationTests: 6 passed; McpLocalizationTests: 7 passed.
- Existing GUI tests exercise 100/150/200%, EN/RU/EN, wrapped preview headers, diagnostics, stable IDs/drafts and F1 parsing/search/anchors. These are automated ImGui tests, not manual desktop visual acceptance.
- Complete catalog structural check: 1513 pairs, identical key sets and named placeholders, nonempty valid UTF-8 values, no ImGui ID delimiters in Russian text: passed.
- All 26 historical Python stage scripts were attempted and returned failures. They assert obsolete stage counts, historical scope/preservation and copied-resource identity. The first run also exposed stale copied documentation; final application relink refreshed it. They are not reported as passing and were not weakened.
- Fourteen current behavioral/resource checks selected from L4-A5 and L7-B/C/D passed after relinking. Historical preservation/additivity tests were excluded from that focused run because this task intentionally edits existing Russian text. Logs: `build-agent/quality-*.log` (ignored).
- GitNexus initially returned database-lock UNKNOWN; direct resource consumers were inspected. Disambiguated `_CreaturePreviewWidget` in its actual header returned LOW, five impacted symbols, three direct callers, zero enumerated processes. The broad Definitions.h forward-declaration candidate had CRITICAL risk; it was not the edited target. No C++ symbol was edited. Index staleness and bounded/dynamic-call limits remain. Change analysis sees the JSON file but no changed code symbols; that zero does not prove resources have no consumers.
- No full GPU suite was run: production engine/shared code is unchanged. No manual Vulkan/desktop acceptance or complete linguistic review is claimed.
- English resources, README, roadmap, images, dependencies and user data are unchanged. AGENTS.md SHA-256 remains `090c381a6f2b1f62597108de236e53131559ffef796146011a5796dbb7ef3368`, with skip-worktree retained.

The singular-index candidate remains unchanged: Russian `.one` also applies to 21, 31 and similar counts, which can carry several indices. Replacing its caption with singular index would introduce a new error. A separate exact-count caption policy would require a presentation change.
