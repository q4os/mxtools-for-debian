/**********************************************************************
 *  packagebackend.h
 **********************************************************************
 * Copyright (C) 2026-2026 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#pragma once

#include <QHash>
#include <QString>
#include <QStringList>

#include "packageinfo.h"
#include "versionnumber.h"

class Cmd;

// The package-manager backend operations that genuinely share a signature across
// apt and pacman. Selected at compile time via PACKAGE_BACKEND -- CMakeLists.txt
// compiles in exactly one of packagebackend_apt.cpp / packagebackend_pacman.cpp --
// matching the same shared-header/two-.cpp-implementations shape already used for
// LockFile's lock-detection split. Operations that don't share a real shape (the
// apt archive-download/AptCache pipeline vs. pacman's live QtConcurrent queries;
// apt's real dependency-simulation confirmActions() vs. pacman's none; AUR search)
// are not here -- they stay private, backend-specific code in mainwindow.cpp.
namespace PackageBackend
{

// Refresh repository metadata ("apt-get update" / "pacman -Sy"), via the
// appropriate maintenance/lib helper action. UI orchestration (lock check,
// progress display, error dialog) is the caller's responsibility.
[[nodiscard]] bool refreshRepositories(Cmd &cmd);

// All installed packages, with version and description. Sets *ok to false (and
// returns an empty hash) if the underlying query failed; the caller decides how
// to surface that.
[[nodiscard]] QHash<QString, PackageInfo> listInstalled(bool *ok = nullptr);

// All installed packages, with just their version.
[[nodiscard]] QHash<QString, VersionNumber> listInstalledVersions(bool *ok = nullptr);

// Mark packages as manually installed (not autoremovable/orphaned).
[[nodiscard]] bool markManuallyInstalled(Cmd &cmd, const QStringList &names);

// Package names that are candidates for autoremoval/orphan cleanup.
[[nodiscard]] QStringList autoremovableCandidates(Cmd &cmd);

// Install/remove packages from the repo (apt's enabled/Test/Backports or pacman's
// sync DBs) -- NOT AUR, which has no privilege-elevation concept in common with this
// (paru builds unprivileged) and stays entirely in mainwindow.cpp's install() once
// the AUR tab exists. extraArgs carries apt-specific flags (recommends, -t release)
// that mainwindow.cpp computes from checkbox/tab state; pacman's implementation
// ignores them (no equivalent concept exists there today).
[[nodiscard]] bool installPackages(Cmd &cmd, const QStringList &names, const QStringList &extraArgs);
[[nodiscard]] bool removePackages(Cmd &cmd, const QStringList &names);

// Compare two version strings using the running backend's real ordering rules:
// apt delegates to the existing dpkg-style VersionNumber; pacman shells out to
// the real `vercmp` (it ships with pacman itself, so it's always available
// wherever this backend runs -- there is no libalpm-dev to link against
// directly here, and a bundled reimplementation risks subtly disagreeing with
// the real thing). The two backends' rules genuinely disagree in common cases
// -- e.g. dpkg treats "~" as always sorting before everything, while pacman's
// vercmp does not -- so PackageModel and PackageFilterProxy must go through
// this instead of constructing a VersionNumber directly whenever the version
// strings being compared could be pacman's. Returns <0 if a is older than b,
// >0 if a is newer, 0 if equal -- the same convention as the real `vercmp` CLI.
[[nodiscard]] int compareVersions(const QString &a, const QString &b);

} // namespace PackageBackend
