<?xml version="1.0"?>
<app>

<category>
Kernels
</category>

<name>
Liquorix 32 bit pae
</name>

<description>
   <am>Liquorix Kernel (PAE, latest from MX AHS repo)</am>
   <ar>Liquorix Kernel (PAE, latest from MX AHS repo)</ar>
   <bg>Liquorix Kernel (PAE, latest from MX AHS repo)</bg>
   <bn>Liquorix Kernel (PAE, latest from MX AHS repo)</bn>
   <ca>Kernel Liquorix (PAE, més recent del dipòsit MX AHS)</ca>
   <cs>Liquorix Kernel (PAE, latest from MX AHS repo)</cs>
   <da>Liquorix Kernel (PAE, latest from MX AHS repo)</da>
   <de>Liquorix Kernel (PAE, der neueste im MX AHS Repo)</de>
   <el>Liquorix Kernel (PAE, τελευταία από το MX AHS repo)</el>
   <en>Liquorix Kernel (PAE, latest from MX AHS repo)</en>
   <es>Liquorix Kernel (PAE, último desde MX AHS repo)</es>
   <et>Liquorix Kernel (PAE, latest from MX AHS repo)</et>
   <eu>Liquorix Kernel (PAE, latest from MX AHS repo)</eu>
   <fa>Liquorix Kernel (PAE, latest from MX AHS repo)</fa>
   <fil_PH>Liquorix Kernel (PAE, latest from MX AHS repo)</fil_PH>
   <fi>Liquorix Kernel (PAE, latest from MX AHS repo)</fi>
   <fr>Liquorix Kernel (PAE, latest from MX AHS repo)</fr>
   <he_IL>Liquorix Kernel (PAE, latest from MX AHS repo)</he_IL>
   <hi>Liquorix Kernel (PAE, latest from MX AHS repo)</hi>
   <hr>Liquorix Kernel (PAE, latest from MX AHS repo)</hr>
   <hu>Liquorix Kernel (PAE, latest from MX AHS repo)</hu>
   <id>Liquorix Kernel (PAE, latest from MX AHS repo)</id>
   <is>Liquorix Kernel (PAE, latest from MX AHS repo)</is>
   <it>Kernel Liquorix (PAE, ultimo dal repo MX AHS)</it>
   <ja_JP>Liquorix Kernel (PAE, latest from MX AHS repo)</ja_JP>
   <ja>Liquorix Kernel (PAE, latest from MX AHS repo)</ja>
   <kk>Liquorix Kernel (PAE, latest from MX AHS repo)</kk>
   <ko>Liquorix Kernel (PAE, latest from MX AHS repo)</ko>
   <lt>Liquorix Kernel (PAE, latest from MX AHS repo)</lt>
   <mk>Liquorix Kernel (PAE, latest from MX AHS repo)</mk>
   <mr>Liquorix Kernel (PAE, latest from MX AHS repo)</mr>
   <nb>Liquorix Kernel (PAE, latest from MX AHS repo)</nb>
   <nl>Liquorix Kernel (PAE, latest from MX AHS repo)</nl>
   <pl>Liquorix Kernel (PAE, latest from MX AHS repo)</pl>
   <pt_BR>Liquorix Kernel (PAE, latest from MX AHS repo)</pt_BR>
   <pt>Núcleo (kernel) (PAE, versão mais recente do MX AHS repo)</pt>
   <ro>Liquorix Kernel (PAE, latest from MX AHS repo)</ro>
   <ru>Liquorix ядро, PAE (последняя версия из репозитория MX AHS)</ru>
   <sk>Liquorix Kernel (PAE, latest from MX AHS repo)</sk>
   <sl>Liquorix jedro (PAE, zadnje iz MX AHS repozitorija)</sl>
   <sq>Liquorix Kernel (PAE, latest from MX AHS repo)</sq>
   <sr>Liquorix Kernel (PAE, latest from MX AHS repo)</sr>
   <sv>Liquorix Kärna (PAE, senaste från MX AHS repo)</sv>
   <tr>Liquorix Kernel (PAE, latest from MX AHS repo)</tr>
   <uk>Liquorix Kernel (PAE, latest from MX AHS repo)</uk>
   <vi>Liquorix Kernel (PAE, latest from MX AHS repo)</vi>
   <zh_CN>Liquorix Kernel (PAE, latest from MX AHS repo)</zh_CN>
   <zh_TW>Liquorix Kernel (PAE, latest from MX AHS repo)</zh_TW>
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
linux-image-liquorix-686-pae
linux-headers-liquorix-686-pae
</install_package_names>


<postinstall>
file=$(apt-cache show linux-image-liquorix-686-pae |grep -m1 Depends) 
file=${file#"Depends:"}
rebuild_dkms_packages.sh $file
rm -f /etc/apt/sources.list.d/mxpitemp.list
apt-get -y remove linux-image-liquorix-686-pae linux-headers-liquorix-686-pae
apt-get update
</postinstall>


<uninstall_package_names>

</uninstall_package_names>
</app>
