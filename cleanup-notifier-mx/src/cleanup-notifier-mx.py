#!/usr/bin/python3

# cleanup-notifier-mx : script to check available space at /boot
#
# fehlix@mxlinux.org, 2022
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License with
# the Debian GNU/Linux distribution in file /usr/share/common-licenses/GPL;
# if not, write to the Free Software Foundation, Inc., 51 Franklin St,
# Fifth Floor, Boston, MA 02110-1301, USA.


import gettext
import gi
import notify2
import os
import pwd
import signal
import sys
import time
from gi.repository import GLib
from subprocess import Popen, run

#-----------------------------------------------------------------------
# some globals
#-----------------------------------------------------------------------
NOTIFICATION_TIMEOUT   = 16       # notification timeout in seconds
MINIMUM_SPACE_LEFT     = 160      # minimum space left in MiB on /boot 
CLEANUP_CHECK_LAST_RUN = 180      # time in seconds we won't run to check

#-----------------------------------------------------------------------
NOTIFICATION_NAME    = "cleanup-notifier-mx"
NOTIFICATION_ICON    = "mx-cleanup"
NOTIFICATION_ICON    = "cleanup-notifier-mx"
NOTIFICATION_ICON_FALLBACK = "dialog-warning"

CLEANUP_DISMISS = "cleanup-dismiss.chk"
CLEANUP_TOOL = "mx-cleanup"
CLEANUP_RUN  = "/usr/bin/mx-cleanup-launcher"
CLEANUP_TIMESTAMP = "cleanup-notifier-timestamp"

#-----------------------------------------------------------------------
HOME   = os.environ['HOME']
UID    = os.getuid()
UNAME  = pwd.getpwuid(UID)[0]

#-----------------------------------------------------------------------
# L10N - localization
domain = "cleanup-notifier-mx"
localedir =  "/usr/share/locale"
_ = gettext.translation(domain, localedir=localedir, fallback=True ).gettext

# TRANSLATORS:
# A message is displayed to warn the user that there is not enough space on the boot partition.
NOTIFICATION_TITLE   = _("Warning")
# TRANSLATORS:
# A message is displayed to warn the user that there is not enough space on the boot partition.
NOTIFICATION_TEXT    = _("Not enough space on %s partition").replace("%s", "/boot")
# TRANSLATORS:
# The user can click the "Dismiss" button so that the warning will not be displayed in the future.
ACTION_DISMISS_TEXT  = _("Dismiss")
# TRANSLATORS:
# The user can click the "Clean Up" button to start the cleanup program.
ACTION_CLEANUP_TEXT  = _("Clean Up")


#-----------------------------------------------------------------------
# cleanup dismiss handling
#
if os.path.isdir(f"{HOME}/.config/MX-Linux"):
    CLEANUP_DISMISS_CHECK = f"{HOME}/.config/MX-Linux/{CLEANUP_DISMISS}"
else:
    CLEANUP_DISMISS_CHECK = f"{HOME}/.config/{CLEANUP_DISMISS}"

def create_cleanup_dismiss_check():
    with open(CLEANUP_DISMISS_CHECK, "w") as cc:
        pass
    return

def remove_cleanup_dismiss_check():
    for chk in [ f"{HOME}/.config/MX-Linux/{CLEANUP_DISMISS}", 
                 f"{HOME}/.config/{CLEANUP_DISMISS}" ]:
        if os.path.exists(chk):
            os.remove(chk)
    return

#-----------------------------------------------------------------------
def available_space_at_boot():
    cmd = "/usr/bin/df --block-size=M --output=avail /boot"
    avail= run(cmd.split(), text=True, capture_output=True).stdout.split("\n")[-2].strip(' M')
    print(f"Cleanup-notifier: available space at '/boot' {avail} MiB")
    return int(avail)

def live_boot():
    cmd = "/usr/bin/df --output=fstype /"
    live = run(cmd.split(), text=True, capture_output=True).stdout.split("\n")[-2]
    if "overlay" in live:
        return True
    else:
        return False

#-----------------------------------------------------------------------
# life boot check 
#
if live_boot():
    print(f"Cleanup-notifier: no check on live boot")
    sys.exit(0)    
    
#-----------------------------------------------------------------------
# check available space or check dismissed by user
# 
if available_space_at_boot() >= MINIMUM_SPACE_LEFT:
    remove_cleanup_dismiss_check()
    sys.exit(0)    
elif os.path.exists(CLEANUP_DISMISS_CHECK):
    print(f"Cleanup-notifier: Cleanup dismiss check found. exit[0]")
    sys.exit(0)    

