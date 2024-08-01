/*
    Copyright (C) 2008  Tim Fechtner < urwald at users dot sourceforge dot net >
    Modfied by Adrian @MXLinux

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "versionnumber.h"

VersionNumber::VersionNumber(const QString &value)
{
    setStrings(value);
}

QString VersionNumber::toString() const
{
    return str;
}

void VersionNumber::setStrings(const QString &value)
{
    // Initialize epoch, upstream, and debian strings
    str = value;
    QString upstream_str;
    QString debian_str;

    // Parse epoch and upstream_version
    int colonIndex = value.indexOf(':');
    if (colonIndex != -1) {
        epoch = value.leftRef(colonIndex).toInt();
        upstream_str = value.mid(colonIndex + 1);
    } else {
        epoch = 0;
        upstream_str = value;
    }

    // Parse debian_revision
    int dashIndex = upstream_str.lastIndexOf('-');
    if (dashIndex != -1) {
        debian_str = upstream_str.mid(dashIndex + 1);
        upstream_str = upstream_str.left(dashIndex);
    }

    upstream_version = groupDigits(upstream_str);
    debian_revision = groupDigits(debian_str);
}

VersionNumber &VersionNumber::operator=(const QString &value)
{
    setStrings(value);
    return *this;
}

bool VersionNumber::operator<(const VersionNumber &value) const
{
    return compare(*this, value) == 1;
}

bool VersionNumber::operator<=(const VersionNumber &value) const
{
    return !(*this > value);
}

bool VersionNumber::operator>(const VersionNumber &value) const
{
    return compare(*this, value) == -1;
}

bool VersionNumber::operator>=(const VersionNumber &value) const
{
    return !(*this < value);
}

bool VersionNumber::operator==(const VersionNumber &value) const
{
    return str == value.str;
}

bool VersionNumber::operator!=(const VersionNumber &value) const
{
    return !(*this == value);
}

// Transform QString into QStringList with digits grouped together
QStringList VersionNumber::groupDigits(const QString &value)
{
    QStringList result;
    int length = value.length();
    result.reserve(length);
    QString cache;

    for (int i = 0; i < length; ++i) {
        QChar currentChar = value.at(i);
        if (currentChar.isDigit()) {
            cache.append(currentChar);
            if (i == length - 1) {
                result.append(cache);
            }
        } else {
            if (!cache.isEmpty()) { // Add accumulated digits
                result.append(cache);
                cache.clear();
            }
            result.append(currentChar);
        }
    }

    return result;
}

// Return 1 if second > first, -1 if second < first, 0 if equal
int VersionNumber::compare(const VersionNumber &first, const VersionNumber &second) const
{
    if (second.epoch > first.epoch) {
        return 1;
    } else if (second.epoch < first.epoch) {
        return -1;
    }
    int upstreamComparison = compare(first.upstream_version, second.upstream_version);
    if (upstreamComparison != 0) {
        return upstreamComparison;
    }
    if (!first.debian_revision.isEmpty() || !second.debian_revision.isEmpty()) {
        return compare(first.debian_revision, second.debian_revision);
    }
    return 0;
}

// Return 1 if second > first, -1 if second < first, 0 if equal
int VersionNumber::compare(const QStringList &first, const QStringList &second)
{
    // Compare QStringList versions
    int minLength = qMin(first.length(), second.length());
    for (int i = 0; i < minLength; ++i) {
        if (first.at(i) == second.at(i)) {
            continue;
        }

        if (first.at(i).startsWith('~') && !second.at(i).startsWith('~')) {
            return 1;
        } else if (second.at(i).startsWith('~') && !first.at(i).startsWith('~')) {
            return -1;
        }

        // if one char length check which one is larger
        if (first.at(i).length() == 1 && second.at(i).length() == 1) {
            int res = compare(first.at(i).at(0), second.at(i).at(0));
            if (res == 0) {
                continue;
            } else {
                return res;
            }
            // one char (not-number) vs. multiple (digits)
        } else if (first.at(i).length() > 1 && second.at(i).length() == 1 && !second.at(i).at(0).isDigit()) {
            return 1;
        } else if (first.at(i).length() == 1 && !first.at(i).at(0).isDigit() && second.at(i).length() > 1) {
            return -1;
        }

        // Compare remaining digits
        if (second.at(i).toInt() > first.at(i).toInt()) {
            return 1;
        } else {
            return -1;
        }
    }

    // If equal till the end of one of the lists, compare list size
    // if the larger list doesn't have "~" it's the bigger version
    if (second.length() > first.length()) {
        return second.at(first.length()).startsWith('~') ? -1 : 1;
    } else if (second.length() < first.length()) {
        return first.at(second.length()).startsWith('~') ? 1 : -1;
    }

    return 0;
}

// Return 1 if second > first, -1 if second < first, 0 if equal
// letters and number sort before special chars
int VersionNumber::compare(QChar first, QChar second)
{
    // Compare QChars
    if (first == second) {
        return 0;
    }

    // Sort letters and numbers before special char
    if (first.isLetterOrNumber() && !second.isLetterOrNumber()) {
        return 1;
    } else if (!first.isLetterOrNumber() && second.isLetterOrNumber()) {
        return -1;
    }

    return first < second ? 1 : -1;
}
