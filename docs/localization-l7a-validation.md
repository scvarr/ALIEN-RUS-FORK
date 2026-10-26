# L7-A bilingual F1 infrastructure validation

Date: 2026-10-26 (Europe/Moscow).
Base: `8d20e14c593efbf73a8a80d021658d870f3343ac`.
Branch: `feat/ru-localization-l7a`.
Scope: L7-A implementation; L7-B–D translation batches are not started.

## Acceptance and L7-A completion

Date: 2026-10-26 (Europe/Moscow).
Accepted implementation: `190592478405d1388852a82146542b0b0136cf98`.

The user accepted L7-A based on the automated checks, review of this report and
user verification that F1 opens and works in the running application.
**L7-A is complete. The entire L7 stage remains open.** This launch confirmation
is limited user acceptance evidence; it does not constitute full manual visual
acceptance of all scenarios, language switches, scales, links or images.

Acceptance retains the recorded limitations. Texture lifecycle tests use a
rendering-facade double and do not establish end-to-end Vulkan texture allocation,
decoding or presentation for every image. The English image assets remain
unchanged; translated images/screenshots and embedded English text still require
review in L7-B–D. Installer-specific packaging and delivery of both language trees
have not been validated. Automated ImGui checks at 100/150/200% and the user's F1
launch confirmation do not replace a complete manual visual review. The existing
external-browser and GitNexus analysis limitations also remain applicable.

No mass translation of real chapters was performed. Production chapter translation
starts with L7-B and continues in L7-C/D; this closure does not start or authorize
L7-B. No claim of complete Russian documentation is made.

This closure changes only this report and the roadmap. Code, tests, catalogs,
English documentation, image assets and dependencies are unchanged. Builds and
tests are not rerun; the accepted implementation evidence below remains applicable.
The user authorizes a documentation commit and push, fast-forward-only integration
into `main`, pushing `main`, synchronization/cleanliness verification and deletion
of the integrated feature branch locally and remotely. `AGENTS.md`, its
skip-worktree flag, user data and settings are preserved.

## Loading architecture and MCP isolation

`DocumentationReader::getChapters`, `parseChapters` and `readChapter` retain their
existing English behavior. Their implementation and the MCP tool implementation,
description and schema are unchanged. No GUI service is consulted by these methods
and no MCP language parameter is added.

The separate GUI API accepts `DocumentationLanguage` and an explicit English root:
`getGuiChapters(language, englishRoot)` returns navigation metadata;
`loadChapter(filename, language, englishRoot)` returns owning text, the actual
`basePath`, and `englishFallback`. The default production root remains
`Const::DocsPath` (`resources/docs`). `Base::Resources` is unchanged.

The GUI uses the current applied `LocalizationService` language. EN reads the
original tree; RU tries `resources/docs/ru/<canonical filename>` before the English
file. The Russian directory is optional; no production chapter is translated in
this change. Canonical filenames remain unchanged.

## Partial translation and failure policy

- English `contents.md` determines the chapter set and order. Unsafe chapter paths
  are excluded from the GUI catalog. The existing MCP catalog parser is unchanged.
- Russian contents only overlays entries for canonical chapters whose Russian
  content was loaded. Unknown entries do not add or reorder chapters. Missing
  entries use the translated chapter's H1 title when available; missing metadata
  otherwise retains the English title/part.
- A missing Russian contents file does not prevent loading translated chapters.
- Missing, unreadable, non-regular, empty, whitespace-only and comment-only Russian
  files fall back to English. Documents producing no Markdown blocks also fall back.
- A readable translated chapter with an intentionally different heading structure
  is still displayed; the stable English fragment aliases described below are then
  unavailable, rather than pointing at arbitrary sections.
- English fallback content is explicitly marked in the window. If neither file is
  usable, a localized missing-chapter message appears instead of claiming that a
  translation or English original is being displayed. A missing tree has its own
  localized message.

## Images and renderer lifecycle

Every chapter carries its actual content root into `MarkdownRenderer::render`.
English fallback chapters therefore resolve images against the English directory.
Russian chapters resolve images against `docs/ru`. Shared English images must be
referenced explicitly, for example `../images/example.png`; there is no silent
second image lookup that could hide a missing translated image.

