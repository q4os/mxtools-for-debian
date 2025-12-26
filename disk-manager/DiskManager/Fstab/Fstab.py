# -*- coding: UTF-8 -*-
#
#  Fstab.py : Provide low level classes to manage fstab/mtab like file
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
import re
import copy
import shutil
import tempfile
import functools
import subprocess
from subprocess import *

from . import FstabData
from .DiskInfo import *
from .FstabError import *
from .FstabUtility import *


class EntryBase(dict) :
    ''' EntryBase([device/uuid/label, path, option, type, freq, paso]) -> new EntryBase\n
        Create a dictionary of information available about device :
        - Merge information provided in argument with DiskInfo information.
        - Store all fstab/mtab related attribute as FSTAB_* '''

    def __init__(self, entry) :

        if "info" not in globals() :
            globals()["info"] = get_diskinfo_backend("auto")()
        for i in FstabData.categorie : self[i] = ""
        for i in range(len(entry)) : self[FstabData.categorie[i]] = entry[i]
        if self["FSTAB_PATH"] :
            self["FSTAB_PATH"] = decode(self["FSTAB_PATH"])
        try :
            self.update(info[info.search_device(entry[0])])
        except :
            pass

    def write(self, type = "FSTAB_NAME") :
        ''' w.write() -> Return a string of the entry in fstab/mtab synthax '''

        result = ""
        if type == "FS_UUID" or type == "FS_LABEL" and "DEVICE" in self :
            result += "#Entry for " + self["DEVICE"] + " :\n%s=" % type.split("_")[-1]
        elif type == "DEVICE" and "FSTAB_NAME" in self and os.path.exists(self["FSTAB_NAME"]) :
            if os.path.samefile(self["DEVICE"], self["FSTAB_NAME"]) :
                type = "FSTAB_NAME"
        result += "%s\t" % self[type]
        result += "%s\t" % encode(self["FSTAB_PATH"])
        result += "\t".join(self[k] for k in FstabData.categorie[2:])
        result += "\n"
        return result

