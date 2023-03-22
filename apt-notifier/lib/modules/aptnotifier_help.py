#! /usr/bin/python3
# -*- coding: utf-8 -*-

import os
import sys
MODULES = "/usr/lib/apt-notifier/modules"

if MODULES not in sys.path:
    sys.path.append(MODULES)

class AptNotifierHelp():

    def __init__(self):
        global xlate
        try: xlate
        except NameError:
            from aptnotifier_xlate import AptNotifierXlate
            xlate = AptNotifierXlate()
        
        global apt_notifier_conf
        try: apt_notifier_conf
        except NameError:
            from aptnotifier_config import AptNotifierConfig
            apt_notifier_conf = AptNotifierConfig()
        self.__apt_notifier_viewer = None
        self.__apt_notifier_viewer = self.apt_notifier_viewer
        
        self.__apt_notifier_help = xlate.get('apt_notifier_help')
    
    @property
    def apt_notifier_viewer(self):
        # list of viewers to check
        # viewer_list = ['mx-viewer', 'antix-viewer']
        viewer_list = apt_notifier_conf.get('apt_notifier_viewer_list')
        viewer_list = viewer_list.replace(',', ' ').split()
        
        # xfce-handling
        if os.getenv('XDG_CURRENT_DESKTOP') == 'XFCE':
            viewer_list += ['exo-open']

        # set use xdg-open last to avoid html opens with tools like html-editor
        viewer_list += ['x-www-browser', 'gnome-www-browser', 'xdg-open']

        # take first found
        from shutil import which
        self.__apt_notifier_viewer = list(filter( lambda x: which(x), viewer_list))[0]
        return self.__apt_notifier_viewer


    def apt_notifier_help(self):
        global apt_notifier_conf
        
        try: apt_notifier_conf
        except NameError:
            from aptnotifier_config import AptNotifierConfig
            apt_notifier_conf = AptNotifierConfig()

        """"show_apt_notifier_help" : "True",
            "apt_notifier_help_url" : "https://mxlinux.org/wiki/help-files/help-mx-apt-notifier",
            "apt_notifier_help_url_fr" : "https://mxlinux.org/wiki/help-files/help-mx-apt-notifier-notificateur-dapt",
            "apt_notifier_viewer_list" : "mx-viewer, antix-viewer",
        
        """        
        show_apt_notifier_help = apt_notifier_conf.get('show_apt_notifier_help')
        if not show_apt_notifier_help:
            return
            
        # check whether a language specific help_url is avalable
        help_url = apt_notifier_conf.get('apt_notifier_help_url')
        lang = ''
        if os.getenv('LANG'):
            lang = os.getenv('LANG').split('_')[0]
        if os.getenv('LANGUAGE'):
            lang = os.getenv('LANGUAGE').split(':')[0]
        lang = lang.lower()
            
        if apt_notifier_conf.get('apt_notifier_help_url' + '_' + lang):
            help_url = apt_notifier_conf.get('apt_notifier_help_url' + '_' + lang)
        
        if not help_url:
            return
        
        
        apt_notifier_viewer = self.apt_notifier_viewer
    
        help_title = xlate.get('apt_notifier_help')
        
        if apt_notifier_viewer in ['mx-viewer', 'antix-viewer' ]:
            cmd = [apt_notifier_viewer, help_url, help_title]
        elif about_viewer == 'exo-open':
            cmd = ['exo-open', '--launch', 'WebBrowser', help_url ]
        else:
            cmd = [apt_notifier_viewer, help_url ]
        debug_p(f"{cmd}")
        import subprocess
        from subprocess import run, DEVNULL
        run(cmd, stdout=DEVNULL, stderr=DEVNULL )


    def open_package_manager_help(self, package_manager=None):
        from subprocess import Popen, check_call, run
        from subprocess import DEVNULL, PIPE, CalledProcessError

        if not package_manager:
            return
        
        global apt_notifier_conf
        try: apt_notifier_conf
        except NameError:
            from aptnotifier_config import AptNotifierConfig
            apt_notifier_conf = AptNotifierConfig()
        
        package_manager = package_manager.lower()
        help_url = apt_notifier_conf.get(package_manager + '_help_url')
        if not help_url:
            return

        # check whether a language specific help_url is configured
        lang = ''
        if os.getenv('LANG'):
            lang = os.getenv('LANG').split('_')[0]
        if os.getenv('LANGUAGE'):
            lang = os.getenv('LANGUAGE').split(':')[0]
        lang = lang.lower()
            
        help_url_lang = apt_notifier_conf.get(package_manager + '_help_url_' + lang)
        
        if not help_url_lang and lang != 'en':
            # check whether   help_url + '-' + lang is available
            help_url_lang = help_url + '-' + lang
            # try with wget check url is valid
            try:
                cmd = f"wget -q --spider {help_url_lang}"
                debug_p(f"{cmd}")
                cmd = cmd.split()
                check_call(cmd, stdout=DEVNULL, stderr=DEVNULL)
                help_url = help_url_lang
            except CalledProcessError as e:
                debug_p(f"wget -q --spider {help_url_lang} : ret = e.returncode: {e.returncode}")
                help_url_lang = None

        if help_url_lang:
            help_url = help_url_lang
        
        if not help_url:
            return
    
        help_title = xlate.get('package_manager_help')
        if package_manager == 'muon':
            help_title = help_title.replace("Synaptic", 'Muon')

        apt_notifier_viewer = self.apt_notifier_viewer
        
        if apt_notifier_viewer in ['mx-viewer', 'antix-viewer' ]:
            cmd = [apt_notifier_viewer, help_url, help_title]
        elif apt_notifier_viewer == 'exo-open':
            cmd = ['exo-open', '--launch', 'WebBrowser', help_url ]
        else:
            cmd = [apt_notifier_viewer, help_url ]
        debug_p(f"{cmd}")
        run(cmd, stdout=DEVNULL, stderr=DEVNULL )

   
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

def __run():

    ah = AptNotifierHelp()
    #ah.apt_notifier_help()
    ah.open_package_manager_help('Synaptic')

def main():
    __run()

if __name__ == '__main__':
    main()
