> Current implementation and validation are recorded in the 2026-10-21 section below. The original blocked attempt is preserved as historical evidence.

# ALIEN L1 validation: baseline build blocked

Date: 2026-10-19 (Europe/Moscow).

**L1 implementation was stopped at the mandatory baseline-build gate.** The original application could not be configured with the installed CUDA toolkit using the required build script. No localization service, catalog, font, UI change, or localization test was added. This document records a prerequisite failure, not completion of L1.

## Repository and safety

- Workspace: `E:\Projects\ALIEN`.
- Original ALIEN source: `d1365091ae51b091ab205407cb4142e61f5644fc`.
- Starting audit commit: `b1e8b75b3067fde8cfba2fd80e6d3dac2d69e177`.
- Local branch created from the current audit branch: `feature/ru-localization-foundation`.
- The audit and repository `AGENTS.md` were read before the build.
- The main repository has no remotes; `push.default` remains `nothing`.
- No upstream fetch/update, push, merge, fork, PR, or submodule update was performed.
- `external/vcpkg` remains at `af752f21c9d79ba3df9cb0250ce2233933f58486`, with no tracked changes. The build installed the pinned dependencies and populated ignored build/download/cache files; it did not change dependency declarations or pins.

Before any possible application/test launch, the existing `HKCU\SOFTWARE\alien` settings key was exported to a backup outside the repository. No running `alien` or `alien-cli` process was found. A second registry export after the failed build had exactly the same SHA-256 as the first export. Existing settings were therefore verified unchanged, and no restore was necessary. Registry contents and credentials are not included in this report or commit.

Local evidence directory:

```text
C:\Users\infarh\AppData\Local\Temp\alien-l1-20261019-103649
```

It contains `baseline-build.log`, `baseline-build-exit.txt`, and the before/after registry exports. These temporary local artifacts are not committed and may be removed by normal temporary-file cleanup. Essential failure evidence is retained below.

## Original baseline build

The tracked working tree was clean before running the requested command from the repository root:

```text
build-windows-ninja.bat agent
```

The output was captured without cancelling the build. The script selected `ninja-agent`, used Ninja Multi-Config, and configured into `build-agent`. The script's default final application configuration is Release. vcpkg also built Debug variants where required by its triplet.

| Item | Observed value |
| --- | --- |
| Visual Studio | Visual Studio 2022, developer prompt 17.14.35, installation `F:\VS` |
| Host compiler | MSVC 19.44.35228.0; toolset directory `14.44.35207` |
| CMake selected by script | 3.31.6-msvc6, bundled with Visual Studio |
| Ninja selected by script | 1.13.1, bundled with Visual Studio |
| CUDA compiler | NVIDIA 13.2.51, toolkit 13.2 |
| GPU | NVIDIA GeForce RTX 5080, reported compute capability 12.0 |
| Driver | 617.42 |
| vcpkg installation | All 87 requested packages installed successfully; reported installation time 9.5 minutes |
| C/C++ compiler checks | Passed |
| CUDA compiler identification / ABI checks | Passed |
| CMake configuration | Failed at root `CMakeLists.txt:396` |
| Overall build script exit code | **1** |
| ALIEN target compilation/linking | Not reached |

The fatal output was:

```text
-- The CUDA compiler identification is NVIDIA 13.2.51 with host compiler MSVC 19.44.35228.0
-- Detecting CUDA compiler ABI info - done
-- Detecting CUDA compile features - done
...
CMake Error at CMakeLists.txt:396 (message):
  Could not find cudart64_*.dll in C:/Program Files/NVIDIA GPU Computing
  Toolkit/CUDA/v13.2/bin

-- Configuring incomplete, errors occurred!
```

The earlier failed `CMAKE_HAVE_LIBC_PTHREAD` probe is not the fatal cause: CMake subsequently reported `Found Threads: TRUE` on Windows.

## Diagnosis

Root `CMakeLists.txt:392–396` derives `CUDA_BIN_DIR` from the directory of `CMAKE_CUDA_COMPILER`, then searches only `${CUDA_BIN_DIR}/cudart64_*.dll`. The search is not recursive. The CMake cache records this compiler:

```text
C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.2/bin/nvcc.exe
```

The installed CUDA runtime was found at:

```text
C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.2/bin/x64/cudart64_13.dll
```

