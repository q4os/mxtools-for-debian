#!/bin/bash

# Code to be used in /usr/local/bin/block-advert.sh

# Improvements to this script:
# -- Use StevenBlack's unified hosts file ( github.com/StevenBlack/hosts )
#	 This is what pihole uses by default
#	 This unifies most of the lists previously present in antiX advert blocker + other useful lists
# -- Options to select StevenBlack's hosts file extensions
#	 ie. options to block fakenews, gambling, porn , social media -- can be useful for parental control , content filtering etc.,
# -- Compress 9 lines in 1 for lesser file size , much lesser number of lines ( about 10x ) and better performance ( as StevenBlack's updateHostsFile.py can do -- https://github.com/StevenBlack/hosts/pull/459)
# -- Do all work in a random subdirectory of /tmp ( not in /tmp as it was before ) for more security, less clutter and less conflicts 
# -- Show a warning if not running as root / sudo 
# -- Update the welcome message to reflect these

#v0.4 created by sc0ttman, August 2010
#GPL license /usr/share/doc/legal/gpl-2.0.txt
#100830 BK added GPL license, amended Exit msg, bug fixes.
# zenity version by lagopus for antiX, Decemder 2010
# modified to yad by Dave for antiX, September 2011
# added sysctl, Jan 2020
# fix update URL to mvps

# advert blocker
# downloads a list of known advert servers
# then appends them to /etc/hosts so that
# many online adverts are blocked from sight

TEXTDOMAINDIR=/usr/share/locale
TEXTDOMAIN=block-advert

export title=$"antiX Advert Blocker"

# the markers used to find the changes in /etc/hosts, which are made by this app
export markerstart='# BEGIN (below) - IPs added by antiX Advert Blocker #'
export markerend='# END (above) - IPs added by antiX Advert Blocker #'

# Do all work inside a random subdirectory of /tmp , as many modern apps do for security, less clutter etc.,
WORKINGDIR=/tmp/antiXadvertblocker.$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 8 ; echo '')
mkdir $WORKINGDIR

info_text=$"The <b>$title</b> tool adds stuff to your /etc/hosts file, so \n\
that many advertising servers and websites can't connect to this PC.\n\
You can choose to block ads, malware, pornography, gambling, fakenews and social media\n\
Blocking ad servers protects your privacy, saves you bandwidth, greatly \n\
improves web-browsing speed and makes the internet much less annoying in general.\n\n\
Do you want to proceed?"

# width of progress dialogs
WIDTH=360

if [ $(whoami) != root ] ; then
	yad --center --image=advert-block --title="$title" --text=$"<b> Failed </b> \n\n\
	antiX advert blocker must be run as root or with sudo " --button="OK"
	exit 1
fi

