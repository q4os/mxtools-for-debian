# MX Conky

[![latest packaged version(s)](https://repology.org/badge/latest-versions/mx-conky.svg)](https://repology.org/project/mx-conky/versions)
[![build result](https://build.opensuse.org/projects/home:mx-packaging/packages/mx-conky/badge.svg?type=default)](https://software.opensuse.org//download.html?project=home%3Amx-packaging&package=mx-conky)

<img width="1080" height="728" alt="image" src="https://github.com/user-attachments/assets/1f6cb35b-2aab-460d-b55a-aa88513df492" />


## Building with CMake

### Prerequisites
- Qt6 development libraries (qt6-base-dev, qt6-tools-dev)
- CMake 3.16 or later
- C++20 compatible compiler (GCC or Clang)

### Build Instructions

#### Standard build:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

#### Build with Clang:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DUSE_CLANG=ON
cmake --build build
```

#### Clean build directory:
```bash
rm -rf build
```

### Debian Package Building

#### Prerequisites for Debian packaging:
```bash
sudo apt install build-essential debhelper-compat cmake qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools
```

#### Build debian package:
```bash
# Build unsigned package for testing
dpkg-buildpackage -us -uc

# Build signed package for distribution
dpkg-buildpackage

# Build source package only
dpkg-buildpackage -S

# Clean build artifacts
fakeroot debian/rules clean
```

#### Install built package:
```bash
# Install the generated .deb file
sudo dpkg -i ../mx-conky_*.deb

# Install dependencies if needed
sudo apt install -f
```

