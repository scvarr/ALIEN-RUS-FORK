# CUDA runtime discovery fix and English build baseline

Date: 2026-10-20 (Europe/Moscow).

The CUDA runtime discovery blocker is fixed and the complete Release build succeeds. Runtime tests expose a separate engine-test failure on RTX 5080. Validation stops for review at that failure; this is not an all-green baseline and does not authorize starting localization L1.

## Scope and original failure

Workspace: `E:\Projects\ALIEN`. Branch: `feature/ru-localization-foundation`. Starting commit: `4008ab76dd2e965d67c923b5835e60b59e655f44`; original ALIEN source: `d1365091ae51b091ab205407cb4142e61f5644fc`.

The original Windows CUDA deployment block derived the directory of `CMAKE_CUDA_COMPILER` and searched only that directory for `cudart64_*.dll`. On this installation `nvcc.exe` is in `CUDA/v13.2/bin`, but the runtime DLL is in `CUDA/v13.2/bin/x64`. Configuration therefore failed before ALIEN compilation, as recorded historically in `docs/localization-l1-validation.md`. That report is unchanged.

Only the root `CMakeLists.txt` and this new report are changed. No source, engine/CUDA algorithm, test expectation, architecture, compiler selection, dependency version, HIP behavior, or localization implementation is changed. No upstream update, submodule update, remote creation, push, merge, or PR was performed.

## Exact CMake behavior

The existing `if(WIN32 AND NOT USE_HIP)` block now:

1. Quotes `CMAKE_CUDA_COMPILER` when obtaining its directory.
2. Searches `${CUDA_BIN_DIR}/cudart64_*.dll`, excluding directories.
3. Only when the first search is empty, searches `${CUDA_BIN_DIR}/x64/cudart64_*.dll`, also excluding directories.
4. Reports both checked glob paths in a fatal error if neither location contains a DLL.
5. Requires exactly one candidate in the selected location. Multiple candidates produce a fatal ambiguity error listing the compiler and candidates; no sorting/index-based choice is made.
6. Retains the existing `POST_BUILD` `copy_if_different` command and `TARGET_FILE_DIR:alien` destination.

The first location has explicit priority. A unique legacy-location DLL wins even if `x64` also contains a DLL. Multiple legacy-location candidates fail rather than falling through to `x64`. There is no recursive or PATH search and no discovery of other toolkit installations. The change introduces no additional CMake module or dependency.

## Discovery checks

Seven isolated CMake script checks used the actual discovery block extracted from the edited root file, temporary fixture directories (including paths with spaces), and the Visual Studio-bundled CMake. They did not configure another project build tree or change an installed toolkit.

| Case | Result |
| --- | --- |
| One DLL beside compiler | Correct legacy path selected |
| One DLL only in `bin/x64` | Correct fallback selected |
| One DLL in each location | Legacy location takes priority |
| Neither checked path contains a DLL; another DLL is placed on PATH | Fatal error; both checked paths reported; PATH DLL ignored |
| Multiple DLLs beside compiler, with a unique fallback also present | Fatal ambiguity error |
| Multiple DLLs only in `bin/x64` | Fatal ambiguity error |
| Directory named like a runtime DLL beside compiler | Directory ignored; unique fallback DLL selected |

Discovery checks: **7 passed, 0 failed**. These are separate from the ALIEN GoogleTest counts below. Temporary scripts were not added to the repository.

## Toolchain and complete build

Command, executed from the repository root:

```text
build-windows-ninja.bat agent
```

| Item | Observed result |
| --- | --- |
| Visual Studio developer prompt | 2022, 17.14.35, installation `F:\VS` |
| MSVC | 19.44.35228.0, toolset directory `14.44.35207` |
| CUDA | Toolkit 13.2, `nvcc` V13.2.51 |
| Selected compiler | `C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.2/bin/nvcc.exe` |
| Selected runtime | `C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.2/bin/x64/cudart64_13.dll` |
| GPU | NVIDIA GeForce RTX 5080, reported compute capability 12.0 |
| Driver | 617.42 |
| Preset / generator | `ninja-agent` / Ninja Multi-Config |
| CMake configuration and generation | Passed |
| Build | **Exit 0; all 403 steps completed** |
| Dependency installation | Existing pinned packages reused; no version/pin changes |

The build produced `build-agent/Release/alien.exe`, `alien-cli.exe`, `BaseTests.exe`, `DataTests.exe`, `EngineInterfaceTests.exe`, `McpToolsTests.exe`, `NetworkTests.exe`, `PersisterTests.exe`, and `EngineTests.exe`. The output layout remains the standard `build-agent/Release` structure. `build-agent/Release/resources/docs/contents.md` exists, confirming resource deployment. IDE-owned build trees were not used.

