#!/bin/bash

# runtest.sh - Convenient script to build and run tests for mx-packageinstaller
# Usage: ./runtest.sh [options]
#   --clean     Clean build before testing
#   --watch     Run tests in watch mode (requires inotify-tools)
#   --single    Run only version number tests
#   --verbose   Verbose output
#   --help      Show this help

set -e  # Exit on any error

# Default options
CLEAN=false
WATCH=false
SINGLE=false
VERBOSE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --watch)
            WATCH=true
            shift
            ;;
        --single)
            SINGLE=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --clean     Clean build before testing"
            echo "  --watch     Run tests in watch mode (requires inotify-tools)"
            echo "  --single    Run only version number tests"
            echo "  --verbose   Verbose output"
            echo "  --help      Show this help"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo "=== MX Package Installer Test Runner ==="

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo "🧹 Cleaning build directory..."
    rm -rf build
fi

# Build with tests
echo "🔨 Building with tests enabled..."
if [ "$VERBOSE" = true ]; then
    cmake -B build -DBUILD_TESTS=ON && cmake --build build
else
    cmake -B build -DBUILD_TESTS=ON >/dev/null && cmake --build build >/dev/null
fi

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful!"

# Run tests
if [ "$WATCH" = true ]; then
    echo "👀 Starting watch mode (press Ctrl+C to stop)..."
    cd build
    make watch_tests
elif [ "$SINGLE" = true ]; then
    echo "🧪 Running VersionNumber tests only..."
    cd build/Testing
    if [ "$VERBOSE" = true ]; then
        ./test_versionnumber
    else
        ./test_versionnumber | grep -E "(PASS|FAIL|✓|❌|===)"
    fi
else
    echo "🧪 Running all tests..."
    cd build/Testing
    if [ "$VERBOSE" = true ]; then
        ctest --verbose
    else
        ctest
    fi
fi

echo ""
echo "✅ Test run complete!"