class Entry(EntryBase) :
    ''' Entry([device/uuid/label, [path], [option], [type], [freq], [paso]]) -> new Entry\n
        Difference with EntryBase :
        - Only device is required. If not provided, other required info are set to defaults.
        - Set FSTAB_OPTION, FSTAB_FREQ and FSTAB_PASO to default when changing FSTAB_TYPE.
        - If FSTAB_PATH is in use, add an "_" until it can be used.
        - Set special option locale=autoset automatically. This possibility will be extend
          to other options in the future.
        - When setting FSTAB_TYPE to FS_TYPE, we use the first value of FS_DRIVERS, which may be
          different. example : FS_TYPE is ntfs and you don't have the ntfs driver, but ntfs-3g '''

    def __init__(self, entry, parent = None) :

        self.parent = parent
        EntryBase.__init__(self, entry)
        if not self["FSTAB_PATH"] :
            if "FS_LABEL_SAFE" in self :
                self["FSTAB_PATH"] = "/media/" + self["FS_LABEL_SAFE"]
            else :
                self["FSTAB_PATH"]  = "/media/" + self["DEV"]
        if not self["FSTAB_TYPE"] :
            self["FSTAB_TYPE"] = self["FS_TYPE"]

    def __setitem__(self, key, item) :
        ''' x.__setitem__(i, y) <==> x[i]=y '''

        if key == "FSTAB_TYPE" and item :
            if "FS_TYPE" in self and item == self["FS_TYPE"] :
                try :
                    item = self["FS_DRIVERS"]["primary"][0][0]
                except :
                    item = self["FS_TYPE"]
            if "FSTAB_OPTION" in self :
                for value in self.listopt():
                    if value not in FstabData.common :
                        self.removeopt(value)
                default = list(FstabData.defaults.get(item, FstabData.defaults["__default__"]))
                if self["FSTAB_OPTION"] :
                    default[0] += "," + self["FSTAB_OPTION"]
                (self["FSTAB_OPTION"], self["FSTAB_FREQ"], self["FSTAB_PASO"]) = default

        if key == "FSTAB_OPTION" and item and "FSTAB_OPTION" in self :
            listopt = item.split(",")
            if "locale=autoset" in listopt :
                listopt.remove("locale=autoset")
                listopt.append("locale=" + os.environ["LANG"])
                item = ",".join(listopt)

        if key == "FSTAB_PATH" and item and "DEVICE" in self \
                and "FSTAB_PATH" in self :
            while not check_path(item, self.parent, entry = self) :
                logging.debug("-> %s is in use." % item)
                item = item + "_"
            item = os.path.normpath(item)

        dict.__setitem__(self, key, item)

    def __eq__(x, y) :

        x2 = dict.copy(x)
        x2["FSTAB_NAME"] = y["FSTAB_NAME"]
        return dict.__eq__(x2, y)

    def make_path(self) :
        ''' x.make_path() -> Create FSTAB_PATH.\n
            If path is already in use add a "_" until it can be used.
            All path created by the class are stored in %s ''' % CREATED_PATH_FILE

        path = self["FSTAB_PATH"]
        logging.debug("Check if %s is created" % path)
        while not check_path(path, self.parent, entry = self) :
            logging.debug("-> %s is in use." % path)
            path = path + "_"
        for i in range(len(path.split("/")) -1) :
            if not os.path.exists("/".join(path.split("/")[:i+2])) :
                logging.debug("-> create path %s" % "/".join(path.split("/")[:i+2]))
                try :
                    os.mkdir("/".join(path.split("/")[:i+2]))
                except OSError:
                    logging.error("Creating %s failled. "
                       "There is something wrong with this path." % "/".join(path.split("/")[:i+2]))
                list_created_path("add", "/".join(path.split("/")[:i+2]))
        if not self["FSTAB_PATH"] == path :
            self["FSTAB_PATH"] = path

    def listopt(self) :
        ''' x.listopt() -> return a list of FSTAB_OPTION '''

        return self["FSTAB_OPTION"].split(",")

    def hasopt(self, option) :
        ''' x.hasopt(option) -> True/False '''

        if option in self.listopt() :
            return True
        return False

    def addopt(self, options) :
        ''' x.addopt([opt1,opt2 ...]) -> Add opt1, opt2 ... to FSTAB_OPTION '''

        if isinstance(options, str) :
            options = options.split(",")
        for value in options :
            if value not in self.listopt() :
                if self["FSTAB_OPTION"] :
                    self["FSTAB_OPTION"] += "," + value
                else :
                    self["FSTAB_OPTION"] = value

    def removeopt(self, options) :
        ''' x.removeopt([opt1,opt2 ...]) -> Remove opt1, opt2 ... from FSTAB_OPTION '''

        if isinstance(options, str) :
            options = options.split(",")
        for value in options :
            if value in self.listopt() :
                opt = self.listopt()
                opt.remove(value)
                self["FSTAB_OPTION"] = ",".join(opt)

    def defaultopt(self) :
        ''' x.defaultopt() -> return the default options for the entry '''

        return FstabData.defaults.get(self["FSTAB_TYPE"], FstabData.defaults["__default__"])[0]

    def setopt(self, options) :
        ''' x.setopt([opt1,opt2 ...]) -> set opt1, opt2 ... to FSTAB_OPTION '''

        self["FSTAB_OPTION"] = ""
        self.addopt(options)

    def mount(self) :
        ''' x.mount() -> Mount the entry and return (exit_code, stderr+stdout)\n
            The path is created if it dont exist yet '''
        self.make_path()
        cmd = "%s %s %s -t %s -o %s" % (MOUNT, self["DEVICE"], escape_special(self["FSTAB_PATH"]), \
                            self["FSTAB_TYPE"], self["FSTAB_OPTION"])
        process = Popen(cmd, stderr=STDOUT, stdout=PIPE, close_fds=True, shell=True, text=True)
        sts = process.wait()
        output = process.stdout.read()
        logging.debug("Mounting %s on %s :\n-> cmd : %s\n-> exit : %s\n-> output : %s" % \
                (self["DEVICE"], self["FSTAB_PATH"], cmd, sts , output))

        fstab_path = self["FSTAB_PATH"]
        chk_bind = "test ! -d /run/systemd/system && grep '" + fstab_path +"' /etc/fstab | awk '/^[a-zA-Z/]/{print $4}' | grep bind"
        try :
            logging.debug("TRY CHK_BIND: %s on %s :\n-> cmd : %s\n" % (self["DEVICE"], fstab_path, chk_bind))
            prc = run(chk_bind, shell=True, stderr=STDOUT, stdout=PIPE, check=True, text=True)
            ret = prc.returncode
            out = prc.stdout
            logging.debug("TRY CHK_BIND: %s" % out)
            try :
                logging.debug("MOUNT: bind mounts")
                prc = run([MOUNT, "-a"], stderr=STDOUT, stdout=PIPE, check=True, text=True)
                ret = prc.returncode
                out = prc.stdout
            except:
                pass
        except:
            pass
        return (sts, output)

    def umount(self, lazy = False) :
        ''' x.umount() -> Unmount the entry and return (exit_code, stderr+stdout)
            fix bind mounts by unmounting all mountpoints related to device '''

        device = self["DEVICE"]
        fstab_path = self["FSTAB_PATH"]

        # get mountpoints based on device as listed in /proc/mounts
        mountpoints = []
        with open('/proc/mounts', 'rt') as mounts:
            for mount in mounts:
                    [dev, mp] = mount.strip().split()[0:2]
                    if dev == device:
                        logging.debug(f"Found mountpoint of {dev} on {mp}")
                        mountpoints += [decode(mp)]
        ret = 0
        out = ""
        # unmount mountpoints in reverse order
        for mountpoint in mountpoints[::-1]:
            if not os.path.ismount(mountpoint):
                continue
            cmd = [ UMOUNT, "--recursive", mountpoint ]
            #cmd = [ UMOUNT, mountpoint ]
            if lazy :
                cmd += ["-l"]
            try:
                prc = run(cmd, stderr=STDOUT, stdout=PIPE, check=True, text=True)
                ret = prc.returncode
                out = prc.stdout
                logging.debug(f"Unmounting {device} on {mountpoint} :\n-> cmd : {' '.join(cmd)}\n-> exit : {ret}\n-> output : {out}")
            except CalledProcessError as e:
                ret = e.returncode
                out = e.stdout
                logging.debug(f"Unmounting {device} on {mountpoint} :\n-> cmd : {' '.join(cmd)}\n-> exit : {ret}\n-> output : {out}")
                break

        return (ret, out)

    def get_is_mounted(self) :
        ''' x.get_is_mounted() -> True/False '''

        return bool(os.path.ismount(self["FSTAB_PATH"]))

    def get_is_system(self) :
        ''' x.get_is_mounted() -> True/False\n
            System partition are FSTAB_PATH before /home/*, /media/* and /opt.
            So /home is sytem, but /opt is not. '''

        path = os.path.normpath(self["FSTAB_PATH"])
        if path in FstabData.system["exact"] + FstabData.system["extand"] :
            return True
        for sys_path in FstabData.system["extand"] :
            if path[:len(sys_path)+1] == "%s/" % sys_path :
                return True
        return False

    def get_size(self) :
        ''' x.get_size() -> Return size of device '''

        if self.get_is_mounted() :
            return os.statvfs(self["FSTAB_PATH"])[2]*os.statvfs(self["FSTAB_PATH"])[1]
        else :
            return self["SIZE"]

    def get_free_size(self) :
        ''' x.get_free_size() -> Return free size of device\n
            If device is not mounted, return 0 '''

        if self.get_is_mounted() :
            stat = os.statvfs(self["FSTAB_PATH"])
            return stat[3]*stat[1]
        else :
            return 0

    def get_available_size(self) :
        ''' x.get_available_size() -> Return available size of device\n
            If device is not mounted, return 0 '''

        if self.get_is_mounted() :
            stat = os.statvfs(self["FSTAB_PATH"])
            return stat[4]*stat[1]
        else :
            return 0

    def get_used_size(self) :
        ''' x.get_used_size() -> Return available size of device\n
            If device is not mounted, return 0 '''

        if self.get_is_mounted() :
            return self.get_size() - self.get_free_size()
        else :
            return 0

    def copy(self) :
        ''' x.copy() -> return an exact copy of the entry '''

        new = self.__class__([self["FSTAB_NAME"], self["FSTAB_PATH"], self["FSTAB_TYPE"], \
            self["FSTAB_OPTION"], self["FSTAB_FREQ"], self["FSTAB_PASO"]], parent = self.parent)
        return new

