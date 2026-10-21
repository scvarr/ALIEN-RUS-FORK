# Communicator direction fixture and English baseline validation

Date: 2026-10-21 (Europe/Moscow).

The failure was caused by the test scene's sender directions. Both senders actually faced east, with the receiver on the boundary accepted by both directional gates. Changing their front angles from 0 to 90 degrees makes them face south and leaves only the intended sender eligible. The original `-1.0f` assertion is unchanged. All active automatic tests now pass. Visual validation and the GUI create/save/open workflow remain for the user to check, as requested during this task.

## Repository scope

- Workspace: `E:\Projects\ALIEN`.
- Branch created from the starting checkout: `fix/communicator-test-direction`.
- Starting commit: `6db2b56fbbbf1d8fbd774d653a0481f21efdc47a`.
- Original ALIEN revision: `d1365091ae51b091ab205407cb4142e61f5644fc`.
- The CUDA runtime CMake fix remains in its existing separate ancestor commit, `6db2b56fbbbf1d8fbd774d653a0481f21efdc47a`.
- Final changes are limited to two front-angle values in `source/Engine/Impl/Tests/CommunicatorTests.cpp` and this report.
- No CUDA kernels, signaling rules, synchronization, expected assertion, disabled-test flags, dependencies, original documentation or localization code were changed. No upstream update, remote creation, push, merge or PR was performed.

GitNexus had no index for ALIEN, so diagnosis used direct source tracing and simulation readbacks instead of creating additional repository infrastructure.

## Root cause

`Channels::CommunicatorAngle` is channel **1**, and `Const::CommunicatorOneway_Default` is **true** (`source/Data/CellTypeConstants.h`). `SenderDesc` inherits that default.

`ObjectConnectionProcessor::calcReferenceDirection` normalizes the direction from a cell to its first connected object, with periodic-world correction. The scene connects sender 100 at `(100,100)` to helper 101 at `(101,100)`, and sender 200 at `(100,120)` to helper 201 at `(101,120)`. Both reference directions are therefore east, `(1,0)`.

The actual facing calculation in `ObjectConnectionProcessor.cuh` is:

```text
rotateClockwise(referenceDirection, frontAngle + angleSignal * 180 degrees)
```

The original fixture gave both senders `frontAngle(0.0f)`. Their angle channel is zero, so both face east. The receiver at `(100,110)` has displacement `(0,+10)` from sender 100 and `(0,-10)` from sender 200. Both dot products with east are zero.

`CommunicatorProcessor::tryTransmitSignal` rejects an oneway transmission only when `dot(toReceiver, senderFacing) > 0`. Equality is accepted. The two senders are distinct creatures, both have initialized fronts, both are within range, and the receiver accepts their colors/lineages. Their activation signals become `+1` and `-1`, both exceeding the manual-trigger threshold in absolute value. Thus both are eligible to transmit.

The receiver lock serializes each signal copy but supplies no deterministic sender-priority rule. The last accepted writer determines channel 0. Requiring one particular writer's `-1` result is invalid for this boundary scene. The observed runs consistently ended with `+1`; variation was not observed in the bounded sample, so this report does not claim an empirically measured random distribution.

This test constructs its senders directly. It does not use `createSenderCreature`, whose default angle-channel bias is `0.5` and would rotate an east reference by 90 degrees. That helper's south-facing default does not apply to this fixture.

## Actual state and timing checks

Temporary test-only instrumentation downloaded simulation data before execution and after each of the six single-timestep advances in `TIMESTEPS_PER_CELL_FUNCTION`. It recorded positions, velocities, first-connected helper positions, front angles, channels 0 and 1, the oneway flag, and receiver channel 0. Facing vectors and dot products were recomputed with the CPU Math functions from those GPU readbacks, using the same rotation rule. No device code was instrumented or changed.

The stepped diagnostic run is separate from the original, uninstrumented repeat run. Readbacks and single-step calls can affect scheduling; they establish state evolution, not a measurement of device write order. All temporary output code and includes were removed before the final build and regression.

