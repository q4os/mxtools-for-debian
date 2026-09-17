# Code Review

**Scope:** entire codebase (Qt GUI sources, privileged helper scripts in `lib/`, packaging, `release.sh`)
**Date:** 2026-07-28
**Verdict:** Core GUI logic is solid, but the root-privileged journald helper has an argument-injection bug via unquoted expansion, and `release.sh`'s flag-compatibility shim silently changes tag-creation behavior.

## Action items

1. - [x] **Unquoted `$ACTION` re-splits/re-globs the privileged journalctl invocation** — `lib/qsig-lib:23,29`
   `journalctl_command` sets `ACTION="$@"` (joining all args into one string) and then runs it unquoted as `$ACTION` (line 29). Bash re-applies word-splitting and pathname expansion to that string. The `--unit=<text>` argument built from the free-text search box (`mainwindow.cpp` `run_journalctl_report()`, passed through `pkexec .../qsig-lib journalctl_command journalctl ...`) is user-controlled: typing e.g. `ssh error` in the journald search field produces `--unit=ssh error`, which after re-splitting becomes two separate argv words (`--unit=ssh` and `error`) passed to the root-run `journalctl`, injecting an extra positional argument into a privileged command. Fix by keeping the args in an array (`ACTION=("$@")`) and executing `"${ACTION[@]}"`, or simply calling `journalctl_command "$@"` from `main()` instead of building a string.

2. - [x] **`--update`/`--force` no longer skip tag creation, so they can trigger an unwanted tag + push** — `release.sh:295-297,343-364,367-382`
   Previously, `./release.sh <version> --update` ran in "update-only" mode that explicitly skipped tag checks/creation and only touched the AUR package. Now `--update`/`--force` are accepted but only print a deprecation warning (line 296) and fall through to the normal flow. If the given version's tag does not already exist, `tag_exists` (line 343) returns false, so the script proceeds to prompt for an annotation and then (line 367-381) creates an annotated tag and runs `git push origin "$version"` — a hard-to-reverse action the caller previously relied on `--update` to avoid. Anyone still using `--update` expecting no tag/push will get a real tag pushed to GitHub. Either restore an explicit update-only path, or make it a hard error instructing the user to omit the flag.

3. - [x] **Shadowed `dialog` makes the Find dialog always parent to `textSysInfo`** — `mainwindow.cpp:541`
   `showFindDialog()` declares `QDialog dialog(ui->textSysInfo);` and then, on the journald tab, `if (...) QDialog dialog(ui->plainTextEditJournald);` — this re-declares a new, same-named `dialog` scoped to just that one `if`-statement, which is constructed and destroyed immediately with no effect. Every line after it operates on the original outer `dialog`, so when using Find from the Journald tab, the Find dialog is always parented to `ui->textSysInfo` instead of `ui->plainTextEditJournald`. Replace with a `QWidget *parentWidget` chosen before constructing the single `dialog`, e.g. `QDialog dialog(ui->tabWidget->currentIndex() == 1 ? ui->plainTextEditJournald : ui->textSysInfo);`
