#! /usr/bin/python3
# -*- coding: utf-8 -*-
import subprocess
from subprocess import run, PIPE
import sys
import os
from os import environ
import shutil

class UnattendedUpgrade:
    """
    Check and set unattended upgrade
    """
    def __init__(self):
        self.__unattended_upgrade = self.detect_unattended_upgrade()
        self.__tokens = {}

        #---------------------------------------------------------------
        from subprocess import run, PIPE
        global xlate
        try: xlate
        except NameError:
            from aptnotifier_xlate import AptNotifierXlate
            xlate = AptNotifierXlate()

        global conf
        try: conf
        except NameError:
            from aptnotifier_config import AptNotifierConfig
            conf = AptNotifierConfig()

        global apt_notifier_rc
        try: apt_notifier_rc
        except NameError:
            from aptnotifier_rc import AptNotifierRC
            apt_notifier_rc = AptNotifierRC()

        cmd = ['xdotool', 'getdisplaygeometry']
        res = run(cmd, capture_output=True, text=True).stdout
        w,h = res.strip().split()
        self.__tokens['width']  = str(int(w) * 3 // 4 )
        self.__tokens['height'] = str(int(h) * 2 // 3 )
        self.__tokens['window_icon'] = conf.config['window_icon']

    def read_apt_config(self, config, default):
        run = subprocess.run(['apt-config', 'shell', 'opt', config ], capture_output=True, text=True)
        res = run.stdout.strip()
        if(not res):
            res = default
        else:
            res = res.split('=',1)[1].strip("'\"")
        if res == 'true':
            res = True
        elif res == 'false':
            res = False
        return(res)

    def detect_unattended_upgrade(self):
        """
        check unattended upgrade state
        """
        self.__unattended_upgrade = self.read_apt_config('APT::Periodic::Unattended-Upgrade/b', False)
        return self.__unattended_upgrade

    def disable_unattended_upgrade(self):
        if self.detect_unattended_upgrade():
            print("Disabling unattended upgrade")
            run = '/usr/lib/apt-notifier/actions/auto-update-disable'
            run = subprocess.run(run)
            self.__unattended_upgrade = self.detect_unattended_upgrade()
        return self.__unattended_upgrade

    def enable_unattended_upgrade(self):
        if not self.detect_unattended_upgrade():
            print("Enabling unattended upgrade")
            run = '/usr/lib/apt-notifier/actions/auto-update-enable'
            run = subprocess.run(run)
            self.__unattended_upgrade = self.detect_unattended_upgrade()
        return self.__unattended_upgrade

    @property
    def unattended_upgrade(self):
        """
        get unattended upgrade state
        """
        return self.__unattended_upgrade

    @unattended_upgrade.setter
    def unattended_upgrade(self, x):
        """
        set unattended upgrade
        """
        if x == True or x == 'true':
            self.__unattended_upgrade = self.enable_unattended_upgrade()
        else:
            self.__unattended_upgrade =  self.disable_unattended_upgrade()
        return self.__unattended_upgrade

    @property
    def autoupdate(self):
        """
        get unattended upgrade state
        """
        return self.__unattended_upgrade

    @autoupdate.setter
    def autoupdate(self, x):
        """
        set unattended upgrade
        """
        if x == True or x == 'true':
            self.__unattended_upgrade = self.enable_unattended_upgrade()
        else:
            self.__unattended_upgrade =  self.disable_unattended_upgrade()
        return self.__unattended_upgrade

    #---------------------------------------------------------------

    def view_unattended_upgrades_logs(self):
        from subprocess import run, PIPE, Popen

        window_icon   = conf.config['window_icon']
        self.__tokens['title'] = xlate.get('title_unattended-upgrades_log_viewer')
        self.__tokens['close'] = xlate.get('close')
        yad ="""
        /usr/bin/yad 
            --window-icon={window_icon}
            --class=apt-notifier
            --width={width}
            --height={height}
            --center
            --title={title}
            --text-info
            --fontname=mono
            --button={close}!gtk-close
            --margins=7
            --borders=5
        """
        yad = yad.strip()
        yad = yad.splitlines()
        yad = [ x.strip().format(**self.__tokens) for x in yad ]

        cmd  = "apt-notifier_unattended_upgrades_log_view"
        pipe1 = Popen(cmd, stdout=PIPE, text=True)
        pipe2 = Popen(yad, stdin=pipe1.stdout, stdout=PIPE, stderr=PIPE, text=True)
        pipe1.stdout.close()  # if pipe2 exits before pipe1, send SIGPIPE to pipe1 to close
        pipe2.communicate()[0]


    def view_unattended_upgrades_dpkg_logs(self):
        from subprocess import run, PIPE, Popen
        window_icon   = conf.config['window_icon']
        self.__tokens['title'] = xlate.get('title_unattended-upgrades_dpkg_log_viewer')
        self.__tokens['close'] = xlate.get('close')
        yad ="""
        /usr/bin/yad 
            --window-icon={window_icon}
            --class=apt-notifier
            --width={width}
            --height={height}
            --center
            --title={title}
            --text-info
            --fontname=mono
            --button={close}!gtk-close
            --margins=7
            --borders=5
        """
        yad = yad.strip()
        yad = yad.splitlines()
        yad = [ x.strip().format(**self.__tokens) for x in yad ]

        cmd  = "apt-notifier_unattended_upgrades_dpkg_log_view"
        pipe1 = Popen(cmd, stdout=PIPE, text=True)
        pipe2 = Popen(yad, stdin=pipe1.stdout, stdout=PIPE, stderr=PIPE, text=True)
        pipe1.stdout.close()  # if pipe2 exits before pipe1, send SIGPIPE to pipe1 to close
        pipe2.communicate()[0]


def run_check_autoupgrade():

    uu = UnattendedUpgrade()

    """print("Check Unattended Upgrade:")
    print(uu.unattended_upgrade)
    print("Toggle Unattended Upgrade:")
    if uu.unattended_upgrade:
        uu.unattended_upgrade = False
    else:
        uu.unattended_upgrade = True
    print(uu.unattended_upgrade)
    """
    uu.view_unattended_upgrades_logs()
    uu.view_unattended_upgrades_dpkg_logs()
    
# General application code
def main():

    run_check_autoupgrade()

if __name__ == '__main__':
    main()

