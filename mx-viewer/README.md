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
```

### Debian Package Build

```bash
# Build Debian package
dpkg-buildpackage -b -uc -us
```

## Architecture

- **MainWindow**: Primary application window with toolbar and tab management
- **WebView**: Custom QWebEngineView with history logging and security features
- **TabWidget**: Multi-tab container for managing multiple web views
- **AddressBar**: URL input field with focus handling
- **DownloadWidget**: Download management interface

## Translation Contributions

- Please join Translation Forum: https://forum.mxlinux.org/viewforum.php?f=96
- Please register on Transifex: https://forum.mxlinux.org/viewtopic.php?t=38671
- Choose your language and start translating: https://app.transifex.com/anticapitalista/antix-development

## License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
