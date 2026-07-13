# Code Review — mx-viewer

**Branch:** main, **Last commit:** `0d3062b Update Debian packaging metadata`
**Generated:** 2026-06-19

| Group | Items | Status |
|-------|-------|--------|
| ✅ A — Build & packaging infra (DONE) | 1–3 | Done |
| ✅ B — Runtime robustness (DONE) | 4–6 | Done |
| ✅ C — Code duplication & assets (DONE) | 7–8 | Done |
| ✅ D — Trivial one-liners (DONE) | 9–11 | Done |
| **Total** | **11** | **0 open** |

---

### ✅ Group A — Build & packaging infra (DONE)

Items in CMakeLists.txt, debian/rules, CLAUDE.md — no C++ source changes.

1. [x] **Redundant lrelease in debian/rules** — Removed the manual `lrelease` call from `debian/rules:override_dh_auto_build`. `qt6_add_translations()` in CMake already compiles .qm files during the build.

2. [x] **CLAUDE.md documents `make` but project uses Ninja** — Rewrote build commands in CLAUDE.md from `mkdir build && cd build && cmake .. && make` to `cmake -S . -B build && cmake --build build`, matching AGENTS.md.

3. [x] **Duplicate `-Wpedantic` and `-pedantic` compiler flags** — Removed the redundant `-pedantic` line from CMakeLists.txt. `-Wpedantic` (the modern form) is sufficient — both GCC and Clang accept both as exact aliases.

### ✅ Group B — Runtime robustness (DONE)

Items affecting user experience during page loading and data integrity.

4. [x] **`loading()` steals keyboard focus to the progress bar** — Removed `progressBar->setFocus()` from `loading()`. `done()` already restores focus to the web view on completion, so focus stays where the user last placed it during load.

5. [x] **`saveMenuItems()` QBuffer `open()` return value unguarded** — Added `if (buffer.open(...))` guard around the icon save path in `saveMenuItems()`, matching the existing guarded pattern in `webview.cpp:211`.

6. [x] **`getUserIDs()` returns zeroed pair on failure without fallback validation** — Added exit-code checks after `id -u` and `id -g` calls. On failure, returns `{0, 0}` which triggers the `nobody` fallback (uid 65534) in `dropElevatedPrivileges()`.

### ✅ Group C — Code duplication & assets (DONE)

Items affecting file organization and stale references.

7. [x] **Duplicate `license.html` at root and in `help/`** — Removed the root `license.html` via `git rm`. Only `help/license.html` was ever installed by `debian/install`.

8. [x] **Dead `custom.css` reference in `license.html`** — Removed the `<link href="custom.css">` tag from `help/license.html`. No `custom.css` file exists in the repository.

### ✅ Group D — Trivial one-liners (DONE)

Safe to batch — single-line changes with no side effects.

9. [x] **Desktop file `fr_BE` Name has "MW Viewer" typo** — Fixed "MW Viewer" to "MX Viewer" in `Name[fr_BE]`.

10. [x] **Deprecated `Categories=Qt;` in desktop file** — Removed the deprecated "Qt" category from `Categories=`. Remaining `Network;Utilities;` are valid FreeDesktop main categories.

11. [x] **Stale version/date in `license.html`** — Removed the stale "Version: 0.2 / Last updated 2015-07-09" footer from `help/license.html`.

---

**Removed during validation:** (none)

**Summary:** 11 items across 4 groups. The most impactful finding is **item 4** — the progress bar stealing focus, which directly affects the user's ability to interact with the browser during page load. The build infra has two redundant operations (double lrelease, duplicate pedantic flag). The remaining items are documentation, asset cleanup, and trivial fixes.
