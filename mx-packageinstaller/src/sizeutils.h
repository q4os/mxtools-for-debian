/**********************************************************************
 *  sizeutils.h
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

#include <QRegularExpression>
#include <QString>
#include <QtGlobal>

#include <cmath>
#include <limits>

namespace SizeUtils
{

namespace detail
{
inline QString normalizeNumber(QString number)
{
    const qsizetype lastComma = number.lastIndexOf(',');
    const qsizetype lastDot = number.lastIndexOf('.');
    const QChar decimalSeparator = lastComma > lastDot ? QChar(',') : QChar('.');

    if (lastComma >= 0 && lastDot >= 0) {
        const QChar thousandsSeparator = decimalSeparator == QChar(',') ? QChar('.') : QChar(',');
        number.remove(thousandsSeparator);
    }

    if (decimalSeparator == QChar(',')) {
        number.replace(',', '.');
    }

    return number;
}
} // namespace detail

// Parses a human-readable size string (as emitted by flatpak/pacman/apt output)
// into a byte count. Returns 0 and sets *ok to false (when ok is non-null) if
// the string cannot be parsed, uses a unit this function doesn't recognize, or
// the computed byte count doesn't fit in a quint64. On success *ok is set to
// true. Callers that don't care about distinguishing failure from a genuine
// zero-byte size may pass ok == nullptr (the default).
inline quint64 sizeStringToBytes(const QString &size, bool *ok = nullptr)
{
    if (ok) {
        *ok = false;
    }

    QString normalized = size.trimmed();
    normalized.replace(QChar(0x00a0), QLatin1Char(' '));
    normalized.replace(QChar(0x202f), QLatin1Char(' '));

    static const QRegularExpression sizeRegex(
        QStringLiteral(R"(^\s*([0-9][0-9.,]*)\s*([A-Za-z]+)?\s*$)"));
    const QRegularExpressionMatch match = sizeRegex.match(normalized);
    if (!match.hasMatch()) {
        return 0;
    }

    bool numberOk = false;
    const double value = detail::normalizeNumber(match.captured(1)).toDouble(&numberOk);
    if (!numberOk || !std::isfinite(value) || value < 0.0) {
        return 0;
    }

    // Flatpak formats sizes with SI decimal units (1 kB = 1000 bytes)
    const QString unit = match.captured(2).toUpper();
    quint64 multiplier = 1;
    if (unit.isEmpty() || unit == QLatin1String("B") || unit == QLatin1String("BYTE")
        || unit == QLatin1String("BYTES")) {
        multiplier = 1ULL;
    } else if (unit == QLatin1String("KB")) {
        multiplier = 1000ULL;
    } else if (unit == QLatin1String("KIB")) {
        multiplier = 1024ULL;
    } else if (unit == QLatin1String("MB")) {
        multiplier = 1000ULL * 1000ULL;
    } else if (unit == QLatin1String("MIB")) {
        multiplier = 1024ULL * 1024ULL;
    } else if (unit == QLatin1String("GB")) {
        multiplier = 1000ULL * 1000ULL * 1000ULL;
    } else if (unit == QLatin1String("GIB")) {
        multiplier = 1024ULL * 1024ULL * 1024ULL;
    } else if (unit == QLatin1String("TB")) {
        multiplier = 1000ULL * 1000ULL * 1000ULL * 1000ULL;
    } else if (unit == QLatin1String("TIB")) {
        multiplier = 1024ULL * 1024ULL * 1024ULL * 1024ULL;
    } else {
        // Unrecognized unit -- don't silently pretend it's a byte count.
        return 0;
    }

    const double bytes = value * static_cast<double>(multiplier);
    // numeric_limits<quint64>::max() (2^64 - 1) isn't exactly representable as a
    // double -- it rounds up to 2^64 -- so compare with >= rather than > to make
    // sure a value that rounds to exactly 2^64 (which is not a valid quint64) is
    // still rejected instead of hitting UB in the cast below.
    constexpr double maxBytes = static_cast<double>(std::numeric_limits<quint64>::max());
    if (!std::isfinite(bytes) || bytes < 0.0 || bytes >= maxBytes) {
        return 0;
    }

    if (ok) {
        *ok = true;
    }
    return static_cast<quint64>(bytes);
}

} // namespace SizeUtils
