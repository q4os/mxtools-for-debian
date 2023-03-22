#! /usr/bin/python3
# -*- coding: utf-8 -*-

import sys
sys.path.append("/usr/lib/apt-notifier/modules")
MODULES = "/usr/lib/apt-notifier/modules"
if MODULES not in sys.path:
    sys.path.append(MODULES)

import gettext
import locale

gettext.bindtextdomain('apt-notifier', '/usr/share/locale')
gettext.textdomain('apt-notifier')

_ = gettext.gettext


class AptNotifierXlate:
    """
    apt-notifier helper class to provide a common place for translations
    """

    def __init__(self):
        global conf
        try: conf
        except NameError:
            from aptnotifier_config import AptNotifierConfig
            conf = AptNotifierConfig()
        self.__domain = conf.config['domain']
        self.__display_domain_in_menu_and_title  = conf.get('display_domain_in_menu_and_title')
        self.__translations = {
            'updater_name'                               : _("MX Updater"),
            'about'                                      : _("About"),
            'about_updater_title'                        : _("About MX Updater"),
            'apt_history'                                : _("History"),
            'apt_notifier_help'                          : _("MX Updater Help"),
            'apt_notifier_preferences'                   : _("Preferences"),
            'auto_close_term_window_when_complete'       : _("automatically close terminal window when full/basic upgrade complete"),
            'auto_close_window'                          : _("automatically close terminal window when upgrade complete"),
            'auto_close_window'                          : _("automatically close terminal window when upgrade complete"),
            'autoremovable_packages_msg1'                : _("Unneeded packages are installed that can be removed."),
            'autoremovable_packages_msg2'                : _("Running apt-get autoremove, if you are unsure type 'n'."),
            'auto_update_checkbox_txt'                   : _("update automatically   (will not add new or remove existing packages)"),
            'about_updater'                              : _("Tray applet to notify of system and application updates"),
            'basic_upgrade'                              : _("basic upgrade"),
            'label_basic_upgrade'                        : _("basic upgrade"),
            'cancel'                                     : _("Cancel"),
            'changelog'                                  : _("Changelog"),
            'check_for_autoremoves'                      : _("check for autoremovable packages after full/basic upgrade"),
            'check_for_updates'                          : _("Check for Updates"),
            'close'                                      : _("Close"),
            'done1basic'                                 : _("basic upgrade complete (or was canceled)"),
            'done1full'                                  : _("full upgrade complete (or was canceled)"),
            'done2'                                      : _("this terminal window can now be closed"),
            'frame_auto_update'                          : _("Auto-update"),
            'frame_Auto_update'                          : _("Auto-update"),
            'frame_icons'                                : _("Icons"),
            'frame_Icons'                                : _("Icons"),
            'frame_left_click_behaviour'                 : _("Left-click behaviour   (when updates are available)"),
            'frame_other_options'                        : _("Other options"),
            'frame_upgrade_behaviour'                    : _("Upgrade mode"),
            'full_upgrade'                               : _("full upgrade"),
            'label_full_upgrade'                         : _("full upgrade   (recommended)"),
            'hide_until_updates_available'               : _("Hide until updates available"),
            'label_autostart'                            : _("start MX Updater at login"),
            'label_classic'                              : _("classic"),
            'label_notifications_with_actions'           : _("use desktop notifications"),
            'label_pulse'                                : _("pulse"),
            'label_wireframe_transparent'                : _("use transparent interior for no-updates wireframe"),
            'label_wireframe'                            : _("wireframe"),
            'left_click_package_manager'                 : _("opens Synaptic"),
            'left_click_viewandupgrade'                  : _("opens MX Updater 'View and Upgrade' window"),
            'left_click_ViewandUpgrade'                  : _("opens MX Updater 'View and Upgrade' window"),
            'lessprompt'                                 : _("Press 'h' for help, 'q' to quit"),
            'license'                                    : _("License"),
            'mx_package_installer'                       : _("MX Package Installer"),
            'no_logs_found'                              : _("No logs found."),
            'no_dpks_logs_found'                         : _("No unattended-upgrades dpkg log(s) found."),
            'package_manager_help'                       : _("Synaptic Help"),
            'popup_msg_1_new_update_available'           : _("You have 1 new update available"),
            'popup_msg_multiple_new_updates_available'   : _("You have $count new updates available"),
            'popup_title'                                : _("Updates"),
            'anykey_to_stop'                             : _("press any key to stop automatic closing"),
            'anykey_to_close'                            : _("press any key to close"),
            'pressanykey'                                : _("press any key to close"),
            'quit_apt_notifier'                          : _("Quit"),
            'restart_apt_notifier'                       : _("Restart MX Updater"),
            'reload'                                     : _("Reload"),
            'reload_tooltip'                             : _("Reload the package information to become informed about new, removed or upgraded software packages. (apt-get update)"),
            'for_less_detailed_view_see_history'         : _("For a less detailed view see 'Auto-update dpkg log(s)' or 'History'."),
            'switch_to_basic_upgrade'                    : _("switch to basic upgrade"),
            'switch_to_full_upgrade'                     : _("switch to full upgrade"),
            'switch_tooltip'                             : _("Switches the type of Upgrade that will be performed, alternating back and forth between 'full upgrade' and 'basic upgrade'."),
            'title_dpkg_log'                             : _("MX Auto-update  --  unattended-upgrades log viewer"),
            'title_unattended-upgrades_log_viewer'       : _("MX Auto-update  --  unattended-upgrades log viewer"),
            'title_unattended-upgrades_dpkg_log_viewer'  : _("MX Auto-update  --  unattended-upgrades dpkg log viewer"),
            'title_dpkg_log_viewer'                      : _("MX Auto-update  --  unattended-upgrades dpkg log viewer"),
            'tooltip_0_updates_available'                : _("0 updates available"),
            'tooltip_0_updates_available'                : _("No updates available"),
            'tooltip_1_new_update_available'             : _("1 new update available"),
            'tooltip_multiple_new_updates_available'     : _("$count new updates available"),
            'unattended_upgrades'                        : _("Unattended-upgrades"),
            'upgrade_tooltip_basic'                      : _("Using basic upgrade (not recommended)"),
            'upgrade_tooltip_full'                       : _("Using full upgrade"),
            'upgrade'                                    : _("upgrade"),
            'upgrade_label'                              : _("upgrade"),
            'upgrade_using_package_manager'              : _("Upgrade using Synaptic"),
            'use_apt_get_dash_dash_yes'                  : _("Automatically answer 'yes' to all prompts during full/basic upgrade"),
            'use_apt_get_dash_dash_yes'                  : _("Automatically answer 'yes' to all prompts during upgrade"),
            'view_and_upgrade'                           : _("View and Upgrade"),
            'view_auto_updates_dpkg_logs'                : _("Auto-update dpkg log(s)"),
            'view_auto_updates_logs'                     : _("Auto-update log(s)"),
            'window_title_basic'                         : _("MX Updater--View and Upgrade, previewing: basic upgrade"),
            'window_title_full'                          : _("MX Updater--View and Upgrade, previewing: full upgrade"),
            'window_title_preferences'                   : _("MX Updater preferences")
        }

        # fix some English spelling change to lower/upper case
        # in preference all 'strings' are in lower case, so we adjust here
        y = self.__translations['use_apt_get_dash_dash_yes']
        if y == "Automatically answer 'yes' to all prompts during upgrade":
            self.__translations['use_apt_get_dash_dash_yes'] = y.lower()

        # fix some English spelling change to lower/upper case
        # in view and upgrade all buttons lablels are capitalized, so we adjust here
        y = self.__translations['upgrade_label']
        if y == 'upgrade':
            self.__translations['upgrade_label'] = 'Upgrade'

        self.__strings = { v:k for k,v in self.__translations.items() }
        self.domainize()

    def get(self,x):
        try:
            y = self.__translations[x]
        except:
            y = _(x)

        mx = 'MX'
        if mx not in y:
            return y
        domain = self.__domain
        if domain != 'antiX':
            return y

        if not self.__display_domain_in_menu_and_title:
            domain = ''

        if len(domain) > 0:
            y = y.replace(mx, domain)
        elif ' ' + mx + ' ' in y:
            y = y.replace(' ' + mx + ' ',' ')
        elif mx +' ' in y:
            y = y.replace(mx +' ', ' ')
        elif ' ' + mx in y:
            y = y.replace(' ' + mx ,' ')
        y = y.strip()
        return y

    def domainize(self):
        self.__xlate = { k:self.xlate(v) for k,v in self.__translations.items() }
        return self.__xlate

    @property
    def xlates(self):
        return self.__xlate

    def xlate(self,x):
        y = self.get(x)
        return y

    def string(self,x):
        try:
            return self.__strings[x]
        except:
            return ""

