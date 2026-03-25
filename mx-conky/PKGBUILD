# Maintainer: Adrian <adrian@mxlinux.org>
pkgname=mx-conky
pkgver=${PKGVER:-25.12.3}
pkgrel=1
pkgdesc="MX Conky - Conky configuration tool for MX Linux"
arch=('x86_64' 'i686')
url="https://mxlinux.org"
license=('GPL3')
depends=('conky' 'qt6-base')
makedepends=('cmake' 'ninja' 'qt6-tools')
source=("mx-conky.install")
sha256sums=("SKIP")
install=mx-conky.install

build() {
    cd "${startdir}"

    # Clean any previous build artifacts
    rm -rf build

    # Configure with CMake
    cmake -G Ninja \
        -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DPROJECT_VERSION_OVERRIDE="${pkgver}"

    # Build
    cmake --build build --parallel
}

package() {
    cd "${startdir}"

    # Install binary
    install -Dm755 build/mx-conky "${pkgdir}/usr/bin/mx-conky"

    # Install translations
    install -dm755 "${pkgdir}/usr/share/mx-conky/locale"
    install -Dm644 build/*.qm "${pkgdir}/usr/share/mx-conky/locale/" 2>/dev/null || true

    # Install desktop file
    install -Dm644 mx-conky.desktop "${pkgdir}/usr/share/applications/mx-conky.desktop"

    # Install icons
    install -Dm644 icons/mx-conky.png "${pkgdir}/usr/share/icons/hicolor/96x96/apps/mx-conky.png" 2>/dev/null || true
    install -Dm644 icons/mx-conky.svg "${pkgdir}/usr/share/icons/hicolor/scalable/apps/mx-conky.svg"

    # Install documentation
    install -dm755 "${pkgdir}/usr/share/doc/mx-conky"
    if [ -d help ]; then
        cp -r help/* "${pkgdir}/usr/share/doc/mx-conky/" 2>/dev/null || true
    fi
    if [ -f debian/changelog ]; then
        gzip -9 -c debian/changelog > "${pkgdir}/usr/share/doc/mx-conky/changelog.gz"
    fi

    # Install license and icon credit
    install -Dm644 LICENSE "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
    if [ -f MXConky_IconCredit ]; then
        install -Dm644 MXConky_IconCredit "${pkgdir}/usr/share/doc/mx-conky/MXConky_IconCredit"
    fi
}
