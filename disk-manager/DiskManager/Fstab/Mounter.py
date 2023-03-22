# -*- coding: UTF-8 -*-
#
#  Mounter.py : A GTK mounter for FstabHandler
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
from xml.sax.saxutils import escape as escape_mkup

from .SimpleGladeApp import SimpleGladeApp
#from gettext import gettext as _

from .Fstabconfig import *
from .FstabDialogs import *
from .Fstab import *
from .FstabUtility import *


class Mounter(SimpleGladeApp) :
    ''' Show a progress bar while mounting a list of entry '''

    def __init__(self, disk, parent = None) :

        self.disk = disk
        self.parent = parent
        self.top_level = None
        self.init_gui()

    def do(self, mount = [], umount = []) :
        ''' x.do([Entries], [Entries]) -> umount the second list, and mount the first one\n
            mounted entry in mount= and unmounted entry in umount= will be ignored.
            Return code is (mount result, umount result) '''

        if not mount and not umount :
            return (0, 0)
        (res_umount, res_mount) = (0, 0)
        self.total = len(mount + umount)
        if not umount and len(mount) == 1 :
            label = _("Mounting partition")
        elif not mount and len(umount) == 1 :
            label = _("Unmounting partition")
        else :
            label = _("Applying changes")
        self.set_gui(label)

        if umount:
            for entry in umount :
                self.safepath = escape_mkup(entry["FSTAB_PATH"])
                label = _("Unmounting %s") % self.safepath
                self.update_gui("<i>%s</i>" % label, umount.index(entry))
                if entry.get_is_mounted() :
                    res_umount += self.umount_device(entry)
                    if res_umount and entry["FSTAB_PATH"] not in self.disk.list("FSTAB_PATH") :
                        self.disk.append(entry)
            self.update_gui("", len(umount))

        if mount and umount :
            time.sleep(0.01)

        if mount :
            for entry in mount :
                self.safepath = escape_mkup(entry["FSTAB_PATH"])
                label = _("Mounting %s") % self.safepath
                self.update_gui("<i>%s</i>" % label, mount.index(entry) + len(umount))
                if not entry.get_is_mounted() :
                    res_mount += self.mount_device(entry)
            self.update_gui("", self.total)

        self.hide_gui()
        return (res_umount, res_mount)

    def umount_device(self, entry) :

        self.lazy = False
        keep_going = True
        while keep_going :
            keep_going = False
            ret = entry.umount(self.lazy)
            if ret[0] :
                keep_going = self.handle_umount_error(entry, ret)
                self.restore_gui()
        return ret[0]

    def mount_device(self, entry) :

        keep_going = True
        while keep_going :
            keep_going = False
            ret = entry.mount()
            if ret[0] :
                keep_going = self.handle_mount_error(entry, ret)
                self.restore_gui()
        return ret[0]

    def handle_umount_error(self, entry, error) :

        type = "warning"
        title = _("Unmounting failed")
        text = _("Unmounting <i>%s</i> failed because of the following error:") % self.safepath
        data = str(error[1]).strip()
        options = None
        action = None
        # TRANSLATORS:
        # Keep the English term 'lazy' for the 'lazy' unmount option
        lazy_string = "\n%s" % _("You can also use the 'lazy' options to detach the filesystem now.")

        self.used_file = get_used_file(entry["FSTAB_PATH"])
        if not self.lazy :
            if self.used_file :
                text = [_("Unmounting <i>%s</i> failed, because\n"
                    "it is currently used by the following applications:") % self.safepath,
                    _("Close all applications that use it and retry to unmount.")]
                data = [ self.used_file, False]
            else :
                text = [text, _("The filesystem might be temporarily busy. Wait few seconds and retry.")]
            if not entry.get_is_system() :
                text[1] += lazy_string
                # TRANSLATORS:
                # Keep the English term 'lazy' for the 'lazy' unmount option
                options = [_("Unmount with the 'lazy' option")]
                action = _("Retry unmount")
        else :
            type = "error"
            # TRANSLATORS:
            # Keep the English term 'lazy' for the 'lazy' unmount option
            text = [text, _("Even with the lazy option, unmounting failed.")]

        ret = dialog(type, title, text, data, action, options, self.top_level)

        if ret[0] == Gtk.ResponseType.REJECT :
            return False
        if ret[2][0] :
            self.lazy = True
        return True

    def handle_mount_error(self, entry, error) :

        title = _("Mounting failed")
        text = _("Mounting <i>%s</i> failed because of the following error:") % self.safepath
        data = str(error[1]).strip()
        options = None
        action = _("Retry mount")
        err_code = None
        if not data.find("dmesg") == -1 :
            dmesg = get_dmesg_output()
            logging.debug("[dmesg]: " + dmesg)
            last_dmesg_log = dmesg.split("\n")[-1].lower()
            data += "\n\ndmesg | tail:\n" + dmesg
        else :
            last_dmesg_log = ""

        err = ("unrecognized mount option", "unknown mount option")
        if err[0] in last_dmesg_log or err[1] in last_dmesg_log :
            try :
                b = re.search("(%s|%s) (\S+)" % err, last_dmesg_log).groups()[1]
                for opt in [b, b[:-1], b[1:-1], b[1:-2]] :
                    if entry.hasopt(opt) :
                        bad_opt = opt
                        err_code = "BADOPT"
                        break
            except AttributeError :
                pass
        elif entry["FSTAB_TYPE"] in ("ntfs-3g", 'ntfs-fuse') and "force" in error[1] \
                and not entry.hasopt("force") :
            err_code = "NTFSUNCLEAN"
        elif "FS_DRIVERS" not in entry :
            err_code = "NODRIVER"
        elif not entry["FS_DRIVERS"]["primary"] :
            err_code = "NODRIVER"
        elif entry["FSTAB_TYPE"] not in list(entry["FS_DRIVERS"]["all"].keys()) or len(data) == 0 :
            err_code = "BADTYPE"

        retry_string = _("You can try one of the following actions:")

        if err_code == "BADOPT" :
            s = _("It seems that option '%s' is not allowed.") % bad_opt
            text = [text, "%s\n%s" % (s, retry_string)]
            options = ([_("Return options to defaults")], True)
            if not entry["FSTAB_OPTION"] == bad_opt :
                options[0].append(_("Don't use the '%s' option") % bad_opt)
        elif err_code == "BADTYPE" :
            s = _("It seems that fs type '%s' is not valid.") % entry["FSTAB_TYPE"]
            text = [text, "%s\n%s" % (s, retry_string)]
            options = []
            for driver in entry["FS_DRIVERS"]["primary"] :
                options.append(_("Using driver '%s' (%s)") % (driver[0], driver[1]))
            options = [ options, True ]
        elif err_code == "NODRIVER" :
            text = [text, _("No driver is available for this type of filesystem : '%s'") % entry["FS_TYPE"]]
            action = None
        elif err_code == "NTFSUNCLEAN" :
            # TRANSLATORS:
            # Keep the English term 'force' for the 'force' unmount option
            text = [text, _("You can try to use the 'force' option. Be aware that this could be risky.")]
            options = [_("Use the 'force' option")]
        else :
            action = None

        ret = dialog("error", title, text, data, action, options, self.top_level)

        if not ret[0] == Gtk.ResponseType.REJECT and err_code :
            if err_code == "BADOPT" :
                if 0 in ret[2][0] :
                    entry["FSTAB_OPTION"] = entry.defaultopt()
                    self.disk.simple_apply()
                elif 1 in ret[2][0] :
                    entry.removeopt(bad_opt)
                    self.disk.simple_apply()
            if err_code == "BADTYPE" :
                if ret[2][0] :
                    entry["FSTAB_TYPE"] = entry["FS_DRIVERS"]["primary"][ret[2][0][0]][0]
                    self.disk.simple_apply()
            if err_code == "NTFSUNCLEAN" :
                if ret[2][0] :
                    entry.addopt("force")
                    self.disk.simple_apply()
            return True
        return False

    def init_gui(self) :

        SimpleGladeApp.__init__(self, GLADEFILE, "window_mounting_progress", domain = PACKAGE)
        self.window_mounting_progress.set_title("")
        if self.parent :
            self.window_mounting_progress.set_transient_for(self.parent)
        self.top_level = self.window_mounting_progress

    def set_gui(self, label) :

        self.progressbar.set_fraction(0.0)
        self.title2.set_label("<big><b>%s</b></big>" % label)
        self.progress_label.set_label("")
        self.window_mounting_progress.show_now()
        while Gtk.events_pending() :
            Gtk.main_iteration()

    def hide_gui(self) :

        time.sleep(0.1)
        self.window_mounting_progress.hide()

    def update_gui(self, text, step) :

        if step > 0:
            self.progressbar.set_fraction(step/float(self.total))
        if text != "":
            self.progress_label.set_label(text)
        while Gtk.events_pending() :
            Gtk.main_iteration()

    def restore_gui(self) :

        while Gtk.events_pending() :
            Gtk.main_iteration()
        time.sleep(0.05)
        while Gtk.events_pending() :
            Gtk.main_iteration()

