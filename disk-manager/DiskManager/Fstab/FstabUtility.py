# -*- coding: UTF-8 -*-
#
#  Utility.py : Various utility for Fstab
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
import sys
import time
import glob
from subprocess import *

from . import FstabData
from .Fstabconfig import *

def get_used_file(path) :

    cmd = "%s +c 0 %s" %(LSOF, escape_special(path))
    lines = getoutput(cmd).split("\n")
    result = []
    for line in lines :
        used = path + line.split(path)[-1].strip()
        if ( used == path or not used.find(path + "/") == -1 ) and len(line.split()) > 0 :
            result.append(line.split()[0] + " -> " + used)
    return result
  
def get_dmesg_output() :

    return "\n".join(getoutput(DMESG + " | tail").split("\n"))
    
def get_fuse_options(entry) :

    cmd = "ps -eo cmd | grep -F %s" % escape_special(entry["FSTAB_PATH"])
    lines = getoutput(cmd).split("\n")
    for line in lines :
        res = line.split()
        if os.path.exists(res[1]) and os.path.realpath(res[1]) == entry["DEVICE"] :
            entry["FSTAB_TYPE"] = entry["FS_TYPE"]
            for driver in glob.glob("/sbin/mount.*") :
                if os.path.basename(res[0]) == os.path.basename(driver) \
                        or os.path.basename(res[0]) == os.path.basename(os.path.realpath(driver)) :
                    entry["FSTAB_TYPE"] = driver.split("mount.")[-1]
            if "-o" in res :
                entry["FSTAB_OPTION"] = res[res.index("-o") + 1]

def encode(string) :
    ''' encode space to \040 si it can be use in fstab '''

    return string.replace(" ", "\\040")
    
def decode(string) :
    ''' decode \040 as space '''
    
    return string.replace("\\040", " ")
    
def escape_special(text) :
    ''' escape special charachter so it can be use in a command line '''

    for char in FstabData.special_char :
        text = text.replace(char, "\\%s" % char)
    return text

def change_tracker(x, y) :
    ''' Track changes between two version of a MntFile '''
        
    (log, to_mount, to_umount, removed) = ([], [], [], [])
    if not x == y :
        for i in range(len(y)) :
        
            iy = y[i]
            iydev = iy["DEVICE"]
            iypath = iy["FSTAB_PATH"]
            if not len(x.search(iydev, keys = ["DEVICE"])) == len(y.search(iydev, keys = ["DEVICE"])) :
                if not x.search(iypath) :
                    log.append("Adding %s on %s" % (iydev, iypath))
                    if not iy.get_is_mounted() :
                        to_mount.append(iy)
                continue
            
            if len(y.search(iydev, keys = ["DEVICE"])) > 1 :
                if x.search(iypath) :
                    ix = x[x.search(iypath)[0]]
                    ixpath = ix["FSTAB_PATH"]
                else :
                   log.append("Adding duplicate %s on %s" % (iydev, iypath))
                   if not iy.get_is_mounted() :
                        to_mount.append(iy)
                   continue
            else :
                ix = x[x.search(iydev, keys = ["DEVICE"])[0]]
                ixpath = ix["FSTAB_PATH"]
            
            if ix == iy :
                continue

            log.append("Changing %s on %s:" % (iydev, ixpath))
            if not iypath == ixpath :
                log.append("+ path %s -> %s" % (ixpath, iypath))
                if not iy.get_is_mounted() and ix.get_is_mounted() :
                    to_umount.append(ix)
                    to_mount.append(iy)

            if not iy["FSTAB_TYPE"] == ix["FSTAB_TYPE"] :
                log.append("+ type: %s -> %s" % (ix["FSTAB_TYPE"], iy["FSTAB_TYPE"]))
                if not iy in to_mount and ix.get_is_mounted() :
                    to_umount.append(ix)
                    to_mount.append(iy)

            if not iy["FSTAB_OPTION"] == ix["FSTAB_OPTION"] :
                log.append("+ options: %s -> %s" % (ix["FSTAB_OPTION"], iy["FSTAB_OPTION"]))
                if not iy in to_mount and ix.get_is_mounted() :
                    for opt in ix.listopt() :
                        if opt not in iy.listopt()  \
                                and opt not in FstabData.dont_need_remount :
                            to_umount.append(ix)
                            to_mount.append(iy)
                            break
                    for opt in iy.listopt() :
                        if opt not in ix.listopt() \
                                and opt not in FstabData.dont_need_remount :
                            to_umount.append(ix)
                            to_mount.append(iy)
                            break
                
            if not iy["FSTAB_PASO"] == ix["FSTAB_PASO"] :
                log.append("+ check at boot: %s -> %s" % (ix["FSTAB_PASO"], iy["FSTAB_PASO"]))

        for i in range(len(x)) :           
            ix = x[i]
            ixdev = ix["DEVICE"]
            ixpath = ix["FSTAB_PATH"]
            if not y.search(ixpath) :
                if not len(y.search(ixdev, keys = ["DEVICE"])) == len(x.search(ixdev, keys = ["DEVICE"])) :
                    log.append("Removing %s on %s" % (ixdev, ixpath))
                    if ix.get_is_mounted() :
                        removed.append(ix)
                elif not len(x.search(ixdev, keys = ["DEVICE"])) == 1 :
                    log.append("Removing duplicate %s on %s" % (ixdev, ixpath))
                    if ix.get_is_mounted() :
                        to_umount.append(ix)

    return (log, to_mount, to_umount, removed)

