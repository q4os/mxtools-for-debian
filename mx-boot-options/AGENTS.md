# AGENTS.md - Development Guidelines for mx-boot-options

## Build Commands
- **Build**: `qmake6 && make` (or `make` if Makefile exists)
- **Clean**: `make clean` or `rm -f *.o moc_* ui_* Makefile*`
- **Release build**: `qmake6 CONFIG+=release && make`
- **Translations**: `/usr/lib/qt6/bin/lrelease translations/*.ts`

## Code Style Guidelines
- **Language**: C++20 with Qt6 framework
- **Headers**: Use `#pragma once` instead of include guards
- **Includes**: Qt headers first, then local headers, then system headers
- **Naming**: camelCase for functions/variables, PascalCase for classes
- **Braces**: Opening brace on same line for functions/classes
- **Spacing**: 4 spaces indentation, no tabs
- **Comments**: Use `/**` for file headers with GPL license template
- **Error handling**: Use Qt's error handling patterns, check return values
- **Memory**: Use Qt's parent-child ownership, smart pointers when needed
- **Enums**: Use scoped enums (`enum struct`) with descriptive names
- **Constants**: Use `const` and `constexpr` appropriately
- **Functions**: Mark functions `[[nodiscard]]` when return value should be used
- **Qt patterns**: Use Qt's signal/slot mechanism, inherit from appropriate Qt classes

## Project Structure
- Main application files in root directory
- Translations in `translations/` directory
- UI files use Qt Designer (.ui files)
- Resources defined in `images.qrc`

## Key Features
- Checks `/proc/cmdline` for "splash" parameter to enable/disable Preview functionality
- Plymouth splash screen configuration and preview
- GRUB boot options management with privilege escalation