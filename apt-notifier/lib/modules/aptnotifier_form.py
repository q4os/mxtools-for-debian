#! /usr/bin/python3
# -*- coding: utf-8 -*-

import sys
MODULES = "/usr/lib/apt-notifier/modules"
if MODULES not in sys.path:
    sys.path.append(MODULES)
import subprocess
from subprocess import run, PIPE, Popen, DEVNULL

import os
from os import environ
import shutil
import re

# Use gettext and specify translation file locations
#import gettext
#gettext.bindtextdomain('apt-notifier', '/usr/share/locale')
#gettext.textdomain('apt-notifier')
#_ = gettext.gettext
#gettext.install('apt-notifier.py')

class Form:
    """
    get Dialog UI-form
    """
    def __init__(self):
        global package_manager
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

        self.__form_file = '/usr/lib/apt-notifier/form/form.xml'
        self.__filled_form = ''
        self.__loaded_form = ''
        self.__form_is_filled = False
        self.__raw_form = ''
        self.__show_left_click_behaviour_frame = False
        self.__show_left_click_behaviour_frame = True
        self.__NotificationCheckBox = """
        <checkbox active="{UseDbusNotifications}">
            <label>{label_notifications_with_actions}</label>
            <variable>UseDbusNotifications</variable>
            <action>:</action>
        </checkbox>"""
        self.__LeftClickBehaviourPackageManagerFrame = """
            <frame {frame_left_click_behaviour}>
              <radiobutton active="{LeftClickBehaviourPackageManager}">
                <label>{left_click_package_manager}</label>
                <variable>LeftClickPackageManager</variable>
                <action>:</action>
              </radiobutton>
              <radiobutton active="{LeftClickBehaviourViewAndUpgrade}">
                <label>{left_click_ViewandUpgrade}</label>
                <variable>LeftClickViewAndUpgrade</variable>
                <action>:</action>
              </radiobutton>
            </frame>
        """

        self.__form_token = {}
        self.__filled_token = {
            'window_title_preferences'              : xlate.get('window_title_preferences'),
            'frame_upgrade_behaviour'               : xlate.get('frame_upgrade_behaviour'),
            'label_full_upgrade'                    : xlate.get('label_full_upgrade'),
            'label_basic_upgrade'                   : xlate.get('label_basic_upgrade'),
            'frame_left_click_behaviour'            : xlate.get('frame_left_click_behaviour'),
            'frame_other_options'                   : xlate.get('frame_other_options'),
            'left_click_package_manager'            : xlate.get('left_click_package_manager'),
            'left_click_ViewandUpgrade'             : xlate.get('left_click_ViewandUpgrade'),
            'use_apt_get_dash_dash_yes'             : xlate.get('use_apt_get_dash_dash_yes'),
            'auto_close_term_window_when_complete'  : xlate.get('auto_close_term_window_when_complete'),
            'check_for_autoremoves'                 : xlate.get('check_for_autoremoves'),
            'frame_Icons'                           : xlate.get('frame_Icons'),
            'label_classic'                         : xlate.get('label_classic'),
            'label_pulse'                           : xlate.get('label_pulse'),
            'label_wireframe'                       : xlate.get('label_wireframe'),
            'frame_Auto_update'                     : xlate.get('frame_Auto_update'),
            'auto_update_checkbox_txt'              : xlate.get('auto_update_checkbox_txt'),
            'label_autostart'                       : xlate.get('label_autostart'),
            'label_wireframe_transparent'           : xlate.get('label_wireframe_transparent'),
            'label_notifications_with_actions'      : xlate.get('label_notifications_with_actions'),

            'window_icon'                           : conf.config['window_icon'],
            'window_icon_name'                      : conf.config['window_icon_name'],

            'classic_none'                       : conf.config['classic_none'],
            'classic_some'                       : conf.config['classic_some'],
            'pulse_none'                         : conf.config['pulse_none'],
            'pulse_some'                         : conf.config['pulse_some'],
            'wireframe_none_dark'                : conf.config['wireframe_none_dark'],
            'wireframe_none_dark_transparent'    : conf.config['wireframe_none_dark_transparent'],
            'wireframe_none_light'               : conf.config['wireframe_none_light'],
            'wireframe_none_light_transparent'   : conf.config['wireframe_none_light_transparent'],
            'wireframe_some'                     : conf.config['wireframe_some']
        }

        self.__filled_values = {
            'UpgradeBehaviourAptGetDistUpgrade':   'true',
            'UpgradeBehaviourAptGetUpgrade'    :   'false',
            'LeftClickBehaviourPackageManager' :   'false',
            'LeftClickBehaviourViewAndUpgrade' :   'true',
            'UpgradeAssumeYes'                 :   'false',
            'UpgradeAutoClose'                 :   'false',
            'IconLookWireframeDark'            :   'true',
            'IconLookWireframeLight'           :   'false',
            'IconLookClassic'                  :   'false',
            'IconLookPulse'                    :   'false',
            'WireframeTransparent'             :   'true',
            'unattended_upgrade'               :   'false',
            'AutoStart'                        :   'true',
            'UseDbusNotifications'             :   'true'
        }

        self.show_notification_checkbox = True
        if conf.get('show_switch_desktop_notifications'):
            self.show_notification_checkbox = True
        else:
            self.show_notification_checkbox = False
        self.load_form()

    @property
    def show_left_click_behaviour_frame(self):
        return self.__show_left_click_behaviour_frame

    @show_left_click_behaviour_frame.setter
    def show_left_click_behaviour_frame(self,x):
        if x == True or x == 'true':
            self.__show_left_click_behaviour_frame = True
        else:
            self.__show_left_click_behaviour_frame = False
        self.prepare_form()
        return self.show_left_click_behaviour_frame

    @property
    def show_notification_checkbox(self):
        return self.__show_notification_checkbox

    @show_notification_checkbox.setter
    def show_notification_checkbox(self,x):
        if x == True or x == 'true':
            self.__show_notification_checkbox = True
        else:
            self.__show_notification_checkbox = False
        self.prepare_form()
        return self.show_notification_checkbox

    @property
    def use_dbus_notifications(self):
        if self.__filled_values['UseDbusNotifications'] == 'true':
            return True
        else:
            return False

    @use_dbus_notifications.setter
    def use_dbus_notifications(self,x):
        if x == True or x == 'true':
            self.__filled_values['UseDbusNotifications'] = 'true'
        else:
            self.__filled_values['UseDbusNotifications'] = 'false'
        return self.use_dbus_notifications

    def fill_values(self, x={}):
        return self.__filled_values.update(x)

    def fill_form(self):
        self.prepare_form()
        self.extract_token()
        if Form.is_valid(self):
            fill =  self.__filled_token
            fill.update(self.__filled_values)

            #form = self.__raw_form.replace("{","{fill['")
            #form = form.replace("}","']}")
            #self.__filled_form = eval('f"""' + form + '"""')

            # replace eval with form.format(**fill)
            self.__filled_form = self.__raw_form.format(**fill)

            self.__form_is_filled = True
        return self.__filled_form

    def load_form(self):
        with open(self.__form_file, 'r') as f:
            self.__loaded_form = f.read()
        self.prepare_form()

    def prepare_form(self):
        s = self.__loaded_form
        if self.show_left_click_behaviour_frame:
            s = s.replace('{LeftClickBehaviourPackageManagerFrame}', self.__LeftClickBehaviourPackageManagerFrame)
        else:
            s = s.replace('{LeftClickBehaviourPackageManagerFrame}', '')
        if self.show_notification_checkbox:
            s = s.replace('{NotificationCheckBox}', self.__NotificationCheckBox)
        else:
            s = s.replace('{NotificationCheckBox}', '')
        self.__raw_form = s

    def extract_token(self):
        r = re.compile(r'{(\w+)}')
        if not self.__form_token:
            self.__form_token = { t:t for t in r.findall(self.__raw_form) }
        return self.__form_token

    """
    'UpgradeBehaviourAptGetDistUpgrade':   'true',
    'UpgradeBehaviourAptGetUpgrade'    :   'false',
    """
    #-----------------------
    @property
    def upgrade_type(self):
        if self.__filled_values['UpgradeBehaviourAptGetDistUpgrade'] == 'true':
            return 'dist-upgrade'
        elif self.__filled_values['UpgradeBehaviourAptGetUpgrade'] == 'true':
            return 'upgrade'
        else:
            # fallback to default
            return 'dist-upgrade'

    @upgrade_type.setter
    def upgrade_type(self,x):
        if x == 'upgrade':
            self.basic_upgrade = True
        elif x == 'dist-upgrade':
            self.full_upgrade = True
        else:
            # fallback to default
            self.full_upgrade = True
        return self.upgrade_type

    @property
    def full_upgrade(self):
        if self.__filled_values['UpgradeBehaviourAptGetDistUpgrade'] == 'true':
            return True
        else:
            return False

    @full_upgrade.setter
    def full_upgrade(self,x):
        if x == True or x == 'true':
            self.__filled_values['UpgradeBehaviourAptGetDistUpgrade'] ='true'
            self.__filled_values['UpgradeBehaviourAptGetUpgrade']     ='false'

        if x == False or x == 'false':
            self.__filled_values['UpgradeBehaviourAptGetDistUpgrade'] ='false'
            self.__filled_values['UpgradeBehaviourAptGetUpgrade']     ='true'
        return self.full_upgrade

    @property
    def basic_upgrade(self):
        if self.__filled_values['UpgradeBehaviourAptGetUpgrade'] == 'true':
            return True
        else:
            return False

    @basic_upgrade.setter
    def basic_upgrade(self,x):
        if x == True or x == 'true':
            self.__filled_values['UpgradeBehaviourAptGetDistUpgrade'] ='false'
            self.__filled_values['UpgradeBehaviourAptGetUpgrade']     ='true'

        if x == False or x == 'false':
            self.__filled_values['UpgradeBehaviourAptGetDistUpgrade'] ='true'
            self.__filled_values['UpgradeBehaviourAptGetUpgrade']     ='false'
        return self.basic_upgrade

    """
    'LeftClickBehaviourPackageManager' :   'false',
    'LeftClickBehaviourViewAndUpgrade' :   'true',
    'LeftClick':   ['ViewAndUpgrade', 'PackageManager'],

    """
    @property
    def left_click(self):
        if self.__filled_values['LeftClickBehaviourViewAndUpgrade'] == 'true':
            return 'ViewAndUpgrade'
        else:
            return 'PackageManager'

    @left_click.setter
    def left_click(self,x):
        if x == 'ViewAndUpgrade':
            self.__filled_values['LeftClickBehaviourViewAndUpgrade']  = 'true'
            self.__filled_values['LeftClickBehaviourPackageManager']  = 'false'
        if x == 'PackageManager':
            self.__filled_values['LeftClickBehaviourViewAndUpgrade']  = 'false'
            self.__filled_values['LeftClickBehaviourPackageManager']  = 'true'
        return self.left_click

    @property
    def left_click_view_and_upgrade(self):
        if self.__filled_values['LeftClickBehaviourViewAndUpgrade']  == 'true':
            return True
        else:
            return False

    @left_click_view_and_upgrade.setter
    def left_click_view_and_upgrade(self,x):
        if x == True or x == 'true':
            self.__filled_values['LeftClickBehaviourViewAndUpgrade']  = 'true'
            self.__filled_values['LeftClickBehaviourPackageManager']  = 'false'
        if x == False or x == 'false':
            self.__filled_values['LeftClickBehaviourViewAndUpgrade']  = 'false'
            self.__filled_values['LeftClickBehaviourPackageManager']  = 'true'
        return self.left_click_view_and_upgrade

    @property
    def left_click_package_manager(self):
        if self.__filled_values['LeftClickBehaviourPackageManager']  == 'true':
            return True
        else:
            return False

    @left_click_package_manager.setter
    def left_click_package_manager(self,x):
        if x == True or x == 'true':
            self.__filled_values['LeftClickBehaviourPackageManager']  = 'true'
            self.__filled_values['LeftClickBehaviourViewAndUpgrade']  = 'false'
        if x == False or x == 'false':
            self.__filled_values['LeftClickBehaviourPackageManager']  = 'false'
            self.__filled_values['LeftClickBehaviourViewAndUpgrade']  = 'true'
        if x.lower() == 'muon':
            y = self.__filled_token['left_click_package_manager']
            self.__filled_token['left_click_package_manager'] = y.replace('Synaptic', 'Muon')

        return self.left_click_package_manager

    """
    'UpgradeAssumeYes'                 :   'false',
    """
    @property
    def upgrade_assume_yes(self):
        if self.__filled_values['UpgradeAssumeYes']  == 'true':
            return True
        else:
            return False

    @upgrade_assume_yes.setter
    def upgrade_assume_yes(self,x):
        if x == True or x == 'true':
            self.__filled_values['UpgradeAssumeYes'] = 'true'
        if x == False or x == 'false':
            self.__filled_values['UpgradeAssumeYes'] = 'false'
        return self.upgrade_assume_yes

    """
    'UpgradeAutoClose'                 :   'false',
    """
    @property
    def upgrade_auto_close(self):
        if self.__filled_values['UpgradeAutoClose']  == 'true':
            return True
        else:
            return False

    @upgrade_auto_close.setter
    def upgrade_auto_close(self,x):
        if x == True or x == 'true':
            self.__filled_values['UpgradeAutoClose'] = 'true'
        if x == False or x == 'false':
            self.__filled_values['UpgradeAutoClose'] = 'false'
        return self.upgrade_auto_close

    #---------------
    """
    'AutoStart'                        :   'true'
    """
    @property
    def autostart(self):
        if self.__filled_values['AutoStart']  == 'true':
            return True
        else:
            return False

    @autostart.setter
    def autostart(self,x):
        if x == True or x == 'true':
            self.__filled_values['AutoStart'] = 'true'
        if x == False or x == 'false':
            self.__filled_values['AutoStart'] = 'false'
        return self.autostart

    """
    'unattended_upgrade'               :   'false',
    """
    @property
    def autoupdate(self):
        if self.__filled_values['unattended_upgrade']  == 'true':
            return True
        else:
            return False

    @autoupdate.setter
    def autoupdate(self,x):
        if x == True or x == 'true':
            self.__filled_values['unattended_upgrade'] = 'true'
        if x == False or x == 'false':
            self.__filled_values['unattended_upgrade'] = 'false'
        return self.autoupdate

    @property
    def unattended_upgrade(self):
        if self.__filled_values['unattended_upgrade']  == 'true':
            return True
        else:
            return False

    @unattended_upgrade.setter
    def unattended_upgrade(self,x):
        if x == True or x == 'true':
            self.__filled_values['unattended_upgrade'] = 'true'
        if x == False or x == 'false':
            self.__filled_values['unattended_upgrade'] = 'false'
        return self.unattended_upgrade


    """
    'IconLook':    ['wireframe-dark', 'wireframe-light', 'classic', 'pulse' ],

    'IconLookWireframeDark'            :   'true',
    'IconLookWireframeLight'           :   'false',
    'IconLookClassic'                  :   'false',
    'IconLookPulse'                    :   'false',
    'WireframeTransparent'             :   'true',
    """
    #---------------

    @property
    def icon_look(self):
        if self.__filled_values['IconLookWireframeDark'] == 'true':
            return 'wireframe-dark'
        elif self.__filled_values['IconLookWireframeLight'] == 'true':
            return 'wireframe-light'
        elif self.__filled_values['IconLookClassic'] == 'true':
            return 'classic'
        elif self.__filled_values['IconLookPulse'] == 'true':
            return 'pulse'
        else:
            # fallback to default
            return 'wireframe-dark'

    @icon_look.setter
    def icon_look(self,x):
        if x == 'wireframe-dark':
            self.__filled_values['IconLookWireframeDark'] = 'true'
            self.__filled_values['IconLookWireframeLight'] = 'false'
            self.__filled_values['IconLookClassic'] = 'false'
            self.__filled_values['IconLookPulse'] = 'false'
        elif x == 'wireframe-light':
            self.__filled_values['IconLookWireframeDark'] = 'false'
            self.__filled_values['IconLookWireframeLight'] = 'true'
            self.__filled_values['IconLookClassic'] = 'false'
            self.__filled_values['IconLookPulse'] = 'false'
        elif x == 'classic':
            self.__filled_values['IconLookWireframeDark'] = 'false'
            self.__filled_values['IconLookWireframeLight'] = 'false'
            self.__filled_values['IconLookClassic'] = 'true'
            self.__filled_values['IconLookPulse'] = 'false'
        elif x == 'pulse':
            self.__filled_values['IconLookWireframeDark'] = 'false'
            self.__filled_values['IconLookWireframeLight'] = 'false'
            self.__filled_values['IconLookClassic'] = 'false'
            self.__filled_values['IconLookPulse'] = 'true'
        else:
            # fallback to default
            self.icon_look = 'wireframe-dark'
        return self.icon_look

    #---------------
    """
    'WireframeTransparent'             :   'true',
    """
    @property
    def wireframe_transparent(self):
        if self.__filled_values['WireframeTransparent']  == 'true':
            return True
        else:
            return False


    @wireframe_transparent.setter
    def wireframe_transparent(self,x):
        if x == True or x == 'true':
            self.__filled_values['WireframeTransparent'] = 'true'
        if x == False or x == 'false':
            self.__filled_values['WireframeTransparent'] = 'false'
        return self.wireframe_transparent

    #---------------

    @property
    def raw_form(self):
        return self.__raw_form

    @property
    def form(self):
        return self.__filled_form

    @property
    def filled_form(self):
        return self.__filled_form

    @property
    def filled_values(self):
        return self.__filled_values

    @property
    def form_token(self):
        return self.__form_token

    @form_token.setter
    def form_token(self, x):
        self.__token = x
        return self.__token

    def show_form_token(self):
        print(list(self.__form_token.keys()))

    def show_filled_token(self):
        print(dict(self.__filled_token))

    def is_valid(self):
        valid = True
        for k in self.__form_token:
            if k not in list(self.__filled_token.keys()) + list(self.__filled_values.keys()):
                valid = False
                print(f"Missing form token {k}")
        return valid

    def is_filled(self):
        return self.__form_is_filled

    def dialog(self):

        if not self.is_valid() or not self.is_filled():
            print("Error[514]: form not valid or not filled")
            return
        ret = self.run_dialog()
        return ret


    def run_dialog(self):
        if not self.is_valid() or not self.is_filled():
            print("Error[423]: form not valid or not filled")
            return False
        else:
            title = self.__filled_token['window_title_preferences']
            class_name = "apt-notifier"
            def set_class_name(title=title, class_name_new=class_name):
                class_name_old = "gtkdialog"
                clx_filler = {
                    'class_name_old': class_name_old,
                    'class_name_new': class_name_new,
                    'title': title,
                }

                clx = """xdotool sleep 0.2
                    search --onlyvisible --classname {class_name_old}
                    search --onlyvisible --class {class_name_old}
                    search --name  {title}
                    set_window  --classname {class_name_new}
                                --class {class_name_new}
                    """
                y = [ x.strip() for x in clx.strip().split('\n') ]
                clx = [ x.format(**clx_filler) for x in ' '.join(y).split() ]
                r = Popen(clx)

            set_class_name(title, class_name)

            d = run(['/usr/bin/gtkdialog', '-c', '-s'],
                capture_output=True,
                text=True,
                input=str(self.__filled_form))
            ret = self.capture_output(d.stdout)
            return ret

    def capture_output(self,dialog_output):

        lines = list(map(lambda x: x.strip(),
                filter(lambda x: '=' in x,
                dialog_output.splitlines())))
        tuples = list(map(lambda x: (x.split('=',1)), lines))
        #dialog_settings  = {k:v.strip("'\"") for (k,v) in tuples}
        dialog  = {k:v.strip("'\"") for (k,v) in tuples}

        if not dialog['EXIT'] == 'OK':
            return False

        self.unattended_upgrade = dialog['AutoUpdate']

        self.autostart = dialog['PrefAutoStart']

        if dialog['UpgradeType_dist-upgrade'] == 'true':
            self.upgrade_type = 'dist-upgrade'
        else:
            self.upgrade_type = 'upgrade'

        if dialog['UpgradeAutoClose'] == 'true':
            self.upgrade_auto_close = 'true'
        else:
            self.upgrade_auto_close = 'false'

        if dialog['UpgradeAssumeYes'] == 'true':
            self.upgrade_assume_yes = 'true'
        else:
            self.upgrade_assume_yes = 'false'

        try:
            if dialog['UseDbusNotifications'] in [ 'true', 'false']:
                self.use_dbus_notifications = dialog['UseDbusNotifications']
        except KeyError:
            pass

        if dialog['WireframeTransparent'] == 'true':
            self.wireframe_transparent = 'true'
        else:
            self.wireframe_transparent = 'false'

        try:
            if dialog['LeftClickViewAndUpgrade'] == 'true':
                self.left_click = 'ViewAndUpgrade'
            else:
                self.left_click = 'PackageManager'
        except KeyError:
            pass

        if dialog['IconLook_wireframe_dark'] == 'true':
            self.icon_look = 'wireframe-dark'
        elif dialog['IconLook_wireframe_light'] == 'true':
            self.icon_look = 'wireframe-light'
        elif dialog['IconLook_classic'] == 'true':
            self.icon_look = 'classic'
        else:
            self.icon_look = 'pulse'
        if self.is_valid():
            self.fill_form()
            return True
        else:
            return False


def __show_form():

    global package_manager
    package_manager = 'muon'
    form = Form()

    form.load_form()
    #form.prepare_form()
    print("Show check:")
    form.show_left_click_behaviour_frame = True
    form.show_notification_checkbox = False

    form.fill_form()
    print("Show check:")
    print("Debug : **** form.is_valid() ***")
    print(form.is_valid())
    print(form.form_token)
    print(form.form)


# General application code
def main():
    __show_form()

if __name__ == '__main__':
    main()
