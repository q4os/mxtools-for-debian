# Code Review

**Scope:** Entire codebase — `src/*.{cpp,h}`, `helper.cpp`, build/package scripts
**Date:** 2026-07-28
**Verdict:** Solid privilege-escalation and input-validation practices in `Cmd`/`helper.cpp`; two related null-pointer-dereference crashes exist in the service list's placeholder-item handling.

## Action items

1. - [x] **Null Service* dereference when selecting a placeholder list item** — `src/mainwindow.cpp:207-209`
   `onSelectionChanged` only guards against `current == nullptr`; it never checks that `current->data(Qt::UserRole).value<Service *>()` is non-null before calling `->getInfo()`, `->isRunning()`, `->isEnabled()`. The "Loading..." item (added at startup, `mainwindow.cpp:83`) and "Refreshing..." item (added in `pushRefresh_clicked`, `mainwindow.cpp:696`) are plain `QListWidgetItem`s with no `Qt::UserRole` data set. Verified with a standalone Qt test that clicking/selecting such an item fires `currentItemChanged` with a valid `QListWidgetItem*` whose `value<Service *>()` is `nullptr` — dereferencing it crashes the app. Add a null check on the `Service*` and return early (mirroring the existing guard in `fetchTooltipDescription`, `mainwindow.cpp:279-283`).

2. - [x] **Null ptrService dereference in button handlers when a placeholder item is selected** — `src/mainwindow.cpp:662-664`, `src/mainwindow.cpp:721-723`
   `pushEnableDisable_clicked` and `pushStartStop_clicked` guard `currentItem` for null but not the `Service*` obtained from `currentItem->data(Qt::UserRole)`. Since `pushEnableDisable`/`pushStartStop` are never disabled during the loading/refresh window (only `pushRefresh` disables itself), a user who selects the "Loading..."/"Refreshing..." placeholder item and then clicks Start/Stop or Enable/Disable will dereference a null `ptrService` (`ptrService->enable()`/`ptrService->start()` etc.), crashing the app. Add a null check on `ptrService` and return early, re-enabling the button first.