The DLL exists and is 555,120 bytes. Both `lib/x64/cudart.lib` and `lib/x64/cudart_static.lib` also exist. Thus the observed failure is the original runtime-deployment search assuming a DLL beside `nvcc.exe`, whereas this installation stores the DLL in `bin/x64`. It is not evidence that the runtime is absent or that RTX 5080 cannot run ALIEN. Conversely, successful CUDA compiler probes do not prove the application or engine tests work.

No DLL was copied into the toolkit, no environment shim was installed, and no CMake/CUDA/source patch was made to conceal or bypass the failure. No retry with altered configuration was performed. The failed build tree and pinned dependency installation were retained for diagnosis and a future baseline retry; the IDE-owned `build-ninja` trees were not used.

Before L1 can proceed, the baseline runtime-discovery/toolchain-layout issue must be resolved in a separately authorized maintenance step, or a supported toolkit installation with a compatible layout must be selected and verified. That resolution must be documented independently of localization. Then rerun the original build, all available suites, and the English application smoke test. Further build/runtime failures may become visible after this first blocker is resolved.

## Tests and visual validation

A recursive check of `build-agent` found none of the ALIEN application/test executables listed below. No stale binaries were used as substitute baseline evidence.

| Validation | Result |
| --- | --- |
| BaseTests | Not run: executable not produced |
| DataTests | Not run: executable not produced |
| EngineInterfaceTests | Not run: executable not produced |
| McpToolsTests | Not run: executable not produced |
| NetworkTests | Not run: executable not produced |
| PersisterTests | Not run: executable not produced |
| EngineTests on RTX 5080 | Not run: executable not produced |
| Original English GUI on RTX 5080 | Not launched: `alien.exe` not produced |
| Russian glyph rendering / mixed text | Not performed: implementation gate not passed |
| Runtime language switch / restart persistence | Not performed: implementation gate not passed |
| Dialog/control ID and layout stability | Not performed: implementation gate not passed |
| Post-localization build/tests | Not applicable: no localization code changes |

The installed GoogleTest dependency being built successfully is not an ALIEN test-suite pass. No visual validation was performed.

## Localization architecture and changed files

**Created localization architecture: none.** The implementation was not started because the user explicitly required stopping on an unsuccessful baseline build. The proposal in `docs/localization-audit.md` remains a design reference, not an implemented capability.

After the prerequisite is resolved, L1 still needs the small CPU-only service for `en`/`ru`, UTF-8 JSON catalogs, canonical English fallback, semantic-key/catalog validation, and `settings.language` persistence. It also needs licensed Cyrillic text fonts with icon/scaling preservation, the Settings/Display Settings translation slice, and backward-compatible stable IDs for that slice. These require validation against actual code rather than treating the audit as a ready patch. No font was added or redistributed during this attempt.

The only tracked file added in this attempt is:

```text
docs/localization-l1-validation.md
```

Existing source code, CUDA, physics, CMake/build scripts, simulation formats, internal parameter names, protocols, MCP schemas, original documentation, dependency declarations, and the audit document remain unchanged. The commit is documentation-only. The complete staged diff and whitespace checks were reviewed before committing; repository remotes and submodule state were checked again at completion.

## Remaining work and L2 readiness

1. Resolve the original baseline configuration failure without mixing it with localization work.
2. Obtain a successful unchanged-English baseline build and record all available suite results and an RTX 5080 smoke test, with original settings protected.
3. Implement and validate all requested L1 functionality, including missing/broken catalog behavior, fallback, key checks, settings restoration across launches, font coverage including Yo, and actual ImGui ID/modal-state stability across language switches.
4. Rebuild and rerun the suites, inspect the full implementation diff, and update this validation report with actual evidence.
5. Begin L2 only after L1 passes. Broader UI extraction, documentation translation, localized search, the separate cell-label atlas UTF-8 defect, third-party UI text, and cloud/filesystem input compatibility remain later work; none is fixed by this baseline diagnosis.

**Readiness:** L1 is incomplete and L2 is not ready to start. No claim of a working localization foundation or successful RTX 5080 application execution is made.

---

## L1 implementation and validation — 2026-10-21

### Scope and ancestry

Implementation started at `bf123b188ac9d15d4f4616e6e0384aa96ffe9e91` on `fix/communicator-test-direction`. The new local branch is `feat/ru-localization-l1`; all previous branches and ancestry are retained. CUDA runtime discovery commit `6db2b56fbbbf1d8fbd774d653a0481f21efdc47a` and the communicator fixture correction are ancestors. The accepted automatic baseline is recorded separately in `docs/communicator-baseline-validation.md`: 2,689 passed, zero failed, two skipped and three disabled. The original failed baseline above remains historical evidence.

