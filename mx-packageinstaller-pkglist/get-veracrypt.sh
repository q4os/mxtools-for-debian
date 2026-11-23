#!/bin/bash

#find current veracrypt version and download

#get version number
#remove any previous tmp file
if [ -e "/tmp/veracrypt.txt" ]; then
	rm /tmp/veracrypt.txt
fi

#get arch
arch=$(dpkg --print-architecture)


#dump web page into text file
wget https://veracrypt.io//en/Downloads.html -O /tmp/veracrypt.txt

if [ ! "$?" = "0" ]; then
	echo "could not find download site, aborting..."
	exit 1
fi

arch=$(dpkg --print-architecture)

#process text file to get version number 
version=$(grep Debian-13 /tmp/veracrypt.txt | cut -d"\"" -f2 |grep $arch | grep -v sig |grep -v console)
echo "Version is: " $version
if [ -e "/tmp/veracrypt.txt" ]; then
	rm /tmp/veracrypt.txt
fi

#get vercrypt deb
wget "$version" -O "/tmp/veracrypt.deb"
if [ -e /tmp/veracrypt.deb ]; then
	apt-get install /tmp/veracrypt.deb
	rm /tmp/veracrypt.deb
else
	exit 1
fi

exit 0


