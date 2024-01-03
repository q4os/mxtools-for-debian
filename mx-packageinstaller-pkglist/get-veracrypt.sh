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

arch=$(dpkg --print-architecture)

#process text file to get version number 
version=$(grep Debian-12 /tmp/veracrypt.txt | grep -v console| grep launchpad |cut -d"=" -f1 |cut -d"<" -f1 |cut -d">" -f1 |grep "$arch"|awk '{print $1}')
trunk=$(echo $version |cut -d"-" -f2)
echo $version
echo $trunk
if [ -e "/tmp/veracrypt.txt" ]; then
	rm /tmp/veracrypt.txt
fi

#get vercrypt deb
wget https://launchpad.net/veracrypt/trunk/$trunk/+download/$version -O /tmp/veracrypt.deb
if [ -e /tmp/veracrypt.deb ]; then
	apt-get install /tmp/veracrypt.deb
	rm /tmp/veracrypt.deb
else
	exit 1
fi

exit 0