| State over steps 0 through 6 | Original scene | Corrected scene |
| --- | --- | --- |
| Sender positions | `(100,100)`, `(100,120)` throughout | Unchanged throughout |
| Helper positions | `(101,100)`, `(101,120)` throughout | Unchanged throughout |
| Receiver position | `(100,110)` throughout | Unchanged throughout |
| Sender velocities | `(0,0)` throughout | `(0,0)` throughout |
| Sender front angles | `0`, `0` throughout | `90`, `90` throughout |
| Sender angle channel 1 | `0` throughout | `0` throughout |
| Sender channel 0 | Initially zero; `+1` / `-1` after step 1 through step 6 | Same |
| Oneway flag | True for both throughout | True for both throughout |
| Facing, computed from readback | `(1,0)` for both | Approximately `(-4.37114e-8,1)` for both |
| Dot product for sender 100 | `0` throughout: accepted boundary | `+10` throughout: rejected |
| Dot product for sender 200 | `0` throughout: accepted boundary | `-10` throughout: accepted |
| Receiver channel 0 | `0` initially, then `+1` on steps 1–6 | `0` initially, then `-1` on steps 1–6 |

`SimulationKernelsService.cu` calculates/sets neural signals before invoking the communicator kernel on a cell-function step. The default identity network copies the helper's channel 0 and leaves channel 1 at zero. Neither movement nor an angle-channel update rotates these senders during the tested interval. With positive Y pointing down in the interface, rotating east clockwise by 90 degrees is south. The corrected gate is well away from the zero-dot boundary.

## Exact permanent change

Within `CommunicatorTests.sender_signalPriority_signalReceived`, and nowhere else:

```diff
-            ObjectDesc().id(100).pos({100.0f, 100.0f}).type(CellDesc().frontAngle(0.0f).cellType(CommunicatorDesc().mode(SenderDesc().range(50)))),
+            ObjectDesc().id(100).pos({100.0f, 100.0f}).type(CellDesc().frontAngle(90.0f).cellType(CommunicatorDesc().mode(SenderDesc().range(50)))),
-            ObjectDesc().id(200).pos({100.0f, 120.0f}).type(CellDesc().frontAngle(0.0f).cellType(CommunicatorDesc().mode(SenderDesc().range(50)))),
+            ObjectDesc().id(200).pos({100.0f, 120.0f}).type(CellDesc().frontAngle(90.0f).cellType(CommunicatorDesc().mode(SenderDesc().range(50)))),
```

`EXPECT_FLOAT_EQ(receiver.getCellRef()._neuralActivity._signals[0], -1.0f)` is retained. The existing comments describing both senders as facing south now match the fixture.

## Reproduction, build and regression

Before editing the scene, the existing Release binary ran:

```text
EngineTests.exe --gtest_filter=CommunicatorTests.sender_signalPriority_signalReceived --gtest_repeat=10
```

Results: **0 passed, 10 failed**, all showing actual `+1` against expected `-1`. No alternative outcome was observed. These repeats reuse the process while the normal fixture resets the simulation between tests.

After removing diagnostics and retaining only the two front-angle changes, the final `build-windows-ninja.bat agent` completed successfully. A bounded repeat with the same filter and `--gtest_repeat=20` produced **20 passed, 0 failed**. Separate instrumented before/after runs each agreed with the state table above; they are not included in the repeat or full-regression counts.

All seven standard Release suites were then run once, from `build-agent/Release`, with GoogleTest XML output. The full EngineTests run completed in approximately 86.476 seconds. No long-duration stress run, kernel synchronization change or reboot was used.

| Suite | Passed | Failed | Skipped | Disabled |
| --- | ---: | ---: | ---: | ---: |
| BaseTests | 25 | 0 | 0 | 0 |
| DataTests | 247 | 0 | 0 | 0 |
| EngineInterfaceTests | 2 | 0 | 0 | 0 |
| McpToolsTests | 40 | 0 | 0 | 0 |
| NetworkTests | 38 | 0 | 0 | 0 |
| PersisterTests | 141 | 0 | 0 | 1 |
| EngineTests | 2,196 | 0 | 2 | 2 |
| **Total** | **2,689** | **0** | **2** | **3** |

