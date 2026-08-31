#!/bin/bash

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
            echo "Unknown option: $1"
            echo "Usage: $0 [--arch]"
            exit 1
            ;;
    esac
done

if [ "$ARCH_BUILD" = true ]; then
    echo "Building Arch Linux package..."

    # Create build directory
    mkdir -p build

    # Build the package
    makepkg -f

    # Move the built package to build directory
    mv *.pkg.tar.zst build/ 2>/dev/null || mv *.pkg.tar.xz build/ 2>/dev/null || mv *.pkg.tar.gz build/ 2>/dev/null || echo "No package file found to move"

    echo "Package built and moved to build/ directory"
else
    echo "Please use --arch flag to build Arch Linux package"
    echo "Usage: $0 --arch"
fi