Only the localization foundation and Settings / Display settings slice are implemented. No engine algorithms, CUDA structures or kernels, simulation formats, existing settings names, protocols, dependencies or submodule pins are changed. The main repository has no remote. No Git fetch, push, merge, PR, submodule update or CodeQL operation was performed. L2 is not started.

### Service and catalog contract

`Base/LocalizationService` is a GUI-thread service with no CUDA, engine, rendering or GLFW dependency. It uses the existing Boost.JSON and Boost.PropertyTree libraries. `en` is the initial and fallback language; `ru` is the other supported language. Initialization runs at the start of the main-window constructor, before GLFW, rendering, fonts and main-loop entities. The existing facade lifecycle is unchanged.

Catalogs are flat UTF-8 JSON objects without a BOM, at `resources/locales/en.json` and `ru.json`. Each member maps a semantic key to a nonempty string, for example:

```json
{
    "display.title": "Display settings",
    "display.fps": "Frames per second"
}
```

Keys match `[a-z][a-z0-9]*(\.[a-z][a-z0-9]*)+`. JSON syntax, object/string types, UTF-8, empty strings, control characters and accidental ImGui `##`/`###` markers are validated. Newlines and tabs are allowed; NUL and other C0 controls are rejected. Boost.JSON validates types and strict UTF-8. PropertyTree retains member counts, allowing duplicate keys, including escaped equivalents, to be rejected instead of silently overwriting text. Invalid catalogs are rejected atomically. Diagnostics use the existing logger and are also available through `getDiagnostics()`.

Unknown requested keys return semantic-key markers and are logged once per load. Unknown Russian entries are diagnosed and ignored; missing Russian entries fall back individually to English. CMake embeds canonical English from the same `en.json`, so two missing or corrupt installed catalogs still leave the shipped interface usable in English. Valid external English entries override or extend that baseline. There is no second hand-maintained fallback dictionary.

To add a message, choose a semantic key, add English and Russian strings, use `tr(key)` for presentation, run completeness tests and rebuild. No service modification is needed. External dictionaries can introduce keys through the English catalog first. The existing full-resource copy remains; a `LocalizationResources` build target additionally refreshes catalogs, fonts and licenses in both build-root and configuration-specific resource directories on every build, even without an executable relink. The Release directory remains the deployment payload used by Windows packaging.

`getText()` and `tr()` return owning strings. Retaining a pointer obtained from a temporary translation is forbidden. Parameters, modal titles, messages and option lists own strings or semantic keys. `.c_str()` is used only during synchronous ImGui calls while its owner is alive. Delayed work captures values, never catalog pointers.

### Persistence and frame boundary

`GlobalSettings::getLanguage()` / `setLanguage()` use the new `settings.language` key. Accepted values are exactly `en` and `ru`; absent, empty, uppercase and unsupported values resolve to `en`. Existing settings names and serialized formats are unchanged. `requestLanguage()` queues a change; `applyPendingLanguage()` persists and applies it before the next `ImGui::NewFrame()`.

Both Settings and Display settings contain the same language selector, with `English` and `Русский` choices. The selector in the dialog permits EN → RU → EN while the modal stays open, since a modal otherwise blocks the main menu. Language changes do not reopen the dialog, reread its pending controls, rebuild fonts or call engine reinitialization. Resolution captions are regenerated from the existing copied mode list without changing selection indices. Interface scaling retains its existing restart requirement; the confirmation is translated. Language changes themselves require no restart.

Two existing Windows settings issues are corrected: registry opening begins with an empty `RegKey`, allowing create-if-missing behavior for a fresh profile; wide-to-UTF-8 conversion uses the same explicit input length for sizing and conversion. Tests cover fresh isolated storage, restoration by a new service instance, and Cyrillic/punctuation round trips. The resolution combo's revert path also checks absent enable/default-enable pointers instead of dereferencing them.

### Dear ImGui identity rules

