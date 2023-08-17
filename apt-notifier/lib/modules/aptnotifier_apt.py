#! /usr/bin/python3
# -*- coding: utf-8 -*-
import sys
import subprocess

class Apt:
    """
    some apt-get tasks
    """
    def __init__(self):
        self.__available_updates = "0"
        pass


    # Check for avalable updates
    def available_updates(self, opts=[]):
        
        from subprocess import run
        cmd = "/usr/lib/apt-notifier/bin/updater_count --count"
        if '-d' in opts or '--dist-upgrade' in opts:
            cmd = cmd + ' --dist-upgrade'
        if '-u' in opts or '--upgrade' in opts:
            cmd = cmd + ' --upgrade'
            
        run = subprocess.run(cmd.split(), capture_output=True, text=True)
        # Read the output into a text string
        self.__available_updates = run.stdout.strip()
        return self.__available_updates

    @property
    def debian_codename(self):
        debian_codename = ""
        try:
            with open("/etc/debian_version","r") as f:
                dv =  f.read().splitlines()[0]
                dv = dv.split(".")[0]
                dv = dv.split("/")[0]
                if dv == "9":
                    debian_codename = "stretch"
                elif dv == "10":
                    debian_codename = "buster"
                elif dv == "11":
                    debian_codename = "bullseye"
                elif dv == "12":
                    debian_codename = "bookworm"
                else:
                    debian_codename = dv
        except FileNotFoundError as e:
            debian_codename = "n/a"
        self.__debian_codename = debian_codename
        return self.__debian_codename
        
    @property
    def updates_count(self):
        return self.available_updates()

    # display apt-history
    def apt_history(self):
        from subprocess import run, PIPE, Popen
        global conf
        try: conf
        except NameError:
            from aptnotifier_config import AptNotifierConfig
            conf = AptNotifierConfig()

        global xlate
        try: xlate
        except NameError:
            from aptnotifier_xlate import AptNotifierXlate
            xlate = AptNotifierXlate()

        window_icon   = conf.get('window_icon')
        window_title  = xlate.get('apt_history')
        
        cmd = ['xdotool', 'getdisplaygeometry']
        res = run(cmd, capture_output=True, text=True).stdout
        width,height = res.strip().split()
        if int(width) >= 1600:
            width  = 1600
            height =  900
        width  = int(width)*3/5
        height = int(height)*2/3
        
        yad_filler = {
            'window_title'  : window_title,
            'window_icon'   : window_icon,
            'width'         : int(width),
            'height'        : int(height),
            }
        yad = """
              /usr/bin/yad
              --text-info
              --title={window_title}
              --window-icon={window_icon}
              --class=apt-notifier
              --width={width}
              --height={height}
              --center
              --button=gtk-close
              --fontname=mono
              --margins=7
              --borders=5
          """
        
        y = [ x.strip() for x in yad.strip().split('\n') ]
        yad = [ x.format(**yad_filler) for x in y ]
        cmd = "apt-history | sed -r 's/:([a-z])/ \\1/' | column -t"
        pipe1 = Popen(cmd, shell=True, text=True, stdout=PIPE)
        pipe2 = Popen(yad, stdin=pipe1.stdout, stdout=PIPE, stderr=PIPE, text=True)
        # if pipe2 exits before pipe1, send SIGPIPE to pipe1 to close
        pipe1.stdout.close()
        pipe2.communicate()[0]

        
def __run():

    global apt
    try: apt
    except NameError:
        from aptnotifier_apt import Apt
        apt = Apt()

    print(f"Debian code_name: {apt.debian_codename}")
    
    apt.apt_history()
    #available_updates = apt.updates_count
    #print(available_updates)

# General application code
def main():
    __run()

if __name__ == '__main__':
    main()

