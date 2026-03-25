# mx-user

[![latest packaged version(s)](https://repology.org/badge/latest-versions/mx-user.svg)](https://repology.org/project/mx-user/versions)
[![build result](https://build.opensuse.org/projects/home:mx-packaging/packages/mx-user/badge.svg?type=default)](https://software.opensuse.org//download.html?project=home%3Amx-packaging&package=mx-user)

A graphical tool for managing user accounts on MX Linux. Features include creating, modifying, and deleting users and groups, password management, autologin configuration, and profile copying between users.

![image](https://github.com/MX-Linux/mx-user/assets/418436/496b5f90-ed63-4c85-b580-f9be07340156)

## Features

- **User Management**: Create, delete, and rename user accounts
- **Password Management**: Change passwords with strength validation using zxcvbn (falls back to a basic length check when libzxcvbn is unavailable)
- **Group Management**: Add/remove groups and manage user membership
- **Profile Copying**: Copy desktop configurations between users
- **Autologin Configuration**: Enable/disable autologin for lightdm and sddm
- **User Options**: Repair user configurations and reset Mozilla settings

## Building

Requires Qt6, ninja-build, and C++20 compiler. 

**Using build script (recommended):**
```bash
./build.sh              # Release build
./build.sh --debug      # Debug build  
./build.sh --clang      # Use clang instead of gcc
./build.sh --debian     # Build Debian package
./build.sh --arch       # Build Arch Linux package (uses makepkg/PKGBUILD)
```

**Manual build:**
```bash
mkdir build && cd build
cmake -G Ninja ..
ninja
```

For Arch packaging directly, run `makepkg` in the project root. Set `PKGVER` to override the version detected from `debian/changelog`.
On Arch, libzxcvbn is disabled in the PKGBUILD; to build with it manually, install the library and pass `-DENABLE_ZXCVBN=ON` to CMake.