Metadata and textures are keyed by normalized actual paths. A document/root change
releases old textures and discards old image metadata, including negative lookups.
`reset()` also clears the document pointer, pending/current anchors, link hover and
click state. The window resets the renderer before replacing Markdown documents.
Hiding the window releases GPU textures, while reopening can reload them. Visible
image decoding remains limited to one image per frame.

Image metadata now uses file bytes from `std::ifstream(filesystem::path)` and
`stbi_info_from_memory`, avoiding a narrow Windows filename conversion. A temporary
Cyrillic directory is covered by the image test. Missing images have localized
presentation. Production PNG files are unchanged.

## Links, anchors and format reference

The common Markdown parser is unchanged. Native fragments use its current exact
anchor spelling, including preserved Cyrillic case and duplicate suffixes (`-1`,
`-2`, ...). Translated links may target these native anchors directly.

For translations preserving the number, order and levels of English headings,
the GUI additionally maps each English anchor to the corresponding translated
heading. Translation batches must preserve this heading correspondence to use
stable English links. Existing English fallback chapters can thus link into a
translated target using an English fragment. Native anchors take precedence if
there is a spelling collision. This is a positional correspondence policy, not
semantic inference about reordered headings.

If the heading structure changes, translators must update native destinations;
the GUI reports unresolved fragments rather than silently jumping to a different
section. Missing chapter destinations also have a localized notice. Empty fragments
open the chapter top. Pending anchors are cleared when navigation does not request
a valid section. HTTP/HTTPS links retain the existing browser dispatch.

`format-reference.md` is appended explicitly to the GUI catalog, once, after the
canonical English chapters. Its title can be overlaid from a Russian contents
entry or H1. It is reachable through navigation, links, search and chapter buttons.
The existing MCP chapter list still has 17 entries and deliberately continues to
reject this GUI-only additional chapter.

## Language switch, identity and search

`DocumentationWindow::processBackground` observes the applied language before
submitting its widgets, including while the window is hidden. It saves the current
canonical filename and visible/pending anchor, resets the renderer, replaces the
chapters and rebuilds section text and search results. The search query is retained;
result indices are cleared before document replacement. Current sections are
restored through English aliases when correspondence is available; otherwise a
matching native anchor is attempted, with chapter-top fallback.

The window object and its canonical ImGui title remain `Documentation`; the drawn
title uses the translation key. English chapter/section captions supply widget IDs
while translated captions are drawn separately. Existing English previous/next
button IDs, the Search ID, child names, startup-toggle ID and all settings keys are
retained. `windows.documentation.chapter` still stores the canonical filename.
Language changes do not write geometry, navigation width or the startup preference.
Previous/next buttons use separate rows when their combined width does not fit.

Search uses the existing strict `Utf8Search` adapter. ASCII Latin and Russian case
folding includes Ё/ё and keeps Е distinct from Ё. Space-separated terms retain AND
semantics across each section's title/body; all-title matches retain priority.
Snippets use original UTF-8 offsets, with a 30-character lead and up to 100 decoded
characters, including long tokens without spaces. Invalid UTF-8 makes that field
unsearchable; an invalid body yields an empty snippet even if a valid title matches.
There is no transliteration, morphology or Unicode normalization.

Nine EN/RU message pairs cover the title, search/no-results, startup preference,
missing tree/chapter/image, invalid links and English fallback notice. Catalog
parity is 1,513 keys; embedded-English fallback tests pass.

## Validation

Windows Release was built with `build-windows-ninja.bat agent Release 16` into
`build-agent`. No IDE build tree was configured or modified. Modified existing
C++ files were verified format-clean at HEAD before whole-file formatting;
clang-format 19.1.5 was used. `git diff --check` passes.

| Executable | Result |
| --- | --- |
| BaseTests | 44 passed |
| DataTests | 247 passed |
| EngineInterfaceTests | 2 passed |
| McpToolsTests | 40 passed |
| NetworkTests | 38 passed |
| PersisterTests | 142 passed |
| GuiLocalizationTests | 159 passed, including 9 documentation tests |
| AccountLocalizationTests | 6 passed |
| McpLocalizationTests | 7 passed |
| EngineTests (CUDA GPU) | 2,196 passed, 2 skipped; 2 additional disabled tests |

