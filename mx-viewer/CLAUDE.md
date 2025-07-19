# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

This is a Qt6 C++ application using qmake as the build system:

- **Build configuration**: `mx-viewer.pro` - Qt6 project file with WebEngine widgets
- **Build command**: `qmake6 && make`
- **Debug build**: `qmake6 CONFIG+=debug && make`
- **Release build**: `qmake6 CONFIG+=release && make` (includes LTO optimizations)
- **Clean**: `make clean`

For Debian packaging:
- **Package build**: Uses `debian/rules` with Qt6 (`QT_SELECT=6`)
- **Translation update**: `/usr/lib/qt6/bin/lrelease translations/*.ts`

## Architecture

MX Viewer is a lightweight Qt6 WebEngine-based browser for displaying URLs. Core architecture:

### Main Components

- **MainWindow** (`mainwindow.h/cpp`): Primary application window with toolbar, address bar, and tab management
- **WebView** (`webview.h/cpp`): Custom QWebEngineView with history logging and window creation handling
- **TabWidget** (`tabwidget.h/cpp`): Tab container managing multiple WebView instances
- **AddressBar** (`addressbar.h`): URL input field with focus handling
- **DownloadWidget** (`downloadwidget.h/cpp/.ui`): Download management interface

### Key Features

- Multi-tab browsing with tab management
- Bookmark and history management via QSettings
- Command-line argument parsing for URL, window title, and security options
- Privilege dropping for security (runs as 'nobody' when executed as root)
- Full-screen mode support
- Translation support (extensive i18n with 100+ language files)

### Security Features

- **Privilege dropping**: `dropElevatedPrivileges()` in main.cpp:45-78 drops root privileges to regular user or 'nobody'
- **Sandbox options**: Command-line flags to disable JavaScript (`-j`) and images (`-i`)
- **Working directory change**: Changes to `/tmp` after privilege drop

### Settings and Configuration

- Uses QSettings for persistent storage of bookmarks, history, and preferences
- Translation files in `translations/` directory (Qt .ts format)
- Desktop file translations in `translations-desktop-file/`

## Development Notes

- **C++ Standard**: C++20 with strict compiler warnings enabled
- **Qt Modules**: Core, GUI, Widgets, WebEngineWidgets
- **Resource file**: `images.qrc` for embedded icons
- **Version header**: `version.h` generated during build from debian/changelog