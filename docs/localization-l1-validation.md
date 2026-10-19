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
