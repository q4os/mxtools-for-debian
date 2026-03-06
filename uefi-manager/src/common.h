/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2024-2025 MX Authors
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

#include <QLatin1StringView>
#include <QString>

inline const QString STARTING_HOME = qEnvironmentVariable("HOME");

// EFI System Partition GUID (GPT) and MBR type
inline constexpr QLatin1StringView ESP_GUID_GPT("c12a7328-f81f-11d2-ba4b-00a0c93ec93b");
inline constexpr QLatin1StringView ESP_TYPE_MBR("0xef");

// Base directory for temporary mounts
inline constexpr QLatin1StringView MOUNT_BASE("/mnt/uefi-manager");

// Log file path
inline constexpr QLatin1StringView LOG_FILE_PATH("/tmp/uefi-manager.log");

// Byte pattern used to scrub sensitive data from memory
inline constexpr char SCRUB_BYTE = static_cast<char>(0xA5);
