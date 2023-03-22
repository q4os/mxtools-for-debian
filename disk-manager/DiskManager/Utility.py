# -*- coding: UTF-8 -*-
#
#  Utility.py : Various utility
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
import pwd
import logging
from subprocess import *
from urllib.parse import quote

#from gettext import gettext as _

from .config import *

pref_browser = { "GNOME" : { "file" : [["gnome-open"], ["nautilus", "--no-desktop"]], \
                             "http" : [["gnome-open"], ["epiphany"], ["firefox"], ["mozilla"]] }, \
                 "KDE"   : { "file" : [["kfmclient", "exec"], ["konqueror"], ["dolphin"]], \
                             "http" : [["kfmclient", "exec"], ["konqueror"], ["opera"]] }, \
                 "XFCE"  : { "file" : [["exo-open"], ["thunar"], ["nautilus", "--no-desktop"]], \
                             "http" : [["exo-open"], ["firefox"], ["mozilla"], ["epiphany"]]}}

def size_renderer(size) :
    ''' Return a string that show the size in a human readable way '''

    if not size : 
        return "0"
    for unit in [_("KB"), _("MB"), _("GB"), _("TB") ] :
        size = float(size)/1024.
        if size/1024. < 1 :
            return str(round(size, 1)) + " " + unit
    return str(round(size, 1)) + " " + unit

def get_user(request = "name") :
    ''' Get user information. Request could be name, uid, gid, dir, shell, ... '''

    user = pwd.getpwnam(os.getlogin())

    if user.pw_uid == "0" :
        logging.warning("Can't find your real username. Some features will not work.")
    return getattr(user, "pw_" + request)

def open_url(url) :
    ''' Select a browser depending on the requested url, and on the desktop currently used.
        We also do some ugly tweaks to be able to execute this browser as non root '''

    if url[:7] == "http://" :
        url = "http://%s" % quote(url.split("http://")[1])
    elif url[:7] == "file://" :
        url = "file://%s" % quote(url.split("file://")[1])
    else :
        url = "file://%s" % quote(url)
    user = get_user()
    desktop = get_desktop(default = "GNOME")
    browsers= pref_browser[desktop][url[:4]]
    [ browsers.extend(k) for k in [ k[url[:4]] for k in list(pref_browser.values()) ] ]
    browser = ""
    for test in browsers :
        if test_cmd(test[0]) :
            browser = " ".join(test)
            break
    if not browser:
        browser = "xdg-open"
        
    logging.debug("Launching %s as user %s with %s" % (url, user, browser))

    if not browser or not user or user == "root" :
        logging.debug("-> Not possible")
        return False
    for key in ["HOME", "USER", "XAUTHORITY", "LOGNAME", "MAIL", "XDG_RUNTIME_DIR"] :
        if key in os.environ :
            setattr(open_url, "current_%s" % key, os.environ[key])
    os.putenv("USER", user)
    os.putenv("LOGNAME", user)
    os.putenv("HOME", get_user("dir"))
    os.putenv("MAIL", "/var/mail/%s" % user)
    os.putenv("XDG_RUNTIME_DIR", "/run/user/%s" % str(get_user("uid")))
    if "%s_user-XAUTHORITY" % PACKAGE in os.environ :
        os.putenv("XAUTHORITY", os.environ["%s_user-XAUTHORITY" % PACKAGE])

    cmd = "runuser -u %s %s  '%s'" % (user, browser, url)

    logging.debug("Command : %s" % cmd)
    #run(cmd, capture_output=False)
    Popen(cmd, close_fds=True, shell=True)

    for key in ["HOME", "USER", "XAUTHORITY", "LOGNAME", "MAIL", "XDG_RUNTIME_DIR"] :
        if hasattr(open_url, "current_%s" % key) :
            os.putenv(key, getattr(open_url, "current_%s" % key))
    return True

def test_cmd(cmd) :

    return not call(["which " + cmd], stdout=PIPE, stderr=PIPE, \
                        close_fds=True, shell=True)
    
def pidof(proc_name) :

    return not call(["pidof " + proc_name], stdout=PIPE, stderr=PIPE, \
                        close_fds=True, shell=True)

def get_desktop(default = None) :
    ''' Get current running desktop. Search first for an environment key, otherwise for 
        a process. This way we are not fool by people running a gnome-panel in a kde 
        session for example, but we still can detect a desktop if keys are unset by a su
        helper (conseolheper for example). '''

    if "KDE_FULL_SESSION" in os.environ or "KDE_MULTIHEAD" in os.environ:
        desktop = "KDE"
    elif "GNOME_DESKTOP_SESSION_ID" in os.environ :
        desktop = "GNOME"
    elif pidof("startkde") or pidof("kicker") :
        desktop = "KDE"
    elif pidof("gnome-session") or pidof("gnome-panel") :
        desktop = "GNOME"
    elif pidof("xfce4-session") or pidof("xfce-mcs-manager") :
        desktop = "XFCE"
    else :
        logging.debug("No desktop environement found. Using default one : %s" % default)
        desktop = default
    logging.debug("Detected %s session" % desktop)
    return desktop   

