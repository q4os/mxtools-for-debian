#pragma once

#include <QString>

// Version + description of a package, as reported by whichever backend queried it
// (dpkg-query / pacman -Qi / the raw Debian archive parser). Shared across both
// backends and the models that display this data, so it doesn't live in aptcache.h,
// which is compiled only for PACKAGE_BACKEND=apt.
struct PackageInfo {
    QString version;
    QString description;
};
