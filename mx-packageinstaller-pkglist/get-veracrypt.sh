#!/bin/bash

#find current veracrypt version and download

#get version number
#remove any previous tmp file
if [ -e "/tmp/veracrypt.txt" ]; then
	rm /tmp/veracrypt.txt
fi

#dump web page into text file
wget https://www.veracrypt.fr/en/Downloads.html -O /tmp/veracrypt.txt

if [ ! "$?" = "0" ]; then
	echo "could not find download site, aborting..."
	exit 1
fi

#process text file to get version number 
version=$(grep Debian-11 /tmp/veracrypt.txt | grep -v console| grep launchpad |cut -d"=" -f1 |cut -d"<" -f1 |cut -d">" -f1|tr -d [:space:])
if [ -e "/tmp/veracrypt.txt" ]; then
	rm /tmp/veracrypt.txt
fi

#get slack desktop deb
wget https://launchpadlibrarian.net/563962360/$version -O /tmp/veracrypt.deb
if [ -e /tmp/veracrypt.deb ]; then
	apt-get install /tmp/veracrypt.deb
	rm /tmp/veracrypt.deb
else
	exit 1
fi

exit 0


