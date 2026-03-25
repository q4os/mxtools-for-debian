# Maintainer: Adrian <adrian@mxlinux.org>
pkgname=mx-service-manager
pkgver=${PKGVER:-25.10}
pkgrel=1
pkgdesc="Service Manager - MX Linux service management tool"
arch=('x86_64' 'i686')
url="https://mxlinux.org"
license=('GPL3')
depends=('qt6-base' 'polkit')
makedepends=('cmake' 'ninja' 'qt6-tools')
source=()
sha256sums=()

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
    install -Dm755 build/mx-service-manager "${pkgdir}/usr/bin/mx-service-manager"

    # Install translations
    install -dm755 "${pkgdir}/usr/share/mx-service-manager/locale"
    install -Dm644 build/*.qm "${pkgdir}/usr/share/mx-service-manager/locale/" 2>/dev/null || true

    # Install helper and polkit policy
    install -dm755 "${pkgdir}/usr/lib/mx-service-manager"
    install -Dm755 build/helper "${pkgdir}/usr/lib/mx-service-manager/helper"
    install -Dm644 scripts/org.mxlinux.pkexec.mxsm-helper.policy \
        "${pkgdir}/usr/share/polkit-1/actions/org.mxlinux.pkexec.mxsm-helper.policy"

    # Install desktop file
    install -Dm644 mx-service-manager.desktop "${pkgdir}/usr/share/applications/mx-service-manager.desktop"

    # Install icons
    install -Dm644 mx-service-manager.png "${pkgdir}/usr/share/icons/hicolor/48x48/apps/mx-service-manager.png"
    install -Dm644 mx-service-manager.png "${pkgdir}/usr/share/pixmaps/mx-service-manager.png"

    # Install documentation
    install -dm755 "${pkgdir}/usr/share/doc/mx-service-manager"
    if [ -d docs ]; then
        cp -r docs/* "${pkgdir}/usr/share/doc/mx-service-manager/" 2>/dev/null || true
    fi

    # Install license
    install -Dm644 LICENSE "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
}
