# -*- coding: UTF-8 -*-
#
#  DiskInfo.py : Detect and get informations about block devices
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
import glob
import logging
from subprocess import *
from subprocess import *

from DiskManager.Fstab.Fstabconfig import *
from DiskManager.Fstab.FstabError import *
from DiskManager.Fstab import FstabData

REQUIRED = ["DEV", "DEVICE", "DEV_NUM", "FS_TYPE"]
DEFAULT_BACKEND = "ToolsBackend"


def get_diskinfo_backend(backend = "auto") :
    ''' get_diskinfo_backend([backend]) -> return the best backend available, 
                                           or the backend specify by backend.\n
        ex : info = get_diskinfo_backend()() '''

    backends = []
    path = os.path.dirname(__file__)
    if not path :
        path = os.getcwd()
    sys.path.append(path)
    names = [ os.path.basename(path)[:-3] for path in os.listdir(path)
              if path.endswith("Backend.py") ]
    for name in names:
        module = __import__( name )
        for attr in dir(module):
            obj = getattr(module, attr)
            if hasattr(obj, "_backend_score") :
                backends.append(obj)
    if len(backends) == 0 :
        logging.warning("No DiskInfo backend found. Trying the default one...")
        module = __import__( DEFAULT_BACKEND )
        return getattr(module, DEFAULT_BACKEND)
    if not backend == "auto" :
        backend = backend[0].upper() + backend[1:] + "Backend"
        if not backend in [ k.__name__ for k in backends ] :
            logging.warning("%s is not available. Using auto backend." % backend)
        elif getattr(module, backend )()._backend_score() :
            return getattr(module, backend)
        else :
            logging.warning("%s is not currently working. Using auto backend." % backend)
    scores = [ k()._backend_score() for k in backends ]
    return backends[scores.index(max(scores))]


