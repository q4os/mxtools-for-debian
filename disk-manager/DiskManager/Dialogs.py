# -*- coding: UTF-8 -*-
#
#  Dialogs.py : Various dialogs for DiskManager
#  Copyright (C) 2007 Mertens Florent <flomertens@gmail.com>
#  Updated 2021 for MX Linux Project by team member Nite Coder
#  Maintenance of project assumed by MX Linux with permission from original author.
#
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

import re

import time
from xml.sax.saxutils import escape as escape_mkup

from gi.repository import Gtk
from gi.repository import GdkPixbuf, GObject, Pango

import gettext
#from gettext import gettext as _

from .SimpleGladeApp import SimpleGladeApp

from .config import *
from .Config  import *
from .Fstab.FstabHandler import *
from .Utility import *

import logging


class EditPartition(SimpleGladeApp) :
    ''' Class to manage the Edit/Add device dialog '''
        
    def __init__(self, disk, entry, parent = None) :
        
        SimpleGladeApp.__init__(self, GLADEFILE, "dialog_edit", domain = "disk-manager")
        self.dialog_edit.set_title("")
        if parent : 
            self.dialog_edit.set_transient_for(parent)

        self.disk = disk
        self.entry = entry
        self.disk.set_mode("virtual")
        
        self.default_button.connect("clicked", self.on_default_clicked)
        self.browser_button.connect("clicked", self.on_browser_clicked)
        self.driver_box.connect("changed", self.on_driver_changed)
        self.path.connect("activate", self.on_apply)
        self.options3.connect("activate", self.on_apply)
        self.apply_button2.connect("clicked", self.on_apply)
        self.dialog_edit.connect("delete-event", self.on_quit)
        self.cancel_button2.connect("clicked", self.on_quit)
        
        # Gtk.Tooltips().set_tip(self.default_button, _("Return options to defaults"))
        # Gtk.Tooltips().set_tip(self.browser_button, _("Select a directory"))
         
        # deduplicate mount options
        self.entry["FSTAB_OPTION"] = ','.join(dict.fromkeys(self.entry["FSTAB_OPTION"].split(',')))
        self.path.set_text(self.entry["FSTAB_PATH"])
        self.options3.set_text(self.entry["FSTAB_OPTION"])
        self.vbox_driver.hide()
        if "FS_DRIVERS" in self.entry :
            self.drivers = self.entry["FS_DRIVERS"]
        else :
            self.drivers = {"primary" : [], "all" : {}, "secondary" : []}
        drivers_list = self.drivers["primary"][:]
        driver = self.entry["FSTAB_TYPE"]
        if not driver in [ k[0] for k in drivers_list ] :
            if driver in self.drivers["all"] :
                drivers_list.append(self.drivers["all"][driver])
            elif driver == "ntfs3" :
                drivers_list.append([driver, "Read-write kernel driver", 0])
            else :
                drivers_list.append([driver, "Unknow driver", 0])
        if len(drivers_list) > 1 or not self.drivers["primary"] :
            self.vbox_driver.show()
            self.driver_box.clear()
            self.liststore = Gtk.ListStore(str)
            renderer = Gtk.CellRendererText()
            self.driver_box.set_model(self.liststore)
            self.driver_box.pack_start(renderer, True)
            self.driver_box.add_attribute(renderer, 'text', 0)
            current_driver_focus = 0
            for i in range(len(drivers_list)) :
                self.driver_box.append_text("%s (%s)" % \
                    (drivers_list[i][0], drivers_list[i][1]))
                if driver == drivers_list[i][0] :
                    current_driver_focus = i
            self.driver_box.set_active(current_driver_focus)
        self.set_driver()
        self.dialog_edit.set_focus(self.path)
        if entry.get_is_system() :
            self.path.set_sensitive(False)
            self.browser_button.set_sensitive(False)
        self.dialog_edit.show()
        
    def on_default_clicked(self, button) :
        ''' Load default options '''

        self.options3.set_text(self.entry.defaultopt())

    def set_driver(self) :
        fsk = 0
        if self.entry["FS_TYPE"] in ('ntfs', 'ntfs3', 'exfat') :
            self.driver_warning_box.hide()
            self.driver_error_box.hide()
        if self.drivers["primary"] :
            if self.entry["FSTAB_TYPE"] in self.drivers["all"] :
                self.driver_warning_box.hide()
                fsk = self.drivers["all"][self.entry["FSTAB_TYPE"]][2]
            else :
                if self.entry["FS_TYPE"] not in ('ntfs', 'ntfs3', 'exfat') :
                    self.driver_warning_box.show()
                    fsk = 0
        else :
            self.driver_box.set_sensitive(False)
            self.driver_error_box.show()
            self.driver_error_label.set_label( \
               _("No driver is available for this type of filesystem : '%s'") \
               % self.entry["FS_TYPE"])
            fsk = 0
        self.check_button.set_active(bool(int(self.entry["FSTAB_PASO"])))
        self.check_button.set_sensitive(bool(self.check_button.get_active() or fsk))

    def on_browser_clicked(self, button) :
    
        """  not used:
        gladexml =  Gtk.glade.XML(GLADEFILE, "dialog_mount_point")
        dialog = gladexml.get_widget("dialog_mount_point")

        builder = Gtk.Builder()
        builder.add_from_file(GLADEFILE)

        dialog = builder.get_object("dialog_mount_point")

        dialog.set_transient_for(self.dialog_edit)
        dialog.select_filename(self.path.get_text())
        for shortcut in (get_user("dir"), "/media", "/") :
            try :
                dialog.add_shortcut_folder(shortcut)
            except gobject.GError : 
                pass
        ret = dialog.run()
        print(ret)
        print(dialog.get_filename())
        
        if ret == 1 :
            self.path.set_text(dialog.get_filename())
        dialog.hide()

        dialog = Gtk.FileChooserDialog(title="Please choose a folder", action=Gtk.FileChooserAction.SELECT_FOLDER)
        """
        
        dialog = Gtk.FileChooserDialog(title=_("Select a directory"), action=Gtk.FileChooserAction.SELECT_FOLDER)

        gtk_select = "_Select"
        
        dialog.add_buttons(
            Gtk.STOCK_CANCEL, 
            Gtk.ResponseType.CANCEL, 
            gettext.translation('gtk30').gettext(gtk_select), 
            Gtk.ResponseType.OK
        )
       #dialog.add_buttons(
       #     Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, "Select", Gtk.ResponseType.OK
       # )

        logging.debug("Mountpoint: " + self.path.get_text())
        user_mp_dir = os.path.dirname(os.path.realpath(self.path.get_text()))

        default_mp_path = "/media"
        if os.path.isdir(user_mp_dir):
             mp_path = user_mp_dir
        else:
            mp_path = default_mp_path
        
        dialog.set_current_folder(mp_path)
        dialog.set_default_size(800, 400)

        response = dialog.run()
        if response == Gtk.ResponseType.OK:
            print("Select clicked")
            print("Folder selected: " + dialog.get_filename())
            self.path.set_text(dialog.get_filename())
        elif response == Gtk.ResponseType.CANCEL:
            print("Cancel clicked")

        dialog.destroy()

    def on_driver_changed(self, button) :
    
        current = self.driver_box.get_active_text().split()[0]
        if self.entry["DEVICE"] and not self.entry["FSTAB_TYPE"] == current :
            self.disk.set(self.entry, type=current)
            # deduplicate mount options
            self.entry["FSTAB_OPTION"] = ','.join(dict.fromkeys(self.entry["FSTAB_OPTION"].split(',')))

            self.options3.set_text(self.entry["FSTAB_OPTION"])
            self.set_driver()

    def on_apply(self, widget) :
    
        if not self.check_new_options() and not self.check_new_path() :
            self.disk.set(self.entry, path=self.path.get_text(), option=self.options3.get_text(), \
                paso=self.check_button.get_active())
            self.disk.apply_now()
            self.on_quit(widget)
    
    def on_quit(self, widget, event = None) :
    
        self.dialog_edit.hide()
        self.disk.set_previous_mode()
        
    def check_new_path(self) :
        ''' Check that the path is correct. Need to success to the check_path() test. '''
        
        path = self.path.get_text()
        if len(path) == 0 :
            self.path.set_text(self.entry["FSTAB_PATH"])
            return 1
        if not path[0] == "/" :
            path = "/media/" + path
            self.path.set_text(path)
        if check_path(path, fstab = self.disk, entry = self.entry) :
            return 0
        dialog("warning", _("Wrong mount point"), \
            _("<i>%s</i> is currently in use.\n" \
            "You should try another one.") % escape_mkup(path), parent = self.dialog_edit)
        self.dialog_edit.set_focus(self.path)
        return 1

    def check_new_options(self) :
        ''' Check that options are of the type : jsjk,skks,skks '''
    
        options = self.options3.get_text()
        if re.search("^([a-zA-Z0-9=_.@/-]+,)*[a-zA-Z0-9=_.@/-]+$", options) :
            return 0     
        dialog("warning", _("Options formatting error"), \
                _("<i>%s</i> is not formatted correctly.\n" \
                "Options should be separated by comma (,)") % escape_mkup(options), \
                parent = self.dialog_edit)
        self.dialog_edit.set_focus(self.options3)
        return 1