# cleanup all leftover files
function cleanup
{
    # remove all temp files
    rm -rf $WORKINGDIR/*
}

# concatenate the downloaded files
# clean out everything but the list of IPs and servers
function build_blocklist_all
{

    #echo "====================YTO"
    # suppress comments anywhere in a line ( not just at the beginning ) , then empty lines, replace tabs by spaces
    # remove double spaces, remove lines not beginning with "0.0.0.0" ,
    # suppress \r at end of line
    # then sort unique by field 2 (url)
    cat $WORKINGDIR/blocklist{1,2,3,4,5} | sed 's:#.*$::g' | \
                               sed '/^$/d' | \
                               sed 's/[\t]/ /g' | \
                               sed 's/  / /g' | \
                               sed -n '/^["0\.0\.0\.0"]/p' | \
                               tr -d '\015' | \
                               sort -u -k 2 \
                               > $WORKINGDIR/blocklist-all
    #echo "====================YTO"
    
    # Compress 9 lines into 1
    # This is taken from StevenBlack's updateHostsFile.py
    # This will decrease file size , decrease number of lines drastically ( about 10x ) and increase performance
    # Writing it in bash seems too slow. So, we use python, which is faster here
	cat << EndOfFile > $WORKINGDIR/compress.py
#!/usr/bin/python3
import subprocess
import os
	
f = open( "$WORKINGDIR/blocklist-all" , "r+" )
f.seek(0)  # reset file pointer
    
target_ip = "0.0.0.0"
target_ip_len = len(target_ip)
lines = [target_ip]
lines_index = 0
for line in f.readlines():
	if line.startswith(target_ip):
		if lines[lines_index].count(" ") < 9:
			lines[lines_index] += (
				" " + line[target_ip_len : line.find("#")].strip()  # remove comments
			)
		else:
			lines[lines_index] += "\n"
			lines.append(line[: line.find("#")].strip()) # remove comments
			lines_index += 1
# Sort and remove duplicates
CleanedLines = sorted(set(lines))

f.truncate(0) # Clear contents of f
f.seek(0) # Move pointer to start of file
	
for line in CleanedLines:
	f.write(line)

f.close()
	
EndOfFile

	chmod +x $WORKINGDIR/compress.py
	$WORKINGDIR/compress.py	

}

# append the list to the /etc/hosts
function append_blocklist
{
	# copy /etc/hosts, but the stuff between the markers, to a temp hosts file
	sed -e "/$markerstart/,/$markerend/d" /etc/hosts > $WORKINGDIR/hosts-temp
	# remove the markers
	sed -i -e "/$markerstart/d" $WORKINGDIR/hosts-temp
	sed -i -e "/$markerend/d"   $WORKINGDIR/hosts-temp
    
	if [ "$unblock" = true ] ; then
		yad --center --window-icon=advert-block --image=advert-block --title="$title" --text=$"Restoring original /etc/hosts."
        exit 1
	else
		# add list contents into the hosts file, below a marker (for easier removal)
		echo "$markerstart" >> $WORKINGDIR/hosts-temp
		echo "# These kinds of sites are blocked : $what_to_block" >> $WORKINGDIR/hosts-temp # Show what blocking options are used
		cat $WORKINGDIR/blocklist-all >> $WORKINGDIR/hosts-temp
		echo "$markerend" >> $WORKINGDIR/hosts-temp
	fi
    # On first use backup original /etc/hosts to /etc/hosts.ORIGINAL
    # If /etc/hosts.original exists, then backup to /etc/hosts.saved
    if [ -f /etc/hosts.ORIGINAL ]; then
    cp "/etc/hosts" "/etc/hosts.saved"
    mv $WORKINGDIR/hosts-temp "/etc/hosts"
    else
    cp "/etc/hosts" "/etc/hosts.ORIGINAL"
    cp "/etc/hosts" "/etc/hosts.saved"
    mv $WORKINGDIR/hosts-temp /etc/hosts
    fi
}

# usage: wget_dialog url file
# $1 : url of the file
# $2 : file: location of the downloaded file
function wget_dialog
{
    #echo "url: [$1]"
    url=$1
    # extract domain name between // and /
    domain=$(echo "$url" | cut -d/ -f3)
    #echo "===> $domain"
    
    # '--progress=dot' prints dots and a percentage at the end of the line
    # print $7 to cut the percentage
    # system("") to flush the output of awk in the pipe
    # sed to delete the ending '%' sign
    # sed -u to flush the output of sed
    # changed -t 0 (tries) to -t 20
    wget -c -4 -t 20 -T 10 --progress=dot -O $2 "$1" 2>&1 | \
        awk '{print $7}; system("")' | sed -u 's/%//' | \
        yad --window-icon=advert-block --title "$title" --progress --width $WIDTH \
               --text=$"Loading  blocklist from $domain" \
               --percentage=0 \
               --auto-close --center
}

# download the ads lists
function download_blocklist
{	
    # UNBLOCK
    if [ "$unblock" == "true" ]; then
    
		if [ ! -f /etc/hosts.ORIGINAL ];then   yad --fixed --window-icon=advert-block --title="$title"  --center --text=$"\n ERROR: \n No /etc/hosts.ORIGINAL was found, so it can't be restored.\n Probably you already UNBLOCKED EVERYTHING. \n You will have to manually edit the file /etc/hosts to remove any unwanted content \n" --button="OK"
			exit
		fi
        sleep 1
        mv -f "/etc/hosts.ORIGINAL" "/etc/hosts" 
        sleep 1
        rm -f "/etc/hosts.saved"
    elif [ -z $what_to_block ] ; then
		# StevenBlack's basic blocklist ( blocks adware, malware etc., ) , without any other extensions
		sleep 1
		wget_dialog https://raw.githubusercontent.com/StevenBlack/hosts/master/hosts $WORKINGDIR/blocklist1
	else 
		# StevenBlack's blocklist with selected extensions
		sleep 1
		wget_dialog "https://raw.githubusercontent.com/StevenBlack/hosts/master/alternates/"$what_to_block"/hosts" $WORKINGDIR/blocklist1
	fi
		

    #100830 BK bug fix: create if not exist...
    touch $WORKINGDIR/blocklist{1,2,3,4,5} 
}

function success
{
	# tell user 
	yad --image=advert-block --title="$title" --window-icon=advert-block --text=$"Success - your settings have been changed.\n\n\
Your hosts file has been updated.\n\
Restart your browser to see the changes." --button="OK" --fixed --center
}

#=======================================================================
# main
#

yad_version=$(yad --version 2>/dev/null |  sed "s/[^0-9].*//")
#echo "yad_version is " $yad_version
BUTTON_OK="gtk-ok"
if [ $(($yad_version)) -gt 1 ]; then
	BUTTON_OK="yad-ok"
fi

# display message and ask to continue
notes=$"\n \n (NOTE1: This application's main window always opens showing the default selection,\n not what's currently selected. \n  NOTE2: RESTART YOUR BROWSER TO SEE THE EFFECTS OF ANY CHANGE!)"
yad --fixed --title="$title" --width="$WIDTH" --image=advert-block  --window-icon=advert-block --center --text "$info_text $notes"
rsp=$?

if [ $rsp != 0 ]; then
    exit 0
fi

#Connectivity check: show notification and exit, if not connected to a network
ip=$(hostname -I)
if [ -n "$ip" ]; then
 check="online"
else
 yad --center --timeout=4  --window-icon=advert-block --title="$title" --picture --filename=/usr/share/icons/Adwaita/48x48/legacy/network-error.png --geometry=300x100-50-50 --inc=256 --button="x" --timeout-indicator=bottom  --undecorated --close-on-unfocus --escape-ok --skip-taskbar
exit
fi

# Main selection dialog, with individual variables, for easier localization (NOTE: the "Block..." variables are essential to the script's logic!!!)
text=$"Choose what to block"
unblock_text=$"UNBLOCK EVERYTHING"
exit_text=$"Cancel"
Block_Ad_and_Malware_websites=$"Block Ad and Malware websites"
Block_Pornographic_websites=$"Block Pornographic websites"
Block_Fakenews_websites=$"Block Fakenews websites"
Block_Gambling_websites=$"Block Gambling websites"
Block_Social_Media_websites=$"Block Social Media websites"
ans=$(yad  --no-headers  --window-icon=advert-block --image=advert-block --title="$title" --center --height=200 --width=450 --text="$text" --button="${exit_text}!/usr/share/icons/papirus-antix/symbolic/actions/window-close-symbolic.png":1 --button="${unblock_text}!/usr/share/icons/papirus-antix/24x24/emblems/emblem-unlocked.png":65 --button=$BUTTON_OK:2 --list  --checklist --column "" --column "" TRUE "$Block_Ad_and_Malware_websites" TRUE "$Block_Pornographic_websites" TRUE "$Block_Gambling_websites" TRUE "$Block_Fakenews_websites" FALSE "$Block_Social_Media_websites") 

#If user pressed the "UNBLOCK" button:
button=$?
if [[ $button -eq 65 ]]; then
        unblock='true'
        selected='yes'
fi

#If user pressed the "exit" button, just exit:
if [[ $button -le 1 ]]; then  exit; fi

# Convert it into a pattern for the download link
# This code is in the order of StevenBlack's hosts links to download
# Wrong order will make the download link invalid
# The order of these extensions is fakenews-gambling-porn-social
if [[ $ans =~ "TRUE|$Block_Fakenews_websites" ]]; then
		echo "$Block_Fakenews_websites"
		what_to_block=$(echo "fakenews-")
		selected='yes'
fi

if [[ $ans =~ "TRUE|$Block_Gambling_websites" ]]; then
		echo "$Block_Gambling_websites"
		what_to_block=$(echo $what_to_block"gambling-")
		selected='yes'
fi

if [[ $ans =~ "TRUE|$Block_Pornographic_websites" ]]; then
		echo "$Block_Pornographic_websites"
		what_to_block=$(echo $what_to_block"porn-")
		selected='yes'
fi

if [[ $ans =~ "TRUE|$Block_Social_Media_websites" ]]; then
		echo "$Block_Social_Media_websites"
		what_to_block=$(echo $what_to_block"social-")
		selected='yes'
fi

if [[ $ans =~ "TRUE|$Block_Ad_and_Malware_website" ]]; then
		echo "$Block_Ad_and_Malware_websites"
        block_ads_and_malware='true'
        selected='yes'
fi

# Remove the trailing hyphen, if any. This is to make it compatible to StevenBlack's hosts download link
what_to_block=$(echo ${what_to_block%-})

# if nothing selected, display warning:
if [ -z $selected ]; then
    yad --window-icon=advert-block --title="$title" --center --text=$"No item selected" --button="OK" --width=400
    exit 0
fi

cleanup
download_blocklist
build_blocklist_all
append_blocklist
cleanup
success
