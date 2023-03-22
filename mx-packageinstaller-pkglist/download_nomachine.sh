#!/bin/bash

#install nomachine 64 bit
#part of mx-packageinstaller

ARCH=$(dpkg --print-architecture)


if [ "$ARCH" = "amd64" ]; then
echo "Downloading nomachine...this could take a few minutes"
wget $(curl -s "https://www.nomachine.com/download/download&id=5" |grep deb | grep href |cut -d\' -f2) -O /tmp/nomachine.deb
fi

if [ "$ARCH" = "i386" ]; then
echo "Downloading nomachine...this could take a few minutes"
wget $(curl -s "https://www.nomachine.com/download/download&id=2" |grep deb | grep href |cut -d\' -f2) -O /tmp/nomachine.deb
fi

exit 0
