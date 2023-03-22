<?xml version="1.0"?>
<app>

<category>
Kernels
</category>

<name>
Debian-Backports 32 bit No PAE
</name>

<description>
   <am>Debian Backports Kernel (32 bit No PAE)</am>
   <ar>Debian Backports Kernel (32 bit No PAE)</ar>
   <bg>Debian Backports Kernel (32 bit No PAE)</bg>
   <bn>Debian Backports Kernel (32 bit No PAE)</bn>
   <ca>Kernel de Debian Backports (32 bit sense PAE)</ca>
   <cs>Debian Backports Kernel (32 bit No PAE)</cs>
   <da>Debian backports kerne (32 bit uden PAE)</da>
   <de>Debian Backports Kernel (32 Bit ohne PAE)</de>
   <el>Debian Backports Kernel (32 bit χωρίς PAE)</el>
   <en>Debian Backports Kernel (32 bit No PAE)</en>
   <es>Debian Backports Kernel (32 bit No PAE)</es>
   <et>Debian Backports Kernel (32 bit No PAE)</et>
   <eu>Debian Backports Kernel (32 bit No PAE)</eu>
   <fa>Debian Backports Kernel (32 bit No PAE)</fa>
   <fil_PH>Debian Backports Kernel (32 bit No PAE)</fil_PH>
   <fi>Debian Backports Kernel (32 bit No PAE)</fi>
   <fr>Noyau Debian Backports (32 bit No PAE)</fr>
   <he_IL>Debian Backports Kernel (32 bit No PAE)</he_IL>
   <hi>Debian Backports Kernel (32 bit No PAE)</hi>
   <hr>Debian Backports Kernel (32 bit No PAE)</hr>
   <hu>Debian Backports Kernel (32 bit No PAE)</hu>
   <id>Debian Backports Kernel (32 bit No PAE)</id>
   <is>Debian Backports Kernel (32 bit No PAE)</is>
   <it>Debian Backports Kernel (32 bit No PAE)</it>
   <ja_JP>Debian Backports Kernel (32 bit No PAE)</ja_JP>
   <ja>Debian Backports Kernel (32 bit No PAE)</ja>
   <kk>Debian Backports Kernel (32 bit No PAE)</kk>
   <ko>Debian Backports Kernel (32 bit No PAE)</ko>
   <lt>Debian Backports Kernel (32 bit No PAE)</lt>
   <mk>Debian Backports Kernel (32 bit No PAE)</mk>
   <mr>Debian Backports Kernel (32 bit No PAE)</mr>
   <nb>Debian Backports Kernel (32 bit No PAE)</nb>
   <nl>Debian Backports Kernel (32 bit Geen PAE)</nl>
   <pl>Debian Backports Kernel (32 bit No PAE)</pl>
   <pt_BR>Debian Backports Kernel (32 bit No PAE)</pt_BR>
   <pt>Núcleo (kernel) do Debian Backports (32 bit No PAE)</pt>
   <ro>Debian Backports Kernel (32 bit No PAE)</ro>
   <ru>Debian ядро Backports, 32 bit no-PAE</ru>
   <sk>Debian Backports Kernel (32 bit No PAE)</sk>
   <sl>Privzeti Backports Kernel (32 bit No PAE)</sl>
   <sq>Debian Backports Kernel (32 bit No PAE)</sq>
   <sr>Debian Backports Kernel (32 bit No PAE)</sr>
   <sv>Debian Backports Kärna (32 bit No PAE)</sv>
   <tr>Debian Backports Kernel (32 bit No PAE)</tr>
   <uk>Debian Backports Kernel (32 bit No PAE)</uk>
   <vi>Debian Backports Kernel (32 bit No PAE)</vi>
   <zh_CN>Debian Backports Kernel (32 bit No PAE)</zh_CN>
   <zh_TW>Debian Backports Kernel (32 bit No PAE)</zh_TW>
</description>

<installable>
32
</installable>

<screenshot>none</screenshot>

<preinstall>
echo "deb http://http.debian.net/debian bullseye-backports main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
apt-get install -t bullseye-backports linux-image-686 linux-headers-686
</preinstall>

<install_package_names>
linux-image-686
linux-headers-686
</install_package_names>


<postinstall>
file=$(apt-cache show linux-image-686 |grep -m1 Depends) 
file=${file#"Depends:"}
rebuild_dkms_packages.sh $file
rm -f /etc/apt/sources.list.d/mxpitemp.list
apt-get -y remove linux-image-686 linux-headers-686
apt-get update
</postinstall>


<uninstall_package_names>

</uninstall_package_names>
</app>
