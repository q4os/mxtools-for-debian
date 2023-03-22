#! /usr/bin/python3
# -*- coding: utf-8 -*-
import subprocess
from subprocess import run, PIPE
import getpass
from string import whitespace
import sys

MODULES = "/usr/lib/apt-notifier/modules"
if MODULES not in sys.path:
    sys.path.append(MODULES)

import os
import os.path
import pwd
from glob import glob
import configparser
from configparser import ConfigParser
from pathlib import Path


class AptNotifierConfig:
    """
    Access apt-notifier config settings
    """
    def __init__(self):
        self.__conf_files = """
        /usr/share/apt-notifier/conf/apt-notifier.conf
        /usr/share/apt-notifier/conf/apt-notifier.conf.d/*.conf
        /etc/apt-notifier/apt-notifier.conf
        /etc/apt-notifier/apt-notifier.conf.d/*.conf
        /usr/local/share/apt-notifier/conf/apt-notifier.conf
        /usr/local/share/apt-notifier/conf/apt-notifier.conf.d/*.conf
        """

        logname = run([ "/usr/bin/logname" ], capture_output=True, text=True).stdout.strip()
        logid   = run([ "/usr/bin/id", "-u", logname ], capture_output=True, text=True).stdout.strip()
        home = pwd.getpwuid(int(logid))[5]
        self.__home = home

        self.__user_conf_file = {}
        for dist in [ "antiX", "MX-Linux" ]:
            #self.__conf_files += f"{home}/.config/{dist}/apt-notifier.conf\n"
            self.__user_conf_file[dist] = f"{home}/.config/{dist}/apt-notifier.conf"

        self.__config_defaults="""
#---------------------------------
# default section (build-in)
#---------------------------------
[DEFAULT]
#
# Build-in default values:
# Default values listed here may be overloaded within the distribution
# specific sections below.
# distribution domain
domain                  = MX
# tray icons
classic_some            = /usr/share/icons/mnotify-some-classic.png
classic_some            = /usr/share/icons/mnotify-some-classic.png
classic_none            = /usr/share/icons/mnotify-none-classic.png
pulse_some              = /usr/share/icons/mnotify-some-pulse.png
pulse_none              = /usr/share/icons/mnotify-none-pulse.png
wireframe_some          = /usr/share/icons/mnotify-some-wireframe.png
wireframe_none_dark     = /usr/share/icons/mnotify-none-wireframe-dark.png
wireframe_none_light    = /usr/share/icons/mnotify-none-wireframe-light.png
wireframe_none_dark_transparent  = /usr/share/icons/mnotify-none-wireframe-dark-transparent.png
wireframe_none_light_transparent = /usr/share/icons/mnotify-none-wireframe-light-transparent.png
use_wireframe_transparent = true

# window icons
window_icon             = /usr/share/icons/apt-notifier.png
window_icon_name        = apt-notifier
about_box_icon          = /usr/share/icons/hicolor/96x96/apps/apt-notifier.png
about_window_icon       = /usr/share/icons/hicolor/scalable/apt-notifier.svg
window_icon_kde         = /usr/share/icons/hicolor/scalable/apt-notifier.svg
notify_icon             = /usr/share/icons/hicolor/scalable/apt-notifier.svg
# license file
license_file            = /usr/share/doc/apt-notifier/license.html
# about
about_box_url           = http://mxlinux.org
about_copyright         = Copyright (c) MX Linux

# changelog
changelog_file          = /usr/share/doc/apt-notifier/changelog.gz
changelog_url           = http://mxrepo.com/mx/repo/pool/main/a/apt-notifier/current.{debian_codename}

# apt-notifier help
apt_notifier_help_url       = https://mxlinux.org/wiki/help-files/help-mx-apt-notifier

# lang specific help-url's for apt_notifier_help
apt_notifier_help_url_fr    = https://mxlinux.org/wiki/help-files/help-mx-apt-notifier-notificateur-dapt

# first package manger found within the list will be shown
package_manager_preference_list = synaptic, muon

# synaptic
show_synaptic           = true
show_synaptic_help      = true
synaptic_help_url       = https://mxlinux.org/wiki/help-files/help-synaptic

# lang specific help-url's for synaptic_help_url
# default url-pattern:  synaptic_help_url + '-xx', with xx lower-case lang-code
synaptic_help_url_it    = https://mxlinux.org/wiki/help-files/help-synaptic-it
synaptic_help_url_ru    = https://mxlinux.org/wiki/help-files/help-synaptic-ru

# muon help
show_muon           = true
# currently muon help not available
show_muon_help      = false
muon_help_url       =
# lang specific help-url's for muon_help_url
# default pattern  muon_help_url + '-xx'
# can be specfied here similar to synaptic_help_url_xx
# muon_help_url_xx =

# viewer list searched before x-www-browser, gnome-www-browser and xdg-open
apt_notifier_viewer_list    = mx-viewer, antix-viewer

# root/user terminal policy
reload_in_root_terminal                     = false
upgrade_in_root_terminal                    = false
allow_passwordless_reload_in_user_terminal  = true

# auto close timeouts
reload_auto_close_timeout    = 6
upgrade_auto_close_timeout   = 10

# check for updates:
# check_for_updates_interval in seconds: min. 15 max. 21600 (= 6h )
check_for_updates_interval        = 15
# forced check even if packages cache has not changed, disable 0, max 60
# defaults to 20, which is 20 * 15s = 5min
check_for_updates_force_counter   = 20

# desktop notifications by dbus not by qt-applet
use_dbus_notifications             = true
# let user switch between qt-notifcation and dbus-notifications
show_switch_desktop_notifications  = true
# options
show_apt_notifier_help             = true
show_changelog                     = true
show_package_manager               = true
show_package-manager-help          = true
show_apt_notifier_help             = true
display_domain_in_menu_and_title   = true

# use nala disabled by default
use_nala  = false

#---------------------------------
# MX Linux section
#---------------------------------
[MX]
domain                  = MX

#---------------------------------
# antiX Linux section
#---------------------------------
[antiX]
domain                   = antiX

"""
        self.__config_file_list = []
        for x in self.__conf_files.split():
            if os.path.isfile(x):
                self.__config_file_list.append(x)
            else:
                g = glob(x)
                g.sort()
                self.__config_file_list.extend(g)

        self.__config_domain = self.config_domain()
        self.__config = self.config_parser()

        global apt
        try: apt
        except NameError:
            from aptnotifier_apt import Apt
            apt = Apt()
        debian_codename = apt.debian_codename
        changelog_url = self.__config['changelog_url']

        self.__config['changelog_url'] = changelog_url.format( debian_codename = debian_codename )

    def config_domain(self):
        if os.path.isfile('/etc/mx-version'):
            self.__config_domain='MX'
            self.__config_dist='MX-Linux'
        elif os.path.isfile('/etc/antix-version'):
            self.__config_domain='antiX'
            self.__config_dist='antiX'
        elif os.path.isfile('/etc/lsb-release'):
            try:
                with open('/etc/lsb-release', 'r') as f:
                    lines  = f.read().splitlines()
                    line   = list(map(lambda x: x.strip(), filter(lambda x: 'DISTRIB_ID=' in x, lines)))
                    self.__config_domain = list(map(lambda x: (x.split('=')), line))[0][1]
                    self.__config_dist = self.__config_domain
            except:
                self.__config_domain = 'DEFAULT'
        else:
            self.__config_domain = 'DEFAULT'
        return self.__config_domain

    def config_parser(self):
        conpar = configparser.ConfigParser(strict=False)
        conpar.read_string(self.__config_defaults)
        default = dict(list((k.replace('-','_'), v) for (k,v) in conpar['DEFAULT'].items()))
        default = dict(list((k.lower(), v) for (k,v) in default.items()))

        try:
            conpar.read(self.__config_file_list)
            conpar.read_string(self.__config_defaults)
        except Exception as e:
            print(e, file = sys.stderr)

        home = self.__home
        dist = self.__config_dist
        domain = self.__config_domain

        user_config_file = f"{home}/.config/{dist}/apt-notifier.conf"
        user_config = f"[{domain}]\n"
        if os.path.exists(user_config_file):
            try:
                with open(user_config_file, 'r') as f:
                    user_config += f.read()
            except:
                pass

        try:
            conpar.read_string(user_config)
        except Exception as e:
            print(f"Error: In user config file '{user_config_file}'")
            print(f">>>: Config items ignored!")
            print(e, file = sys.stderr)
            pass

        try:
            res = dict(list((k.replace('-','_'), v) for (k,v) in conpar[self.__config_domain].items()))
        except:
            res = default

        #res = dict(list((k.lower(), v) for (k,v) in res.items()))
        #res = { k.lower():res[k] for k in res.keys() }


        single_quote = "'"
        double_quote = '"'
        strip_chars = f"{whitespace}{single_quote}{double_quote}"
        dollar_sign = "$"
        back_quote = "`"

        # sanity check
        #res = { k.lower():res[k].split('#')[0].strip(strip_chars).replace(dollar_sign,'').replace(back_quote,'').replace(double_quote,'').replace(single_quote,'') for k in res.keys() }
        res = { k.lower():res[k].split('#')[0].strip(strip_chars).replace(back_quote,'').replace(double_quote,'').replace(single_quote,'') for k in res.keys() }

        res = dict(filter( lambda i: i[0] in default.keys(), res.items()))
        for k,v in res.items():
            rv = v
            if v.startswith("$HOME"):
                rv = v.replace("$HOME/", f"{home}/")
            elif v.startswith("~/"):
                rv = v.replace("~/", f"{home}/")
            rv = rv.replace(dollar_sign, "")
            if v.lower() in ['true', 'yes']:
                rv = True
            elif v.lower() in ['false', 'no']:
                rv = False
            res[k] = rv

        self.__config = res
        return self.__config

    @property
    def config(self):
        return  self.__config

    def get(self,x):
        if x in self.__config.keys():
            return self.__config[x]
        return ""

