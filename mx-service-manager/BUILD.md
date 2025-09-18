# Build System

This project uses CMake with Ninja as the build system for fast, efficient builds.

## Prerequisites

- CMake 3.16+
- Ninja build system
- Qt6 (Core, Gui, Widgets, LinguistTools)
- C++20 compatible compiler (GCC 14+ or Clang)

## Quick Start

### Using the build script (recommended)
```bash
./build.sh           # Release build
./build.sh --debug   # Debug build
./build.sh --clang   # Build with clang
./build.sh --clean   # Clean rebuild
```

### Using make targets
```bash
make                 # Default release build
make debug           # Debug build
make clang           # Build with clang
make clean           # Clean rebuild
make install         # Install (requires sudo)
```

### Direct CMake/Ninja commands
```bash
# Configure and build
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Clean
rm -rf build

# Install
cmake --install build  # requires sudo
```

## Build Options

### Build Types
- **Release**: Optimized build with `-O3`, LTO (GCC only), and `NDEBUG`
- **Debug**: Debug symbols, no optimization

### Compiler Options
- **GCC** (default): Standard GNU compiler with all warnings as errors
- **Clang**: Alternative compiler via `--clang` or `-DUSE_CLANG=ON`

### Features
- **Colored Output**: Automatic colored diagnostics for Ninja builds
- **Compile Commands**: `compile_commands.json` generated for IDE support
- **Parallel Builds**: Ninja automatically uses all CPU cores
- **Qt Integration**: Automatic MOC, UIC, and RCC processing

## Output

- Executable: `build/mx-service-manager`
- Translations: Generated automatically during build
- Compile commands: `build/compile_commands.json`

## Development Tools

Generate compile commands for IDE integration:
```bash
make compile-commands
```

This creates `build/compile_commands.json` for use with clangd, Qt Creator, or other LSP-compatible editors.

## Migration from qmake

This project has been migrated from qmake to CMake. The old qmake files (*.pro) are kept for reference but are no longer used. Key improvements:

- **Faster builds**: Ninja generator with parallel compilation (development)
- **Better dependency management**: Modern CMake practices
- **IDE integration**: Native compile_commands.json support
- **Cross-platform**: Better Windows/macOS support
- **Dependency removal**: No longer requires jq/awk external dependencies
- **Debian compatibility**: Uses Make generator for official packaging