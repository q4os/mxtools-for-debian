# **********************************************************************
# * Copyright (C) 2025 MX Authors
# *
# * Authors: Adrian <adrian@mxlinux.org>
# *          MX Linux <http://mxlinux.org>
# *
# * This file is part of mx-datetime.
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *     http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# **********************************************************************/

# Maintainer: Adrian <adrian@mxlinux.org>
pkgname=mx-datetime
pkgver=${PKGVER:-25.11mx23}
pkgrel=1
pkgdesc="Date and time configuration tool for MX Linux"
arch=('x86_64' 'i686')
url="https://github.com/MX-Linux/mx-datetime"
license=('Apache')
depends=('chrony' 'util-linux' 'qt6-base' 'polkit')
makedepends=('cmake' 'ninja' 'qt6-tools')
source=()
sha256sums=()

build() {
    cd "${startdir}"

    rm -rf build

    cmake -G Ninja \
        -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DPROJECT_VERSION_OVERRIDE="${pkgver}"

    cmake --build build --parallel
}

package() {
    cd "${startdir}"

    install -Dm755 build/mx-datetime "${pkgdir}/usr/bin/mx-datetime"

    install -dm755 "${pkgdir}/usr/share/mx-datetime/locale"
    install -Dm644 build/*.qm "${pkgdir}/usr/share/mx-datetime/locale/" 2>/dev/null || true

    install -dm755 "${pkgdir}/usr/lib/mx-datetime"
    install -Dm755 build/helper "${pkgdir}/usr/lib/mx-datetime/helper"

    install -Dm644 scripts/org.mxlinux.pkexec.mx-datetime-helper.policy \
        "${pkgdir}/usr/share/polkit-1/actions/org.mxlinux.pkexec.mx-datetime-helper.policy"

    install -Dm644 mx-datetime.desktop "${pkgdir}/usr/share/applications/mx-datetime.desktop"

    install -Dm644 images/mx-datetime.png "${pkgdir}/usr/share/icons/hicolor/48x48/apps/mx-datetime.png"
    install -Dm644 images/mx-datetime.png "${pkgdir}/usr/share/pixmaps/mx-datetime.png"
    install -Dm644 images/mx-datetime.svg "${pkgdir}/usr/share/icons/hicolor/scalable/apps/mx-datetime.svg"

    install -dm755 "${pkgdir}/usr/share/doc/mx-datetime"
    if [ -d help ]; then
        cp -r help/* "${pkgdir}/usr/share/doc/mx-datetime/" 2>/dev/null || true
    fi
}
