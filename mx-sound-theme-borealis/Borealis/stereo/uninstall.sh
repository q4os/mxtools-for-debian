#!/bin/bash
# Installation script for the Borealis sound theme ver.1.0
# Copyright 2004 - Ivica Ico Bukvic
# http://meowing.ccm.uc.edu/~ico/
# This install script is made available under GPL license
# For more info regarding this license, please see http://www.gnu.org/copyleft/gpl.html

name='Borealis sound theme v.0.9a'
title='by Ivica Ico Bukvic
ico@fuse.net
http://meowing.ccm.uc.edu/~ico/
'
directory='/usr/share/sounds/Borealis'
config="$HOME/.kde/share/config"
homepage='http://meowing.ccm.uc.edu/~ico/'
kde_page='http://kde-look.org/content/show.php?content=12584'

#==============================================================

echo "The $name $title"

#Let's see if we are root
if [ `id -u` -ne 0 ]
then
  echo "You must be superuser (su) to run this script. Please also use 'su -p' command when entering the superuser mode in order to preserve your user environment variables."
  exit 1
fi

#Let's see if environment variables have been preserved
testhome="$HOME"
testuser="$USER"
if [ $testhome == '/root' ] && [ $testuser != 'root' ]
then
  echo "You are either not logged in as a superuser (su) with a '-p' variable that preserves your user environment variables or you are trying to run this script using 'sudo'. Please use the superuser mode with the '-p' flag by typing 'su -p' before running this script."
  exit 1
fi

# Step 1
if [ -e /usr/share/sounds/Borealis ]
then
  echo "Uninstalling $name..."
  rm -rf /usr/share/sounds/Borealis
	if [ -e ${HOME}/.kde/share/config/kickerrc ]; then
		if [ -w ${config}/knotify.eventsrc.preBorealis ]; then
			echo "Reverting old sound settings..."
			mv -f ${config}/knotify.eventsrc.preBorealis ${config}/knotify.eventsrc
			mv -f ${config}/konsole.eventsrc.preBorealis ${config}/konsole.eventsrc
			mv -f ${config}/kwin.eventsrc.preBorealis ${config}/kwin.eventsrc
			mv -f ${config}/kdevelop.eventsrc.preBorealis ${config}/kdevelop.eventsrc
			mv -f ${config}/kopete.eventsrc.preBorealis ${config}/kopete.eventsrc
			mv -f ${config}/kmail.eventsrc.preBorealis ${config}/kmail.eventsrc
			mv -f ${config}/ksysguard.eventsrc.preBorealis ${config}/ksysguard.eventsrc
			mv -f ${config}/proxyscout.eventsrc.preBorealis ${config}/proxyscout.eventsrc
			mv -f ${config}/k3b.eventsrc.preBorealis ${config}/k3b.eventsrc
			echo "Adjusting permissions..."
			chown --reference=/home/${USER}/.bashrc ${config}/*
		else
			echo -e "No backup sound settings found, you will need to reconfigure your desktop sounds manually...\n"
		fi
	else
	  echo -e "You are apparently running a non-KDE Window Manager. Skipping restoring of the old KDE settings...\n"
	fi
else
  echo -e "This theme is currently not installed. Skipping uninstall...\n"
fi


echo -e "\n\n\n
Uninstall complete!. Thank you for your interest in my creations.

For latest news and updates please visit
$kde_page

Have a nice day!\n\n"

exit 0
