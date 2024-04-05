#! /usr/bin/python3
# -*- coding: utf-8 -*-

import os
import sys
sys.path.append("/usr/lib/apt-notifier/modules")
import subprocess

from PyQt5 import QtWidgets, QtGui, QtCore
from PyQt5.QtWidgets import QApplication, QPushButton, QMessageBox
from PyQt5.QtGui import QIcon

BUILD_VERSION='24.03.01'

class AptnotifierAbout():

    def __init__(self):
        os.environ["QT_LOGGING_RULES"] = "qt.qpa.xcb.warning=false"
        global xlate
        try: xlate
        except NameError:
            from aptnotifier_xlate import AptNotifierXlate
            xlate = AptNotifierXlate()

        global conf
        global config

        try: conf
        except NameError:
            from aptnotifier_config import AptNotifierConfig
            conf = AptNotifierConfig()
            config = conf.config
        self.__about_viewer = None
        self.__about_viewer = self.about_viewer

    @property
    def about_viewer(self):

        # list of viewers to check
        viewer_list = ['mx-viewer', 'antix-viewer']

        # xfce-handling
        if os.getenv('XDG_CURRENT_DESKTOP') == 'XFCE':
            viewer_list += ['exo-open']

        # set use xdg-open last to avoid html opens with tools like html-editor
        viewer_list += ['x-www-browser', 'gnome-www-browser', 'xdg-open']

        # take first found
        from shutil import which
        self.__about_viewer = list(filter( lambda x: which(x), viewer_list))[0]
        return self.__about_viewer


    def About(self, aboutBox):
        from subprocess import Popen, check_call, run
        from subprocess import DEVNULL, PIPE, CalledProcessError

        # xlate
        updater_name         = xlate.get('updater_name')
        about_updater        = xlate.get('about_updater')
        about_updater_title  = xlate.get('about_updater_title')
        Changelog            = xlate.get('Changelog')
        Close                = xlate.get('Close')
        License              = xlate.get('License')

        # config
        about_copyright         = config.get('about_copyright')
        about_window_icon       = config.get('about_window_icon')
        about_box_icon          = config.get('about_box_icon')
        about_box_url        = config.get('about_box_url')
        changelog_window_icon   = config.get('changelog_window_icon')
        changelog_window_icon   = config.get('window_icon')
        changelog_file          = config.get('changelog_file')
        changelog_url           = config.get('changelog_url')
        license_file            = config.get('license_file')

        license_title =  updater_name + ' - ' + License

        changelog_title = updater_name + ' - ' + Changelog
        license_title        = updater_name + ' - ' + License

        Changelog_Button = Changelog
        Close_Button     = Close
        License_Button   = License

        global BUILD_VERSION
        cmd = "dpkg-query -f ${Version} -W apt-notifier".split()
        updater_version = run(cmd, capture_output=True, text=True).stdout.strip()
        updater_version = BUILD_VERSION

        aboutText= f'''
        <p align=center><b><h2>{updater_name}</h2></b></p>
        <p align=center>Version: {updater_version}</p>
        <p align=center><h3>{about_updater}</h3></p>
        <p align=center><a href={about_box_url}>{about_box_url}</a>
        <br></p><p align=center>{about_copyright}<br /><br/></p>
         '''
        icon_pixmap = QtGui.QPixmap(about_box_icon)
        aboutBox.setIconPixmap(icon_pixmap)

        aboutBox.setWindowTitle(about_updater_title)
        aboutBox.setWindowIcon(QtGui.QIcon(about_window_icon))
        aboutBox.setText(aboutText)
        changelogButton = aboutBox.addButton( (Changelog_Button), QMessageBox.ActionRole)
        licenseButton   = aboutBox.addButton( (License_Button)  , QMessageBox.ActionRole)
        closeButton     = aboutBox.addButton( (Close_Button)    , QMessageBox.RejectRole)
        aboutBox.setDefaultButton(closeButton)
        aboutBox.setEscapeButton(closeButton)

        class_name = "apt-notifier"

        def set_class_name(class_name_new=class_name):
            class_name_old = "aptnotifier_about.py"
            clx = f"""xdotool sleep 0.3 
                search --onlyvisible --classname {class_name_old}  
                search --class {class_name_old}  
                set_window --classname {class_name_new} --class {class_name_new}
                """
            debug_p(clx)               
            y = [ x.strip() for x in clx.strip().split('\n') ]
            clx =  ' '.join(y)
            r = Popen(clx.split())
        
        set_class_name(class_name)
            
        while True:
            
            reply = aboutBox.exec_()
            #print(reply)
            if aboutBox.clickedButton() == closeButton:
                sys.exit(reply)

            if aboutBox.clickedButton() == licenseButton:
                about_viewer = self.about_viewer
                if about_viewer in ['mx-viewer', 'antix-viewer' ]:
                    cmd = [about_viewer, license_file, license_title]
                    clx_filler = {
                        'about_viewer': about_viewer,
                        'license_title': license_title,
                        'class_name': class_name,
                    }
                    clx = """
                        xdotool sleep 0.3 
                        search --onlyvisible --class {about_viewer} 
                        search --classname {about_viewer}
                        search --name {license_title} 
                        set_window --classname {class_name} --class {class_name}
                        """

                    y = [ x.strip() for x in clx.strip().split('\n') ]
                    clx = [ x.format(**clx_filler) for x in ' '.join(y).split() ]
                    debug_p(clx)               
                    r = Popen(clx)
                    
                elif about_viewer == 'exo-open':
                    cmd = ['exo-open', '--launch', 'WebBrowser', license_file ]
                else:
                    cmd = [about_viewer, license_file ]
                r = run(cmd, capture_output=True, text=True)

            if aboutBox.clickedButton() == changelogButton:
                cmd = ['xdotool', 'getdisplaygeometry']
                res = run(cmd, capture_output=True, text=True).stdout
                W, H = res.strip().split()
                if int(W) >= 1600:
                    W = "1600"
                    H = "900"
                width  = int(W)*3/5
                height = int(H)*2/3
                yad_filler = {
                    'width'                 : int(width),
                    'height'                : int(height),
                    'changelog_window_icon' : changelog_window_icon,
                    'changelog_title'       : changelog_title,
                    'close'                 : Close,
                    'class_name'            : class_name
                    }
                yad = """
                      /usr/bin/yad
                      --title={changelog_title}
                      --class={class_name}
                      --window-icon={changelog_window_icon}
                      --width={width}
                      --height={height}
                      --center
                      --button={close}!gtk-close
                      --fontname=mono
                      --margins=7
                      --borders=5
                      --text-info
                    """
                y = [ x.strip() for x in yad.strip().split('\n') ]
                yad = [ x.format(**yad_filler) for x in y ]
                #from pprint import pprint
                #pprint(yad)

                godo = False
                if not godo:
                    # try local changelog file
                    try:
                        with open(changelog_file):
                            godo = True
                    except FileNotFoundError as e:
                        changelog_file = ''
                    if godo:
                        pipe1 = Popen(['zcat', changelog_file], stdout=PIPE)
                        pipe2 = Popen(yad, stdin=pipe1.stdout, stdout=PIPE, stderr=PIPE, text=True)
                        # if pipe2 exits before pipe1, send SIGPIPE to pipe1 to close
                        pipe1.stdout.close()
                        pipe2.communicate()[0]

                if not godo:
                    # try with curl file exists on sever
                    try:
                        cmd = f"curl --output /dev/null --silent --fail -r 0-0 {changelog_url}"
                        cmd = cmd.split()
                        check_call(cmd)
                        godo = True
                    except CalledProcessError as e:
                        ret = e.returncode
                    if godo:
                        cmd = ['curl', '--silent', changelog_url ]
                        pipe1 = Popen(cmd, stdout=PIPE, text=True)
                        pipe2 = Popen(yad, stdin=pipe1.stdout, stdout=PIPE, stderr=PIPE, text=True)
                        pipe1.stdout.close()  # if pipe2 exits before pipe1, send SIGPIPE to pipe1 to close
                        pipe2.communicate()[0]

                if not godo:
                    # try with wget check file exists on sever
                    try:
                        cmd = f"wget --spider {changelog_url}"
                        cmd = cmd.split()
                        check_call(cmd, stdout=DEVNULL, stderr=DEVNULL)
                        godo = True
                    except CalledProcessError as e:
                        print(f"ret = e.returncode: {e.returncode}")
                        ret = e.returncode
                    if godo:
                        cmd = ['curl', '--silent', changelog_url ]
                        pipe1 = Popen(cmd, stdout=PIPE, text=True)
                        pipe2 = Popen(yad, stdin=pipe1.stdout, stdout=PIPE, stderr=PIPE, text=True)
                        # if pipe2 exits before pipe1, send SIGPIPE to pipe1 to close
                        pipe1.stdout.close()
                        pipe2.communicate()[0]

                # last atempt with apt-get changelog
                if not godo:

                    pipe1 = Popen(['apt-get', 'changelog', 'apt-notifier'],env={'PAGER':'cat'},
                                    stdout=PIPE, stderr=subprocess.STDOUT,text=True)
                    pipe2 = Popen(yad, stdin=pipe1.stdout, stdout=PIPE, stderr=PIPE, text=True)
                    # if pipe2 exits before pipe1, send SIGPIPE to pipe1 to close
                    pipe1.stdout.close()
                    pipe2.communicate()[0]

    def displayAbout(self):
        app = QApplication(sys.argv)
        aboutBox = QMessageBox()
        about = AptnotifierAbout()
        about.About(aboutBox)
        aboutBox.show()
        sys.exit(app.exec_())

def debugging():
    """
    simple debugging helper
    """
    import os
    global debug_apt_notifier
    try:
        debug_apt_notifier
    except:
        try:
            debug_apt_notifier = os.getenv('DEBUG_APT_NOTIFIER')
        except:
            debug_apt_notifier = False

    return debug_apt_notifier

def debug_p(text=''):
    """
    simple debug print helper -  msg get printed to stderr
    """
    if debugging():
        print("Debug: " + text, file = sys.stderr)

def __run():

    about = AptnotifierAbout()
    about.displayAbout()

def main():
    __run()

if __name__ == '__main__':
    main()