class DiskInfoBase(list) :
    ''' Base class for all DiskInfo backend. This class implememt all the infrastructure
        to manage DiskInfo class, and leave only to backends the works to get sensible
        informations about a device. '''

    def __init__(self) :

        self._loaded = False
        self._loading = False
        self._driver_db = {}
        
    def _backend_score(self) :
        ''' Return the reliability score of the backend (0-100). You must override
            this method when creating a backend. You might probably make this score
            dependant of the versions of the tools you use. Keep also in mind that
            the score of the default backend (ToolsBackend) is set to 50 '''
             
        return 0
        
    def _get_devices(self) :
        ''' This function try to get all block devices from /proc/partitions. You might or not
            override this method, dependending on how reliable you think it is. '''
    
        fd = open("/proc/partitions")
        devices = fd.readlines()[2:]
        fd.close()
        return [x for x in devices if all(y not in x for y in ["fd", "loop", "sr", "ram"])]
        
    def load_database(self, reload = False) :
        ''' x.load_database([reload]) -> load the database.\n
            At DiskInfo creation, the database is not created. You have to call load_database
            to create it. But you should not need to explicitly doing it, since all other methods
            that use the database should do it. In fact this method is keeped public only
            for it's reload option : when calling load_database with reload = True, the
            database is reloaded '''
            
        if (self._loaded or self._loading) and not reload : 
            return
        logging.debug("Starting DiskInfo database creation...")
        self._loading = True
        del self[:]
        devices = self._get_devices()
        for i in range(len(devices)) :
            self.append({})
            self[i] = self._get_device_info(devices[i])
            self[i] = self._get_common_device_info(self[i])
            logging.debug("-"*10)
        self._load_reverse_database()
        self._loaded = True
        self._loading = False
        logging.debug("End DiskInfo database creation.")
        
    def _get_device_info(self, device) :
        ''' This is the method that all backends must override. It should returns a dict of
            information about device. Informations are :
            "DEV"       : simple device name, like sda1 or dm-0. (required)
            "DEVICE"    : the device path, like /dev/sda1, or /dev/mapper/*. (required)
            "FS_UUID"   : the UUID of the device (required)
            "FS_LABEL"  : the label of the device (required if available)
            "FS_TYPE"   : the type of the device, like ext3. (required)
            "FS_USAGE"  : the usage of the fs, like filesystem, or crypto.
                          Devices with FS_USAGE != filesystem are ignored. (required)
            "REMOVABLE" : set to True if the device is from a removable drive.
                          Removable devices are ignored. (required)
            "MINOR"     : the minor number of the device (required)
            "MAJOR"     : the major number of the device (required)
            "SYSFS_PATH": the sysfs path (recommanded)
            "SIZE"      : the size of the device (recommanded)
            "FS_VERSION": the version of the type (recommanded)
            "MODEL", "SERIAL_SHORT", "BUS", "VENDOR", "SERIAL", "TYPE", "REVISION" :
            some informations about the device, and its drive. (optionnal) '''
    
        #Impement your backend here
        return device
        
    def _get_common_device_info(self, dev) :
        ''' This method complete device informations with common attributes that should not
            be the job of a backend. Also set here what we ignore or not. '''

        # Set FS_DRIVERS
        if "FS_TYPE" in dev and "FS_USAGE" in dev and dev["FS_USAGE"] == "filesystem" :
            dev["FS_DRIVERS"] = self.get_drivers(dev["FS_TYPE"])
            
        # Set FS_LABEL_SAFE
        if "FS_LABEL" in dev and "FS_LABEL_SAFE" not in dev :
            label = dev["FS_LABEL"]
            label = label.replace("/", "")
            for char in FstabData.special_char :
                label = label.replace(char, "_")
            if label :
                logging.debug("-> Set   FS_LABEL_SAFE : " + label)
                dev["FS_LABEL_SAFE"] = label
                
        # Set DEV_NUM
        if "MINOR" in dev and "MAJOR" in dev :
            dev["DEV_NUM"] = os.makedev(int(dev["MAJOR"]), int(dev["MINOR"]))

        # Ignore every dev if FS_USAGE != filesystem or if REMOVABLE
        # also ignore dev that don't have all REQUIRED attributes
        dev["IGNORE"] = False
        if "FS_USAGE" not in dev :
            dev["IGNORE"] = True
        elif not dev["FS_USAGE"] == "filesystem" :
            dev["IGNORE"] = True
        if not min(list(map(lambda x: x in dev, REQUIRED))) :
            dev["IGNORE"] = True
        if dev["REMOVABLE"] :
            dev["IGNORE"] = True

        # Ignore older entry with the same DEVICE
        if "DEVICE" in dev :
            for device in self.search(dev["DEVICE"], keys = ["DEVICE"]) :
                if not self[device]["DEV"] == dev["DEV"] :
                    logging.debug("W: " + "Ignore duplicate entry : " + self[device]["DEV"] \
                        + " -> " + self[device]["DEVICE"])
                    self[device]["IGNORE"] = True

        return dev
        
    def _load_reverse_database(self) :
    
        self._reverse_database = {}
        for i in range(len(self)) :
            if "DEV_NUM" in self[i] :
                self._reverse_database["DEV_NUM=%s" % self[i]["DEV_NUM"]] = i
            if "FS_UUID" in self[i] :
                self._reverse_database["UUID=%s" % self[i]["FS_UUID"]] = i
            if "FS_LABEL" in self[i] :
                self._reverse_database["LABEL=%s" % self[i]["FS_LABEL"]] = i
        
    def get_drivers(self, type, reload = False) :
        ''' x.get_drivers(type, [reload]) -> return a dict of available driver for this type.\n
            Return dict is of the type : {primary : [[name1, description1, fsck1], ...], \
                secondary : [[name2, description2, fsck2], ...], \
                all : {name1 : [name1, description1, fsck1], name2 : [name2, description2, fsck2], ...}}
            fsck = 1 if it exists an fsck for this driver, otherwise it is 0.
            drivers are checked in /proc/filesystem, modprobe -l, and in /sbin/mount.*.
            To avoid having to do the work again and again, result is cached, and so
            you ll need to set reload to True to have up to date results. '''
    
        if type in self._driver_db and not reload :
            return self._driver_db["type"]
        self._driver_db["type"] = { "primary" : [], "secondary" : [], "all" : {} }
        if type in open("/proc/filesystems").read() or self._check_module(type) :
            self._driver_db["type"]["primary"].append([type, "Default driver"])
        for special in glob.glob("/sbin/mount.%s*" % type) :
            if os.path.isfile(special) :
                special = special.split("mount.")[-1]
                self._driver_db["type"]["primary"].append([special, \
                    FstabData.special_driver.get(special, FstabData.special_driver["__unknow__"])])
        if "__all__" in FstabData.secondary_driver :
            self._driver_db["type"]["secondary"].append([ \
                FstabData.secondary_driver["__all__"], "Secondary Driver"])
        if type in FstabData.secondary_driver :
            self._driver_db["type"]["secondary"].append([ \
                FstabData.secondary_driver[type], "Secondary Driver"])
        for driver in self._driver_db["type"]["primary"] + self._driver_db["type"]["secondary"] :
            driver.append(int(os.path.isfile("/sbin/fsck.%s" % driver[0])))
            self._driver_db["type"]["all"][driver[0]] = driver
        return self._driver_db["type"]

    def _check_module(self, module) :
        try:
            cmd = [MODPROBE, "-n", "-i", module]
            run(cmd, check=True, stderr=DEVNULL, stdout=DEVNULL)
            return True
        except CalledProcessError:
            return False

    def __getitem__(self, item) :
        
        self.load_database()
        if type(item) == int :
            if item < len(self) :
                return list.__getitem__(self, item)
            else :
                raise NotInDatabase("Index %i out of range" % item)
        else :
            try :
                return list.__getitem__(self, self.search(item)[0])
            except :
                raise NotInDatabase("Can't find %s in the database" % item)
            
    def list(self, col = "DEVICE", ignored = True, keep_index = False) :
        ''' x.list([col], [ignored], [keep_index]) -> List all values of attribute col.
                                                      Default to "DEVICE"\n
            If ignored is set to False, don't list device with IGNORE=True. Default to True.
            If keep_index is set to True and ignored to False, all ignored device result to
            an empty string, to keep the index with the database. Default to False. ''' 

        self.load_database()
        result = []
        for k in self :
            if not ignored and k["IGNORE"] :
                if keep_index :
                    result.append("")
                continue
            if col in k :
                result.append(k[col])
            else :
                result.append("")
        return result
        
    def get(self, item, attribute) :
        ''' x.get(item, attribute) -> return attribute of item '''
    
        self.load_database()
        try :
            return self[item][attribute]
        except :
            return "None"
            
    def search_reverse(self, pattern, ignored = True) :
        ''' x.search_reverse(pattern, [ignored]) -> search for pattern in the reverse database\n
            This method is faster than the search method, but not as flexible, since pattern is
            searched in fixed keys : "DEV_NUM", "FS_UUID", "FS_LABEL"
            Set ignored to False, to not return device with IGNORE=True. Default to True '''

        self.load_database()
        if self._loaded and pattern in self._reverse_database :
            result = self._reverse_database[pattern]
            if ignored or not self[result]["IGNORE"] :
                return result
        return None

    def search_device(self, entry, ignored = True) :
        ''' x.search_device(entry, ignored = True) -> search entry in the reverse database\n
            search_device is a convenient method that will check if device is of
            the type dev_file, "UUID=", "LABEL=" and will search in the reverse database
            the appropriate value.
            Set ignored to False, to not return device with IGNORE=True. Default to True '''

        if os.path.exists(entry) :
            entry = "DEV_NUM=%s" % os.stat(os.path.realpath(entry)).st_rdev
        return self.search_reverse(entry, ignored)

    def search(self, pattern, keys = ["DEV", "DEVICE"], ignored = True) :
        ''' x.search(pattern, [list], [keys], [ignored]) -> search for pattren in each
                                                            keys of each Entry of x\n
            Default keys are : ["DEV", "DEVICE"]
            Set ignored to False, to not return device with IGNORE=True. Default to True '''

        self.load_database()
        result = []
        for col in keys :
            i = 0
            for value in self.list(col, ignored, keep_index = True) :
                if value == pattern and i not in result :
                    result.append(i)
                if col == "FS_UUID" :
                    if "UUID=" + value == pattern and i not in result :
                        result.append(i)
                if col == "FS_LABEL" :
                    if "LABEL=" + value == pattern and i not in result :
                        result.append(i)
                i = i + 1
        return result

    def export(self, device) :
        ''' x.export(device) -> query database for device, and return a string of printable
                                informations about device.\n
            If device is set to "all", query database for all devices '''
    
        self.load_database()
        result = ""
        if device == "all" :
            result += "Query database for all devices :\n\n"
            for i in range(len(self)) :
                result += "Info for " + self[i]["DEV"] + " :\n"
                result += "\n".join(["-> %s=%s" % (k, v) for k, v in list(self[i].items())])
                result += "\n\n"
        else :
            result += "Query database for " + device + " :\n\n"
            if self.search(device) :
                result += "Info for " + self[device]["DEV"] + " :\n"
                result += "\n".join(["-> %s=%s" % (k, v) for k, v in list(self[device].items())])
                result += "\n\n"
            else :
                result += device + " not in the database"
        return result
