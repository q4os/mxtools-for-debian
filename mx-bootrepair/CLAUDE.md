# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

MX Boot Repair is a Qt6 GUI application for re-installing and repairing GRUB bootloader in MX Linux. It provides a simple interface for common boot repair operations including GRUB installation, initramfs regeneration, and boot record backup/restore.

## Build System and Commands

This project uses Qt6 with qmake build system:

### Building
- `qmake6` - Generate Makefiles from .pro file
- `make` or `make release` - Build release version
- `make debug` - Build debug version  
- `make clean` - Clean build artifacts
- `make distclean` - Clean all generated files including Makefiles

### Translations
Translation files are managed using Qt's translation system:
- `lrelease translations/*.ts` - Compile translation files to .qm format
- Translation source files (.ts) are in `translations/` directory
- Desktop file translations are in `translations-desktop-file/po/`

### Debian Packaging
The project includes Debian packaging in `debian/` directory:
- `dpkg-buildpackage` - Build Debian package
- Version is automatically extracted from `debian/changelog` into `version.h`

## Code Architecture

### Core Components

#### Main Application (`main.cpp`)
- Handles application initialization, root privilege checking, and translations
- Sets up logging to `/tmp/mx-boot-repair.log`
- Requires root privileges or pkexec/gksu for elevated operations

#### MainWindow (`mainwindow.h/.cpp`)
- Main GUI dialog inheriting from QDialog
- Manages disk/partition selection and GRUB repair operations
- Key methods:
  - `installGRUB()` - Install GRUB bootloader
  - `repairGRUB()` - Repair existing GRUB installation
  - `regenerateInitramfs()` - Rebuild initramfs
  - `backupBR()` / `restoreBR()` - Backup/restore boot records

#### Command Execution (`cmd.h/.cpp`)
- Wraps QProcess for executing system commands
- Provides both normal and elevated (root) command execution
- Methods:
  - `run()` / `runAsRoot()` - Execute shell commands
  - `proc()` / `procAsRoot()` - Execute processes with arguments
  - `getCmdOut()` - Get command output

#### Helper Scripts
- `scripts/helper` - Simple script wrapper for elevated operations
- `scripts/mxbr-lib` - Contains root-level functions like log copying
- `scripts/org.mxlinux.pkexec.mxbr-helper.policy` - PolicyKit authorization

### Key Features

#### Disk and Partition Management
- Automatic detection of available disks and partitions
- LUKS encrypted volume support with automatic mapping
- Mount point management for chroot operations
- ESP (EFI System Partition) detection and handling

#### GRUB Operations
- Installation to various targets (disk, partition, EFI)
- Support for both BIOS and UEFI systems
- Chroot environment setup for repair operations
- Automatic partition guessing based on system configuration

#### User Interface
- Uses Qt Designer (.ui files) for GUI layout
- Progress indication during operations
- Integrated help system and about dialog
- Output display for command results

## Development Guidelines

### Qt6 Migration
- Project has been updated to Qt6 (check recent commits)
- Uses Qt6-specific APIs and build configuration
- Maintains C++17 standard compliance

### Translation Support
- All user-facing strings should be wrapped with `QObject::tr()`
- New translations are added as .ts files in `translations/`
- Desktop file translations use separate .po system

### Root Privilege Handling
- Application automatically checks for root privileges
- Uses pkexec or gksu for privilege escalation
- Critical operations are performed through helper scripts

### Error Handling and Logging
- Comprehensive logging system writes to `/tmp/mx-boot-repair.log`
- Log rotation (creates .old backup)
- Both console and file output for debugging