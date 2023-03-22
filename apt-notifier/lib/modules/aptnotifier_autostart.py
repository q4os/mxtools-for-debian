#! /usr/bin/python3
# -*- coding: utf-8 -*-
import subprocess
from subprocess import run, PIPE
import sys
import os
import shutil


class AutoStart:
    """
    Check and set apt-notifier autostart
    """
    def __init__(self):

        self.__usr_autostart = os.getenv('HOME') + '/.config/autostart/apt-notifier-autostart.desktop'
        self.__xdg_autostart = '/etc/xdg/autostart/apt-notifier-autostart.desktop'
        self.__session = ''
        self.detect_distro()
        self.detect_session()
        # patterns to detect apt-notifier startup
        # current use in MX fluxbox ( "/usr/bin/apt-notifer.py" )
        self.pattern_1 = '^[^#].*apt-notifier.py.*'   
        # used in antiX and now also in MX ( "/usr/bin/apt-notifer" )
        self.pattern_2 = '^[^#].*apt-notifier.*'   
        self.__pat_exec='*(/usr/bin/)?apt-notifier.*'
        self.__pat_hash='[[:space:]#]*#[[:space:]#]*'
        self.__pat_text='[^[:alpha:]#]*(sleep)?[^[:alpha:]#]*'
        
    def detect_distro(self):
        """
        detect distro
        """
        if os.path.exists("/etc/mx-version"):
            self.__distro  = 'MX'
            self.__startup = self.__usr_autostart
        elif os.path.exists("/etc/antix-version"):
            self.__distro = 'antiX'
            self.__startup = os.getenv('HOME') + '/.desktop-session/startup'
        else:
            self.__distro = 'other'
            self.__startup = self.__usr_autostart
        
        return self.__distro

    def detect_session(self):
        """
        detect desktop session
        """
        from subprocess import run, PIPE, DEVNULL
        # check fluxbox
        try:
            cmd = 'pidof  fluxbox'
            run(cmd.split(), check=True, stdout=DEVNULL)
            self.__session = 'fluxbox'
            if not self.__distro:
                self.detect_distro()
            if self.__distro != 'antiX':
                self.__startup = os.getenv('HOME') + '/.fluxbox/startup'
            return self.__session
        except subprocess.CalledProcessError:
            pass
        # check kde-plasma
        try:
            cmd = 'pidof plasmashell'
            run(cmd.split(), check=True, stdout=DEVNULL)
            self.__session = 'plasma'
            return self.__session
        except subprocess.CalledProcessError:
            pass
        # check XDG_CURRENT_DESKTOP
        if os.getenv('XDG_CURRENT_DESKTOP',''):
            self.__session = os.getenv('XDG_CURRENT_DESKTOP','').lower()
            return self.__session

    def detect_autostart(self):
        """
        detect apt-notifier autostart
        """
        from subprocess import run, PIPE, DEVNULL

        if not self.__distro:
            self.detect_distro()

        #antiX
        if self.__distro == 'antiX':
            '''
            detect autostart in ~/.desktop-session/startup
            '''
            pat = f"^{self.__pat_text}{self.__pat_exec}"
            ret = run(['grep', '-sqE', pat, self.__startup ], stdout=DEVNULL, stderr=DEVNULL)
            if ret.returncode == 0:
                self.__autostart = True
            else:
                self.__autostart = False
        #MX fluxbox
        elif self.__session == 'fluxbox':
            '''
            detect autostart in ~/.fluxbox/startup
            '''
            pat = f"^{self.__pat_text}{self.__pat_exec}"
            ret = run(['grep', '-sqE', pat, self.__startup ], stdout=DEVNULL, stderr=DEVNULL)

            if ret.returncode == 0:
                self.__autostart = True
            else:
                self.__autostart = False
        # MX xdg user autostart
        else:
            '''
            detect autostart in xdg autostarts 
            '''
            ret = run(['grep', '-sq', '^Hidden=true', self.__usr_autostart, self.__xdg_autostart ])
            if ret.returncode == 0:
                self.__autostart = False
            else:
                self.__autostart = True
        return self.__autostart

    def disable_autostart(self):
        """
        disable apt-notifier autostart
        """
        from subprocess import run, PIPE, DEVNULL
        autostart = self.detect_autostart()
        if not autostart:
            #print("Autostart already disabled!")
            return autostart
            
        if not self.__distro:
            self.detect_distro()
        #antiX
        if self.__distro == 'antiX':
            '''
            disable autostart in ~/.desktop-session/startup
            '''
            # comment out startup line(s) found
            pat = f"^({self.__pat_text}{self.__pat_exec})"
            cmd = f"sed -i -r \\@{pat}@s@@#&@"
            run( cmd.split() + [ self.__startup ], stdout=DEVNULL, stderr=DEVNULL)
            return self.detect_autostart()
        #MX fluxbox
        elif self.__session == 'fluxbox':
            '''
            disable autostart in ~/.fluxbox/startup
            '''
            # comment out startup line(s) found
            pat = f"^({self.__pat_text}{self.__pat_exec})"
            cmd = f"sed -i -r \\@{pat}@s@@#&@"
            run( cmd.split() + [ self.__startup ], stdout=DEVNULL, stderr=DEVNULL)
            return self.detect_autostart()
        #MX or another
        else:
            '''
            disable autostart in ~/.config/autostart/apt-notifier-autostart.desktop'
            '''
            try:
              shutil.copyfile(self.__xdg_autostart, self.__usr_autostart)
            except PermissionError:
                print("Error: Permission denied.")
                return False
            except Exception as e:
                print(e)
                print(f"Error occurred while copying file from {self.__xdg_autostart} to {self.__usr_autostart} .")
                print("Error occurred while copying file.")
                return False
            #
            run(['sed', '-i', '-e', '/^Hidden=/d', '-e', '/^Exec=/aHidden=true', self.__usr_autostart ], stderr=DEVNULL)
            return self.detect_autostart()
        return autostart


    def enable_autostart(self):
        """
        enable apt-notifier autostart
        """
        from subprocess import run, PIPE, DEVNULL
        autostart = self.detect_autostart()
        if autostart:
            # print("Autostart already enabled!")
            return autostart

        if not self.__distro:
            self.detect_distro()

        #antiX
        if self.__distro == 'antiX':
            '''
            enable autostart in ~/.desktop-session/startup
            '''
            # uncomment first startup line found
            pat = f"^{self.__pat_hash}({self.__pat_text}{self.__pat_exec})"
            cmd = f"sed -i -r 0,\\@{pat}@s@@\\1@"
            run( cmd.split() + [ self.__startup ], stdout=DEVNULL, stderr=DEVNULL)
            return self.detect_autostart()

        #MX enable fluxbox startup
        elif self.__session == 'fluxbox':
            '''
            enable autostart in ~/.fluxbox/startup
            '''
            # uncomment first startup line found
            pat = f"^{self.__pat_hash}({self.__pat_text}{self.__pat_exec})"
            cmd = f"sed -i -r 0,\\@{pat}@s@@\\1@"
            run( cmd.split() + [ self.__startup ], stdout=DEVNULL, stderr=DEVNULL)
            return self.detect_autostart()

        #MX non-fluxbox startup
        else:
            '''
            enable autostart in ~/.config/autostart/apt-notifier-autostart.desktop'
            we copy to make it visible in plasma
            '''
            try:
              shutil.copyfile(self.__xdg_autostart, self.__usr_autostart)
            except PermissionError:
                print("Error: Permission denied.")
                return False
            except Exception as e:
                print(e)
                print(f"Error occurred while copying file from {self.__xdg_autostart} to {self.__usr_autostart} .")
                print("Error occurred while copying file.")
                return False
            run(['sed', '-i', '-e', '/^Hidden=/d', self.__usr_autostart ])
            return self.detect_autostart()
        return autostart

    def set_autostart(autostart: bool):

        if autostart:
            if detect_autostart():
                print(f"already set autostart(autostart: {autostart})")
            else:
                print(f"need to enable autostart(autostart: {autostart})")
                enable_autostart()
        else:
            if detect_autostart():
                print(f"need to disable autostart(autostart: False)")
                disable_autostart()
            else:
                print(f"already disabled autostart(autostart: {autostart})")

        return autostart

    @property
    def autostart(self):
        return self.detect_autostart()

    @autostart.setter
    def autostart(self,x):
        if x:
            self.enable_autostart()
        else:
            self.disable_autostart()
        return x

    def fix_fluxbox_autostart(self):
        """
        fix  apt-notifier autostart within ~/.fluxbx/startup
        and replace obsolete startup line containing /usr/bin/apt-notifier.py
        with '(sleep 6; /usr/bin/apt-notifier )&'
        """
        from subprocess import run, PIPE, DEVNULL

        # detect fluxbox startup files exists
        startup = os.getenv('HOME') + '/.fluxbox/startup'
        if not os.path.isfile(startup):
            # no startup file found - nothing to fix
            return

        # detect 'old' autostart entry
        #print("detect 'old' autostart entry")
        #hash='[[:space:]#]*#[[:space:]#]*'
        old='(nohup|ionice).*(/usr/bin/)?apt-notifier[.]py.*'
        new='(sleep 6; /usr/bin/apt-notifier)\&'
        pat = f"^({self.__pat_hash})?{old}"
        #cmd = f"grep -sqE '{pat}'  {startup}"
        #print(cmd)
        cmd = f"grep -sqE  {pat}  {startup}"
        ret = run(cmd.split(), stdout=DEVNULL, stderr=DEVNULL).returncode
        if ret:
            # OK. fluxbox startup does not contain an old entry
            # print(" OK. fluxbox startup does not contain an old entry")
            return True
        
        # replace 'old' startup entry with the new startup line 
        # but keep commented out in case
        # cmd=['sed', '-i.pre_fix', '-r',  f's%^({self.__pat_hash})?({old})%\\1{new}%', startup]
        cmd=['sed', '-i', '-r',  f's%^({self.__pat_hash})?({old})%\\1{new}%', startup]
        #print(cmd)
        run(cmd)
        #run(cmd, stdout=DEVNULL, stderr=DEVNULL)
        
        # check whether fix applied
        pat = f"^({self.__pat_hash})?{old}"
        cmd = f"grep -sqE '{pat}'  {startup}"
        #print(cmd)
        cmd = f"grep -sqE  {pat}  {startup}"
        ret = run(cmd.split(), stdout=DEVNULL, stderr=DEVNULL).returncode
        if ret:
            #print("Ok fix worked - no old startup line found")
            pass
        else:
            #print("Nope - fix not work - old startup line found")
            return False
        
        # check and detect new startup line
        pat = f"^({self.__pat_hash})?({self.__pat_text}{self.__pat_exec})"
        cmd = f"grep -sqE {pat}"
        ret = run(cmd.split() + [ startup ], stdout=DEVNULL, stderr=DEVNULL)
        if ret.returncode:
            #print("Opps fix not worked - no new startup line found")
            return False
        else:
            #print("Ok fix worked - new startup line found")
            return True
            
        

def __run_check_autostart():

    ast = AutoStart()

    #print(f"Detected AutoStart: {ast.autostart}")
    print(f"Fix fluxbox autostart: {ast.fix_fluxbox_autostart()}")

    """
    print("Toggle AutoStart:")
    if ast.autostart:
        ast.autostart = False
    else:
        ast.autostart = True
    print(ast.autostart)
    """
    
# General application code
def main():

    __run_check_autostart()

if __name__ == '__main__':
    main()

