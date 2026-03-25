# Job Scheduler

[![latest packaged version(s)](https://repology.org/badge/latest-versions/job-scheduler.svg)](https://repology.org/project/job-scheduler/versions)
[![build result](https://build.opensuse.org/projects/home:mx-packaging/packages/job-scheduler/badge.svg?type=default)](https://software.opensuse.org//download.html?project=home%3Amx-packaging&package=job-scheduler)

## Description

Job Scheduler is a graphical job scheduling utility for Linux that provides an intuitive interface to create, edit, and remove cron jobs. It uses crontab as the backend and is suitable for both novice and advanced users to simplify task automation and scheduling.

Based upon qroneko 0.5.4, released in 2005 by korewaisai (korewaisai@yahoo.co.jp)
More details: http://qroneko.sourceforge.net/

## Dependencies

- Qt6 (Core, Gui, Widgets, LinguistTools)
- CMake >= 3.16
- Ninja build system
- cron or cronie

## Building

### Prerequisites

Set up Qt6 development environment and install build dependencies:

```bash
# Debian/Ubuntu
sudo apt install cmake ninja-build qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools

# Fedora
sudo dnf install cmake ninja-build qt6-qtbase-devel qt6-qttools-devel

# Arch/Manjaro
sudo pacman -S --needed base-devel cmake ninja qt6-base qt6-tools cronie
makepkg -si  # builds with the provided PKGBUILD
```

### Build Commands

#### Quick Build
```bash
./build.sh
```

#### Build Options
```bash
./build.sh --help                # Show all options
./build.sh --clean              # Clean build
./build.sh --debug              # Debug build
./build.sh --clang              # Use clang compiler
./build.sh --debian             # Build Debian package
./build.sh --arch               # Build Arch Linux package
```

#### Manual Build
```bash
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

### Debian Package

Build and install the Debian package:

```bash
./build.sh --debian
sudo dpkg -i debs/job-scheduler_*.deb

### Arch Package

Build and install the Arch package:

```bash
./build.sh --arch
sudo pacman -U job-scheduler-*.pkg.tar.zst
```
```

## Project Structure

```
job-scheduler/
├── src/                    # Source files (.cpp, .h)
├── translations/          # Translation files (.ts, .qm)
├── images/               # Application icons
├── help/                 # Documentation files
├── scripts/              # Launcher scripts and policies
├── debian/               # Debian packaging files
├── CMakeLists.txt        # CMake build configuration
├── build.sh             # Build script
└── application.qrc       # Qt resource file
```
