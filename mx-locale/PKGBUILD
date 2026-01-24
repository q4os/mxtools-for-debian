# Maintainer: Dolphin Oracle <dolphinoracle@gmail.com>
pkgname=mx-locale
pkgver=${PKGVER:-25.10.1}
pkgrel=1
pkgdesc="Locale configuration tool for MX Linux"
arch=('x86_64' 'i686')
url="https://github.com/MX-Linux/mx-locale"
license=('LGPL3')
depends=('glibc' 'xdg-utils' 'qt6-base' 'polkit')
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
        -DPROJECT_VERSION_OVERRIDE="${pkgver}" \
        -DARCH_BUILD=ON

    cmake --build build --parallel
}

package() {
    cd "${startdir}"

    install -Dm755 build/mx-locale "${pkgdir}/usr/bin/mx-locale"

    install -Dm644 mx-locale.desktop "${pkgdir}/usr/share/applications/mx-locale.desktop"

    install -dm755 "${pkgdir}/usr/share/doc/mx-locale/help"
    if [ -d help ]; then
        cp -r help/* "${pkgdir}/usr/share/doc/mx-locale/help/" 2>/dev/null || true
    fi
    install -Dm644 license.html "${pkgdir}/usr/share/doc/mx-locale/license.html"

    gzip -c debian/changelog > "${pkgdir}/usr/share/doc/mx-locale/changelog.gz"

    install -dm755 "${pkgdir}/usr/lib/mx-locale"
    install -Dm755 lib/helper "${pkgdir}/usr/lib/mx-locale/helper"
    install -Dm644 lib/locale.gen "${pkgdir}/usr/lib/mx-locale/locale.gen"
    install -Dm644 lib/locale.lib "${pkgdir}/usr/lib/mx-locale/locale.lib"

    install -Dm644 polkit-actions/org.mxlinux.pkexec.mx-locale.policy \
        "${pkgdir}/usr/share/polkit-1/actions/org.mxlinux.pkexec.mx-locale.policy"

    install -dm755 "${pkgdir}/usr/share/mx-locale/locale"
    install -Dm644 build/*.qm "${pkgdir}/usr/share/mx-locale/locale/" 2>/dev/null || true
}
