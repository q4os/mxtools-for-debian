#!/bin/bash

#find current slack-desktop version and download

#get version number
#remove any previous tmp file
if [ -e "/tmp/slack.txt" ]; then
	rm /tmp/slack.txt
fi

#dump web page into text file
wget https://slack.com/downloads/linux -O /tmp/slack.txt

if [ ! "$?" = "0" ]; then
	echo "could not find download site, aborting..."
	exit 1
fi

#process text file to get version number 
a=$(cat /tmp/slack.txt)
rm /tmp/slack.txt
b=${a#*Version}
c=${b%span*}
version=$(echo $c |cut -d"<" -f1)

#get slack desktop deb
wget https://downloads.slack-edge.com/releases/linux/$version/prod/x64/slack-desktop-$version-amd64.deb -O /tmp/slack-desktop.deb
if [ -e /tmp/slack-desktop.deb ]; then
	apt-get install /tmp/slack-desktop.deb
	rm /tmp/slack-desktop.deb
else
	exit 1
fi

exit 0
