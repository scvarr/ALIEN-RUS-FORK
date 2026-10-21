# Updating ALIEN-RUS-FORK from upstream

This repository preserves the original ALIEN Git ancestry and adds Russian localization. The maintained branch is `main`; the original developer's branch is `develop` in `chrxh/alien`. An independent repository with shared ancestry can be synchronized using Git even when GitHub does not display it as a fork or offer a Sync fork button.

Use an ordinary merge on a temporary integration branch. This retains both upstream and localization commits. Published localization commits must not be rebased, reset to upstream, or overwritten by a force push. A merge preserves history, but conflicts and behavior changes still require review and testing.

These are manual PowerShell instructions. Run one section at a time and stop on a failed command, unexpected history, or conflict. This publication task does not integrate newer upstream changes automatically.

## 1. Check remotes

The original workspace is configured as follows:

```text
origin    https://github.com/scvarr/ALIEN-RUS-FORK.git  (fetch and push)
upstream  https://github.com/chrxh/alien.git           (fetch)
upstream  DISABLED                                   (push)
```

Check before updating:

```powershell
Set-Location 'E:\Projects\ALIEN'
git remote -v
git branch -vv
git status --short
```

In a new clone, `origin` is already created by Git. Add `upstream` once if it is missing:

```powershell
git remote add -t develop upstream https://github.com/chrxh/alien.git
git remote set-url --push upstream DISABLED
git config --local remote.pushDefault origin
git config --local push.default simple
```

Do not re-add an existing remote or replace a different remote without checking its purpose. `DISABLED` deliberately prevents an accidental push to the original developer; it does not affect fetching. The original workspace's previous local `push.default=nothing` publication guard is replaced with `simple`, targeting `origin` and a same-name tracking branch.

## 2. Save work and prepare a backup

Commit intentional source changes first. Ignored build outputs, settings, autosaves, and GitNexus indexes are not backups and are not included in a Git bundle. Back up user settings and autosaves separately before any real GUI validation. Do not use personal simulations as test fixtures.

Require a clean tracked/untracked working tree, then update from your own published branch without creating an implicit merge:

```powershell
if (git status --porcelain) { throw 'Commit or safely save local changes first.' }
git switch main
if ($LASTEXITCODE -ne 0) { throw 'Could not switch to main.' }
git fetch --no-recurse-submodules origin main
if ($LASTEXITCODE -ne 0) { throw 'Could not fetch origin/main.' }
git merge --ff-only origin/main
if ($LASTEXITCODE -ne 0) { throw 'main and origin/main need review; stop here.' }
```

Create a backup branch and an external bundle containing all local refs:

```powershell
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$backupBranch = "backup/before-upstream-$stamp"
$syncBranch = "sync/upstream-$stamp"
$bundlePath = "..\ALIEN-RUS-FORK-before-upstream-$stamp.bundle"
git branch $backupBranch main
if ($LASTEXITCODE -ne 0) { throw 'Could not create backup branch.' }
git bundle create $bundlePath --all
if ($LASTEXITCODE -ne 0) { throw 'Could not create history backup.' }
git bundle verify $bundlePath
if ($LASTEXITCODE -ne 0) { throw 'History backup verification failed.' }
```

Keep these PowerShell variables in the same terminal for the remaining steps. The bundle is outside the repository so it cannot be accidentally committed. It contains Git history and refs, not uncommitted/ignored files, submodule repositories, or Git LFS object payloads.

## 3. Review upstream on an integration branch

```powershell
git fetch --no-recurse-submodules upstream develop
if ($LASTEXITCODE -ne 0) { throw 'Could not fetch upstream/develop.' }
git log --oneline main..upstream/develop
git diff --stat main...upstream/develop
git switch -c $syncBranch main
if ($LASTEXITCODE -ne 0) { throw 'Could not create integration branch.' }
git merge --no-ff --no-commit upstream/develop
```

The final command intentionally leaves a successful merge uncommitted for review, or stops with conflicts. If it reports `Already up to date`, there is no upstream merge to publish: switch back to `main` and stop. If no merge base exists, check the remote and history; do not add `--allow-unrelated-histories` to bypass that discrepancy.

Review both new upstream code and the resulting staged diff. Do not run another pull, reset, or branch switch while an unresolved merge is active.

## 4. Resolve conflicts without discarding localization

```powershell
git status
git diff --name-only --diff-filter=U
```

Resolve each conflict in context, combining upstream behavior with localization. In this merge, `ours` means the fork/integration branch, and `theirs` means upstream. Avoid whole-tree `--ours`, `--theirs`, or merge strategies that silently discard one side.

Pay particular attention to:

