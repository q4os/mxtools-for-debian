# Code Review

**Scope:** Entire codebase: application sources, UI forms, helpers, build/release scripts, and packaging configuration; excluding generated files, translations, and third-party code.
**Date:** 2026-11-13
**Verdict:** All ten findings are fixed and passed Claude’s review; main-repository fixes are committed, while the reviewed AUR update remains uncommitted at the owner’s request.

## Action items

1. - [x] **[P1] Restrict cleanup to this project's artifacts** — `build.sh:149`
   Running `./build.sh --clean` deletes any sibling file whose name contains `build`, including an unrelated `../unrelated-build-notes.txt`; this was reproduced in a temporary directory. The Debian build branch also moves broadly matched sibling artifacts and repeats the deletion. Limit both collection and cleanup to explicitly identified mx-locale package artifacts and build directories.

2. - [x] **[P1] Compare canonical locale identities before disabling the current locale** — `helper.cpp:191`
   After selecting Serbian, the saved language is `sr_RS.UTF-8`, but its generation entry is `sr_RS UTF-8`; the raw comparison therefore comments out the current locale when “Disable all locales except the one in use” is clicked, and the next locale-gen removes it. The same mismatch bypasses the individual-disable guard in `mainwindow.cpp:365`. Compare identities derived from the locale name, charset, and modifier in both paths, including entries whose UTF-8 charset is implicit in the name.

3. - [x] **[P2] Prevent list rebuilding while a checkbox operation is pending** — `mainwindow.cpp:387`
   `Cmd::runAsRoot()` processes GUI events while this function retains a raw `QListWidgetItem*`, and Reset remains enabled; a nested successful reset calls `displayLocalesGen()`, which deletes that item. If the original checkbox command fails, the rollback at line 392 or 401 dereferences the freed item; this sequence reproduced a segmentation fault with an unprivileged helper stub. Disable conflicting management actions for the entire pending operation, or avoid retaining item pointers across the nested event loop and refresh from authoritative state afterward.

4. - [x] **[P2] Explicitly reset category overrides on Arch** — `helper.cpp:156`
   With `LANG=en_US.UTF-8` and `LC_TIME=de_DE.UTF-8`, clicking “Set all locale categories to current language” sends only `LANG=en_US.UTF-8` to localectl and leaves the German time override unchanged. Current systemd-localed calls `locale_context_merge()`, which preserves omitted variables, so the helper reports success without performing the requested reset. Explicitly set every locale category to the current LANG, or use a configuration replacement mechanism that clears the overrides, rather than relying on a LANG-only call to unset them; see the upstream [SetLocale implementation](https://github.com/systemd/systemd/blob/main/src/locale/localed.c) and [merge implementation](https://github.com/systemd/systemd/blob/main/src/shared/locale-setup.c).

5. - [x] **[P2] Reconcile category overrides during bulk locale removal** — `helper.cpp:203`
   With `LANG=en_US.UTF-8` and `LC_TIME=de_DE.UTF-8`, bulk disabling comments out the German generation entry but leaves `LC_TIME` unchanged. After regeneration, subsequent sessions reference a locale that no longer exists, and update-locale's validation can reject later settings changes. Preserve locales required by configured categories, or remove the corresponding overrides as part of the bulk operation and refresh the category buttons.

6. - [x] **[P2] Decode quoted locale assignments before removing overrides** — `helper.cpp:304`
   Disabling `de_DE.UTF-8 UTF-8` leaves a valid assignment such as `LC_TIME="de_DE.UTF-8"` in place because the comparison includes its quotes. Regeneration then removes the locale while the category still references it. Parse the configuration's quoted values before normalizing and comparing them, so both quoted and unquoted assignments are removed consistently.

7. - [x] **[P2] Match locale-generation entries by fields instead of whitespace** — `helper.cpp:273`
   A valid entry such as `fr_FR.UTF-8  UTF-8` is displayed as disabled because `readLocaleFile()` compares it literally against the single-space SUPPORTED entry. Checking and then unchecking it adds and removes a separate single-space entry while leaving the original enabled, so the GUI reports a successful disable that never occurred. Parse the locale and charset fields consistently in the UI and both helper toggle operations, accepting equivalent spaces and tabs and handling all matching entries.

8. - [x] **[P2] Include UTF-8 locales with modifiers in the chooser** — `choosedialog.cpp:46`
   Generating Valencian produces `ca_ES.utf8@valencia` and `ca_ES@valencia`, but neither passes the filter requiring the name to end in `.utf8` or `.UTF-8`. The locale therefore remains unavailable for language or category selection even after the user enables it as instructed. Accept an optional modifier after the encoding and preserve that modifier when returning the selection.

9. - [x] **[P2] Check the address category when deciding whether to offer reset** — `mainwindow.cpp:109`
   `setButtons()` inserts Address first and LANG last, but `anyDifferentSubvars()` skips the first button under the assumption that it is LANG. When only `LC_ADDRESS` differs, the method returns false and hides “Set all locale categories to current language,” preventing the intended reset. Exclude the LANG button by its ID or pointer rather than by list position.

10. - [x] **[P2] Create the man-page destination in both Arch package recipes** — `PKGBUILD:40`, `aur/PKGBUILD:51`
    Both the recipe used by `./build.sh --arch` and the AUR recipe published by `release.sh` pass a nonexistent directory ending in `/` to `install -D`, which fails with “Not a directory”; `2>/dev/null || true` hides the failure and ships packages without their manual page. Use `install -Dm644 -t "${pkgdir}/usr/share/man/man1/" help/*.1`, as the OBS recipe already does, and stop suppressing installation errors. The main-repository fix is committed; the reviewed AUR fix includes a source update to 26.09 (26.03 has no man page) and is intentionally uncommitted at the owner’s request.
