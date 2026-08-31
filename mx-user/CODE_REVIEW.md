# Code Review

**Scope:** changes from the four most recent commits (copyright years, en_US removal, debian/rules dwz override, release.sh refactor) against the existing codebase
**Date:** 2026-07-28
**Verdict:** No new issues introduced; both existing items from the previous review are now fixed.

## Action items

1. - [x] **Copy tab "browse..." destination bypasses the same-user protection, enabling a destructive self-sync** — `mainwindow.cpp:149-157`, `mainwindow.cpp:295-334`, `mainwindow.cpp:1178-1196`
   `refreshCopy()`/`fromUserComboBox_activated()` explicitly remove the logged-in user (`logname`) and the selected source user from `toUserComboBox`'s dropdown list so a user cannot pick their own live home as a copy destination. However, the same combo box also offers a `"browse..."` entry (`mainwindow.cpp:157`) that opens `QFileDialog::getExistingDirectory()` with no root restriction and no check against `logname`/the source user's home. If the user browses to their own home (or any subdirectory of it) and checks "Synchronize" (`syncRadioButton`), `applyDesktop()` sets `job.toDir` to that browsed path and calls `runHelper({"rsync-copy", ..., "sync"})`, which runs rsync with `--delete-after` (`helper.cpp` `opRsyncCopy`) — `isAllowedCopyDestination()` in `helper.cpp` only restricts the root (`/home`, `/media`, `/mnt`, `/run/media`), not the specific user. This silently defeats the dropdown-based protection and can delete the acting user's own files. Fix by validating the browsed directory against the excluded home(s) (or the current user's home) in `toUserComboBox_activated()` before accepting it, mirroring the exclusion already applied to the dropdown list.

2. - [x] **`release.sh` computes "latest tag" from the wrong git remote** — `release.sh:66-74` vs `release.sh:58-63` and `release.sh:380`
   `tag_exists()` checks the `mxlinux` remote (the org repo that `main()` also pushes new tags to at `release.sh:380`), but `get_latest_tag()` queries the `origin` remote instead (`git ls-remote --tags origin` at `release.sh:69`), which in this repo is a personal fork. If the fork's tags lag behind or differ from `mxlinux`'s, the version-progression check at `release.sh:356-359` (`Version $clean_version is not higher than latest tag ...`) could pass or fail incorrectly. Change the remote name in `get_latest_tag()` from `origin` to `mxlinux` to match the rest of the script.