def debug_p(text=''):
    """
    simple debug print helper
    msg get printed to stderr
    """
    import os, sys
    debug_me = ''
    try:
        debug_me = os.getenv('DEBUG_APT_NOTIFIER')
    except:
        debug_me = False
    if debug_me:
        print("Debug: " + text, file = sys.stderr)


def __run_config():

    conf = AptNotifierConfig()
    print(conf.config)
    import json
    #print(json.dumps(conf.config, indent=4))
    print("(")
    list(map(lambda x: print(f'["{x[0]}"]="{x[1]}"'), sorted(list(conf.config.items()))))
    print(")")
    print("{")
    list(map(lambda x: print(f'    "{x[0]}" : "{x[1]}",'), sorted(list(conf.config.items()))))
    print("}")
    print("conf.get('show_apt_notifier_help')")
    print(conf.get('show_apt_notifier_help'))
    print("conf.config['show_apt_notifier_help']")
    print(conf.config['show_apt_notifier_help'])
    """
    "window_icon"           : "/usr/share/icons/apt-notifier.png",
    "window_icon_kde"       : "/usr/share/icons/hicolor/scalable/apt-notifier.svg",
    "window_icon_name"      : "apt-notifier",
    "wireframe_some"        : "/usr/share/icons/mnotify-some-wireframe.png",
    "wireframe_none_dark"   : "/usr/share/icons/mnotify-none-wireframe-dark.png",
    "wireframe_none_light"  : "/usr/share/icons/mnotify-none-wireframe-light.png",
    "wireframe_none_light_transparent" : "/usr/share/icons/mnotify-none-wireframe-light-transparent.png",
    "wireframe_none_dark_transparent" : "/usr/share/icons/mnotify-none-wireframe-dark-transparent.png",
    """
    print(f"window_icon                        : {conf.get('window_icon')}")
    print(f"window_icon_kde                    : {conf.get('window_icon_kde')}")
    print(f"window_icon_name                   : {conf.get('window_icon_name')}")
    print(f"wireframe_some                     : {conf.get('wireframe_some')}")
    print(f"wireframe_none_dark                : {conf.get('wireframe_none_dark')}")
    print(f"wireframe_none_light               : {conf.get('wireframe_none_light')}")
    print(f"wireframe_none_light_transparent   : {conf.get('wireframe_none_light_transparent')}")
    print(f"wireframe_none_dark_transparent    : {conf.get('wireframe_none_dark_transparent')}")


# main
def main():

    __run_config()

if __name__ == '__main__':
    main()