""" 
# (not used)

class HistoryDialog(SimpleGladeApp) :
    ''' Class the manage the History dialog '''
             
    def __init__(self, disk, parent = None):
    
        SimpleGladeApp.__init__(self, GLADEFILE, "dialog_history", domain = "disk-manager")
        self.dialog_history.set_title("")
        if parent: 
            self.dialog_history.set_transient_for(parent)
        
        self.disk = disk
        
        self.apply_button.connect("clicked", self.on_apply_clicked)
        self.see_button.connect("clicked", self.on_see_clicked)
        self.clean_button.connect("clicked", self.on_clean_clicked)
        self.close_button.connect("clicked", self.on_quit)
        self.dialog_history.connect("delete-event", self.on_quit)
        
        renderer = Gtk.CellRendererText()
        column = Gtk.TreeViewColumn("historic", renderer, text=0, editable=1)
        self.treeview.append_column(column)
        
        self.tree_store = Gtk.ListStore(str, bool)
        self.treeview.set_model(self.tree_store)
        self.update_dialog()
        
    def update_dialog(self) :
        
        self.tree_store.clear()
        self.versions = self.disk.get_listlog()
        self.versions.sort()
        self.versions.reverse()
        if "Original" in self.versions :
            self.versions.remove("Original")
            self.versions.append("Original")
        for value in self.versions :
            if not value == "Original" : 
                value = time.strftime("%d %b %Y %H:%M:%S", time.localtime(float(value)))
            self.tree_store.append((value, False))
        self.treeview.connect("cursor_changed", self.on_cursor_changed)
        first = self.tree_store.get_path(self.tree_store.get_iter_first())
        self.treeview.set_cursor(first)
              
    def on_cursor_changed(self, treeview) :
    
        self.apply_button.set_sensitive(False)
        version = self.versions[self.treeview.get_cursor()[0][0]]
        changes = self.disk.get_changes_current_to(version)
        buf = Gtk.TextBuffer()
        tag = buf.create_tag(weight=Pango.Weight.BOLD)
        buf.insert_with_tags(buf.get_start_iter(), "Commit:\n", tag)
        buf.insert_at_cursor(self.disk.get_logcommit(version))
        if changes :
            iter = buf.get_end_iter()
            buf.insert_with_tags(iter, "\nChanges between this version and current one:\n", tag)
            buf.insert_at_cursor("\n".join(changes))
            self.apply_button.set_sensitive(True)
        self.textview.set_buffer(buf)
        
    def on_apply_clicked(self, button) :
    
        version = self.versions[self.treeview.get_cursor()[0][0]]
        ret = dialog("question", _("Reverting to an older version?"), \
            [_("This will apply the following changes:"), _("Do you want to continue?")],
            "\n".join(self.disk.get_changes_current_to(version)), parent = self.dialog_history)
        if ret[0] == Gtk.ResponseType.YES :
            self.disk.revert_to(version)
        self.on_cursor_changed(self.treeview)
        
    def on_see_clicked(self, button) :
    
        version = self.versions[self.treeview.get_cursor()[0][0]]
        gladexml =  Gtk.glade.XML(GLADEFILE, "dialog_fstab")
        dialog = gladexml.get_widget("dialog_fstab")
        dialog.set_title("")
        dialog.set_transient_for(self.dialog_history)
        textview = gladexml.get_widget("textview_fstab")
        buf = Gtk.TextBuffer()
        if not version == "Original" :
            buf.set_text(FstabData.header)
        buf.insert_at_cursor(self.disk.get_logfile(version))
        textview.set_buffer(buf)
        dialog.run()
        dialog.hide()
        
    def on_clean_clicked(self, button) :
    
        ret = dialog("question", _("Emptying History?"), \
            _("Do you want to empty the History?"), parent = self.dialog_history)
        if ret[0] == Gtk.ResponseType.YES :
            self.disk.cleanlog()
            self.update_dialog()
            
    def on_quit(self, button, event = None) :
    
        self.dialog_history.hide()
"""
        
        
class InfoDialog(SimpleGladeApp) :

    def __init__(self, entry, disk, parent = None) :

        SimpleGladeApp.__init__(self, GLADEFILE, "dialog_info", domain = "disk-manager")
        if parent :
            self.dialog_info.set_transient_for(parent)
        self.entry = entry
        self.disk = disk
        self.infobuttonclose.connect("clicked", self.on_quit)

    def update_dial(self) :
        gtk_information = "Information"
        self.dialog_info.set_title(self.entry["DEV"] + " " + gettext.translation('gtk30').gettext(gtk_information))
        self.size_label.set_label(size_renderer(self.entry.get_size()))
        if self.entry.get_is_mounted() :
            self.free_label.set_label(size_renderer(self.entry.get_free_size()))
            self.avail_label.set_label(size_renderer(self.entry.get_available_size()))
            self.used_label.set_label(size_renderer(self.entry.get_used_size()))
        else :
            self.free_label.set_label(_("Not available"))
            self.avail_label.set_label(_("Not available"))
            self.used_label.set_label(_("Not available"))
        self.path_label.set_label(self.entry["FSTAB_PATH"])
        self.opt_label.set_label(escape_mkup(self.entry["FSTAB_OPTION"]))
        self.dev_label.set_label(self.entry["DEVICE"])
        self.label_label.set_label(self.disk.get_attribute(self.entry, "FS_LABEL"))
        self.uuid_label.set_label(self.disk.get_attribute(self.entry, "FS_UUID"))

        if self.disk.get_attribute(self.entry, "FS_VERSION") in "None":
            self.type_label.set_label("%s" % \
                    (self.disk.get_attribute(self.entry, "FS_TYPE")))
        else:
            self.type_label.set_label("%s (%s)" % \
                    (self.disk.get_attribute(self.entry, "FS_TYPE"), \
                    self.disk.get_attribute(self.entry, "FS_VERSION")))

        if self.disk.get_attribute(self.entry, "VENDOR") not in "None":
            self.vendor_label.set_label(self.disk.get_attribute(self.entry, "VENDOR"))

        if self.disk.get_attribute(self.entry, "MODEL") not in "None":
            self.model_label.set_label(self.disk.get_attribute(self.entry, "MODEL"))

        if self.disk.get_attribute(self.entry, "SERIAL") not in "None":
            self.serial_label.set_label(self.disk.get_attribute(self.entry, "SERIAL"))

        if self.disk.get_attribute(self.entry, "BUS") not in "None":
            self.bus_label.set_label(self.disk.get_attribute(self.entry, "BUS"))
        
    def on_quit(self, button, event = None) :
    
        self.dialog_info.hide()

