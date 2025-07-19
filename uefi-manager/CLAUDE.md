# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

Initial setup (run once):
```bash
qmake6 uefi-manager.pro
```

Build the application (defaults to release):
```bash
make
```

Build debug version:
```bash
make debug
```

Build release version:
```bash
make release
```

Clean build artifacts:
```bash
make clean
```

Complete rebuild:
```bash
make distclean
qmake6 uefi-manager.pro
make
```

Install application:
```bash
make install
```

Run application:
```bash
./uefi-manager
```

## Architecture Overview

UEFI Manager is a Qt6 application for managing UEFI boot entries on Linux systems.

### Core Components

- **MainWindow**: Primary UI controller with tabbed interface for boot entries, EFI stub installation, and frugal installations
- **Cmd**: Process execution wrapper that handles both regular and elevated (root) command execution using pkexec
- **Log**: Logging facility for application events
- **About**: Standard about dialog

### Key Architecture Patterns

- **Privileged Operations**: Uses pkexec via helper scripts in `scripts/` directory for root-level operations
- **Multi-tab UI**: Three main tabs (Entries, StubInstall, Frugal) with wizard-style navigation
- **Device Detection**: Automatically detects UEFI support, partitions, and boot configurations
- **Translation Support**: Full internationalization with Qt translation system

### File Structure

- Main application code in root directory (`.cpp`, `.h`, `.ui` files)  
- Helper scripts in `scripts/` for privileged operations
- Translations in `translations/` and `translations-desktop-file/`
- Debian packaging in `debian/`

### Dependencies

- Qt6 (Core, GUI, Widgets modules)
- C++17 standard
- System utilities: efibootmgr, grub tools
- Root privileges via pkexec for UEFI modifications

### Build Configuration

- Uses qmake6 build system with Qt6 (Core, GUI, Widgets modules)
- Supports debug/release configurations with LTO optimization in release mode
- Strict compiler warnings enabled (-Werror for critical warnings)
- C++17 standard required
- Application version defined in `version.h`

### Development Workflow

- Main application logic in `mainwindow.cpp` with UI defined in `mainwindow.ui`
- Command execution wrapper in `cmd.cpp` handles both regular and elevated operations
- Privileged operations use `scripts/helper` via pkexec for security
- UI is internationalized with `.ts` translation files in `translations/`
- Build generates separate debug and release object files in respective directories

### Security Model

- Root privileges required for UEFI modifications via efibootmgr
- Uses PolicyKit/pkexec for secure privilege escalation
- Helper script (`scripts/helper`) executes privileged commands
- Policy file defines permissions for root operations