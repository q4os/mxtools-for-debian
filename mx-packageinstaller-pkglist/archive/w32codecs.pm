<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Audio
</category>

<name>
w32codecs
</name>

<description>
library of older windows, quicktime, and realplayer codecs
</description>

<installable>
32
</installable>

<screenshot></screenshot>

<preinstall>
ARCH=$(dpkg --print-architecture)
SITE="https://www.deb-multimedia.org/"
TARGET=$(curl -s  https://www.deb-multimedia.org/dists/stable/non-free/binary-${ARCH}/package/w32codecs |grep Filename |cut -d":" -f2 |cut -d" " -f2)
TARGET="${TARGET%deb*}deb"
wget $SITE${TARGET} -O /tmp/w32codecs.deb
apt-get install /tmp/w32codecs.deb
</preinstall>

<install_package_names>

</install_package_names>


<postinstall>
rm /tmp/w32codecs.deb
</postinstall>


<uninstall_package_names>
w32codecs
</uninstall_package_names>
</app>
