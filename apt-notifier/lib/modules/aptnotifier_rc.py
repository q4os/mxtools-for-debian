#! /usr/bin/python3
# -*- coding: utf-8 -*-
import subprocess
from subprocess import run, PIPE
import sys
import os
from os import environ
import shutil


class AptNotifierRC:
    """
    Access apt-notifier settings
    """
    def __init__(self):
        self.__changed = False
        self.__dont_show = False
        self.__dont_show_line = "[DontShowIcon] #Remove this entry if you want the apt-notify icon to show even when there are no upgrades available"
        self.__rc_file = environ.get('HOME') + '/.config/apt-notifierrc'
        self.__default_settings = {
            'LeftClick': 'ViewAndUpgrade',
            'IconLook': 'wireframe-dark',
            'UpgradeType': 'dist-upgrade',
            'UpgradeAssumeYes': 'false',
            'UpgradeAutoClose': 'false',
            'WireframeTransparent': 'true'
            }

        '''
        # not used
          'CheckForAutoRemovables': 'false',
          'CheckForAutoRemovables': ['false', 'true'],
        '''

        self.__valid_settings = {
            'LeftClick':   ['ViewAndUpgrade', 'PackageManager'],
            'IconLook':    ['wireframe-dark', 'wireframe-light', 'classic', 'pulse' ],
            'UpgradeType': ['dist-upgrade', 'upgrade'],
            'UpgradeAssumeYes': ['false', 'true'],
            'UpgradeAutoClose': ['false', 'true'],
            'WireframeTransparent': ['false', 'true'],
            'UseDbusNotifications': ['false', 'true'],
            }

        self.__settings = {}
        self.read_settings()

    def read_settings(self):
        lines = []
        exists_rc = True
        update_rc = False
        default_settings = self.default_settings
        settings =  default_settings
        valid_settings =  self.__valid_settings
        debug_p(f"default_settings: {default_settings}")
        debug_p(f"valid_settings: {valid_settings}")
        debug_p(f"settings: {settings}")
        try:
            with open(self.rc_file, 'r') as f:
                apt_notifier_rc =  f.read()
                if '[DontShowIcon]' in apt_notifier_rc:
                    self.__dont_show = True
                    update_rc = True
                import string
                if any(w in apt_notifier_rc for w in string.whitespace):
                    update_rc = True
                lines = list(map(lambda x: x.strip(), filter(lambda x: '=' in x, apt_notifier_rc.splitlines())))
        except FileNotFoundError as e:
            exists_rc = False

        tuples = list(map(lambda x: (x.split('=',1)), lines))
        rc_settings  = {k.strip():v.strip() for (k,v) in tuples}

        for k in valid_settings.keys():
            if k in rc_settings.keys():
                if rc_settings[k] in valid_settings[k]:
                    settings[k] = rc_settings[k]
                else:
                    update_rc = True
            else:
                update_rc = True
        self.__settings = settings
        if not exists_rc or update_rc:
            self.write_settings()
        debug_p(f"settings: {settings}")
        debug_p(f"self.__settings: {self.__settings}")
        return self.__settings

    def read(self):
        lines = []
        exists_rc = True
        default_settings = self.default_settings
        settings =  default_settings
        valid_settings =  self.__valid_settings
        debug_p(f"default_settings: {default_settings}")
        debug_p(f"valid_settings: {valid_settings}")
        debug_p(f"settings: {settings}")
        with open(self.rc_file, 'r') as f:
                apt_notifier_rc =  f.read()
                lines = list(map(lambda x: x.strip(), filter(lambda x: '=' in x, apt_notifier_rc.splitlines())))

        tuples = list(map(lambda x: (x.split('=',1)), lines))
        rc_settings  = {k.strip():v.strip() for (k,v) in tuples}

        for k in valid_settings.keys():
            if k in rc_settings.keys():
                settings[k] = rc_settings[k]
        self.__settings = settings
        return self.__settings

    def write_settings(self):
        settings =  self.__settings
        debug_p(f"write settings: {settings}")
        outlist=list(map(lambda x: x + '=' + settings[x], sorted(settings)))
        if self.__dont_show:
            outlist = [ self.__dont_show_line ] + outlist
        out = '\n'.join(outlist)
        out += '\n'
        with open(self.rc_file, 'w') as f:
            f.write(out)
            f.flush()
        self.changed = False
        self.read()
        return out

    def write(self):
        return self.write_settings()

    def settings(self):
        return self.__settings

    @property
    def default_settings(self):
        return self.__default_settings

    @property
    def rc_file(self):
        return self.__rc_file

    @property
    def update(self):
        if self.changed:
            self.write()
            self.changed = False
            return True
        else:
            return False

    """
    #not used
    #@property
    #def check_for_auto_removables(self):
    #    return self.__settings['CheckForAutoRemovables']

    #@check_for_auto_removables.setter
    #def check_for_auto_removables(self,x):
    #    self.__settings['CheckForAutoRemovables'] = x
    #    return self.__settings['CheckForAutoRemovables']
    """

    def get(self,x):
        try: y = self.__settings[x]
        except: y = None
        if y == True or y == 'true':
            y = True
        elif y == False or y == 'false':
            y = False
        return y

    @property
    def changed(self):
        return self.__changed

    @changed.setter
    def changed(self,x):
        if x in ( True, False):
            self.__changed = x
        return self.__changed

    @property
    def icon_look(self):
        return self.__settings['IconLook']

    @icon_look.setter
    def icon_look(self,x):
        if self.icon_look != x:
            self.__settings['IconLook'] = x
            self.changed = True
        return self.icon_look

    @property
    def left_click(self):
        return self.__settings['LeftClick']

    @left_click.setter
    def left_click(self,x):
        if self.left_click != x:
            self.__settings['LeftClick'] = x
            self.changed = True
        return self.left_click

    @property
    def upgrade_assume_yes(self):
        if self.__settings['UpgradeAssumeYes'] == 'true':
            return True
        else:
            return False

    @upgrade_assume_yes.setter
    def upgrade_assume_yes(self,x):
        x = str(x).lower()
        if str(self.upgrade_assume_yes).lower() != x:
            self.__settings['UpgradeAssumeYes'] = x
            self.changed = True
        return self.upgrade_assume_yes

    @property
    def use_dbus_notifications(self):
        x = 'UseDbusNotifications'
        s = self.__settings
        if x in s.keys() and s[x] == 'true':
            return True
        elif x in s.keys() and s[x] == 'false':
            return False
        else:
            return None

    @use_dbus_notifications.setter
    def use_dbus_notifications(self,x):
        x = str(x).lower()
        v = str(self.use_dbus_notifications).lower()
        if  v != x:
            self.__settings['UseDbusNotifications'] = x
            self.changed = True
        return self.use_dbus_notifications

    @property
    def upgrade_auto_close(self):
        if self.__settings['UpgradeAutoClose'] == 'true':
            return True
        else:
            return False

    @upgrade_auto_close.setter
    def upgrade_auto_close(self,x):
        x = str(x).lower()
        if str(self.upgrade_auto_close).lower() != x:
            self.__settings['UpgradeAutoClose'] = x
            self.changed = True
        return self.upgrade_auto_close

    @property
    def upgrade_type(self):
        return self.__settings['UpgradeType']

    @upgrade_type.setter
    def upgrade_type(self,x):
        if self.upgrade_type != x:
            self.__settings['UpgradeType'] = x
            self.changed = True
        return self.upgrade_type

    @property
    def wireframe_transparent(self):
        if self.__settings['WireframeTransparent'] == 'true':
            return True
        else:
            return False

    @wireframe_transparent.setter
    def wireframe_transparent(self,x):
        x = str(x).lower()
        if str(self.wireframe_transparent).lower() != x:
            self.__settings['WireframeTransparent'] = x
            self.changed = True
        return self.wireframe_transparent

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

def __run_rc():

    apt_notifier_rc = AptNotifierRC()

    print("User settings in apt-notifierrc:")
    print(apt_notifier_rc.settings())
    print(f"apt_notifier_rc.get('UseDbusNotifications') = {apt_notifier_rc.get('UseDbusNotifications')}")
# main
def main():

    __run_rc()

if __name__ == '__main__':
    main()
