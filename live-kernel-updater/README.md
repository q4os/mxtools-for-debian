# live-kernel-updater
This is a command line program that updates the kernel on a antiX or MX
live-usb. It can update a running live-usb or a live-usb that is plugged
into a host system.

### Quick Start

    sudo apt-get update       # if needed
    sudo apt-get install git  # if needed
    git clone https://github.com/BitJam/live-kernel-updater
    git clone https://github.com/BitJam/cli-shell-utils
    cd live-kernel-updater
    sudo ./live-kernel-updater

## Synopsis


This program is command line only for now.  We try to avoid making your
live-usb unbootable so you need to install a new kernel and then do a
remaster before we will attempt to update the live kernel.  We do this
by always looking for new kernels inside of the linuxfs file so we
know for sure it will be available when you next boot.

It can update a running live system or a live-usb that you plug
into any system.  You will be prompted with choices if you don't
specify what you want to do with command line options.

The log file is at `/var/log/live-kernel-updater.log`.

It will automatically create a config file for itself at
`/root/.config/live-kernel-updater/live-kernel-updater.conf`.


## Usage

```
Usage: live-kernel-updater [options] [command]

Update the kernel on a running antiX/MX live-usb or on an antiX/MX live-usb
that is plugged into another system.  The new kernel must already be installed.
You will be prompted for information that is needed but was not given in the
command line arguments.

Commands:
   all         All commands below
   unpack      Unpack the old initrd
   copy        Copy kernel modules into initrd
   repack      Repack the new initrd
   install     Copy new initrd and vmlinuz to the live boot directory

Options:
  -a --auto             Non-interactive.  Always assume the safe answer
  -C --color=<xxx>      Set color scheme to off|low|low1|bw|dark|high
  -d --device=<device>  live-usb device to update the kernel on
                        (use "live" to force updating a running live system)
  -F --force=XXXX       Force the options specfied:
                             flock:  ignore missing flock program
                               usb:  Allow non-usb devices (dangerous!)
                             clear:  remove previous initrd directory
  -h --help             Show this usage
  -i --initrd           Only update the initrd using the file:
                            /usr/lib/iso-template/template-initrd.gz
                        If that file is not found, use:
                            /usr/lib/iso-template/initrd.gz
     --initrd=<file>    Only update the initrd using file <file>
                        leading / then treated as full path to alternate initrd
  -I --ignore-config    Ignore the configuration file
  -k --kernel=<kernel>  The version (uname -r) of the new kernel
  -K --keep-old         Keep the old module directory in the initrd
     --pause            Wait for user input before existing
  -m --modules=<list>   Only add listed modules to the existing initrd
  -p --pretend          Don't actually install the new kernel or initrd.gz
  -q --quiet            Print less
  -R --reset-config     Write fresh config file with default options
  -v --verbose          Print more, show commands when run
  -W --write-config     Write/update config file preserving current options

Notes:
  - short options stack. Example: -pq instead of --pretend --quiet
  - options can be intermingled with commands and parameters
  - config file: /root/.config/live-kernel-updater/live-kernel-updater.conf
  - the config file will be sourced if it exists
  - it will be created if it doesn't exist
```
