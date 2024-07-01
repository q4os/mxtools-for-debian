<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Video
</category>

<name>
w64codecs
</name>

<description>
library of older windows, quicktime, and realplayer codecs
</description>

<installable>
64
</installable>

<screenshot></screenshot>

<preinstall>
ARCH=$(dpkg --print-architecture)
SITE="https://www.deb-multimedia.org/"
TARGET=$(curl -s  https://www.deb-multimedia.org/dists/stable/non-free/binary-${ARCH}/package/w64codecs |grep Filename |cut -d":" -f2 |cut -d" " -f2)
TARGET="${TARGET%deb*}deb"
wget $SITE${TARGET} -O /tmp/w64codecs.deb
apt-get install /tmp/w64codecs.deb
</preinstall>

<install_package_names>

</install_package_names>


<postinstall>
rm /tmp/w64codecs.deb
</postinstall>


<uninstall_package_names>
w64codecs
</uninstall_package_names>
</app>