The generated ALIEN link recipe includes `cudart.lib`. Runtime-dependent engine execution was exercised by the standard GPU suite below. The deployed `build-agent/Release/cudart64_13.dll` is 555,120 bytes and has the same SHA-256 as the selected toolkit DLL:

```text
7c50bf7f7398b5d1b654af19ff6a679988cf251cab677dbd8abac69fa1cce310
```

Thus runtime discovery and copying are verified independently of whether a GUI simulation works. Successful linking and packaged DLL presence alone are not an application smoke-test result.

## Profile protection and automatic tests

Before running the ALIEN test applications, no `alien`/`alien-cli` process was running and the existing `HKCU\SOFTWARE\alien` key was exported outside the repository. A second export after validation had the same SHA-256; the user settings remained unchanged and no restoration was needed. No registry contents or credentials are committed.

All seven standard test applications ran from `build-agent/Release` with GoogleTest XML output saved outside the repository. The first six exited 0. The complete standard `EngineTests` suite ran to completion without cancellation, exiting 1 after approximately 82.983 seconds. No additional GPU stress run, targeted retry, automatic reboot, or engine fix was attempted.

| Suite | Registered | Passed | Failed | Skipped | Disabled | Exit |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| BaseTests | 25 | 25 | 0 | 0 | 0 | 0 |
| DataTests | 247 | 247 | 0 | 0 | 0 | 0 |
| EngineInterfaceTests | 2 | 2 | 0 | 0 | 0 | 0 |
| McpToolsTests | 40 | 40 | 0 | 0 | 0 | 0 |
| NetworkTests | 38 | 38 | 0 | 0 | 0 | 0 |
| PersisterTests | 142 | 141 | 0 | 0 | 1 | 0 |
| EngineTests | 2,200 | 2,195 | 1 | 2 | 2 | 1 |
| **Total** | **2,694** | **2,688** | **1** | **2** | **3** | — |

The first six suites account for **493 passed**. The table and overall total above are calculated from the XML results. Disabled tests were not executed; skipped tests are not counted as passed. No XML test-framework errors were reported.

The failure is:

```text
CommunicatorTests.sender_signalPriority_signalReceived
source/Engine/Impl/Tests/CommunicatorTests.cpp:277
Expected receiver.getCellRef()._neuralActivity._signals[0] == -1.0f
Actual: 1
```

The test sets up two senders and an intermediate receiver, expecting the sender in the permitted opposite half-plane to determine the received signal. This run violates that expectation. The cause, reproducibility, hardware specificity, and whether the test or implementation is wrong have not been established. This is a separate engine-validation blocker exposed after successful configuration/build, not a reason to modify communicator code in this task.

The two skipped EngineTests cases are `ConstructorTests_ProvideEnergy/ConstructorTests_ProvideEnergy_Separation.provideEnergy_insufficientEnergy/2` and `/5`.

## RTX 5080 application verification and stopping point

The standard GPU suite demonstrably executed on the available RTX 5080 and completed without a hang. This establishes substantial engine-test execution, but not complete engine correctness: one assertion failed.

Following the user's instruction to stop for review when new independent failures are found, no GUI/CLI smoke scenarios were started after the engine-suite failure:

| Scenario | Result |
| --- | --- |
| `alien.exe` starts and closes normally | Not tested |
| Create a simple GUI simulation | Not tested |
| Save/open a simple GUI simulation | Not tested |
| `alien-cli` help / simple simulation run | Not tested; executable built successfully |
| Visual rendering / interactive simulation on RTX 5080 | Not tested |

No claim is made that the interactive application works or that the baseline is ready for localization. Original user settings were not altered by a GUI run.

## Evidence and remaining blockers

The complete build log, suite logs/XML, exit codes and profile backups are stored locally under:

```text
C:\Users\infarh\AppData\Local\Temp\alien-cuda-baseline-20261020-195723
```

Discovery fixtures are under `%TEMP%/alien-cuda-runtime-cases`. These are temporary uncommitted evidence; essential results are preserved in this report. Build outputs remain in ignored `build-agent` directories.

Remaining work, subject to review:

1. Investigate the communicator signal-priority test failure as a separate task and determine its scope before changing engine code or expectations.
2. Re-establish an accepted automated baseline after that review.
3. Complete the protected-profile English GUI launch/close and small simulation create/save/open checks on RTX 5080, plus a bounded CLI smoke check.
4. Decide explicitly whether the baseline permits proceeding with L1. No localization work has started here.

At completion, the full diff and whitespace checks were reviewed, the historical reports remained unchanged, `external/vcpkg` had no tracked changes, and the main repository's `git remote -v` output was empty. The local commit contains only the runtime-discovery fix and this report.
