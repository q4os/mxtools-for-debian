/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023-2024 MX Authors
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

#include <QString>

inline const QString startingHome {qEnvironmentVariable("HOME")};

// System paths
namespace Paths
{
// Locale configuration files
inline const QString localeGen {"/etc/locale.gen"};
#ifdef MX_LOCALE_ARCH
inline const QString defaultLocale {"/etc/locale.conf"};
#else
inline const QString defaultLocale {"/etc/default/locale"};
#endif

// Application data directories
inline const QString mxLocaleLib {"/usr/lib/mx-locale"};
inline const QString mxLocaleDoc {"/usr/share/doc/mx-locale"};

// System locale directories
inline const QString i18nSupported {"/usr/share/i18n/SUPPORTED"};
inline const QString i18nSupportedLocal {"/usr/local/share/i18n/SUPPORTED"};
inline const QString i18nLocales {"/usr/share/i18n/locales"};
inline const QString i18nLocalesLocal {"/usr/local/share/i18n/locales"};

// Standard system directories
inline const QString usrLib {"/usr/lib"};
inline const QString usrShare {"/usr/share"};
inline const QString usrShareDoc {"/usr/share/doc"};
} // namespace Paths
