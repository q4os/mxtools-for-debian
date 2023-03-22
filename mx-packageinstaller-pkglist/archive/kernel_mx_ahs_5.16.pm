<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
Debian 5.16 64 bit (AHS)
</name>

<description>
   <am>Debian 5.16, 64 bit latest from MX repo</am>
   <ar>Debian 5.16, 64 bit latest from MX repo</ar>
   <bg>Debian 5.16, 64 bit latest from MX repo</bg>
   <bn>Debian 5.16, 64 bit latest from MX repo</bn>
   <ca>Debian 5.16, 64 bit, més recent del dipòsit MX</ca>
   <cs>Debian 5.16, 64 bit latest from MX repo</cs>
   <da>Debian 5.16, 64 bit latest from MX repo</da>
   <de>Debian 5.16, 64 bit, der neueste im MX Repo</de>
   <el>Debian 5.16, 64 bit πιο πρόσφατο από το MX repo</el>
   <en>Debian 5.16, 64 bit latest from MX repo</en>
   <es>Debian 5.16, 64 bit último desde MX repo</es>
   <et>Debian 5.16, 64 bit latest from MX repo</et>
   <eu>Debian 5.16, 64 bit latest from MX repo</eu>
   <fa>Debian 5.16, 64 bit latest from MX repo</fa>
   <fil_PH>Debian 5.16, 64 bit latest from MX repo</fil_PH>
   <fi>Debian 5.16, 64 bit latest from MX repo</fi>
   <fr>Debian 5.16, 64 bit latest from MX repo</fr>
   <he_IL>Debian 5.16, 64 bit latest from MX repo</he_IL>
   <hi>Debian 5.16, 64 bit latest from MX repo</hi>
   <hr>Debian 5.16, 64 bit latest from MX repo</hr>
   <hu>Debian 5.16, 64 bit latest from MX repo</hu>
   <id>Debian 5.16, 64 bit latest from MX repo</id>
   <is>Debian 5.16, 64 bit latest from MX repo</is>
   <it>Debian 5.16, 64 bit ultimo dal MX ahs</it>
   <ja_JP>Debian 5.16, 64 bit latest from MX repo</ja_JP>
   <ja>Debian 5.16, 64 bit latest from MX repo</ja>
   <kk>Debian 5.16, 64 bit latest from MX repo</kk>
   <ko>Debian 5.16, 64 bit latest from MX repo</ko>
   <lt>Debian 5.16, 64 bit latest from MX repo</lt>
   <mk>Debian 5.16, 64 bit latest from MX repo</mk>
   <mr>Debian 5.16, 64 bit latest from MX repo</mr>
   <nb>Debian 5.16, 64 bit latest from MX repo</nb>
   <nl>Debian 5.16, 64 bit latest from MX repo</nl>
   <pl>Debian 5.16, 64 bit latest from MX repo</pl>
   <pt_BR>Debian 5.16, 64 bit latest from MX repo</pt_BR>
   <pt>Debian 5.16, 64 bit versão mais recente do MX repo</pt>
   <ro>Debian 5.16, 64 bit latest from MX repo</ro>
   <ru>Debian 5.16 64 bit (актуальное из репозитория MX)</ru>
   <sk>Debian 5.16, 64 bit latest from MX repo</sk>
   <sl>Debian 5.16, 64 bitno, zadnje iz MX repozitorija</sl>
   <sq>Debian 5.16, 64 bit latest from MX repo</sq>
   <sr>Debian 5.16, 64 bit latest from MX repo</sr>
   <sv>Debian 5.16, 64 bit senaste från MX repo</sv>
   <tr>Debian 5.16, 64 bit latest from MX repo</tr>
   <uk>Debian 5.16, 64 bit latest from MX repo</uk>
   <vi>Debian 5.16, 64 bit latest from MX repo</vi>
   <zh_CN>Debian 5.16, 64 bit latest from MX repo</zh_CN>
   <zh_TW>Debian 5.16, 64 bit latest from MX repo</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
echo "deb http://mxrepo.com/mx/repo/ bullseye ahs">/etc/apt/sources.list.d/mxpitemp.list
apt-get update 
</preinstall>

<install_package_names>
linux-image-5.16.0-6mx-amd64-unsigned
linux-headers-5.16.0-6mx-amd64
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
apt-get update
rebuild_dkms_packages.sh linux-image-5.16.0-6mx-amd64-unsigned
</postinstall>


<uninstall_package_names>
linux-image-5.16.0-6mx-amd64-unsigned
linux-headers-5.16.0-6mx-amd64
</uninstall_package_names>
</app>