- `source/Base/LocalizationService.*`, `DefaultLocale.h.in`, and `GlobalSettings.*`: semantic catalogs, English fallback, owning strings, and `settings.language` persistence.
- `resources/locales/en.json` and `ru.json`: retain both translations, validate UTF-8, reject duplicate keys, and keep matching key sets for the translated slice.
- `resources/fonts/` and `StyleService.cpp`: preserve font licenses, Cyrillic coverage, icon merges, and atlas construction before simulation-rendering initialization.
- `AlienGui`, `AlienWindow`, `AlienDialog`, and `ModalWindow`: preserve stable identity separately from captions and keep owner menus active while their popup hierarchy is focused.
- `MainLoopController`, `LocalizationGui`, and `DisplaySettingsDialog`: preserve deferred language application and pending UI control state.
- CMake resource deployment, the CUDA-runtime discovery fix if still needed, and localization/startup/mouse-interaction tests.
- `README.md` and `.gitignore`: retain the fork introduction, the upstream credit/license, and local-artifact exclusions while incorporating useful upstream documentation changes.

Use explicit paths when staging resolved files. Do not stage the pinned `external/vcpkg` submodule or run `git submodule update`. If an upstream update changes that pin or requires a dependency/toolchain upgrade, stop and handle that compatibility decision separately before completing this integration; do not hide the discrepancy by committing a different local submodule checkout.

To abandon an uncommitted merge and return to its starting state:

```powershell
git merge --abort
```

The backup branch and bundle remain available. Do not use `git reset --hard` as the normal conflict-recovery procedure.

## 5. Validate before committing

```powershell
git diff --cached --check
git diff --cached
.\build-windows-ninja.bat agent
```

Follow `AGENTS.md`: use only the agent build tree, never run CodeQL or update submodules, and do not cancel long builds/tests. CUDA structure/constant-memory/kernel-header changes require a fresh agent build tree. The local MSVC include-prefix issue recorded in the L1 report also requires a fresh tree to reliably validate header changes until that toolchain issue is fixed.

For a full upstream integration, run all available suites from `build-agent/Release`, stopping on a failure:

```powershell
Push-Location .\build-agent\Release
try {
    foreach ($suite in 'BaseTests', 'DataTests', 'EngineInterfaceTests', 'McpToolsTests', 'NetworkTests', 'PersisterTests', 'GuiLocalizationTests', 'EngineTests') {
        & ".\$suite.exe"
        if ($LASTEXITCODE -ne 0) { throw "$suite failed; do not publish the integration." }
    }
} finally {
    Pop-Location
}
```

The ordinary `EngineTests` suite needs the NVIDIA GPU and can take several minutes. Do not substitute a long stress run. Before publishing a GUI-affecting integration, manually check EN → RU → EN, Display settings remaining open, settings persistence after restart, font/scaling quality, and a small disposable simulation. Restore the protected user profile afterward. Passing compilation alone does not establish that the translation or GUI state still works.

When all conflicts are staged and validation passes, create the merge commit with a short English message, for example:

```powershell
git commit -m "Merge upstream develop updates"
```

If validation fails, keep working on the integration branch or abort the uncommitted merge. Do not advance `main` or push a broken integration.

## 6. Publish the validated result

```powershell
git switch main
if ($LASTEXITCODE -ne 0) { throw 'Could not switch to main.' }
git merge --ff-only $syncBranch
if ($LASTEXITCODE -ne 0) { throw 'main changed during integration; review it before continuing.' }
git push origin main
```

The fast-forward moves `main` to the already reviewed merge commit; it does not rewrite either project's ancestry. If the push is rejected because someone else updated `origin/main`, fetch and integrate their commits on a review branch as well, validate again, and retry normally. Do not force-push over the remote branch.

Keep the backup branch/bundle until the updated application is accepted. The historical localization branches remain available, but new development should branch from maintained `main`, not from an old L1 snapshot.

## GitNexus and tracked files

`.gitignore` excludes `.gitnexus*`, generated GitNexus skill folders/configuration, and local `CLAUDE.md`, as well as build trees and runtime/private state. Ignore rules do not remove already tracked content or erase history. Project instructions in `AGENTS.md` and the inherited tracked reviewer agent are intentional source files, not disposable GitNexus indexes. Do not publish generated context appended to an instruction file blindly; review its diff like any other tracked change.

Reference: GitHub's documentation for [configuring an upstream remote](https://docs.github.com/en/pull-requests/how-tos/work-with-forks/configuring-a-remote-repository-for-a-fork) and [synchronizing with upstream](https://docs.github.com/en/pull-requests/how-tos/work-with-forks/syncing-a-fork). This guide adds a review/backup/validation branch around the standard Git merge workflow.