The two engine skips are the existing
`ConstructorTests_ProvideEnergy_Separation.provideEnergy_insufficientEnergy/2`
and `/5` parameter cases. No running build or test was cancelled. No kernel or
constant-memory header changed, so a clean CUDA rebuild was not required.

Reader tests use unique temporary trees for complete EN/RU, partial/missing Russian
contents, untranslated/empty/unreadable chapters, missing trees and invalid paths.
GUI tests exercise real ImGui components and the native `AlienWindow` process at
100/150/200%, including EN/RU/EN, active searches, filename restoration after TOC
reordering, duplicate headings, English aliases, invalid links, section scrolling,
window identity/geometry, navigation width and startup preference. Previous/next
buttons receive actual ImGui mouse input with their legacy English IDs.

The texture test uses real temporary PNG file bytes and a rendering-facade double
that records texture loads/deletions. It checks reuse, visibility-triggered loading,
English/shared/Russian paths, hiding/reopening, negative-cache invalidation and
balanced release. The shipped English-tree test checks every supported internal
link destination and image metadata, including format-reference reachability.

`DocumentationBaseline.h` records the base commit's exact serialized English MCP
contents, tool description and FNV-1a 64-bit hashes of all 17 returned chapter texts
(with Windows text-mode newline handling). Each is checked in EN/RU/EN, including
chapter requests with fragments. The schema retains only the existing `chapter`
property. All 47 original documentation assets (19 Markdown, 28 PNG) were compared
against base Git blobs with checkout newline normalization; none differ. No
English document or MCP implementation file is included in the diff.

Settings tests use the existing isolated registry scope and temporary directories.
`AGENTS.md` was not edited and retains `skip-worktree`; its final SHA-256 is
`090c381a6f2b1f62597108de236e53131559ffef796146011a5796dbb7ef3368`.
User worlds, genomes, settings, autosaves and the pinned vcpkg submodule are not
changed. No CodeQL scan or submodule update was run.

## GitNexus evidence and limits

Pre-edit query/context and upstream impact analysis covered the reader, window,
renderer and common parser. Known consumers include F1, the main GUI loop/window,
`GenericMessageDialog` and `McpDocumentationTools`. Reader method calls were
reported UNKNOWN despite these text-confirmed consumers; zero graph callers were
not treated as evidence that the methods were unused. The window's disambiguated
class report was LOW (5 impacted symbols); the renderer was MEDIUM (36 impacted).
A broad forward declaration initially produced a CRITICAL candidate and was
explicitly disambiguated. `WindowController` is HIGH with a virtual-dispatch
boundary; its only edit grants the isolated documentation test fixture access.

The index was refreshed using `node .gitnexus/run.cjs analyze --index-only`.
Analyzer diagnostics report bounded process enumeration, receiver-resolution gaps,
cross-language field boundaries and excluded large vendor files. The server can
retain an earlier index view, so final graph analysis is also performed through
the CLI. Newly added methods and test macros are supplemented by source review,
compilation, complete GUI regressions and exact MCP baseline checks.

Final `detect_changes(scope="all")` was run after staging all 16 changed files.
Both CLI and MCP report 96 changed symbols, 5 affected rendering processes and
MEDIUM risk. The structured MCP response includes all 96 symbols and has neither
`partial: true` nor `truncated: true`. The affected flows are Render → GetSpanFont,
Render → GetSpanColor, Render → Scale, Render → Max and Render → Get; these cover
heading/text styling, scaling and rendering-facade access. The common renderer's
other consumer, `GenericMessageDialog`, is covered by the full GUI regressions.
This complete change-analysis response does not remove the analyzer's documented
static-resolution and process-enumeration limits.

## Boundaries and remaining work

Automated ImGui checks are not manual screenshot/visual acceptance. Texture facade
calls are verified with a double; no claim is made about a new end-to-end Vulkan
texture allocation test. External URL targets are parsed and preserved, and the
existing dispatch branch is unchanged; tests do not launch a real browser.

L7-B–D must translate the real chapters/contents, review terminology and screenshots,
maintain heading correspondence or update native links, and validate new assets.
This implementation adds no translated production chapter and makes no claim of
Russian documentation completeness. Installer-specific packaging and semantic
heading-reordering detection remain outside this stage; nested resource copying
continues to use the existing CMake resource-copy mechanism.
