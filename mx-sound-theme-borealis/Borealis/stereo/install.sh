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

test=`pwd | grep Borealis`
if [ -z "$test" ]
then
  echo "This script needs to be run from the install folder. Exiting..."
  exit 1
fi

# Step 1
if [ -e /usr/share/sounds/Borealis ]
then
  echo "Step 1: Removing old content. All of the content in the $directory folder will be removed. Press 'y' to continue or any other key to abort..."
  read text

  case $text in
    [y] | [Y] )
      rm -rf ${directory}/*
    ;;
    * )
      echo "Aborting install of the $name..."
      exit 0
    ;;
  esac	
else
  echo "This is a new install, creating new directory $directory."
  mkdir $directory
fi
echo "Step 1 complete."

# Step 2
echo "Step 2: Copying new content into the '$directory' folder..."
cp -r * $directory
echo "Step 2 complete."

# Step 3
echo "Step 3: Setting up permissions..."
chmod 755 $directory
chmod 644 ${directory}/*
chmod 755 ${directory}/Config
chmod 600 ${directory}/Config/*
echo "Step 3 complete."

# Step 4
if [ -e ${HOME}/.kde/share/config/kickerrc ]
then
	echo -e "Step 4: Copying new config files into your $config directory and setting permissions.\n\n  WARNING! THIS WILL OVERRIDE YOUR CURRENT SOUND SETTINGS!\n\nPress 'y' if you would like me to backup your old settings or any other key to continue without backing them up."
	read text
	case $text in
		[y] | [Y] )
			if [ -w ${config}/knotify.eventsrc.preBorealis ]; then
				echo "Found old backup files, skipping backup..."
			else
				echo "Moving old setingst to *.preBorealis (i.e. ${config}/knotify.eventsrc -> ${config}/knotify.eventsrc.preBorealis)..."
				mv -f ${config}/knotify.eventsrc ${config}/knotify.eventsrc.preBorealis
				mv -f ${config}/konsole.eventsrc ${config}/konsole.eventsrc.preBorealis
				mv -f ${config}/kwin.eventsrc ${config}/kwin.eventsrc.preBorealis
				mv -f ${config}/kdevelop.eventsrc ${config}/kdevelop.eventsrc.preBorealis
				mv -f ${config}/kopete.eventsrc ${config}/kopete.eventsrc.preBorealis
				mv -f ${config}/kmail.eventsrc ${config}/kmail.eventsrc.preBorealis
				mv -f ${config}/ksysguard.eventsrc ${config}/ksysguard.eventsrc.preBorealis
				mv -f ${config}/proxyscout.eventsrc ${config}/proxyscout.eventsrc.preBorealis
				mv -f ${config}/k3b.eventsrc ${config}/k3b.eventsrc.preBorealis
				echo "Done."
			fi
		;;
		* )
			echo "Proceeding without backup..."
		;;
	esac
	
	echo "Copying config files to $config directory and adjusting permissions..."
	cp -f ${directory}/Config/* ${config}/
	chown --reference=${HOME}/.kde/share/config/kickerrc ${config}/*
	echo "Step 4 complete."
else
  echo "Your configuration is apparently missing KDE settings suggesting that you are running a different Window Manager. Skipping install of the KDE-specific configuration files. You can customize your WM's sounds by using its sounds settings panel and pointing to the files located in the /usr/share/sounds/Borealis/ folder."
fi

echo -e "\n\n\n
Install complete!

Your desktops sounds should be now enabled. If you do not hear anything, please make sure to turn up the sound in the sound mixer as well as check whether the arts server is up and running.

Thank you for your interest in my creations! Please do not forget to vote.
Your comments and suggestions are highly appreciated!

For latest news and updates please visit
$kde_page

Have a nice day!\n\n"

exit 0