- In the affected `AlienGui` wrappers, `.name()` retains the original stable identity, while optional `.label()` supplies display text. Existing callers retain their behavior. Display settings keeps legacy widget scopes, including sliders and revert buttons.
- Native labels use `translated caption + "###stable-id"`. `##suffix` only hides the suffix and still hashes the translated prefix, so it does not isolate identity from language. Settings menu/button identities are constant semantic identifiers.
- `AlienWindow` retains its original internal `Begin()` name and ID stack. An optional title key affects only its custom titlebar. Existing windows are not migrated or mass-refactored.
- `AlienDialog` delegates an optional title key to `ModalWindow`. A localized modal uses the same stable suffix for `OpenPopup()` and `BeginPopupModal()`; its content `PushID()` retains the original title.
- Loading `gui.imgui settings` migrates only the legacy `Display settings` and `Content scaling` window-section headers in memory to stable `###` identities. Position/size values and the settings key are preserved. Unrelated windows and dialogs keep their existing names.
- The scaling confirmation stores message/title keys in a localized mode of the existing message dialog. Yes/No are translated only for that mode; other messages remain English.

### Fonts and licenses

Three unmodified static fonts are shipped: Noto Sans Regular, Noto Sans Bold and Noto Sans Mono Regular. They come from the [Noto repository at commit ffebf8c1ee449e544955a7e813c54f9b73848eac](https://github.com/notofonts/noto-fonts/tree/ffebf8c1ee449e544955a7e813c54f9b73848eac). Source paths are recorded in `resources/fonts/README.md`. The complete copyright notice and [SIL Open Font License 1.1](https://github.com/notofonts/noto-fonts/blob/ffebf8c1ee449e544955a7e813c54f9b73848eac/LICENSE) are distributed as `resources/fonts/OFL.txt`.

`TextFontService` supplies persistent glyph ranges for Latin, Cyrillic, typographic punctuation and U+2116. All ordinary, tiny, bold and monospace roles use these complete assets at their previous sizes multiplied by the existing content-scale factor. Ё/ё, №, en/em dashes, guillemets and curly quotation marks are covered. FontAwesome merges, AlienIconFont and Reef remain in place. Both languages share the startup atlas; switching language does not replace font pointers or GPU font resources. The separate simulation cell-label atlas and its byte-wise Unicode defect are intentionally unchanged.

### Translated elements

The shipped slice contains 25 semantic entries with identical English/Russian key sets:

| Element | Russian display |
| --- | --- |
| Settings | Настройки |
| Save on exit | Сохранять при выходе |
| Dark mode | Тёмная тема |
| Display settings menu item and dialog title | Настройки экрана |
| Network settings menu item | Настройки сети |
| Debug mode | Режим отладки |
| Language, in menu and dialog | Язык |
| Language choices | English / Русский |
| Full screen | Полноэкранный режим |
| Resolution | Разрешение |
| Desktop option | Как на рабочем столе |
| Refresh-rate suffix Hz | Гц |
| Frames per second | Частота кадров |
| Frame-rate tooltip | Russian explanation of GPU workload and simulation steps per second |
| Adopt scaling from OS | Использовать масштаб ОС |
| Automatic-scaling tooltip | Russian explanation of operating-system display scaling |
| Content scaling and confirmation title | Масштаб интерфейса |
| Scaling confirmation message | Russian save-and-close confirmation |
| Adopt / Cancel | Применить / Отмена |
| Scaling confirmation Yes / No | Да / Нет |
| Revert changes tooltip in Display settings | Отменить изменения |

Network settings itself, other windows/editors, engine messages, documentation and internal configuration/serialization/protocol keys remain unchanged.

### Automatic validation

The required `build-windows-ninja.bat agent` completed successfully in Release. A fresh agent build compiled all **411 steps**; the IDE-owned trees were not used. MSVC 19.44, CUDA 13.2 and the existing RTX 5080 were retained. No CUDA structure/header changed.

The local CMake/MSVC include-prefix detection is misencoded: Ninja reports zero tracked header dependencies for affected C++ objects. Setting `VSLANG=1033` did not correct this installation's generated prefix. To exclude stale ABI/layout objects, the old `build-agent` was moved into ignored `temp/l1-build-before-clean-20261021`, a new tree was configured, and only the unchanged pinned dependency installation was reused. No compiled ALIEN object/library was reused from the archive. Recursive deletion had been rejected by automatic command review, so the reversible archive was used. This local incremental-build issue predates L1 and still needs separate toolchain maintenance; use a fresh agent tree for header changes until then.

All eight Release suites ran with GoogleTest XML output. The first ordinary GPU regression completed in **275.403 seconds**. A subsequent artifact rebuild also rebuilt CUDA objects, so the affected GUI, persistence and engine suites were rerun; the second full GPU regression passed in **362.225 seconds**. Neither run was cancelled, and no stress test or `gtest_repeat` run was used. The table counts each test once, not the sum of repeated validation runs:

| Suite | Passed | Failed | Skipped | Disabled |
| --- | ---: | ---: | ---: | ---: |
| BaseTests | 40 | 0 | 0 | 0 |
| DataTests | 247 | 0 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 | 0 |
| PersisterTests | 141 | 0 | 0 | 1 |
| EngineTests | 2,196 | 0 | 2 | 2 |
| GuiLocalizationTests | 7 | 0 | 0 | 0 |
| **Total** | **2,711** | **0** | **2** | **3** |

This preserves all 2,689 baseline passes and adds **15 service/settings tests plus 7 headless GUI tests**. Existing skips and disabled cases are unchanged. Service tests exercise both shipped catalogs, missing/damaged/duplicate catalogs, invalid structures and UTF-8, unknown keys, English fallback, dictionary extension, deferred switching, owning strings, persistence, invalid language values, settings UTF-8 round trips and complete matching translations. GUI tests exercise actual ImGui widgets and modal/window wrappers, legacy ID scopes, open state/geometry/edit preservation through EN → RU → EN, persisted geometry migration, real glyph coverage in every text role, retained icons/atlas, and scaled font loading at 1x, 1.5x, 2x and 4x. They require no application window or initialized GPU engine.

After a final nonfunctional constant-name cleanup, the required build succeeded again and all seven CPU/headless suites passed again. Engine sources and CUDA objects were unchanged by that cleanup; their full-regression evidence is the second GPU run above. No further source changes followed these checks.

Catalogs, all three font binaries, license and provenance file were compared byte-for-byte with both output resource directories: **7 files matched in each destination**. The same Release layout is archived by the existing packaging workflow. No release upload or package publication was attempted. A fontTools cmap inspection independently found all 66 Russian letters and the required punctuation/symbols in each new font.

clang-format **19.1.5** checks pass for all 23 eligible modified/new C++ files. `GlobalSettings.h` and `AlienGui.cpp` were already non-clean at HEAD and were not whole-file formatted. The complete source/catalog/documentation diff and whitespace checks were reviewed. The original failed report remains a verbatim substring of this document.

Temporary evidence (build logs, XML, format and packaging checks, private profile backups) is stored outside Git at:

```text
C:\Users\infarh\AppData\Local\Temp\alien-l1-20261021-nt3i7t9o
```

The initial implementation build/test failures were corrected before final regression and are not counted as passes. Baseline test expectations and fixtures are unchanged.

### Profile protection and GUI status

At the user's explicit request, no GUI application launch, screenshot, visual inspection or real simulation workflow was attempted. An already running installed copy at `E:/Program files/alien-develop/alien.exe` was left untouched. Before executable checks, registry exports, typed snapshots and copies of source/output autosave directories were saved outside the repository.

Localization persistence tests redirect only their own process's `HKEY_CURRENT_USER` to a dedicated temporary test key, then restore the mapping and remove the key. The user's profile is not a test fixture. Linux tests use a process-local temporary XDG directory that outlives GlobalSettings. Headless GUI tests create only an ImGui context and CPU atlas, without initializing GLFW, Vulkan or the simulation facade. No user simulation is loaded by these tests.

The test registry branches were removed; none remained after the runs. The real profile retained the same value names and types. Its only observed content difference was `gui.imgui settings` while the separate installed application remained running; that live state was not rolled back. Localization settings tests were confined to their redirected process-local profile. Source autosaves matched the initial backup; the preexisting output autosave was restored to the fresh build tree and verified by SHA-256. No private settings, credentials or autosaves are committed.

Manual checks remain explicitly unverified: EN → RU → EN in the real renderer, switching with Display settings open, visual text/layout quality, restart persistence, OS/manual scaling, and behavior of a running simulation. The user will perform them. Automatic ID, font and persistence checks do not replace visual validation.

### Limitations and L2 recommendations

The L1 implementation is limited to the selected slice. Visual validation remains pending by user choice. Only the Windows CUDA build and available GPU were exercised; Linux/HIP portability has not been established by execution. There is no runtime file watcher, plural selection, interpolation grammar or generalized Unicode search; add these when later screens need them. English fallback is resilience behavior, not evidence of complete translation.

Expand L2 one reviewed screen at a time, extending completeness and state-preservation tests. Keep specifications, enums, settings names, serialized fields and protocols canonical. Address the cell-label atlas, Unicode search and filesystem/cloud/input compatibility as separately tested changes. Preserve font licenses and package checks. Bulk translation is not started automatically.

## L1 startup regression correction — 2026-10-21

After commit `681b79acf540bc1c74801d777f8a08b4afcd5d38`, the user reported that `build-agent/Release/alien.exe` displayed a console briefly and exited. The existing application journal reached successful CUDA/Vulkan initialization and stopped at `initialize main loop elements`; Windows recorded exception `0xc0000005`. This was a startup regression in L1, not a missing-resource working-directory failure.

The cause was removal of the explicit atlas build from `StyleService::setup()`. In ImGui 1.91.9, adding fonts does not initialize `ImFont::ContainerAtlas`; atlas construction does. `SimulationView::setup()` immediately passes the default font to simulation rendering, whose existing cell-label preparation dereferences `labelFont->ContainerAtlas` before the first ImGui frame. With the unbuilt atlas that pointer was null. The headless GUI fixture had called `Fonts->Build()` itself, hiding the production startup defect.

The fix builds the complete atlas once at the end of `StyleService::setup()`, after all text and icon fonts have been registered, and reports a normal exception if construction fails. No cell-label renderer, engine, font ranges, locale switching or GPU synchronization logic is changed. Language switching still uses the same startup atlas without rebuilding it.

The test fixture no longer performs the missing production work. A new regression test checks that the atlas and every text/icon font are ready immediately after setup and exercises the default font's atlas pixel access used by rendering, before any frame or explicit test-side build. It failed before the fix (`IsBuilt()` was false) and passed afterward. The required `build-windows-ninja.bat agent` succeeded, and all **8 GuiLocalizationTests passed**. clang-format 19.1.5 and diff whitespace checks passed. The earlier full-suite results remain historical evidence; only the affected headless GUI suite was rerun for this GUI-only correction.

Evidence is stored alongside the previous logs as `startup-regression-before.*`, `startup-fix-build.log` and `startup-fix-gui.*`. ALIEN has no GitNexus index, so diagnosis used the actual startup source, pinned ImGui source and existing crash evidence directly. No GUI launch, visual check, user-profile modification or real-simulation test was performed by the agent for this correction. Real-window validation remains with the user.

To launch the rebuilt application from PowerShell with its resource directory correctly resolved:

```powershell
Set-Location 'E:\Projects\ALIEN\build-agent\Release'
.\alien.exe
```

## L1 language-popup interaction correction — 2026-10-21

The user reported that the Settings language selector lost focus before Russian could be selected. The custom `AlienGui::BeginMenu()` closed the menu whenever the pointer left the menu/button rectangles. A combo list is a separate ImGui popup and can extend beyond its parent menu, so moving within that list incorrectly stopped submission of the owning menu and selector.

The outside-rectangle check now preserves a menu while its focused popup hierarchy is active. This uses ImGui's popup and child-focus queries; it does not force focus back to the parent window. Normal pointer-leave closure remains when no popup is active, and an unrelated focused window does not keep the menu open. Locale persistence and application frame-boundary handling are unchanged.

A new headless integration test drives actual ImGui mouse events through the production menu and language selector. It opens the combo, moves into the portion of the popup beyond the parent rectangle, selects Russian, reopens Settings and selects English, then verifies ordinary closure after leaving the menu with the popup dismissed. The old code failed because `menuOpen` became false while the pointer was still inside the popup; the corrected code passes. The test also verifies that queued language changes take effect and that each selection dismisses the combo.

`build-windows-ninja.bat agent` succeeded, and all **9 GuiLocalizationTests passed**. The test source passes clang-format 19.1.5; `AlienGui.cpp` remains exempt from whole-file formatting because it was already non-clean at HEAD. Whitespace/diff checks passed. This GUI-only correction does not rebuild or change engine/CUDA code, so the full GPU regression was not repeated. Logs and XML are stored with the earlier evidence as `language-popup-before.*`, `language-popup-fix-build.log`, `language-popup-final-build.log` and `language-popup-gui.*`.

No real GUI launch or visual inspection was performed by the agent. Mouse events were simulated entirely inside the headless ImGui test context with isolated test settings. The user should restart the rebuilt `build-agent/Release/alien.exe` to load the correction.
