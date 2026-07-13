#!/bin/bash
# Build script - see below for command information.
set -e

# Parse arguments
ARCH_BUILD=false
while [[ $# -gt 0 ]]; do
    case $1 in
        --arch)
            ARCH_BUILD=true
            shift
            ;;
        *)
            break
            ;;
    esac
done

if [ "$ARCH_BUILD" = true ]; then
    echo "Building Arch package..."
    mkdir -p build
    cmake --workflow --preset default

    # Get version from debian/changelog
    VERSION=$(dpkg-parsechangelog -SVersion 2>/dev/null || echo "25.09.02")

    # Create package structure
    mkdir -p pkg/usr/bin pkg/usr/share/polkit-1/actions pkg/usr/share/applications pkg/usr/lib/quick-system-info-gui pkg/usr/share/quick-system-info-gui/locale pkg/usr/share/icons/hicolor/48x48/apps pkg/usr/share/doc/quick-system-info-gui

    # Install files
    cp _build_/Release/quick-system-info-gui pkg/usr/bin/
    cp actions/* pkg/usr/share/polkit-1/actions/
    cp *.desktop pkg/usr/share/applications/
    cp lib/* pkg/usr/lib/quick-system-info-gui/
    cp _build_/*.qm pkg/usr/share/quick-system-info-gui/locale/
    cp icons/* pkg/usr/share/icons/hicolor/48x48/apps/
    cp help/* pkg/usr/share/doc/quick-system-info-gui/

    # Create .PKGINFO file
    cat > pkg/.PKGINFO << EOF
pkgname = quick-system-info-gui
pkgver = $VERSION-1
pkgdesc = A utility to show quick-system-info (inxi, etc...) in a gui
url = https://github.com/MX-Linux/quick-system-info-gui
builddate = $(date +%s)
packager = opencode
size = $(du -sb pkg/usr | cut -f1)
arch = x86_64
license = GPL3
depend = qt6-base
depend = libarchive
depend = polkit
optdepend = inxi: for system information display
optdepend = grep: for text processing
EOF

    # Skip .MTREE file

    # Create tar.zst package
    cd pkg
    tar -c .PKGINFO usr/ | zstd -T0 -19 > ../build/quick-system-info-gui-$VERSION-1-x86_64.pkg.tar.zst
    cd ..
    rm -rf pkg
    echo "Arch package created: build/quick-system-info-gui-$VERSION-1-x86_64.pkg.tar.zst"
    exit 0
fi

case "${1:-all}" in
	clean)
		echo "Performing ultimate clean..."
		rm -rf _build_
		;;

	configure)
		echo "Configuring project..."
		cmake --preset default
		;;

	make-clean)
		echo "Cleaning build artifacts..."
		cmake --build --preset default --target clean
		;;

	make)
		echo "Building project..."
		cmake --build --preset default
		;;

	all)
		echo "Configuring and building project..."
		cmake --workflow --preset default
		;;

	fresh)
		echo "Fresh build (clean first, then configure and build)..."
		cmake --workflow --preset default --fresh
		;;

	*)
		echo "Usage: $0 [options] [command]"
		echo "Options:"
		echo "  --arch       - Build tar.zst package for Arch Linux and place in build/"
		echo "Commands:"
		echo "  clean        - Ultimate clean (rm -rf build)"
		echo "  configure    - Configure only"
		echo "  make-clean   - Clean build artifacts only"
		echo "  make         - Build only"
		echo "  all          - Configure and build (default)"
		echo "  fresh        - Clean first then configure and build"
		exit 1
		;;
esac