All **22 Communicator cases**, including parameterized cases, passed with no skips. Compared with the previous baseline (2,688 passed, 1 failed, 2 skipped, 3 disabled), the single failure has become a pass and skip/disable counts are unchanged. Disabled and skipped cases are not counted as passed.

clang-format **19.1.5** dry-run checks passed for both the original test file and the final file. The final diff contains only the intended two replacements and the new report; whitespace checks passed.

## GUI and CLI observations

Hardware/toolchain remain NVIDIA GeForce RTX 5080, driver 617.42, CUDA 13.2, and the existing Windows Release build. GUI interaction used the computer-use skill; there were no changes to rendering configuration or application code.

The first helper launch did not expose a targetable window. An explicit launch of the built executable with `build-agent/Release` as its working directory exposed a responsive English ALIEN window, titled `Alien - Simulation: ALIEN - Galactic Collision`. Its journal confirmed CUDA initialization, loading of the existing startup/autosave scene, and normal UI operation. No deliberate heavy simulation run was started.

Screen capture failed with `FrameArrived timed out: timed out waiting on channel`; after refreshing the window binding, the single recovery capture failed with `window capture timed out: timed out waiting on channel`. Accessibility exposed only the root window/title, with no ImGui controls. These are visual-automation limitations; no application crash or hang was observed in the controlled launch.

The normal window-close action (`Alt+F4`) was delivered to the identified ALIEN window. The process exited and the journal recorded autosave, `simulation closed`, `network: logout`, and `user interface shut down`. No forced termination or reboot was used.

The user subsequently requested to perform visual checks personally. No further capture or interactive workflow attempts were made.

| Check | Observed result |
| --- | --- |
| English GUI launch on RTX 5080 | Responsive titled window; startup and simulation-loading journal entries confirmed |
| Normal GUI close | Passed, with normal shutdown journal and process exit |
| Visual rendering quality | Not verified; delegated to the user at their request |
| Create a small simulation in GUI | Not verified; manual check pending |
| Save and reopen that small simulation | Not verified; manual check pending |
| `alien-cli.exe --help` | Exit 0; English help and options printed |
| `alien-cli.exe --plain -t 1` without input | Expected exit 1 and clear `No input file given.` diagnostic |
| CLI simulation calculation from a small saved file | Not tested; no small GUI-created input was produced |

Startup/loading and orderly exit do not prove that the small-simulation editing or save/open workflow works. GPU engine correctness evidence comes from the automatic suites; visual correctness still requires the user's checks.

## User-profile protection

Before executable checks, the existing `HKCU\SOFTWARE\alien` key was exported outside the repository. Before GUI work, a typed registry snapshot and backups of both `resources/autosave` and `build-agent/Release/resources/autosave` were also created. No unrelated ALIEN instance was running.

The GUI wrote normal settings and autosave data during use and shutdown. After confirming all ALIEN processes had exited, the original settings were restored from the initial registry export, and original autosave files were restored with SHA-256 verification. Final registry verification compared every exported key/value record against the initial export; all names, types and values match. Export byte ordering is not used as a substitute for this semantic comparison. No credentials or profile contents are committed.

## Evidence, limitations and next decision

Temporary local evidence is under:

```text
C:\Users\infarh\AppData\Local\Temp\alien-communicator-20261021-090654
```

It includes before/after repeat logs, temporary diagnostic build/state logs, final build log, all suite logs/XML, CLI output, GUI journal, and private profile backups/restore verification. These files are not committed and may be cleared by normal temporary-directory cleanup. The report preserves essential results.

The communicator baseline blocker is resolved by a test-fixture correction. Automatic regression is green for all active tests; the existing skips/disabled cases remain limitations. No new engine failure or application hang was observed. Broader hardware/compiler portability and repeated scheduling behavior outside the bounded sample have not been established.

The remaining baseline task is the user's visual and small-simulation GUI validation, with an optional bounded CLI calculation using the resulting saved file. The repository remains on the isolated fix branch, with no main-repository remote and no tracked submodule changes. Localization L1 has not started and is not started automatically by this validation result.
