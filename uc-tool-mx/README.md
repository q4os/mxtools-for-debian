# uc-tool

Generate AMD/Intel CPU microcode CPIO images from system firmware and
optionally inject them into a live initrd, strip them out, or list initrd content.

Designed for use with [antiX Linux](https://antixlinux.com) and
[MX Linux](https://mxlinux.org) live toolset:
live-usb-maker, mx-snapshot/iso-snapshot, live-remaster, live-kernel-updater, and build-iso.
Works standalone on any Debian/Ubuntu system.

## What it does

| Mode | Command |
|------|---------|
| Generate microcode images + update initrd | `uc-tool -i /boot/initrd.img` |
| Generate images only, save to dir | `uc-tool -d /tmp/ucode` |
| Auto-detect initrd and dir from boot mode | `uc-tool -D` |
| Update initrd, keep the .img files | `uc-tool -i /boot/initrd.img -k` |
| Use firmware from a squashfs image | `uc-tool -F /path/to/linuxfs -i initrd.gz` |
| Strip microcode from initrd (no replacement) | `uc-tool -S -i /boot/initrd.img` |
| Check whether initrd contains microcode | `uc-tool -H -i /boot/initrd.img` |
| List ucode CPIOs (no writes) | `uc-tool -l -i /boot/initrd.img` |
| List all sections with sizes and compression | `uc-tool -l all -i /boot/initrd.img` |
| List all sections + files inside each | `uc-tool -l unpack -i /boot/initrd.img` |
| Extract ucode CPIOs only | `uc-tool -x -d /tmp/out -i /boot/initrd.img` |
| Extract all sections as raw files | `uc-tool --extract=all -d /tmp/out -i /boot/initrd.img` |
| Extract all sections + unpack each | `uc-tool --extract=unpack -d /tmp/out -i /boot/initrd.img` |
| Repack sections back into initrd | `uc-tool --repack=/tmp/out -i /boot/initrd.img` |

When updating an initrd the tool:
- strips any existing microcode CPIOs from the head of the initrd
- prepends the freshly generated ones
- refreshes the `.md5` sidecar file if present
- runs `e4defrag` if the target is on ext4
- verifies write integrity before removing the backup

## Dependencies

| Package | Purpose |
|---------|---------|
| `cpio`, `coreutils`, `findutils` | required |
| `iucode-tool` | required for Intel microcode |
| `mount` | required only when `-F` points to a squashfs file |

## Installation

**From the .deb (recommended):**
```sh
sudo dpkg -i uc-tool_*.deb
```

**Manually:**
```sh
sudo install -m 0755 uc-tool /usr/bin/uc-tool
```

## Usage

```
Usage:  uc-tool [options]

  -u  --ucode=<list>       Vendors to build: amd, intel, or amd,intel
                           [default: amd,intel]
  -d  --directory=<dir>    Directory for .img files (generate/extract output)
                           or sections to repack (repack input)
  -D  --default            Set directory and initrd from current boot mode:
                             live:      initrd = /live/boot-dev/antiX/initrd.gz
                                        dir    = ./initrd.gz.D
                             installed: initrd = /boot/initrd.img-<kver>
                                        dir    = ./initrd.img-<kver>.D
                           explicit -d or -i always overrides -D
      --amd-dir=<dir>      AMD firmware source  [default: /lib/firmware/amd-ucode]
      --intel-dir=<dir>    Intel firmware source [default: /lib/firmware/intel-ucode]
  -F  --firmware=<path>    Firmware base: directory (auto-sets amd-ucode/ and
                           intel-ucode/ sub-dirs) or squashfs image (auto-mounted)
  -i  --initrd=<file>      Update this initrd in-place:
                             strip old ucode CPIOs, prepend new ones,
                             refresh <initrd>.md5 if present,
                             run e4defrag if on ext4
  -k  --keep               Keep generated .img files when using a temp dir
                           (only meaningful when -i is given without -d)
  -S  --strip              Strip ucode CPIOs from -i <initrd> without adding new
                           ones; other preamble sections are kept intact
  -H  --has-ucode          Check if -i <initrd> contains ucode CPIOs;
                           exits 0=found, 1=none  (no writes)
  -x  --extract[=ucode|all|unpack]
                           Extract initrd sections to -d <dir> (must be empty or new):
                             ucode    ucode CPIOs only [default]
                             all      all sections as raw files
                             unpack   all sections + unpack each into <name>.D/
  -l  --list[=ucode|all|unpack]
                           List initrd sections (no files written):
                             ucode    ucode CPIOs only -- vendor, size, files [default]
                             all      all sections -- name, size, compression
                             unpack   all sections + list files inside each
      --repack=<dir>        Repack sections from <dir> back into -i <initrd>;
                             uses <name>.D/ if it exists, else the raw file
  -h  --help               Show this usage
  -n  --no-color           Suppress ANSI color codes in output
  -q  --quiet              Suppress all progress output; only fatal errors shown
  -s  --silent             Only show error messages
  -V  --verbose            Show commands as they run
  -v  --version            Show version and exit
```

## Building the .deb

**Requirements:** `devscripts`, `debhelper`

```sh
sudo apt install devscripts debhelper
```

From the repository root:

```sh
cd deb-src
make
```

This copies the current `uc-tool` script into the source tree and runs
`debuild -uc -us` to produce `uc-tool_<version>_all.deb` in `deb-src/`.

Or build manually:

```sh
cd deb-src/uc-tool-mx
cp ../../uc-tool .
debuild -uc -us
```

To bump the version before building, only the changelog needs updating —
version and date are injected into the installed script and man page automatically:
```sh
cd deb-src/uc-tool-mx
dch -v 26.06.1 "Your change summary"
```
Then run `make` from `deb-src/`.

## License

GNU General Public License v3 — see [debian/copyright](deb-src/uc-tool-mx/debian/copyright)
or <https://www.gnu.org/licenses/gpl-3.0.html>.

Copyright © 2026 fehlix (MX Linux)  
Copyright © 2026 MX Linux Development Team <https://mxlinux.org>