class MntFile(list) :
    ''' MntFile(filename, [fd], [minimal], [naming], [backend]) -> new MntFile object\n
        MntFile create a list of Entry object for each manageable entry of filename (or fd).
        An entry should be a line of 6 field separeted by at least one space :
        device/uuid/label, path, option, type, freq, and paso. (see man fstab)
        A manageable device is a none ignored device of DiskInfo dadatbase.
        Other 6 field entry are stored as EntryBase respectively in x.dev for virtual device,
        x.other for other device, x.comment for commented entry (ex : #device path ...).
        You can get an Entry from the MntFile, either by specify is index,
        just like with list, but also by specify it path.
        ex : x[i] -> Entry at index i of MntFile
             x[\"/\"] - > Entry with FSTAB_PATH = \"/\"
        If the Entry can't be found, the NotInDatabase exception will be raised
        Option available :
        - fd : create MntFile from fd
        - minimal : if True, create a minimal MntFile, with only EntryBase object.
          Faster to create and to manage. Default to False
        - naming : specify the naming that should be used when writing filename back
          available naming are : uuid, dev or auto. auto will use uuid if at least
          one entry of the file use uuid. Default to auto.
        - backend : specify the backend that should be use for DiskInfo. Default to auto '''

    def __init__(self, filename, fd = None, minimal = False, \
            naming = "auto", backend = "auto") :

        if "info" not in globals() :
            globals()["info"] = get_diskinfo_backend(backend)()
        self.info = info
        (self.dev, self.other, self.comment) = ([], [], [])
        self.filename = filename
        self.naming = naming
        if fd :
            mntfile = fd
        else :
            logging.debug(f"==>> Scanning {self.filename}")
            mntfile = open(self.filename)
        lines = mntfile.readlines()
        mntfile.close()
        if not minimal :
            logging.debug("Creating MntFile object for %s" % filename)
        seen = []
        for line in lines :
            entry = line.split()
            if len(entry) in [4,5] and not entry[0][0] == "#" :
                if len(entry) == 4 :
                    entry.append("0")
                if len(entry) == 5 :
                    entry.append("0")
            if not len(entry) == 6 :
                continue
            if self.filename == "/etc/mtab":
                if entry[0] in seen:
                    logging.debug(f"Skipping seen device in {filename}: {line.strip()}")
                    continue
                else:
                    seen += [ entry[0] ]

                #if entry[2] in "fuseblk":
                #    logging.debug(f"Skipping fuseblk mounts in {filename}: {line.strip()}")
                #    continue
                #else:
                #    seen += [ entry[0] ]

            if minimal :
                if not entry[0][0] == "#" :
                    self.append(EntryBase(entry))
                continue
            i = info.search_device(entry[0], ignored = False)
            if i :
                logging.debug("-> Matching %s on %s as %s" % (entry[0], entry[1], info.get(i, "DEV")))
                self.append(Entry(entry, self))
            elif entry[0] in FstabData.virtual_dev  :
                logging.debug("-> Matching %s on %s as virtaul fs" % (entry[0], entry[1]))
                self.dev.append(EntryBase(entry))
            elif not line.strip()[0] == "#" :
                logging.debug("-> Matching %s on %s as other fs" % (entry[0], entry[1]))
                self.other.append(EntryBase(entry))
            elif re.search(r"^#\S", entry[0]) :
                self.comment.append(EntryBase(entry))

    def __getitem__(self, item) :
        ''' x.__getitem__(y) <==> x[y] '''

        if isinstance(item, int) :
            try :
                return list.__getitem__(self, item)
            except :
                raise NotInDatabase(item)
        elif isinstance(item, slice) :
            try :
                return list.__getitem__(self, item)
            except :
                raise NotInDatabase(item)
        elif isinstance(item, str) :
            try :
                return list.__getitem__(self, self.search(item)[0])
            except :
                raise NotInDatabase(item)
        else :
            raise DatabaseTypeError(type(item), (int, str))

    def add(self, entry) :
        ''' x.add(entry) -> Create an Entry object with entry and add it to x.\n
            Return the new created entry '''

        self.append(Entry(entry, self))
        return self[-1]

    def remove(self, entry) :
        ''' x.remove(entry) -> remove entry from x\n
            entry should be the index of the Entry to delete a path name or the entry itself '''

        if isinstance(entry, int) :
            try :
                list.__delitem__(self, entry)
            except :
                raise NotInDatabase(entry)
        elif isinstance(entry, Entry) :
            try :
                list.remove(self, entry)
            except :
                raise NotInDatabase(entry)
        elif isinstance(entry, str) :
            try :
                list.__delitem__(self, self.search(entry, strict = "yes")[0])
            except :
                raise NotInDatabase(entry)
        else :
            raise DatabaseTypeError(type(entry), (int, str, Entry))

    def list(self, col = "DEVICE") :
        ''' x.list([col]) -> List all values of attribute col. Default to DEVICE '''

        result = []
        for k in self :
            if col in k :
                result.append(k[col])
            else :
                result.append("")
        return result

    def search(self, pattern, strict = "yes", keys = ["FSTAB_PATH"]) :
        ''' x.search(pattern, [strict], [list], [keys]) -> search for pattern in each
            keys of each Entry of x\n
            Default keys are : ["FSTAB_PATH"]
            If strict == "no", index entry where a substring pattern is found. Default to "yes". '''

        result = []
        for col in keys :
            i = 0
            for value in self.list(col) :
                if strict == "yes" :
                    if value == pattern and i not in result :
                        result.append(i)
                else :
                    if value.find(pattern) != -1 and i not in result :
                        result.append(i)
                i = i + 1
        return result

    def make_all_path(self) :
        ''' x.make_all_path() -> check that path used in MntFile are created, and create them if needed\n
            See make_path method of Entry object. '''

        logging.debug("Checking that all path used in %s are created :" % self.filename)
        for entry in self :
            entry.make_path()

    def write(self) :
        ''' x.write() -> Return a string of the MntFile in fstab/mtab default synthax '''

        result = ""
        for i in range(len(self.dev)) :
            result += self.dev[i].write()
        if not self.naming in ("auto", "dev", "uuid") :
            logging.warning("Fstab naming %s is not supported. Using auto naming." % self.naming)
            self.naming = "auto"
        if self.naming == "auto" and self.search("UUID=", strict = "no", keys = ["FSTAB_NAME"]) :
            self.naming = "uuid"
        self.sort(key=functools.cmp_to_key(self._sort_path))
        for entry in self + self.other :
            if "LABEL=" in entry["FSTAB_NAME"] and "FS_LABEL" in entry \
                    and len(self.search(entry["FS_LABEL"], keys = ["FS_LABEL"])) < 2 \
                    and os.path.exists("/dev/disk/by-label/%s" % entry["FS_LABEL"]) :
                type = "FS_LABEL"
            elif self.naming == "uuid" and "FS_UUID" in entry \
                    and len(self.search(entry["FS_UUID"], keys = ["FS_UUID"])) < 2 \
                    and os.path.exists("/dev/disk/by-uuid/%s" % entry["FS_UUID"]) :
                type = "FS_UUID"
            elif "DEVICE" in entry :
                type = "DEVICE"
            else :
                type = "FSTAB_NAME"
            result += entry.write(type)
        result += "\n"
        for i in range(len(self.comment)) :
            result += self.comment[i].write()
        result += "\n"
        return result

    def _sort_path(self, x, y) :

        x = x["FSTAB_PATH"]
        y = y["FSTAB_PATH"]
        if x == y :
            return 0
        if x.startswith(y) :
            return 1
        if y.startswith(x) :
            return -1
        ix = iy = 0
        for i, path in enumerate(FstabData.path_order) :
            if x.startswith(path) :
                ix = i
            if y.startswith(path) :
                iy = i
        return ix - iy

    def copy(self) :
        ''' x.copy() -> Create an exact copy of the MntFile '''

        new = copy.copy(self)
        for i in range(len(self)) :
                new[i] = self[i].copy()
        return new

    def apply(self) :
        ''' x.apply() -> Write MntFile to filename '''

        tmpfile = tempfile.NamedTemporaryFile()
        tmpfile.write(FstabData.header.encode('utf-8'))
        tmpfile.write(self.write().encode('utf-8'))
        tmpfile.seek(0)
        mntfile = open(self.filename, "wb")
        shutil.copyfileobj(tmpfile, mntfile)
        mntfile.close()
        tmpfile.close()


