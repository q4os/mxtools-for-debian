# -*- coding: UTF-8 -*-
#
#  FstabHandler.py : High level class for managing fstab file
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
import time
import shutil
import logging
import tempfile
import threading
import configparser
#from gettext import gettext as _

from .Fstabconfig import *
from .FstabUtility import *
from .Fstab import *
from .FstabDialogs import *
from .FstabError import *
from .EventHandler import *
from .Mounter import *
from . import FstabData

available_mode = ["real", "delay", "virtual"]

class FstabHandler(MntFile) :
    ''' FstabHandler(filename, [fd], [mode], [naming], [backend], [log], [parent]
                    [sanity_check]) -> new FstabHandler object\n
        FstabHandler is the high level class of MntFile. It try to provide high level methods.
        In particular, FstabHanler apply change automaticly, show a nice progress bar when
        mounting/unmouting device, can check for unknow and duplicate entry, integreate a logger,
        integrate an undo method... For now FstabHandler can only works with GTK, but
        in the future, Widget class will be abstracted to be able to use other backends\n
        Options are :
        filename, fd, mode, naming, backend : MntFile options. See Fstab.
            The only difference is that filaname is default to filename
        log : If True, save an original version of filename, and allow to use savelog method.
            Default to True.
        parent : specify the parent widget. Default to None
        sanity_check : if True, check for unknow and duplicate entry on creation.
            Default to False.
        external_change_watch : emit events on changes not due to this class
        mode : this is probably the more powerfull option of FstabHandler. 
            There is currently 3 mode available :
            real : the default mode. All changes are apply immediatly.
            delay : changes are only apply when calling the apply_now()
                methd, or when switching to the real mode.
            virtual : all changes are only apply when calling
                the apply_now() method. All not applied changes
                are lost when switching to an other mode. '''

    def __init__(self, filename = FSTAB, fd = None, mode = "real", naming = "auto", \
             backend = "auto", log = True, parent = None, external_change_watch = False) :

        self._parent = parent

        # Create a bckup of filename :
        bckup = "%s-%s-save" % (filename, PACKAGE)
        if not os.path.isfile(bckup) :
            shutil.copy(filename, bckup)

        # Load filename
        MntFile.__init__(self, filename, fd = fd, naming = naming, backend = backend)

        # Move it to fstab, and complete with other available device
        self._build_object()

        # Create copy
        self._current = self.copy()
        self._current.fstab = self.fstab.copy()
        self._original = self.fstab.copy()
        self._lastsave = self.fstab.copy()

        # Init log
        self.logchanges = log
        if not os.getuid() == 0 :
            self.logchanges = False
        self._initlog()

        # Set mode
        self.set_mode(mode)

        # Create Mounter object
        self._mounter = Mounter(self, parent = self._parent)
        self._mounter.hide_gui()

        # Start Event Handler
        self._event = EventHandler(self, external_change_watch)

    def _build_object(self, fstab = None) :

        logging.debug("Building FstabHandler...")
        t = time.time()
        if fstab :
            self[:] = fstab
        self.fstab = self.copy()
        self.fstab[:] = self[:]
        mtab = MntFile(MTAB, minimal = True)
        for device in self.info.list("DEVICE", ignored = False) :
            entries = [ mtab[k] for k in mtab.search(device, keys = ["DEVICE"]) ]
            if entries :
                for entry in entries :
                    path = entry["FSTAB_PATH"]
                    if path not in self.fstab.list("FSTAB_PATH") :
                        if entry["FSTAB_TYPE"] in ("fuse", "fuseblk") :
                            get_fuse_options(entry)
                        type = entry["FSTAB_TYPE"]
                        opt = entry["FSTAB_OPTION"].replace("rw", "defaults")
                        entry = self.add([device, path, type, opt])
                        logging.debug("-> Adding mounted device %s on %s" % (device, \
                                        entry["FSTAB_PATH"]))
            else :
                if device not in self.fstab.list() :
                    entry = self.add([device])
                    logging.debug("-> Adding %s on %s [type: %s]" % (device, entry["FSTAB_PATH"], entry["FSTAB_TYPE"]))
                    if entry["FSTAB_TYPE"] in "ntfs":
                       entry["FSTAB_TYPE"] = "ntfs-3g"
        logging.debug("FstabHandler build in %s s", time.time() -t)

    def _rebuild_object(self, fstab) :

        logging.debug("Rebuilding FstabHandler...")
        t = time.time()
        new = fstab.copy()
        new[:] = fstab[:]
        for entry in self[:] :
            if entry not in new :
                if not entry in self.fstab or not entry["DEVICE"] in new.list() \
                    or (entry["FSTAB_PATH"] not in new.list("FSTAB_PATH") and \
                            entry.get_is_mounted()) :
                    logging.debug("-> Adding %s on %s  (type: %s)" % (entry["DEVICE"], \
                            entry["FSTAB_PATH"], entry["FSTAB_TYPE"] ))
                    new.append(entry)
        self[:] = new
        self.fstab = fstab
        self._check_duplicate(self)
        logging.debug("FstabHandler rebuild in %s s", time.time() -t)

    def _copy(self) :

        new = self.fstab.copy()
        new.fstab = new.copy()
        new.fstab[:] = new[:]
        for entry in self :
            if entry not in new :
                new.append(entry.copy())
        new.current = self._current.copy()
        new.current.fstab = self._current.fstab.copy()
        return new

    def _restore_copy(self, copy) :

        self[:] = copy[:]
        self.fstab = copy.fstab
        self._current = copy.current
        self._current.fstab = copy.current.fstab

    def connect(self, event, fct, *kargs) :
        ''' x.connect(event, fct, *kargs) -> connect event to fct\n
            When event is detected, the function fct will be called with optional 
            parameters kargs. Available events are for now :
            - "external_fstab_changed" : emitted when filename is updated but not due to
                                         this class.
            - "external_mtab_changed" : emitted when MTAB is updated but not due to
                                        this class.
            - "external_changed" : emitted when one of the previous event is emitted
            - "internal_changed" : emitted when this class do a changed 
                                   (mount/unmount/configure...)
            - "configuration_changed" : emitted when one of the previous event is emitted
            - "size_changed" : emitted when size of all mounted device changed significantly
            - "any_changed" : emitted when one of the previous event is emitted '''

        if hasattr(self._event, "on_%s" % event) :
            logging.debug("Connecting event '%s' to %s%s" % (event, fct.__name__, kargs))
            setattr(self._event, "%s_fct" % event, [fct, kargs])
            setattr(self, "%s_fct" % event, [fct, kargs])
        else :
            raise UnknowEvent(self._event, event)

    def shutdown(self) :
        ''' x.shutdown() -> Shutdown FstabHandler process and save log\n
            It is important to call this function if you use the external change watch
            to shutdown it gracefully.'''

        logging.debug("Calling Shutdown...")
        if self.lastsave_has_changed() :
            self.savelog()
        list_created_path("clean")
        self._event.shutdown()

    def set_mode(self, mode) :
        ''' x.set_mode(mode) -> set mode to mode\n
            Be aware that when quiting a virtual mode, all Entry of x are recreated '''

        if not mode in available_mode :
            logging.warning("No FstabHanler mode named %s. Starting with 'real' mode" % mode)
            self._mode = "real"
        if hasattr(self, "_mode") :
            self._previous_mode = self._mode
            t = time.time()
            logging.debug("Switching mode from %s to %s" % (self._mode, mode))
            if self._mode == "virtual" and not mode == "virtual" and hasattr(self, "_real") :
                logging.debug("Restoring state...")
                self._restore_copy(self._real)
            elif self._mode == "delay" and not mode == "delay" :
                logging.debug("Exiting from delay mode. Applying changes...")
                self.apply_now()
            if not self._mode == "virtual" and mode == "virtual" : 
                self._real = self._copy()
            logging.debug("Mode switched in %s s", time.time() -t)
        else :
            logging.debug("Starting in %s mode" % mode)
        self._mode = mode
        
    def set_previous_mode(self) :
        ''' x.set_previous_mode() -> set mode to it's previous value '''
    
        if hasattr(self, "_previous_mode") :
            self.set_mode(self._previous_mode)

    def get_mode(self) :
        ''' x.get_mode() -> get the current mode '''
    
        return self._mode
            
    def get_configured(self) :
        ''' x.get_configured() -> get all device configured in filename '''
        
        return [ k for k in self.fstab ]
        
    def get_all(self) :
        ''' x.get_all() -> get all manageable devices '''
    
        return [ k for k in self ]
        
    def get_new(self) :
        ''' x.get_new() -> get all manageable devices non configures in filename '''
    
        return [ k for k in self if k not in self.fstab ]
        
    def get_duplicate(self, item) :
        ''' x.get_duplicate(item) -> get duplicate entry of item\n
            item might be an entry or a device name.
            if in duplicate, return a list of entry with the same dev, including iteself.
            return ann empty list otherwise '''
            
        if isinstance(item, Entry) :
            dev = item["DEVICE"]
        else :
            dev = item
        res = self.search(dev, keys = ["DEV", "DEVICE"])
        if len(res) > 1 :
            return [ self[k] for k in res ]
        else :
            return []
        
    def _get_entry(self, item) :
    
        if isinstance(item, Entry) :
            return item
        else :
            return self[item]
        
    def get_attribute(self, item, attribute) :
        ''' x.get_attribute(item, attribute) -> get the attribute of item\n
            item can be a FSTAB_PATH or an entry '''
    
        entry = self._get_entry(item)
        if attribute in entry :
            return entry[attribute]
        return "None"
        
    def get_property(self, item, property) :
        ''' x.get_property(item, attribute) -> get a property of item\n
            item can be a FSTAB_PATH or an entry
            current available property are :
            - is_system
            - is_mounted
            - size, free_size, availale_size, used_size '''
    
        entry = self._get_entry(item)
        if hasattr(entry, "get_" + property) :
            return getattr(entry, "get_" + property)()
        return None

    def set(self, item, path=None, type=None, option=None, paso=None) :
        ''' x.set(item, [path], [type], [option], [paso]) -> set item path/type/option 
                                                             and/or paso\n
            item can be a FSTAB_PATH or an entry '''

        entry = self._get_entry(item)
        if path :
            old_path = entry["FSTAB_PATH"]
            entry["FSTAB_PATH"] = path
        if type :
            entry["FSTAB_TYPE"] = type
        if option :
            if option[0] == "+" :
                entry.addopt(option[1:])
            elif option[0] == "-" :
                entry.removeopt(option[1:])
            else :
                entry.setopt(option)
        if not paso == None :
            entry["FSTAB_PASO"] = str(int(bool(paso)) + \
                int(bool(paso) and not entry.get_is_system()))
        self._apply()
            
    def set_default_option(self, item) :
        ''' x.set_default_option(item) -> set item options to default\n
            item can be a a FSTAB_PATH or an entry '''
    
        entry = self._get_entry(item)
        entry["FSTAB_OPTION"] = entry.defaultopt()
          
    def mount(self, item) :
        ''' x.mount(item) -> mount item\n
            item can be a FSTAB_PATH or an entry '''
    
        entry = self._get_entry(item)
        if entry.get_is_mounted() :
            logging.debug("%s on %s already mounted" % (entry["DEVICE"], \
                            entry["FSTAB_PATH"]))
            return 0
        self._event.emit("internal_changed_prepare")
        result = self._mounter.do(mount = [entry])
        if not result[1] :
            if not self._current == self or not self._current.fstab == self.fstab :
                logging.debug("Detected changement.")
                self.simple_apply()
        else :
            clean_path(entry["FSTAB_PATH"])
        self._event.emit("internal_changed")
        return max(result)

    def umount(self, item) :
        ''' x.umount(item) -> unmount item\n
            item can be a FSTAB_PATH or an entry '''
    
        entry = self._get_entry(item)
        if not entry.get_is_mounted() :
            logging.debug("%s on %s already unmounted" % (entry["DEVICE"], \
                            entry["FSTAB_PATH"]))
            return 0
        self._event.emit("internal_changed_prepare")
        result = self._mounter.do(umount = [entry])
        if not result[0] :
            self._check_duplicate([entry])
            clean_path(entry["FSTAB_PATH"])
            self._event.emit("internal_changed")
        else :
            self._event.emit("internal_changed_cancel")
        return max(result)
        
    def configure(self, item) :
        ''' x.configure(item) -> add item to filename and mount it\n
            item can be a FSTAB_PATH or an entry '''
    
        entry = self._get_entry(item)
        if not entry in self.fstab :
            self.fstab.append(entry)
            self._apply()
        
    def unconfigure(self, item) :
        ''' x.unconfigure(item) -> remove item from filename and ask user if 
                                   he want to unmount it\n
            item can be a FSTAB_PATH or an entry '''
    
        entry = self._get_entry(item)
        if entry in self.fstab :
            self.fstab.remove(entry)
            self._apply()
            self._check_duplicate([entry])
                
    def _check_duplicate(self, entries) :
    
        for entry in entries[:] :
            if not entry.get_is_mounted() \
                    and len(self.search(entry["DEV"], keys = ["DEV"])) > 1 \
                    and not entry in self.fstab and entry in self :
                self.remove(entry)
                logging.debug("Removing unused duplicate : %s on %s" \
                        % (entry["DEVICE"], entry["FSTAB_PATH"]))
                if entry in self._current :
                    self._current.remove(entry)

    def _apply(self) :

        # Exit if we are in virtual or delay mode
        if self._mode == "virtual" or self._mode == "delay" :
            logging.debug("Mode %s enable, can't apply now" % self._mode)
            logging.debug("Expected changes :\n%s" % self._get_state())
            return
            
        # inhibit the watch so we don't trigger it while we apply the change
        self._event.emit("internal_changed_prepare")
        logging.debug("Applying changes")
        
        # Detect changes via an ugly hack that track global & local change
        change = change_tracker(self._current, self)
        change2 = change_tracker(self._current.fstab, self.fstab)
        for i in range(len(change)) :
            [ change[i].append(k) for k in change2[i] if k not in change[i] ]
        
        # Apply the change now
        self.simple_apply()
        
        # Create path if needed
        self.fstab.make_all_path()
        
        # Mount entry added and changed
        to_mount = change[1]
        logging.debug("-> Device to mount :\n-> |%s" % \
            "\n-> |".join([ "%s on %s" % (k["DEVICE"], k["FSTAB_PATH"]) for k in to_mount ]))
                
        # Unmount entry changed, and ask user for non duplicate disabled entry
        to_umount = change[2]
        disabled = change[3][:]
        if disabled :
            dial = dialog("warning", _("Unmounting disabled devices?"), \
                [_("You disabled the following devices:"), _("Do you want to unmount them?")], \
                [ "%s on %s" % (k["DEV"], k["FSTAB_PATH"]) for k in change[3] ], \
                _("Unmount selected"), parent = self._parent)
            if not dial[0] == Gtk.ResponseType.REJECT :
                for i in dial[1][0] :
                    to_umount.append(change[3][i])
        logging.debug("-> Device to umount :\n-> |%s" % \
            "\n-> |".join([ "%s on %s" % (k["DEVICE"], k["FSTAB_PATH"]) for k in to_umount ]))

        # Require reboot if some device couldn't be unmounted
        if self._mounter.do(mount = to_mount, umount = to_umount)[0] :
            dialog("warning", _("Reboot required"), \
                _("In order to apply all your changes,\n"\
                "you'll need to reboot your computer."), parent = self._parent)
                
        # Remove duplicate that are no more needed
        self._check_duplicate(to_umount)
                    
        # Delete uneeded path created here
        clean_all_path()
                
        # If state changed between the apply and here, do a simple_apply.
        # This can be due to a FSTAB_PATH change during the mounting
        if not self._current == self or not self._current.fstab == self.fstab :
            logging.debug("Detected changement.")
            self.simple_apply()
            
        # Emit internal_changed event
        self._event.emit("internal_changed")
                    
    def apply_now(self) :
        ''' x.apply_now() -> apply the changes now\n
            should only be used in virtual or delay mode, since
            all changes are automaticly apply in real mode '''
    
        if self._current == self and self._current.fstab == self.fstab :
            logging.debug("Nothing to apply")
            return
        mode = self._mode
        self._mode = "real"
        self._apply()
        self._mode = mode
        if self._mode == "virtual" :
            self._real = self._copy()
            
    def simple_apply(self) :
        ''' x.simple_apply() -> apply change without mounting/unmounting/emmtting event \n '''
        
        logging.debug("Updating current situation:\n%s" % self._get_state())
        # Apply the change now
        MntFile.apply(self.fstab)
        
        # Update current situtation
        self._current = self.copy()
        self._current.fstab = self.fstab.copy()
        
    def _get_state(self, obj = None) :
    
        if not obj :
            obj = self
        change1 = change_tracker(obj._current, obj)[0]
        change2 = change_tracker(obj._current.fstab, obj.fstab)[0]
        return "-> Global change :\n-> |%s\n-> Local change :\n-> |%s" % \
                ("\n-> |".join([ k.replace("\n","") for k in change1 ]), \
                "\n-> |".join([ k.replace("\n","") for k in change2 ]))

    def _initlog(self) :
    
        if self.logchanges :
            self._logconf = configparser.RawConfigParser()
            self._logconf.read(FSTAB_LOG)
            if not self._logconf.has_section("Original") :
                logging.debug("Creating original backup")
                self._logconf.add_section("Original")
                self._logconf.set("Original","log","Original configuration")
                fstabfile = open(self.filename)
                self._logconf.set("Original","fstab",fstabfile.read())
                fstabfile.close()
                self._logfile = open(FSTAB_LOG, "w")
                self._logconf.write(self._logfile)
                self._logfile.close()        
                     
    def savelog(self, name = None) :
        ''' x.savelog(name) -> save log as name\n
            name is default to the date of saving '''
    
        if not name :
            name = str(time.time())
        if self.logchanges and self.lastsave_has_changed() :
            logging.debug("Save log as %s", name)
            self._logconf.add_section(name)
            log = change_tracker(self._lastsave, self.fstab)[0]
            self._logconf.set(name, "log", "\n".join(log).strip())
            self._logconf.set(name, "fstab", self.fstab.write())
            self._logfile = open(FSTAB_LOG, "w+")
            self._logconf.write(self._logfile)
            self._logfile.close()
            self._lastsave = self.fstab.copy()
            self._initlog()
            
    def get_listlog(self) :
        ''' x.listlog() -> list all the current log recorded '''
    
        return self._logconf.sections()
        
    def get_logcommit(self, name) :
        ''' x.get_commit(name) -> get commit of log name '''
        
        return self._logconf.get(name, "log")
        
    def get_logfile(self, name) :
        ''' x.get_logfile(name) -> get file of log name '''
        
        return self._logconf.get(name, "fstab")
        
    def get_changes_current_from(self, name) :
        ''' x.get_changes_current_from(name) -> get log of change between name version -> current version '''
    
        tmpfile = tempfile.NamedTemporaryFile('w+t')
        tmpfile.write(self._logconf.get(name, "fstab"))
        tmpfile.seek(0)
        previous = MntFile(self.filename, tmpfile)
        return change_tracker(previous, self.fstab)[0]
        
    def get_changes_current_to(self, name) :
        ''' x.get_changes_current_to(name) -> get log of change between current version -> name version '''
    
        tmpfile = tempfile.NamedTemporaryFile('w+t')
        tmpfile.write(self._logconf.get(name, "fstab"))
        tmpfile.seek(0)
        previous = MntFile(self.filename, tmpfile)
        return change_tracker(self.fstab, previous)[0]
            
    def revert_to(self, name) :
        ''' x.revert_to(name) -> revert to version name '''
    
        logging.debug("Revert version to %s", name)
        tmpfile = tempfile.NamedTemporaryFile('w+t')
        tmpfile.write(self._logconf.get(name, "fstab"))
        tmpfile.seek(0)
        self._rebuild_object(MntFile(self.filename, tmpfile))
        self._apply()
        
    def cleanlog(self) :
        ''' x.cleanlog() -> clean the log and save current filename as Original '''
    
        self._logfile = open(FSTAB_LOG, "w")
        self._logfile.close()
        self._initlog()

    def get_changes_current_to_original(self) :
    
        return change_tracker(self.fstab, self._original)[0]
        
    def undo(self) :
        ''' x.undo() -> undo all changes made since the creation of the object '''

        self._rebuild_object(self._original.copy())
        self._apply()
        
    def original_has_changed(self) :
        ''' x.original_has_changed() -> return True if the original version at
                                        the creation of the object has changed '''
    
        return bool(not self.fstab == self._original)
        
    def lastsave_has_changed(self) :
        ''' x.original_has_changed() -> return True if the version has changed since
                                        last savelog '''

        return bool(not self.fstab == self._lastsave)

