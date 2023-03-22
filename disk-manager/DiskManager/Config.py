# -*- coding: UTF-8 -*-
#
#  Config.py : Manage configuration file
#  Copyright (C) 2007 Mertens Florent <flomertens@gmail.com>
#  Updated 2021 for MX Linux Project by team member Nite Coder
#  Maintenance of project assumed by MX Linux with permission from original author.

#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

import os
import pwd
import logging
import configparser
from .config import CONF_FILE
from .Utility import get_user

# Default configuration :

general = {
"notify"       : "True" ,
"ntfs_driver"  : "ntfs" ,
"backend"      : "auto" ,
"fstab_naming" : "auto" ,
}

gui = {
"main_width"     : "700" ,
"main_height"    : "450" ,
"selected"       : "2" ,
"selected_order" : "ascending" 
}

detected = {
"detected" : "" ,
"known" : ""
}

categorie = {
"General"         : general ,
"Gui Config"      : gui ,
"Detected Device" : detected
} 


class Config(configparser.SafeConfigParser) :

    def __init__(self) :
    
        configparser.SafeConfigParser.__init__(self)
        try :
            self.read_config()
        except configparser.ParsingError :
            logging.warning("Parsing error while loading conf file. Using default configuration...")
        for section in list(categorie.keys()) :
            if not self.has_section(section) :
                self.add_section(section)
            for option in list(categorie[section].keys()) :
                if not self.has_option(section,option) :
                    self.set(section,option,categorie[section][option])   
        self.apply_change()
    
    def set(self, section, option, value) :
    
        configparser.SafeConfigParser.set(self, section, option, str(value))
        self.apply_change()
        
    def set_default(self, section, option) :
    
        configparser.SafeConfigParser.set(self, section, option, categorie[section][option])
        self.apply_change()
        return configparser.SafeConfigParser.get(self, section, option)
        
    def read_config(self) :
        ''' Ugly stuff. We actually store config file in the user home, to be able to
            read it when checking if there is new device '''

        self.user_config = get_user("dir") + CONF_FILE
        self.root_config = pwd.getpwuid(0).pw_dir + CONF_FILE
        if not self.user_config in self.read([self.root_config, self.user_config]) :
            conffile = open(self.user_config, "w")
            conffile.close()
            os.chown(self.user_config, get_user("uid"), get_user("gid"))
            

    def apply_change(self) :
    
        conffile = open(self.user_config, "w")
        self.write(conffile)
        conffile.close()
        self.read([self.root_config, self.user_config])