def list_created_path(action = "list", path = None) :
    ''' Action :
        "list" : list all path created by python-fstab
        "add"  : add a path to the list
        "del"  : remove a path from the list
        "clean": clean the file '''

    created_file = open(CREATED_PATH_FILE, "a+")
    li = created_file.read().strip().split("\n")
    created_file.close()
    created_file = open(CREATED_PATH_FILE, "w+")
    if action == "add" and not encode(path) in li :
        li.append(encode(path))
    if action == "del" and encode(path) in li :
        li.remove(encode(path))
    if action == "clean" :
        for path in li[:] :
            if not os.path.exists(decode(path)) :
                logging.debug("Clean %s from the list of created path" % path)
                li.remove(path)
    created_file.write("\n".join(li).strip())
    created_file.close()
    if not "".join(li) and os.path.exists(CREATED_PATH_FILE) :
        os.remove(CREATED_PATH_FILE)
    if action == "list" :
        if len(li) == 1 and len(li[0]) == 0 :
            return []
        return list(map(decode, li))

def clean_path(path) :
    ''' Remove the path if not in use '''

    logging.debug("Check if path can be deleted : %s" % path)
    while path in list_created_path() :
        if os.path.isdir(path) and check_path(path) or not os.path.exists(path) :
            logging.debug("-> delete path : %s" % path)
            try :
                os.rmdir(path)
                list_created_path("del", path)
            except OSError :
                logging.error("Deleting %s failled. "
                    "There is something wrong with this path." % path)
        path = os.path.dirname(path)

