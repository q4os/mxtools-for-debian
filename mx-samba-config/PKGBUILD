# Maintainer: Adrian <adrian@mxlinux.org>
pkgname=mx-samba-config
pkgver=${PKGVER:-25.12.3}
pkgrel=1
pkgdesc="Samba configuration tool for MX Linux"
arch=('x86_64' 'i686')
url="https://mxlinux.org"
license=('GPL3')
depends=('samba' 'qt6-base' 'polkit' 'xdg-utils')
makedepends=('cmake' 'ninja' 'qt6-tools')
install=mx-samba-config.install
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
    install -Dm755 build/mx-samba-config "${pkgdir}/usr/bin/mx-samba-config"

    # Install translations
    install -dm755 "${pkgdir}/usr/share/mx-samba-config/locale"
    install -Dm644 build/*.qm "${pkgdir}/usr/share/mx-samba-config/locale/" 2>/dev/null || true

    # Install desktop file
    install -Dm644 mx-samba-config.desktop "${pkgdir}/usr/share/applications/mx-samba-config.desktop"

    # Install icons
    install -Dm644 images/mx-samba-config.svg "${pkgdir}/usr/share/icons/hicolor/scalable/apps/mx-samba-config.svg"

    # Install helper scripts
    install -dm755 "${pkgdir}/usr/lib/mx-samba-config"
    install -Dm755 scripts/mx-samba-config-lib "${pkgdir}/usr/lib/mx-samba-config/mx-samba-config-lib"
    install -Dm755 scripts/mx-samba-config-list-users "${pkgdir}/usr/lib/mx-samba-config/mx-samba-config-list-users"

    # Install PolicyKit policies
    install -dm755 "${pkgdir}/usr/share/polkit-1/actions"
    install -Dm644 actions/org.mxlinux.mx-samba-config-lib.policy \
        "${pkgdir}/usr/share/polkit-1/actions/org.mxlinux.mx-samba-config-lib.policy"
    install -Dm644 actions/org.mxlinux.mx-samba-config-list-users.policy \
        "${pkgdir}/usr/share/polkit-1/actions/org.mxlinux.mx-samba-config-list-users.policy"

    # Install documentation
    install -dm755 "${pkgdir}/usr/share/doc/mx-samba-config"
    if [ -d docs ]; then
        cp -r docs/* "${pkgdir}/usr/share/doc/mx-samba-config/" 2>/dev/null || true
    fi

    # Install changelog
    gzip -c debian/changelog > "${pkgdir}/usr/share/doc/mx-samba-config/changelog.gz"

    # Install license
    install -Dm644 LICENSE "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
}
