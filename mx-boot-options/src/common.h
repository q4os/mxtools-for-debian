/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
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

const QString starting_home {qEnvironmentVariable("HOME")};

// Exit code the write-file helper action uses when the replacement file was already renamed into
// place but the containing directory could not be fsynced afterward, so the rename's durability
// across a crash is not guaranteed. Distinct from the generic failure code (which always means the
// original file was left untouched) so callers can report accurately instead of claiming the
// previous configuration is still in place.
constexpr int EXIT_CODE_WRITE_FILE_DURABILITY_UNCERTAIN = 2;
