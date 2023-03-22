# -*- coding: UTF-8 -*-
#
#  EventHandler.py : Execute actions on events detection
#  Copyright (C) 2007 Mertens Florent <flomertens@gmail.com>
#  Updated 2021 for MX Linux Project by team member Nite Coder
#  Maintenance of project assumed by MX Linux with permission from original author.

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
import logging
import threading

from .Fstabconfig import *
from .Fstab import *
from .FstabError import *

class EventHandler :

    def __init__(self, disk, enable_watch) :

        self.disk = disk
        self.enable_watch = enable_watch
        self.filename = self.disk.filename
        self.inhibit =  False
        self.start_watch()
        
    def start_watch(self) :

        if not self.enable_watch :
            return
        logging.debug("Starting Watch...")
        self.th_watch = threading.Thread(target=self.start_watch_thread, name="WatchThread")
        self.th_watch.setDaemon(True)
        self.th_watch.start()
                
    def start_watch_thread(self) :
    
        self.alive = True
        self.mtime_mtab = os.stat(MTAB).st_mtime
        self.mtime_fstab = os.stat(self.filename).st_mtime
        self.size = [ str(k.get_free_size())[:4] for k in self.disk ]
        while self.alive :
            time.sleep(0.5)
            if not self.inhibit and not self.mtime_fstab == os.stat(self.filename).st_mtime :
                logging.debug("FSTAB update detected")
                self.emit("external_fstab_changed")
            if not self.inhibit and not self.mtime_mtab == os.stat(MTAB).st_mtime :
                logging.debug("MTAB update detected")
                self.emit("external_mtab_changed")  
            if not self.inhibit and not self.size == [ str(k.get_free_size())[:4] for k in self.disk ] :
                logging.debug("Size update detected")
                self.emit("size_changed")
                
    def stop_watch(self) :

        if hasattr(self, "th_watch") and self.th_watch.is_alive() :
            self.alive = False
            logging.debug("Stopping Watch...")
            self.th_watch.join()

    def shutdown(self) :
        
        t = time.time()
        self.stop_watch()
        logging.debug("EventHandler shutdown in %s s", time.time() -t)
        
    def emit(self, event) :
    
        if not hasattr(self, "on_%s" % event) :
            raise UnknowEvent(self, event)
        t = time.time()
        logging.debug("Emitting '%s' event" % event)
        if getattr(self, "on_%s" % event)() :
            if hasattr(self, "%s_fct" % event) :
                fct = getattr(self, "%s_fct" % event)
                if callable(fct[0]) :
                    logging.debug("-> Call %s%s for event '%s'" % (fct[0].__name__, fct[1], event))
                    fct[0](*fct[1])
            else :
                logging.debug("-> No function define for event '%s'" % event)
        logging.debug("Event '%s' managed in %s s", event, time.time() -t)

    def on_external_fstab_changed(self) :
    
        fstab = MntFile(self.filename)
        if self.disk.fstab[:] == fstab[:] :
            self.mtime_fstab = os.stat(self.filename).st_mtime
            logging.debug("In fact, no real changes.")
            return False
        self.disk._rebuild_object(fstab)
        self.disk._current = self.disk.copy()
        self.disk._current.fstab = self.disk.fstab.copy()
        self.emit("external_changed")
        self.mtime_fstab = os.stat(self.filename).st_mtime
        
    def on_external_mtab_changed(self) :
    
        mtab = MntFile(MTAB, minimal = True)
        self.disk._build_object(self.disk.fstab)
        self.disk._current = self.disk.copy()
        self.disk._current.fstab = self.disk.fstab.copy()
        self.emit("external_changed")
        self.mtime_mtab = os.stat(MTAB).st_mtime
        self.size = [ str(k.get_free_size())[:4] for k in self.disk ]
        
    def on_internal_changed_prepare(self) :

        logging.debug("Inhibit watch for internal change...")
        self.inhibit = True
        
    def on_internal_changed_cancel(self) :

        logging.debug("Restart watch...")
        self.inhibit = False
        
    def on_internal_changed(self) :
    
        self.emit("configuration_changed")
        self.mtime_mtab = os.stat(MTAB).st_mtime
        self.mtime_fstab = os.stat(self.filename).st_mtime
        self.size = [ str(k.get_free_size())[:4] for k in self.disk ]
        logging.debug("Restart watch...")
        self.inhibit = False
        
    def on_external_changed(self) :
    
        self.emit("configuration_changed")
        
    def on_configuration_changed(self) :
    
        self.emit("any_changed")
    
    def on_size_changed(self) :
        
        self.emit("any_changed")
        self.size = [ str(k.get_free_size())[:4] for k in self.disk ]
            
    def on_any_changed(self) :
    
        return True