#-----------------------------------------------------------------------
# runtime check: don't if lastime run is less than 3 minutes
try: 
    runtime_dir = os.environ['XDG_RUNTIME_DIR']
except KeyError:
    runtime_dir = f"/run/user/{UID}"

if os.path.isdir(runtime_dir):
    RUNTIME_CHECK = f"{runtime_dir}/{CLEANUP_TIMESTAMP}.run"
else:
    RUNTIME_CHECK = f"/tmp/{CLEANUP_TIMESTAMP}-{UNAME}.run"
    
if os.path.exists(RUNTIME_CHECK):
    with open(RUNTIME_CHECK, "r") as f:
        dba = f.read()
        if os.getenv("DBUS_SESSION_BUS_ADDRESS", "None") in dba:
            now = int(time.time())
            last = os.path.getmtime(RUNTIME_CHECK)
            if now - last < CLEANUP_CHECK_LAST_RUN:
                print(f"Cleanup-notifier: Last run less then {CLEANUP_CHECK_LAST_RUN} seconds; exit(1)")
                sys.exit(1)

# set runtime check  
with open(RUNTIME_CHECK, "w") as f:
    f.write(os.getenv("DBUS_SESSION_BUS_ADDRESS", "None"))

#-----------------------------------------------------------------------
# check cleanup tool is available
def cleanup_tool_is_installed():
    cmd = "dpkg-query -f ${db:Status-Abbrev} -W " + f"{CLEANUP_TOOL}"
    x = run(cmd.split(), text=True, capture_output=True).stdout.strip()
    if "ii" in x:
        return True
    else:
        return False
   
#-----------------------------------------------------------------------
# notification handling
notification = None

# callbacks
def closed_cb(n):
    #print(f"Notification {n.id} closed" )
    global mainloop
    global action
    action = "closed"
    mainloop.quit()
    n.close()

def cleanup_cb(n, a):
    assert a == "Cleanup"
    global action
    action = a
    pid = Popen([CLEANUP_RUN]).pid
    mainloop.quit()
    n.close()

def dismiss_cb(n, a):
    assert a == "Dismiss"
    global action
    action = a
    create_cleanup_dismiss_check()
    mainloop.quit()
    n.close()

def notification_icon():
    if cleanup_tool_is_installed():
        icon = NOTIFICATION_ICON
    else:
        icon = NOTIFICATION_ICON_FALLBACK
    return icon
    
#-----------------------------------------------------------------------
# fix python3-notify2 KeyError on closed dbus-connections
#
def try_closed_callback(nid, reason):
    nid, reason = int(nid), int(reason)
    try:
        n = notify2.notifications_registry[nid]
        n._closed_callback(n)
        del notify2.notifications_registry[nid]
    except KeyError:
        pass

notify2._closed_callback = try_closed_callback

#-----------------------------------------------------------------------
# signal handling
#
def terminate(sig, frame):
    try:
        strsig = f" '{signal.strsignal(sig)}' "
    except:
        strsig = " "
    
    print (f"Cleanup-notifier: Received signal[{sig}]{strsig}- closing notification")
    global mainloop
    global notification
    mainloop.quit()
    notification.close()
    sys.exit(sig)

#-----------------------------------------------------------------------
def main():
    global mainloop
    global notification

    mainloop = GLib.MainLoop()
    action = "none"
    notify2.init(NOTIFICATION_NAME, "glib")
    notification = notify2.Notification(NOTIFICATION_TITLE, NOTIFICATION_TEXT, notification_icon())
    notification.connect('closed', closed_cb)
    notification.timeout = NOTIFICATION_TIMEOUT * 1000
    
    if ('actions' in notify2.get_server_caps()):
        notification.add_action("Dismiss", ACTION_DISMISS_TEXT, dismiss_cb)
        if cleanup_tool_is_installed():
            notification.add_action("Cleanup", ACTION_CLEANUP_TEXT, cleanup_cb)
    
    notification.show()
    
    # try capture keyboard interupt
    try:
        mainloop.run()
    except KeyError:
        notification.close()
    except KeyboardInterrupt:
        notification.close()
    notify2.uninit()

if __name__ == "__main__":
    
    # capture some sinals and close notifications gracefully
    signal.signal(signal.SIGHUP, terminate)
    signal.signal(signal.SIGINT, terminate)
    signal.signal(signal.SIGBUS, terminate)
    signal.signal(signal.SIGQUIT, terminate)
    signal.signal(signal.SIGTERM, terminate)
    signal.signal(signal.SIGUSR1, terminate)
    signal.signal(signal.SIGUSR2, terminate)
    sys.exit(main())    
