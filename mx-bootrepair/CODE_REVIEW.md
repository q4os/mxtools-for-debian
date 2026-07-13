# Code Review: mx-bootrepair

**Branch:** master (`385a632`) — "Harden session log handling"
**Repo:** `/home/adrian/Sync/programs/qt/mx-bootrepair`
**Build:** CMake 3.16+, Qt6, C++20, static library + GUI + CLI + helper targets
**Structure:** `src/core/` (shared lib), `src/gui/` (Qt Widgets), `src/cli/` (standalone + embedded), `src/helper.cpp` (pkexec helper)
**Last commit:** 2026-05-30 (clean tree, no untracked changes to tracked files)

---

## Security

- [ ] **1. Hardcoded elevation paths in Cmd constructor** — `src/core/cmd.cpp:48-50` checks `/usr/bin/pkexec`, `/usr/bin/sudo`, `/usr/bin/gksu` via `QFile::exists`. If installed at a non-standard path (e.g. `/usr/local/bin/pkexec`), elevation silently breaks. Should use `QStandardPaths::findExecutable()`.

- [ ] **2. Accumulating signal-slot connections in Cmd::proc()** — `src/core/cmd.cpp:224` calls `connect(this, &QProcess::finished, this, &Cmd::done)` without disconnecting first. Each call adds a duplicate connection. Qt allows duplicates by default. The `this→this` sender/receiver pair is persistent, so connections accumulate across calls. (The `this→loop` connection at line 235 is fine — `loop` is a local, auto-cleaned on scope exit.)

- [ ] **3. LUKS password passed as const with unnecessary `const_cast`, never zeroed** — `src/core/bootrepair_engine.cpp:449` takes `const QByteArray &pass` then does `const_cast<QByteArray&>(pass)` — the cast is unnecessary since `execProcAsRoot`'s input parameter is already `const QByteArray*`. Additionally, the GUI entries in `mainwindow.cpp` (lines 175-194, 237-256, 289-308) store the password in a local `QByteArray` and never scrub it before scope exit.

## Code Quality

- [ ] **4. Duplicate compiler flag: `-Wpedantic` and `-pedantic`** — `CMakeLists.txt:156-157`, `201-202`, `211-212` — three targets (bootrepair-core, mx-boot-repair, helper) each set both flags. They're aliases on both GCC and Clang; one per target is redundant.

- [ ] **5. LTO flags are unconditional** — `CMakeLists.txt:247-253` sets `-flto=thin` for Clang and `-flto=auto` for GCC without `check_cxx_compiler_flag` guards. Works in practice but a guard (as in deb-installer) would catch toolchain mismatches gracefully.

- [ ] **6. Per-property lsblk calls scatter disk queries** — `bootrepair_engine.cpp` has five functions (`isEspPartition`, `isLinuxPartitionType`, `labelContains`, `filesystemType`, `partitionLabel`) that each make a separate process call for a single lsblk column. Callers like `isPreferredRootCandidate()` in `mainwindow.cpp` then call two of these, spawning 2+ processes for what one could deliver. The `listDisks()` and `listPartitions()` bulk calls already exist — augmenting them with the needed columns would avoid the scatter.

- [ ] **7. `detectArch()` uses shell-based `getCmdOut("uname -m")`** — `src/core/bootrepair_engine.cpp:69-70` calls `shell->getCmdOut("uname -m")` which goes through `/bin/bash -c`. Should use `proc("uname", {"-m"})` (no shell) instead. Note: `QSysInfo::currentCpuArchitecture()` would be incorrect here — it reports compile-time arch, not the running kernel's machine type.

## Robustness

- [ ] **8. No unit tests** — `debian/rules:18-19` explicitly skips tests. The Cmd class, BootRepairEngine, path validation in helper.cpp, and the CLI argument parsing have no automated coverage.

- [ ] **9. `isMountedTo()` double probe with no fallback logging** — `src/core/bootrepair_engine.cpp:339-341` tries `MOUNTPOINTS`, then `MOUNTPOINT`, but if both fail returns an empty string with no warning or log entry. Callers assume the result is valid.

- [ ] **10. Password zeroing gap across GUI entry points** — `mainwindow.cpp` `installGRUB()` (line 194), `repairGRUB()` (line 256), and `regenerateInitramfs()` (line 308) all store the LUKS password in a local `QByteArray` and pass it to the engine, but the buffer is never scrubbed before going out of scope. The engine's `openLuks()` also doesn't scrub (covered by finding 3).

## Build System

