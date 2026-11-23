# MX Viewer

[![latest packaged version(s)](https://repology.org/badge/latest-versions/mx-viewer.svg)](https://repology.org/project/mx-viewer/versions)
[![build result](https://build.opensuse.org/projects/home:mx-packaging/packages/mx-viewer/badge.svg?type=default)](https://software.opensuse.org//download.html?project=home%3Amx-packaging&package=mx-viewer)

A lightweight Qt6 WebEngine-based browser designed for displaying web content in standalone windows. Built specifically for MX Linux, this tool provides a secure, minimal browser interface with multi-tab support
![image](https://github.com/MX-Linux/mx-viewer/assets/418436/86d65a22-cb58-46c5-81a5-7e9614554dd3)

## Features

- **Multi-tab browsing** with tab management
- **Security-focused design** with privilege dropping and sandbox options
- **Bookmark and history management** via persistent settings
- **Full-screen mode support**
- **Download management** with dedicated interface
- **Command-line interface** with extensive options
- **WebEngine integration** for modern web standards support

## Usage

```bash
mx-viewer [OPTIONS] URL [title]
```

### Basic Examples

```bash
# Open a website
mx-viewer https://google.com

# Open with custom window title
mx-viewer https://example.com "My Custom Title"

# Open local file
mx-viewer file:///home/user/document.html

# Open in full-screen mode
mx-viewer -f https://example.com

# Disable JavaScript for security
mx-viewer -j https://untrusted-site.com
```

### Command-Line Options

- `-f, --full-screen` - Start in full-screen mode
- `-i, --disable-images` - Disable automatic image loading
- `-j, --disable-js` - Disable JavaScript execution
- `-s, --enable-spatial-navigation` - Enable keyboard spatial navigation
- `-n, --force-nobody` - Drop privileges to 'nobody' user (root only)
- `-h, --help` - Display help information
- `-v, --version` - Show version information

## Security Features

MX Viewer implements several security measures:

- **Privilege dropping**: Automatically drops root privileges to regular user or 'nobody'
- **Sandbox options**: Disable JavaScript and images for untrusted content
- **Working directory isolation**: Changes to `/tmp` after privilege drop
- **Secure execution**: Prevents privilege escalation after rights are dropped

## Building

### Requirements

- Qt6 (Core, GUI, Widgets, WebEngineWidgets, LinguistTools)
- CMake 3.16+
- C++20 compatible compiler
- dpkg-dev (for version extraction from changelog)

### Build Commands

```bash
# Standard build
mkdir build && cd build
cmake ..
make

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# Using Clang compiler
cmake -DUSE_CLANG=ON ..
make

# Fail on compiler warnings (build.sh and CI do this)
cmake -DWARNINGS_AS_ERRORS=ON ..
make
```

### Debian Package Build

```bash
# Build Debian package
dpkg-buildpackage -b -uc -us
```

### Arch Package Build

`arch/PKGBUILD` builds from the committed `debs/mx-viewer_26.09.tar.xz` source
archive. On an Arch system, place the PKGBUILD and matching archive in one
directory, then run `makepkg -si`. The archive version must match `pkgver`.

OBS uses the existing `home:mx-packaging/mx-viewer` package. Its `_service`
must extract `arch/PKGBUILD` alongside `debs/*.dsc` and `debs/*.tar.xz`:

```xml
<param name="extract">arch/PKGBUILD</param>
```

Enable the `Arch` repository for that OBS package with
`<enable repository="Arch"/>` in its package metadata. Commit and push the
recipe before triggering the OBS source service. The release workflow keeps
`pkgver` in step with `debian/changelog` when it refreshes `debs/`.

## Architecture

- **MainWindow**: Primary application window with toolbar and tab management
- **WebView**: Custom QWebEngineView with history logging and security features
- **TabWidget**: Multi-tab container for managing multiple web views
- **AddressBar**: URL input field with focus handling
- **DownloadWidget**: Download management interface

`MainWindow` implementations are grouped by responsibility in `src/`:

| File | Responsibility |
| --- | --- |
| `mainwindow.cpp` | Window construction, actions, menus, and event handling |
| `navigation.cpp` | Tabs, address/search navigation, session restore, and find |
| `browsercontext.cpp` | Profile lifetime, download routing, preferences, page settings, and zoom |
| `bookmarks.cpp` | Bookmark menus, bar, editor, and persistence |
| `newtabpage.cpp`, `historypage.cpp`, `settingspage.cpp` | Internal page rendering and request handling |
| `readermode.cpp` | Article extraction and reader rendering |
| `dialogs.cpp` | Browsing-data, shortcuts, about, file, and print dialogs |
| `windowchrome.cpp`, `resizegrip.h`, `tabmenubar.h` | Window layout, decorations, resizing, and fullscreen |
| `iconutils.cpp`, `pageutils.cpp`, `mainwindowhelpers.h` | Shared icon, cache, bookmark-title, and form helpers |

These files implement the existing `MainWindow` interface; page translations retain
its translation context. CMake lists each source explicitly.

## Translation Contributions

- Please join Translation Forum: https://forum.mxlinux.org/viewforum.php?f=96
- Please register on Transifex: https://forum.mxlinux.org/viewtopic.php?t=38671
- Choose your language and start translating: https://app.transifex.com/anticapitalista/antix-development

## License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
