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

export title="antiX Advert Blocker"

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
	yad --image "info" --title "$title" --text=$"<b> Failed </b> \n\n\
	antiX advert blocker must be run as root or with sudo "
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
		yad --image="info" --title "$title" --text=$"Restoring original /etc/hosts."
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
        yad --title "$title" --progress --width $WIDTH \
               --text=$"Loading  blocklist from $domain" \
               --percentage=0 \
               --auto-close
}

# download the ads lists
function download_blocklist
{	
    # UNBLOCK
    if [ "$unblock" == "true" ]; then
        mv -f "/etc/hosts.ORIGINAL" "/etc/hosts" 
        rm -f "/etc/hosts.saved"
    elif [ -z $what_to_block ] ; then
		# StevenBlack's basic blocklist ( blocks adware, malware etc., ) , without any other extensions
		wget_dialog https://raw.githubusercontent.com/StevenBlack/hosts/master/hosts $WORKINGDIR/blocklist1
	else 
		# StevenBlack's blocklist with selected extensions
		wget_dialog "https://raw.githubusercontent.com/StevenBlack/hosts/master/alternates/"$what_to_block"/hosts" $WORKINGDIR/blocklist1
	fi
		

    #100830 BK bug fix: create if not exist...
    touch $WORKINGDIR/blocklist{1,2,3,4,5} 
}


function success
{
	# tell user 
	yad --image "info" --title "$title" --text=$"Success - your settings have been changed.\n\n\
Your hosts file has been updated.\n\
Restart your browser to see the changes."
}

#=======================================================================
# main
#

# display message and ask to continue
yad --title "$title" --width "$WIDTH" --image "question" --text "$info_text"
rsp=$?

if [ $rsp != 0 ]; then
    exit 0
fi

# selection dialog
ans=$(yad --title "$title" \
             --width "$WIDTH" --height 250 \
             --list --separator=":" \
             --text $"Choose what to block" \
             --checklist  --column "Pick" --column "To be blocked"\
             TRUE "Block_Ad_and_Malware_websites" \
             TRUE "Block_Pornographic_websites" \
             TRUE "Block_Gambling_websites" \
             TRUE "Block_Fakenews_websites" \
             FALSE "Block_Social_Media_websites" \
             FALSE "UNBLOCK_everything" )

#echo $ans

# transform the list separated by ':' into arr
arr=$(echo $ans | tr ":" "\n")

selected=""
what_to_block=""
for x in $arr
do
    #echo "> [$x]"
    
    case $x in
    Block_Fakenews_websites)
        block_fakenews='true'
        selected='yes'
        ;;
    Block_Gambling_websites)
        block_gambling='true'
        selected='yes'
        ;;
    Block_Pornographic_websites)
        block_porn='true'
        selected='yes'
        ;;
    Block_Social_Media_websites)
        block_social_media='true'
        selected='yes'
        ;;
    Block_Ad_and_Malware_websites)
        block_ads_and_malware='true'
        selected='yes'
        ;;
    UNBLOCK_everything)
        unblock='true'
        selected='yes'
        ;;
    esac    
done

# Convert it into a pattern for the download link
# This code is in the order of StevenBlack's hosts links to download
# Wrong order will make the download link invalid
# The order of these extensions is fakenews-gambling-porn-social
if [ "$block_fakenews" == true ] ; then
	what_to_block=$(echo "fakenews-")
fi

if [ "$block_gambling" == true ] ; then
	what_to_block=$(echo $what_to_block"gambling-")
fi

if [ "$block_porn" == true ] ; then
	what_to_block=$(echo $what_to_block"porn-")
fi

if [ "$block_social_media" == true ] ; then
	what_to_block=$(echo $what_to_block"social-")
fi

# Remove the trailing hyphen, if any. This is to make it compatible to StevenBlack's hosts download link
what_to_block=$(echo ${what_to_block%-})

if [ -z $selected ]; then
    # nothing selected
    echo $"No item selected"
    exit 0
fi

cleanup
download_blocklist
build_blocklist_all
append_blocklist
cleanup
success