- [ ] **11. Tests explicitly skipped** — `debian/rules:18-19`: `override_dh_auto_test:` → `# Skip tests as no test targets are defined`.

- [ ] **12. LTO on helper target adds nothing** — The `helper` binary is short-lived and unlikely to benefit from LTO. Minor.

## Minor

- [ ] **13. Comment typo** — `mainwindow.cpp:377`: "by default the same root and /boot partion" → "partition".

---

## Groups (fix together)

### ✅ Group A — Build & packaging infra (DONE)
| # | Item | What changed |
|---|------|-------------|
| 4 | Duplicate `-Wpedantic` / `-pedantic` | Removed the redundant `-pedantic` from all 3 targets (`bootrepair-core`, `mx-boot-repair`, `helper`) in `CMakeLists.txt`. |
| 5 | LTO unconditional | Added `include(CheckCXXCompilerFlag)`. Wrapped Clang's `-flto=thin` and GCC's `-flto=auto`/`-flto` behind `check_cxx_compiler_flag()` checks with fallback. |
| 11 | Tests skipped | Replaced `# Skip tests` with `dh_auto_test` in `debian/rules` and added `-DBUILD_TESTS=ON` to `override_dh_auto_configure`. |
| 12 | LTO on helper (minor) | Already unaffected — the LTO section only applies to `mx-boot-repair`. Verified, no change needed. |

### ✅ Group B — Cmd class hardening (DONE)
| # | Item | What changed |
|---|------|-------------|
| 1 | Hardcoded elevation paths | `src/core/cmd.cpp`: Moved `asRoot` initialization from the initializer list to the constructor body. Uses `QStandardPaths::findExecutable("sudo")` (CLI mode), then `"pkexec"`, then `"gksu"`. No more hardcoded `/usr/bin/` paths. |
| 2 | Accumulating signal connections | `src/core/cmd.cpp`: Added `disconnect(this, &QProcess::finished, this, &Cmd::done)` before the `connect()` in `proc()`. Each call starts clean — no duplicate connections accumulate. |

### ✅ Group C — Password handling (DONE)
| # | Item | What changed |
|---|------|-------------|
| 3 | Unnecessary const_cast + no zeroing in engine | `src/core/bootrepair_engine.cpp:449`: Removed the `const_cast<QByteArray&>` — `&pass` already returns `const QByteArray*`, which is what `execProcAsRoot` expects. |
| 10 | Password buffer never scrubbed in GUI | `src/gui/mainwindow.cpp`: Added RAII `SecureBuffer` struct in the anonymous namespace (zeroes on scope exit via destructor). Used in `installGRUB()`, `repairGRUB()`, and `regenerateInitramfs()` to scrub the local password buffer. Also added explicit `opt.luksPassword.fill(0); clear()` after each engine call to scrub the copy stored in the options struct. |

### ✅ Group D — Disk query consolidation (DONE)
| # | Item | What changed |
|---|------|-------------|
| 7 | `detectArch()` uses shell `getCmdOut("uname -m")` | `src/core/bootrepair_engine.cpp`: Replaced `getCmdOut("uname -m")` (shell, `/bin/bash -c`) with `proc("uname", {"-m"})` (argument array, no shell). |
| 9 | `isMountedTo()` silent failure on double probe | `src/core/bootrepair_engine.cpp`: Added `qWarning()` when both `MOUNTPOINTS` and `MOUNTPOINT` probes return empty. |

Item 6 (per-property lsblk scatter) is a larger refactor — the five functions are called from different code paths across the engine, GUI, and CLI, and consolidating them requires either mutable caching or a new batch API. Marked as future work rather than a quick fix.

### ✅ Group E — Trivial one-liners (DONE)
| # | Item | What changed |
|---|------|-------------|
| 13 | Comment typo "partion" | `src/gui/mainwindow.cpp`: Fixed "partion" → "partition" in `guessPartition()` comment. |

---

**13 items across 5 groups. Removed:**
- Item 4 (old): `displayOutput()`/`disableOutput()` are properly paired — connections do not accumulate.
- Item 17 (old): Not an issue, just a note.
- Item 9 (old): `canUnlockLuks()` close validation — acceptable risk for a temporary probe mapper.
- Item 10 (old): `loadChangelogText()` zcat — pragmatic choice; `qUncompress` doesn't handle gzip.
- Item 14 (old): `handleElevationFailure()` pattern — interleaved checks are intentional early-abort guards.
- Item 14 (old numbering, now 15): Moot — covered by items 4/5.