class SanityCheck :

    def __init__(self, parent = None, conf = None) :
    
        self.parent = parent
        self.fstab = MntFile(FSTAB)
        self.conf = conf
        if not self.conf :
            self.conf = Config()
        
    def unknow(self) :

        unknow_block = []
        known = self.conf.get("Detected Device", "known").split()
        for entry in self.fstab.other :
            if re.search("(^UUID=|^LABEL=|^/dev/)", entry["FSTAB_NAME"]) \
                and entry["FSTAB_TYPE"] not in FstabData.ignore_fs \
                and entry["FSTAB_NAME"] not in FstabData.ignore_dev \
                and re.search("^/", entry["FSTAB_PATH"]) \
                and not entry["FSTAB_NAME"] in known :
                unknow_block.append(entry)
        if unknow_block :
            dial = dialog("warning", _("Removing unknown devices from\nthe configuration file?"), \
                [_("I can not find any existing block devices\n"
                "corresponding to the following devices:"), \
                _("It is advisable to remove them in order to avoid failed mount\n"
                "at start-up. Select the ones you want to remove.")], \
                [ "%s on %s" % (k["FSTAB_NAME"], k["FSTAB_PATH"]) for k in unknow_block ], \
                _("Remove selected"), [_("Don't show me this warning in the future.")], self.parent)
            if not dial[0] ==  Gtk.ResponseType.REJECT :
                for i in dial[1][0] :
                    logging.debug("Removing unknow entry : %s" % unknow_block[i]["FSTAB_NAME"])
                    self.fstab.other.remove(unknow_block[i])
                if dial[2][0] :
                    for i in dial[1][1] :
                        logging.debug("Always keep entry : %s" % unknow_block[i]["FSTAB_NAME"])
                        known.append(unknow_block[i]["FSTAB_NAME"])
                    self.conf.set("Detected Device", "known", " ".join(known))
                self.fstab.apply()      

    def duplicate(self) :

        checked = []
        for path in self.fstab.list(col = "FSTAB_PATH") :
            index = self.fstab.search(path)
            index.reverse()
            for i in index[:] :
                if self.fstab.count(self.fstab[i]) > 1 :
                    logging.warning("Removing exact duplicate entry : %s on %s" % \
                        (self.fstab[i]["DEVICE"], self.fstab[i]["FSTAB_PATH"]))
                    self.fstab.remove(i)
                    self.fstab.apply()
            index = self.fstab.search(path)
            index.reverse()
            if len(index) > 1 and not path in checked :
                entries = [ "%s on %s (%s)" % (self.fstab[k]["FSTAB_NAME"], \
                    self.fstab[k]["FSTAB_PATH"], self.fstab[k]["FSTAB_TYPE"]) for k in index ]
                data = []
                for i in range(len(index)) :
                    if self.fstab[index[i]]["FSTAB_PATH"] in device_is_mounted(self.fstab[index[i]]["DEVICE"]) :
                        data.append([entries[i], True])
                    else :
                        data.append([entries[i], False])
                dial = dialog("warning", _("Duplicate entry detected"), \
                [_("The following entries of your configuration file use the same mount point:"), \
                _("Two different devices can not share the same mount point.\n"
                "You should select the one you want, and the others will be commented.\n"
                "They will be kept in the configuration file, but will be disabled.")], \
                data, _("Keep selected"), parent = self.parent)
                if not dial[0] ==  Gtk.ResponseType.REJECT :
                    for j in dial[1][1] :
                        i = index[j]
                        logging.debug("Commenting duplicate : %s" % entries[j])
                        entry = ["#%s" % self.fstab[i]["FSTAB_NAME"]]
                        entry.extend([ self.fstab[i][u] for u in FstabData.categorie[1:] ])
                        self.fstab.remove(i)
                        self.fstab.comment.append(EntryBase(entry))
                    self.fstab.apply()
            checked.append(path)
        
        
def about_dialog(button, parent) :

    dial = Gtk.AboutDialog()
    dial.set_name(_("Disk Manager"))
    dial.set_version(VERSION)
    dial.set_copyright("Copyright © 2007 Mertens Florent \n 2021 MX Linux")
    dial.set_license( \
        "This program is free software; you can redistribute it and/or\n"
        "modify it under the terms of the GNU General Public License\n"
        "as published by the Free Software Foundation; either\n"
        "version 2 of the License, or (at your option) any later version.\n"
        "\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
        "See the GNU General Public License for more details.\n"
        "\n"
        "You should have received a copy of the GNU General Public License\n"
        "along with this library; if not, write to the Free Software\n"
        "Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA")
    dial.set_comments(_("Easily manage your filesystem configuration"))
    dial.set_website(HOMEPAGE)
    dial.set_website_label("https://github.com/MX-Linux/disk-manager")
    dial.set_authors(AUTHORS)
    dial.set_translator_credits(_("translator-credits"))
    dial.set_logo_icon_name("disk-manager")
    dial.run()
    dial.hide()
          
