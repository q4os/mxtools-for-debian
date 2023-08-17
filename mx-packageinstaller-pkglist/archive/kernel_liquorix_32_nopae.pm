<?xml version="1.0"?>
<app>

<category>
Kernels
</category>

<name>
Liquorix 32 bit no-pae
</name>

<description>
   <am>Liquorix Kernel (no PAE, latest from MX AHS repo)</am>
   <ar>Liquorix Kernel (no PAE, latest from MX AHS repo)</ar>
   <bg>Liquorix Kernel (no PAE, latest from MX AHS repo)</bg>
   <bn>Liquorix Kernel (no PAE, latest from MX AHS repo)</bn>
   <ca>Kernel Liquorix (no PAE, recent del dipòsit MX AHS)</ca>
   <cs>Liquorix Kernel (no PAE, latest from MX AHS repo)</cs>
   <da>Liquorix Kernel (no PAE, latest from MX AHS repo)</da>
   <de>Liquorix Kernel (no PAE, der neueste im MX AHS Repo)</de>
   <el>Liquorix Kernel (χωρίς PAE, τελευταίες από το MX AHS repo)</el>
   <en>Liquorix Kernel (no PAE, latest from MX AHS repo)</en>
   <es>Liquorix Kernel (no PAE, último desde MX AHS repo)</es>
   <et>Liquorix Kernel (no PAE, latest from MX AHS repo)</et>
   <eu>Liquorix Kernel (no PAE, latest from MX AHS repo)</eu>
   <fa>Liquorix Kernel (no PAE, latest from MX AHS repo)</fa>
   <fil_PH>Liquorix Kernel (no PAE, latest from MX AHS repo)</fil_PH>
   <fi>Liquorix Kernel (no PAE, latest from MX AHS repo)</fi>
   <fr>Liquorix Kernel (no PAE, latest from MX AHS repo)</fr>
   <he_IL>Liquorix Kernel (no PAE, latest from MX AHS repo)</he_IL>
   <hi>Liquorix Kernel (no PAE, latest from MX AHS repo)</hi>
   <hr>Liquorix Kernel (no PAE, latest from MX AHS repo)</hr>
   <hu>Liquorix Kernel (no PAE, latest from MX AHS repo)</hu>
   <id>Liquorix Kernel (no PAE, latest from MX AHS repo)</id>
   <is>Liquorix Kernel (no PAE, latest from MX AHS repo)</is>
   <it>Kernel Liquorix (no PAE, ultimo dal repo MX AHS)</it>
   <ja_JP>Liquorix Kernel (no PAE, latest from MX AHS repo)</ja_JP>
   <ja>Liquorix Kernel (no PAE, latest from MX AHS repo)</ja>
   <kk>Liquorix Kernel (no PAE, latest from MX AHS repo)</kk>
   <ko>Liquorix Kernel (no PAE, latest from MX AHS repo)</ko>
   <lt>Liquorix Kernel (no PAE, latest from MX AHS repo)</lt>
   <mk>Liquorix Kernel (no PAE, latest from MX AHS repo)</mk>
   <mr>Liquorix Kernel (no PAE, latest from MX AHS repo)</mr>
   <nb>Liquorix Kernel (no PAE, latest from MX AHS repo)</nb>
   <nl>Liquorix Kernel (no PAE, latest from MX AHS repo)</nl>
   <pl>Liquorix Kernel (no PAE, latest from MX AHS repo)</pl>
   <pt_BR>Liquorix Kernel (no PAE, latest from MX AHS repo)</pt_BR>
   <pt>Núcleo (kernel) Liquorix (no PAE, versão mais recente do MX AHS repo)</pt>
   <ro>Liquorix Kernel (no PAE, latest from MX AHS repo)</ro>
   <ru>Liquorix ядро, no-PAE (последняя версия из репозитория MX AHS)</ru>
   <sk>Liquorix Kernel (no PAE, latest from MX AHS repo)</sk>
   <sl>Liquorix jedro (brez PAE, zadnj iz MX AHS repozitorija)</sl>
   <sq>Liquorix Kernel (no PAE, latest from MX AHS repo)</sq>
   <sr>Liquorix Kernel (no PAE, latest from MX AHS repo)</sr>
   <sv>Liquorix Kärna (no PAE, senaste från MX AHS repo)</sv>
   <tr>Liquorix Kernel (no PAE, latest from MX AHS repo)</tr>
   <uk>Liquorix Kernel (no PAE, latest from MX AHS repo)</uk>
   <vi>Liquorix Kernel (no PAE, latest from MX AHS repo)</vi>
   <zh_CN>Liquorix Kernel (no PAE, latest from MX AHS repo)</zh_CN>
   <zh_TW>Liquorix Kernel (no PAE, latest from MX AHS repo)</zh_TW>
</description>

<installable>
32
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
linux-image-liquorix-686
linux-headers-liquorix-686
</install_package_names>


<postinstall>
file=$(apt-cache show linux-image-liquorix-686 |grep -m1 Depends) 
file=${file#"Depends:"}
rebuild_dkms_packages.sh $file
rm -f /etc/apt/sources.list.d/mxpitemp.list
apt-get -y remove linux-image-liquorix-686 linux-headers-liquorix-686
apt-get update

</postinstall>


<uninstall_package_names>

</uninstall_package_names>
</app>
