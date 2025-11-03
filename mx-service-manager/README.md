# MX Service Manager

A Qt6-based application for managing init services on MX Linux and Debian-based systems. Provides a user-friendly graphical interface to manually enable, disable, start, and stop system services.

[![latest packaged version(s)](https://repology.org/badge/latest-versions/mx-service-manager.svg)](https://repology.org/project/mx-service-manager/versions)
[![build result](https://build.opensuse.org/projects/home:mx-packaging/packages/mx-service-manager/badge.svg?type=default)](https://software.opensuse.org//download.html?project=home%3Amx-packaging&package=mx-service-manager)

![image](https://github.com/MX-Linux/mx-service-manager/assets/418436/9c728b15-ceb1-4869-bc24-2122cd023b0b)

## Features

- **Service Management**: Start, stop, enable, and disable system services
- **Multi-Init Support**: Works with systemd, SysV init, and other init systems
- **User-Friendly Interface**: Clean Qt6-based GUI with intuitive controls
- **Privilege Escalation**: Automatic privilege elevation for service operations
- **Internationalization**: Multi-language support with 60+ translations
- **Real-time Status**: Live display of service states and statuses

## Installation

### Prerequisites

- Qt6 (Core, Gui, Widgets, LinguistTools)
- C++20 compatible compiler (GCC 14+ or Clang)
- CMake 3.16+
- Ninja build system (recommended)

### Building from Source

#### Quick Build
```bash
./build.sh           # Release build
./build.sh --debug   # Debug build
./build.sh --clean   # Clean rebuild
```

#### Using Make Targets
```bash
make                 # Default release build
make debug           # Debug build
make install         # Install (requires sudo)
make clean           # Clean rebuild
```

#### Manual CMake Build
```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
sudo cmake --install build
```

See [BUILD.md](BUILD.md) for detailed build instructions and options.

### Package Installation

On MX Linux and compatible Debian-based systems:
```bash
sudo apt update
sudo apt install mx-service-manager
```

## Usage

1. Launch the application from the application menu or run `mx-service-manager` in terminal
2. The application will request administrative privileges for service management
3. Browse the list of available services
4. Use the action buttons to:
   - **Start**: Begin running a stopped service
   - **Stop**: Stop a running service
   - **Enable**: Configure service to start at boot
   - **Disable**: Prevent service from starting at boot

## Technical Details

- **Language**: C++20
- **Framework**: Qt6
- **Build System**: CMake with Ninja generator
- **License**: GPL v3
- **Version**: 25.08.06

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
- **MX Linux Team** <http://mxlinux.org>

## Links

- [MX Linux](http://mxlinux.org)
- [Source Repository](https://github.com/MX-Linux/mx-service-manager)
- [Issue Tracker](https://github.com/MX-Linux/mx-service-manager/issues)
- [OpenSUSE Build Service](https://build.opensuse.org/projects/home:mx-packaging/packages/mx-service-manager)

