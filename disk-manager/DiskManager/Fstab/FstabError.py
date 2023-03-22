# -*- coding: UTF-8 -*-
#
#  FstabError.py : Define Fstab exceptions
#  Copyright (C) 2007 Mertens Florent <flomertens@gmail.com>
#  Updated 2021 for MX Linux Project by team member Nite Coder
#  Maintenance of project assumed by MX Linux with permission from original author.
#
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


class NotInDatabase(Exception) : 

    def __init__(self, value) :
    
        self.value = value
        
    def __str__(self) :
        
        if isinstance(self.value, int) :
            return "Index %i out of range" % self.value
        else :
            return "Can't find '%s' in the database" % self.value
            
class DatabaseTypeError(Exception) :

    def __init__(self, type, accepted) :
    
        self.type = type
        self.accepted = accepted
        
    def __str__(self) :
    
        return "Wrong type : %s. Expected : %s" % \
            (self.type.__name__, ", ".join([ k.__name__ for k in self.accepted ]))
            
class UnknowEvent(Exception) :

    def __init__(self, event_handler, value) :

        self.value = value
        
    def __str__(self) :
    
        return "No event named %s" % self.value

