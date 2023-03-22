<?xml version="1.0"?>
<app>

<category>
Kernels
</category>

<name>
Debian-Backports 64 bit
</name>

<description>
   <am>Debian Backports kernel Meltdown patched, 64 bit</am>
   <ar>Debian Backports kernel Meltdown patched, 64 bit</ar>
   <bg>Debian Backports kernel Meltdown patched, 64 bit</bg>
   <bn>Debian Backports kernel Meltdown patched, 64 bit</bn>
   <ca>Kernel de Debian Backports 64 bit, amb pedaç per Meltdown</ca>
   <cs>Debian Backports kernel Meltdown patched, 64 bit</cs>
   <da>Debian backports kerne meltdown-rettelse, 64 bit</da>
   <de>Debian Backports-Kernel Meltdown gepatcht, 64 Bit</de>
   <el>Ο πυρήνας του Debian Backports Meltdown patched, 64 bit</el>
   <en>Debian Backports kernel Meltdown patched, 64 bit</en>
   <es>Debian Backports kernel con parche Meltdown, 64 bit</es>
   <et>Debian Backports kernel Meltdown patched, 64 bit</et>
   <eu>Debian Backports kernel Meltdown patched, 64 bit</eu>
   <fa>Debian Backports kernel Meltdown patched, 64 bit</fa>
   <fil_PH>Debian Backports kernel Meltdown patched, 64 bit</fil_PH>
   <fi>Debian Backports kernel Meltdown patched, 64 bit</fi>
   <fr>Noyau Debian Backports correctifs Meltdown, 64 bit</fr>
   <he_IL>Debian Backports kernel Meltdown patched, 64 bit</he_IL>
   <hi>Debian Backports kernel Meltdown patched, 64 bit</hi>
   <hr>Debian Backports kernel Meltdown patched, 64 bit</hr>
   <hu>Debian Backports kernel Meltdown patched, 64 bit</hu>
   <id>Debian Backports kernel Meltdown patched, 64 bit</id>
   <is>Debian Backports kernel Meltdown patched, 64 bit</is>
   <it>Debian Backports kernel con la patch per Meltdown, 64 bit</it>
   <ja_JP>Debian Backports kernel Meltdown patched, 64 bit</ja_JP>
   <ja>Debian Backports kernel Meltdown patched, 64 bit</ja>
   <kk>Debian Backports kernel Meltdown patched, 64 bit</kk>
   <ko>Debian Backports kernel Meltdown patched, 64 bit</ko>
   <lt>Debian Backports kernel Meltdown patched, 64 bit</lt>
   <mk>Debian Backports kernel Meltdown patched, 64 bit</mk>
   <mr>Debian Backports kernel Meltdown patched, 64 bit</mr>
   <nb>Debian Backports kernel Meltdown patched, 64 bit</nb>
   <nl>Debian Backports kernel Meltdown patched, 64 bit</nl>
   <pl>Debian Backports kernel Meltdown patched, 64 bit</pl>
   <pt_BR>Debian Backports kernel Meltdown patched, 64 bit</pt_BR>
   <pt>Núcleo (kernel) do Debian Backports com correcção para a falha Meltdown, 64 bit</pt>
   <ro>Debian Backports kernel Meltdown patched, 64 bit</ro>
   <ru>Debian ядро Backports с патчем от Meltdown, 64 bit</ru>
   <sk>Debian Backports kernel Meltdown patched, 64 bit</sk>
   <sl>Debian Backports kernel Meltdown patched, 64 bit</sl>
   <sq>Debian Backports kernel Meltdown patched, 64 bit</sq>
   <sr>Debian Backports kernel Meltdown patched, 64 bit</sr>
   <sv>Debian Backports kärna Meltdown patched, 64 bit</sv>
   <tr>Debian Backports kernel Meltdown patched, 64 bit</tr>
   <uk>Debian Backports kernel Meltdown patched, 64 bit</uk>
   <vi>Debian Backports kernel Meltdown patched, 64 bit</vi>
   <zh_CN>Debian Backports kernel Meltdown patched, 64 bit</zh_CN>
   <zh_TW>Debian Backports kernel Meltdown patched, 64 bit</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
echo "deb http://http.debian.net/debian bullseye-backports main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
apt-get install -t bullseye-backports linux-image-amd64 linux-headers-amd64
</preinstall>

<install_package_names>
linux-image-amd64
linux-headers-amd64
</install_package_names>


<postinstall>
file=$(apt-cache show linux-image-amd64 |grep -m1 Depends) 
file=${file#"Depends:"}
rebuild_dkms_packages.sh $file
rm -f /etc/apt/sources.list.d/mxpitemp.list
apt-get -y remove linux-image-amd64 linux-headers-amd64
apt-get update
</postinstall>


<uninstall_package_names>

</uninstall_package_names>
</app>
