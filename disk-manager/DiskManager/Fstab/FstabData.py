# -*- coding: UTF-8 -*-
#
#  FstabData.py : Fstab data
#  Copyright (C) 2007 Mertens Florent <flomertens@gmail.com>
#  Updated 2021 for MX Linux Project by team member Nite Coder
#  Maintenance of project assumed by MX Linux with permission from original author.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
import os
import grp
from pwd import getpwnam
uid = getpwnam(os.getlogin())[2]
gid = grp.getgrnam('users')[2]

gvfs = ",x-gvfs-show"

# Default filesystem options
defaults = { "btrfs"        :   ("defaults" + gvfs, "0", "2"),
             "ext4"         :   ("defaults,noatime" + gvfs, "0", "2"),
             "ext3"         :   ("defaults,noatime" + gvfs, "0", "2"),
             "ext2"         :   ("defaults,noatime" + gvfs, "0", "2"),
             "exfat"        :   ("defaults" + ",uid=" + str(uid) + ",gid=" + str(gid) + ",dmask=0002,fmask=0113,utf8" + gvfs, "0", "2"),
             "exfat-fuse"   :   ("defaults" + ",uid=" + str(uid) + ",gid=" + str(gid) + ",dmask=0002,fmask=0113,iocharset=utf8,namecase=0,nonempty" + gvfs, "0", "2"),
             "vfat"         :   ("defaults" + ",uid=" + str(uid) + ",gid=" + str(gid) + ",dmask=0002,fmask=0113,utf8" + gvfs, "0", "2"),
             "ntfs"         :   ("defaults" + ",uid=" + str(uid) + ",gid=" + str(gid) + ",dmask=0002,fmask=0113,utf8" + gvfs, "0", "0"),
             "ntfs-3g"      :   ("defaults" + ",uid=" + str(uid) + ",gid=" + str(gid) + ",dmask=0002,fmask=0113,utf8" + gvfs, "0", "0"),
             "ntfs3"        :   ("defaults" + ",uid=" + str(uid) + ",gid=" + str(gid) + ",dmask=0002,fmask=0113,iocharset=utf8" + gvfs, "0", "0"),
             "jfs"          :   ("defaults,iocharset=utf8" + gvfs, "0", "0"),
             "__default__"  :   ("defaults" + gvfs, "0", "0")}

# Known special driver
special_driver = { "ntfs-3g"    : "Read-write driver",
                   "ntfs3"      : "Read-write kernel driver",
                   "ntfs"       : "Read-write driver",
                   "ntfs-fuse"  : "Read-write driver",
                   "__unknow__" : "Unknow driver" }

if os.path.exists('/sbin/mount.exfat-fuse'):
    special_driver["exfat-fuse"] = "exFAT-fuse driver"


# Secondary driver
secondary_driver = { "ext3"     : ("ext2"),
                     "exfat"    : ("exFAT"),
                     "vfat"     : ("msdos"),
                     "__all__"  : ("auto")}

# List type of device that have an FS_TYPE, but that we don't want to configure
ignore_fs = ("swap", "iso9660", "udf", "iso9660,udf", "udf,iso9660")

# List device that we should ignore
ignore_dev = ("/dev/fd0", "/dev/fd1", "/dev/fd2", "/dev/sr0", "/dev/sr1", "/dev/sr2")

# List of virtual device name
virtual_dev = ("proc", "devpts", "tmpfs", "sysfs", "shmfs", "usbfs")


# Common options. Keep them when we change fs
common = ("atime","noatime","diratime","nodiratime","auto","noauto","dev","nodev","exec",\
          "noexec","mand","nomand","user","nouser","users","group","_netdev","owner","suid","nosuid",\
          "ro","rw","sync","async","dirsync", "x-gvfs-show", "nofail")

# List of options that don't require a remount
dont_need_remount = ("auto", "noauto", "check=none", "nocheck", "errors=continue", "errors=remount-ro", "error=panic")


# Write entry in MntFile in this order :
path_order = ("/", "/usr", "/home")

# System partitions :
system = { "exact"  : ("/", "/home", "/tmp", "/boot", "/boot/efi", "/boot/grub"),
           "extand" : ("/usr", "/var", "/sys", "/proc")}


# MntFile header
header = "# /etc/fstab: static file system information.\n" + \
         "#\n" +\
         "# <file system> <mount point>   <type>  <options>       <dump>  <pass>\n\n"

# Categories of an entry
categorie = ("FSTAB_NAME", "FSTAB_PATH", "FSTAB_TYPE", "FSTAB_OPTION", "FSTAB_FREQ", "FSTAB_PASO")


# Divers
special_char = ('"', "'", " ", "<", ">", "&", "$", "(", ")", "`", "-", "|", ";", "~", "{", "}", "^")


