#! /usr/bin/python3
# -*- coding: utf-8 -*-
import subprocess
import sys
import os
from os import environ
import shutil
import re

class ViewAndUpgrade:
    """
    view and upgrade dialog
    """
    def __init__(self):
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

        from aptnotifier_rc import AptNotifierRC
        apt_notifier_rc = AptNotifierRC()
        
        self.__tokens = {}
        cmd = ['xdotool', 'getdisplaygeometry']
        res = run(cmd, capture_output=True, text=True).stdout
        w,h = res.strip().split()
        if int(w) >= 1600:
            w = 1600
            h =  900
        self.__tokens['width'] = str(int(w) * 3 // 5 )
        self.__tokens['height'] = str(int(h) * 2 // 3 )
        self.__tokens['icon'] = conf.config['window_icon']
        self.__tokens['upgrade_type'] = apt_notifier_rc.upgrade_type
        self.__tokens['reload_tooltip'] = apt_notifier_rc.upgrade_type

        self.upgrade_auto_close = apt_notifier_rc.upgrade_auto_close
        self.upgrade_assume_yes = apt_notifier_rc.upgrade_assume_yes

        self.__tokens['reload_button'] = xlate.get('reload')
        self.__tokens['reload_tooltip'] = xlate.get('reload_tooltip')
        self.__tokens['use_apt_get_dash_dash_yes'] = xlate.get('use_apt_get_dash_dash_yes')
        self.__tokens['auto_close_window'] = xlate.get('auto_close_window')
        self.__tokens['close'] = xlate.get('close')

        self.__tokens['upgrade_label'] = xlate.get('upgrade_label')
        if self.__tokens['upgrade_type'] == 'upgrade':
            self.__tokens['upgrade_tooltip'] = xlate.get('upgrade_tooltip_basic')
        else:
            self.__tokens['upgrade_tooltip'] = xlate.get('upgrade_tooltip_full')

        if self.__tokens['upgrade_type'] == 'upgrade':
            self.__tokens['title'] = xlate.get('window_title_basic')
        else:
            self.__tokens['title'] = xlate.get('window_title_full')

        if self.__tokens['upgrade_type'] == 'upgrade':
            self.__tokens['upgrade'] = xlate.get('basic_upgrade')
        else:
            self.__tokens['upgrade'] = xlate.get('full_upgrade')

        self.__apt_list = self.apt_list_run
        

    @property
    def apt_list(self):
        return self.__apt_list

    @property
    def apt_list_run(self):
        from subprocess import run, PIPE
        self.__apt_list = run(['/usr/lib/apt-notifier/bin/updater_list'], 
                                capture_output=True, text=True).stdout
        return self.__apt_list

    @property
    def upgrade(self):
        return self.__tokens['upgrade']

    @property
    def window_title(self):
        return self.__tokens['title']
    @property
    def window_width(self):
        return self.__tokens['width']
    @property
    def window_height(self):
        return self.__tokens['height']
    @property
    def window_icon(self):
        return self.__tokens['icon']
        
    @property
    def upgrade_auto_close(self):
        x = self.__tokens['upgrade_auto_close']
        if str(x).lower() == 'true':
            self.__tokens['upgrade_auto_close'] = 'true'
        else:
            self.__tokens['upgrade_auto_close'] = 'false'
        return self.__tokens['upgrade_auto_close'] 
        
    @upgrade_auto_close.setter
    def upgrade_auto_close(self,x):
        if str(x).lower() == 'true':
            self.__tokens['upgrade_auto_close'] = 'true'
        else:
            self.__tokens['upgrade_auto_close'] = 'false'
        return self.__tokens['upgrade_auto_close'] 
        
    @property
    def upgrade_assume_yes(self):
        x = self.__tokens['upgrade_assume_yes']
        if str(x).lower() == 'true':
            self.__tokens['upgrade_assume_yes'] = 'true'
        else:
            self.__tokens['upgrade_assume_yes'] = 'false'
        return self.__tokens['upgrade_assume_yes']

    @upgrade_assume_yes.setter
    def upgrade_assume_yes(self,x):
        if str(x).lower() == 'true':
            self.__tokens['upgrade_assume_yes'] = 'true'
        else:
            self.__tokens['upgrade_assume_yes'] = 'false'
        return self.__tokens['upgrade_assume_yes']


    @property
    def yad_returncode(self):
        return self.__yad_returncode
    @property
    def yad_stdout(self):
        return self.__yad_stdout

    @property
    def yad_stderr(self):
        return self.__yad_stderr
        
    @property
    def yad(self):
        from subprocess import run

        upgrade_assume_yes = self.upgrade_assume_yes
        upgrade_auto_close = self.upgrade_auto_close
        spaces = '  '
        text = spaces + self.upgrade + '\n' + self.apt_list
        text = text.replace('\n','\n' + spaces)
        text = text.replace('\n','\\n') + '\n'
        text = text.replace('\n','\\n') + '\n'
        text += upgrade_assume_yes + '\n'
        text += upgrade_auto_close + '\n'

        # remove default item separator '!' within translated strings if any
        self.__tokens = { k:v.replace('!','') for k,v in self.__tokens.items() }

        yad="""
        /usr/bin/yad
            --window-icon={icon}
            --class=apt-notifier
            --width={width}
            --height={height}
            --center
            --title={title}
            --form
            --field=:TXT
            --field={use_apt_get_dash_dash_yes}:CHK 
            --field={auto_close_window}:CHK 
            --button={reload_button}!gtk-refresh!{reload_tooltip}:8
            --button={upgrade_label}!{icon}!{upgrade_tooltip}:0
            --button={close}!gtk-close:2
            --buttons-layout=spread
            --margins=7
            --borders=5
            --escape-ok 
            --response=2
        """

        yad = yad.strip()    
        yad = yad.splitlines()    
        yad = [ x.strip().format(**self.__tokens) for x in yad ]

        yad = subprocess.run(yad, capture_output=True, text=True, input=text)
        self.__yad_stdout = yad.stdout
        self.__yad_stderr = yad.stderr
        self.__yad_returncode = yad.returncode

        yes, close = self.yad_stdout.split('|')[1:3]
        self.upgrade_assume_yes  = yes.lower()
        self.upgrade_auto_close  = close.lower()
        return yad.stdout

def __run():

    from subprocess import run, PIPE
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

    vau = ViewAndUpgrade()
    
    while True:    
        vau.yad
        if vau.yad_returncode not in [ 0, 8]:
            break
        else:
            apt_notifier_rc.upgrade_assume_yes = vau.upgrade_assume_yes
            apt_notifier_rc.upgrade_auto_close = vau.upgrade_auto_close
            apt_notifier_rc.update

        if vau.yad_returncode == 0:
            run('/usr/lib/apt-notifier/bin/updater_upgrade_run')  
            break
        
        if vau.yad_returncode == 8:
            run('/usr/lib/apt-notifier/bin/updater_reload_run')
            vau.apt_list_run  
        
# General application code
def main():
    __run()

if __name__ == '__main__':
    main()