def debug_p(text=''):
    """
    simple debug print helper
    msg get printed to stderr
    """
    import os,sys
    debug_me = ''
    try:
        debug_me = os.getenv('DEBUG_APT_NOTIFIER')
    except:
        debug_me = False
    if debug_me:
        print('Debug: ' + text, file = sys.stderr)



def __run():

    import os
    xlate = AptNotifierXlate()
    debug_p(xlate.get('window_title_basic'))
    debug_p(xlate.get('window_title_full'))
    debug_p(xlate.get('label_full_upgrade'))
    debug_p(xlate.get('label_basic_upgrade'))


    form_token = {
        'window_title'                         : xlate.get("MX Updater preferences"),
        'frame_upgrade_behaviour'              : xlate.get("Upgrade mode"),
        'label_full_upgrade'                   : xlate.get("full upgrade   (recommended)"),
        'label_basic_upgrade'                  : xlate.get("basic upgrade"),
        'frame_left_click_behaviour'           : xlate.get("Left-click behaviour   (when updates are available)"),
        'frame_other_options'                  : xlate.get("Other options"),
        'left_click_package_manager'           : xlate.get("opens Synaptic"),
        'left_click_ViewandUpgrade'            : xlate.get("opens MX Updater 'View and Upgrade' window"),
        'use_apt_get_dash_dash_yes'            : xlate.get("Automatically answer 'yes' to all prompts during full/basic upgrade"),
        'auto_close_term_window_when_complete' : xlate.get("automatically close terminal window when full/basic upgrade complete"),
        'check_for_autoremoves'                : xlate.get("check for autoremovable packages after full/basic upgrade"),
        'frame_Icons'                          : xlate.get("Icons"),
        'label_classic'                        : xlate.get("classic"),
        'label_pulse'                          : xlate.get("pulse"),
        'label_wireframe'                      : xlate.get("wireframe"),
        'frame_Auto_update'                    : xlate.get("Auto-update"),
        'auto_update_checkbox_txt'             : xlate.get("update automatically   (will not add new or remove existing packages)"),
        'label_autostart'                      : xlate.get("start MX Updater at login"),
        'label_wireframe_transparent'          : xlate.get("use transparent interior for no-updates wireframe"),
        'label_notifications_with_actions'     : xlate.get("display notifications with action-buttons")
    }

    debug_p(f'Test: xlate.get("MX Updater preferences") : {xlate.get("MX Updater preferences")}')

    # shell
    debug_p("(")
    list(map(lambda x: debug_p(f'    ["{x[0]}"]="{x[1]}"'), list(xlate.xlates.items())))
    debug_p(")")

    debug_p(f"Debug: {xlate.get('apt_notifier_help')}")

# General application code
def main():
    __run()

if __name__ == '__main__':
    main()
