# MX Welcome

A Qt 6 Quick welcome application for MX Linux that provides quick access to useful resources, tools, and system information. Its responsive dashboard uses the desktop palette, fonts, and icon theme on both light and dark desktops.

[![latest packaged version(s)](https://repology.org/badge/latest-versions/mx-welcome.svg)](https://repology.org/project/mx-welcome/versions)
[![build result](https://build.opensuse.org/projects/home:mx-packaging/packages/mx-welcome/badge.svg?type=default)](https://software.opensuse.org//download.html?project=home%3Amx-packaging&package=mx-welcome)

<img width="771" height="774" alt="image" src="https://github.com/user-attachments/assets/ac9b8fb6-f064-43ac-9009-8a20f4f14fb7" />

## Features

- **Welcome Dashboard**: A straightforward collection of buttons for documentation, forums, videos, and system tools
- **Responsive Navigation**: Sidebar navigation on wide windows and compact controls on smaller screens
- **System Page**: System information display with hardware details and MX Linux version
- **System Reports**: Generate comprehensive system information reports
- **Auto-start Option**: Can be configured to launch automatically on login
- **Internationalization**: Multi-language support with 40+ translations
- **Resource Links**: Direct access to MX Tools, manual, forum, wiki, and videos

## Installation

### Prerequisites

- Qt6 (Core, Gui, Widgets, Qml, Quick, QuickControls2, LinguistTools)
- Qt 6 Declarative development files (`qt6-declarative-dev` on Debian/MX)
- C++20 compatible compiler (GCC 14+ or Clang)
- CMake 3.16+
- Ninja build system (recommended)

### Building from Source

#### Quick Build
```bash
./build.sh           # Release build
./build.sh --debug   # Debug build
./build.sh --clean   # Clean rebuild
./build.sh --debian  # Build Debian package
```

#### Manual CMake Build
```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
sudo cmake --install build
```

#### Manual Debian Package Build
```bash
debuild -us -uc
```

### Package Installation

On MX Linux and compatible Debian-based systems:
```bash
sudo apt update
sudo apt install mx-welcome
```

## Usage

1. Launch from the application menu or run `mx-welcome` in terminal
2. **Welcome Dashboard**: Select a button to access:
   - **MX Tools**: Launch the MX Tools collection
   - **Manual**: Open the MX Linux manual
   - **Forum**: Access the MX Linux community forum
   - **Wiki**: Browse the MX Linux wiki
   - **Videos**: View MX Linux tutorial videos
   - **Contribute**: Learn how to contribute to MX Linux
3. **About This System**: View system information including:
   - MX Linux version and build details
   - Hardware information (CPU, memory, graphics)
   - Generate detailed system reports

### Command Line Options

```bash
mx-welcome [options]

Options:
  -a, --about    Start with About tab selected
  -t, --test     Run in test mode
  -v, --version  Display version information
  -h, --help     Show help information
```

## Technical Details

- **Language**: C++20
- **Framework**: Qt 6 with a Qt Quick/QML frontend
- **Build System**: CMake with Ninja generator
- **License**: GPL v3
- **Version**: 25.04.01

### Source Layout

- `src/backend.*`: configuration, environment policy, persistence, filtering, and process launching
- `src/actionmodel.*`: model exposed to QML for dashboard actions
- `qml/Main.qml`: application window, responsive navigation, system page, and dialogs
- `qml/components/`: reusable action cards and themed controls

The application intentionally starts with `QApplication` and retains the Widgets library so MX/XFCE's Qt GTK integration can provide the correct desktop palette to QML. It does not force a Qt Quick Controls style.

## Development

### Code Style
- Modern C++20 with Qt6 framework
- Header guards using `#pragma once`
- Member variables use camelCase

### Contributing

1. Fork the repository
2. Create a feature branch
3. Follow the existing code style
4. Test your changes thoroughly
5. Submit a pull request

### Translation Contributions

- Please join Translation Forum: https://forum.mxlinux.org/viewforum.php?f=96
- Please register on Transifex: https://forum.mxlinux.org/viewtopic.php?t=38671
- Choose your language and start translating: https://app.transifex.com/anticapitalista/antix-development

## License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

See [LICENSE](LICENSE) for the full license text.

## Authors

- **Adrian** <adrian@mxlinux.org>
- **Paul David Callahan**
- **Dolphin Oracle**
- **MX Linux Team** <http://mxlinux.org>

## Links

- [MX Linux](http://mxlinux.org)
- [Source Repository](https://github.com/MX-Linux/mx-welcome)
- [Issue Tracker](https://github.com/MX-Linux/mx-welcome/issues)
- [OpenSUSE Build Service](https://build.opensuse.org/projects/home:mx-packaging/packages/mx-welcome)