def clean_all_path() :
    ''' Check all path in list_created_path, and reomove them if not in use '''

    logging.debug("Checking all path for deletation :")
    for path in list_created_path() :
        clean_path(path)

def check_path(path, fstab = None, entry = None) :
    ''' check that path is not in use '''

    path = os.path.normpath(path)
    if os.path.ismount(path) :
        if entry and path == entry["FSTAB_PATH"] :
            return True
        return False
    else :
        not_in_use = bool(not os.path.exists(path) or ( os.path.isdir(path) and len(os.listdir(path)) == 0 ))
        if not fstab :
            fstab = MntFile(FSTAB, minimal = True)
        if fstab.search(path) :
            if entry and path == entry["FSTAB_PATH"] and not_in_use :
                return True
            return False
        if not_in_use :
            return True
        return False

def device_is_mounted(device) :
    ''' Return path used by device if device is mounted.
        This function differ from the get_is_mounted() method of Entry as this function
        try to find device in mtab, and don't just look if a device is mounted at path '''

    mtab = MntFile(MTAB, minimal = True)
    return [ mtab[i]["FSTAB_PATH"] for i in mtab.search(device, keys = ["DEV", "DEVICE"]) ]

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG, format='%(levelname)s : %(message)s')
    fstab = MntFile("fstab.test", minimal=True)
    print(fstab.list("FSTAB_PATH"))
    fstab.sort(cmp=fstab._sort_path)
    print(fstab.list("FSTAB_PATH"))
