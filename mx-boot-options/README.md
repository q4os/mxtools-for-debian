# mx-boot-options

A GUI application for managing boot options and UEFI settings in MX Linux.

[![latest packaged version(s)](https://repology.org/badge/latest-versions/mx-boot-options.svg)](https://repology.org/project/mx-boot-options/versions)
[![build result](https://build.opensuse.org/projects/home:mx-packaging/packages/mx-boot-options/badge.svg?type=default)](https://software.opensuse.org//download.html?project=home%3Amx-packaging&package=mx-boot-options)

![image](https://github.com/MX-Linux/mx-boot-options/assets/418436/66564ee6-1f3d-44d3-b273-ec063a3874ea)

## Features

- **Boot Parameter Management**: Configure boot parameters and startup choices
- **UEFI Settings**: Adjust system boot behavior and UEFI configuration
- **Plymouth Preview**: Preview and configure Plymouth splash screens
- **GRUB Configuration**: Manage GRUB boot options with privilege escalation
- **Multi-language Support**: Available in 40+ languages

## Building

### Prerequisites

- CMake 3.16 or later
- Ninja build system
- Qt6 development packages:
  - `qt6-base-dev`
  - `qt6-base-dev-tools` 
  - `qt6-tools-dev`
  - `qt6-tools-dev-tools`

### Build Commands

```bash
# Quick build
./build.sh

# Debug build
./build.sh --debug

# Clean build
./build.sh --clean

# Using clang compiler
./build.sh --clang

# Build Debian package
./build.sh --debian
```

### Manual Build

```bash
mkdir build
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Project Structure

```
mx-boot-options/
├── src/                    # Source code
│   ├── *.cpp, *.h         # C++ source files
│   ├── *.ui               # Qt Designer files
│   └── version.h.in       # Version template
├── translations/          # Translation files
├── images/                # Application icons
├── scripts/               # Helper scripts and policies
├── help/                  # Documentation
├── debian/                # Debian packaging
├── CMakeLists.txt         # CMake configuration
└── build.sh              # Build script
```

## Installation

After building, install with:

```bash
sudo cmake --install build
```

Or build a Debian package:

```bash
./build.sh --debian
sudo dpkg -i debs/*.deb
```

## Development

The project uses:
- **Language**: C++20 with Qt6 framework
- **Build System**: CMake with Ninja generator
- **Code Style**: 4 spaces, camelCase functions, PascalCase classes
- **Error Handling**: Qt patterns with return value checking

## License

GPL-3.0 - See LICENSE file for details.
