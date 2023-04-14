<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
Debian 6.1 64 bit (AHS)
</name>

<description>
   <am>Debian 6.1, 64 bit latest from MX repo</am>
   <ar>Debian 6.1, 64 bit latest from MX repo</ar>
   <bg>Debian 6.1, 64 bit latest from MX repo</bg>
   <bn>Debian 6.1, 64 bit latest from MX repo</bn>
   <ca>Debian 6.1, 64 bit, més recent del dipòsit MX</ca>
   <cs>Debian 6.1, 64 bit latest from MX repo</cs>
   <da>Debian 6.1, 64 bit latest from MX repo</da>
   <de>Debian 6.1, 64 bit, der neueste im MX Repo</de>
   <el>Debian 6.1, 64 bit πιο πρόσφατο από το MX repo</el>
   <en>Debian 6.1, 64 bit latest from MX repo</en>
   <es>Debian 6.1, 64 bit último desde MX repo</es>
   <et>Debian 6.1, 64 bit latest from MX repo</et>
   <eu>Debian 6.1, 64 bit latest from MX repo</eu>
   <fa>Debian 6.1, 64 bit latest from MX repo</fa>
   <fil_PH>Debian 6.1, 64 bit latest from MX repo</fil_PH>
   <fi>Debian 6.1, 64 bit latest from MX repo</fi>
   <fr>Debian 6.1, 64 bit latest from MX repo</fr>
   <he_IL>Debian 6.1, 64 bit latest from MX repo</he_IL>
   <hi>Debian 6.1, 64 bit latest from MX repo</hi>
   <hr>Debian 6.1, 64 bit latest from MX repo</hr>
   <hu>Debian 6.1, 64 bit latest from MX repo</hu>
   <id>Debian 6.1, 64 bit latest from MX repo</id>
   <is>Debian 6.1, 64 bit latest from MX repo</is>
   <it>Debian 6.1, 64 bit ultimo dal MX ahs</it>
   <ja_JP>Debian 6.1, 64 bit latest from MX repo</ja_JP>
   <ja>Debian 6.1, 64 bit latest from MX repo</ja>
   <kk>Debian 6.1, 64 bit latest from MX repo</kk>
   <ko>Debian 6.1, 64 bit latest from MX repo</ko>
   <lt>Debian 6.1, 64 bit latest from MX repo</lt>
   <mk>Debian 6.1, 64 bit latest from MX repo</mk>
   <mr>Debian 6.1, 64 bit latest from MX repo</mr>
   <nb>Debian 6.1, 64 bit latest from MX repo</nb>
   <nl>Debian 6.1, 64 bit latest from MX repo</nl>
   <pl>Debian 6.1, 64 bit latest from MX repo</pl>
   <pt_BR>Debian 6.1, 64 bit latest from MX repo</pt_BR>
   <pt>Debian 6.1, 64 bit versão mais recente do MX repo</pt>
   <ro>Debian 6.1, 64 bit latest from MX repo</ro>
   <ru>Debian 6.1 64 bit (актуальное из репозитория MX)</ru>
   <sk>Debian 6.1, 64 bit latest from MX repo</sk>
   <sl>Debian 6.1, 64 bitno, zadnje iz MX repozitorija</sl>
   <sq>Debian 6.1, 64 bit latest from MX repo</sq>
   <sr>Debian 6.1, 64 bit latest from MX repo</sr>
   <sv>Debian 6.1, 64 bit senaste från MX repo</sv>
   <tr>Debian 6.1, 64 bit latest from MX repo</tr>
   <uk>Debian 6.1, 64 bit latest from MX repo</uk>
   <vi>Debian 6.1, 64 bit latest from MX repo</vi>
   <zh_CN>Debian 6.1, 64 bit latest from MX repo</zh_CN>
   <zh_TW>Debian 6.1, 64 bit latest from MX repo</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
[ $(apt-get update --print-uris | grep -c -m1 -E "/mx/repo/dists/bullseye/ahs/") = 0 ] || exit 0
MXREPO=$(apt-get update --print-uris | grep -m1 -oE "https?://.*/mx/repo/dists/bullseye/main" | tail -1 | sed "s:^:deb :; s:/repo/dists/:/repo/ :; s:/main: ahs:")
: ${MXREPO:=deb http://mxrepo.com/mx/repo/ bullseye ahs}
echo "$MXREPO" > /etc/apt/sources.list.d/mxpitemp.list
apt-get update 
</preinstall>

<install_package_names>
linux-image-6.1.0-7mx-ahs-amd64-unsigned
linux-headers-6.1.0-7mx-ahs-amd64
</install_package_names>


<postinstall>
if [ -f /etc/apt/sources.list.d/mxpitemp.list ]; then
rm /etc/apt/sources.list.d/mxpitemp.list
apt-get update
fi
rebuild_dkms_packages.sh linux-image-6.1.0-7mx-ahs-amd64-unsigned
</postinstall>


<uninstall_package_names>
linux-image-6.1.0-7mx-ahs-amd64-unsigned
linux-headers-6.1.0-7mx-ahs-amd64
</uninstall_package_names>
</app>
