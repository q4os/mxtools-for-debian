# uefi-manager
Tool for managing UEFI boot entries

[![latest packaged version(s)](https://repology.org/badge/latest-versions/uefi-manager.svg)](https://repology.org/project/uefi-manager/versions)
[![build result](https://build.opensuse.org/projects/home:mx-packaging/packages/uefi-manager/badge.svg?type=default)](https://software.opensuse.org//download.html?project=home%3Amx-packaging&package=uefi-manager)
[![Continous Integration](https://github.com/AdrianTM/uefi-manager/actions/workflows/main.yml/badge.svg)](https://github.com/AdrianTM/uefi-manager/actions/workflows/main.yml)

![image](https://github.com/user-attachments/assets/c76f920b-33e8-424e-a885-7ad26925b508)

## Building

```bash
./build.sh
```

Use `./build.sh --debug` for debug builds or `./build.sh --clang` for clang builds.

## Functionality

**Manage UEFI entries** - View, add, modify, and delete UEFI boot entries using efibootmgr

**EFI stub installer** - Copy kernel and initrd to ESP and create direct UEFI boot entry bypassing GRUB

**Frugal EFI stub installer** - Create bootable entries for MX/antiX frugal installations with direct EFI boot
