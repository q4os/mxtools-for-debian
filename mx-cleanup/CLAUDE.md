# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## About mx-cleanup

mx-cleanup is a Qt-based GUI application for MX Linux that provides disk cleanup and system maintenance functionality. It allows users to free space and clear logs through a user-friendly interface.

## Build System

This project uses qmake (Qt's build system):

- **Build command**: `qmake6 && make`
- **Clean build**: `make clean && qmake6 && make`
- **Translation compilation**: `/usr/lib/qt6/bin/lrelease translations/*.ts`
- **Debug build**: Build creates both debug and release variants by default
- **Desktop file translations**: Use `make` in `translations-desktop-file/` directory

The project is configured for Qt6 and requires C++20 (`c++20` config). Release builds use LTO optimization.

## Architecture Overview

- **Main Application**: Single-window Qt application (`MainWindow` inherits from `QDialog`)
- **Entry Point**: `main.cpp` handles initialization, translation loading, and root privilege checks
- **Core Logic**: All functionality implemented in `MainWindow` class with UI defined in `mainwindow.ui`
- **Helper Scripts**: Privileged operations executed via `scripts/helper` with pkexec elevation
- **Translations**: Multi-language support with `.ts` files in `translations/` directory

## Key Components

- **MainWindow** (`mainwindow.h/cpp`): Main application logic, handles all cleanup operations
- **Helper Scripts**: Located in `scripts/` - used for operations requiring root privileges
- **Privilege Escalation**: Uses pkexec for root operations, with policy file in `scripts/org.mxlinux.pkexec.mx-cleanup-helper.policy`
- **Settings**: Uses QSettings for persistent configuration
- **Version Management**: Version defined in `version.h` (auto-generated during build)

## Development Notes

- Application performs root privilege checks on startup but runs as normal user
- Privileged operations are delegated to helper scripts via pkexec
- UI layout and components defined in Qt Designer `.ui` file
- Project uses Qt's resource system for images (`images.qrc`)
- Translations managed through Qt's `.ts` system with extensive language support (40+ languages)
- Helper script (`scripts/helper`) is a simple bash wrapper that evaluates passed commands with root privileges
- Desktop file has separate translation system in `translations-desktop-file/` directory