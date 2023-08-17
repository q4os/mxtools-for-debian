<?xml version="1.0"?>
<app>

<category>
Kernels
</category>

<name>
Liquorix 64 bit
</name>

<description>
   <am>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</am>
   <ar>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</ar>
   <bg>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</bg>
   <bn>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</bn>
   <ca>Kernel Liquorix esmenat per Meltdown, 64 bit, més recent del dipòsit MX AHS</ca>
   <cs>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</cs>
   <da>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</da>
   <de>Liquorix Kernel Meltdown gepatcht, 64 bit, der neueste im MX AHS Repo</de>
   <el>Liquorix πυρήνα Meltdown patched, 64 bit πιο πρόσφατο από το MX AHS repo</el>
   <en>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</en>
   <es>Liquorix kernel Meltdown parcheado, 64 bit último desde MX AHS repo</es>
   <et>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</et>
   <eu>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</eu>
   <fa>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</fa>
   <fil_PH>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</fil_PH>
   <fi>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</fi>
   <fr>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</fr>
   <he_IL>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</he_IL>
   <hi>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</hi>
   <hr>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</hr>
   <hu>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</hu>
   <id>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</id>
   <is>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</is>
   <it>Kernel Liquorix con la patch per Meltdown, 64 bit ultimo dal repo MX AHS</it>
   <ja_JP>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</ja_JP>
   <ja>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</ja>
   <kk>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</kk>
   <ko>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</ko>
   <lt>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</lt>
   <mk>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</mk>
   <mr>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</mr>
   <nb>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</nb>
   <nl>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</nl>
   <pl>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</pl>
   <pt_BR>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</pt_BR>
   <pt>Núcleo (kernel) Liquorix com correcção para a falha Meltdown, 64 bit versão mais recente do MX AHS repo</pt>
   <ro>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</ro>
   <ru>Liquorix ядро с патчем от Meltdown 64 bit (актуальное из репозитория MX AHS)</ru>
   <sk>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</sk>
   <sl>Liquorix Meltdown jedro - zakrpano, 64 bitno, zadnje iz MX AHS repozitorija</sl>
   <sq>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</sq>
   <sr>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</sr>
   <sv>Liquorix kärna Meltdown fixad, 64 bit senaste från MX AHS repo</sv>
   <tr>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</tr>
   <uk>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</uk>
   <vi>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</vi>
   <zh_CN>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</zh_CN>
   <zh_TW>Liquorix kernel Meltdown patched, 64 bit latest from MX AHS repo</zh_TW>
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
linux-image-liquorix-amd64
linux-headers-liquorix-amd64
</install_package_names>


<postinstall>
file=$(apt-cache show linux-image-liquorix-amd64 |grep -m1 Depends) 
file=${file#"Depends:"}
rebuild_dkms_packages.sh $file
rm -f /etc/apt/sources.list.d/mxpitemp.list
apt-get -y remove linux-image-liquorix-amd64 linux-headers-liquorix-amd64
apt-get update
</postinstall>


<uninstall_package_names>

</uninstall_package_names>
</app>
