<?xml version="1.0"?>
<app>

<category>
Kernels
</category>

<name>
Liquorix 32 bit pae
</name>

<description>
   <am>Liquorix Kernel (PAE, latest from MX TEST repo)</am>
   <ar>Liquorix Kernel (PAE, latest from MX TEST repo)</ar>
   <bg>Liquorix Kernel (PAE, latest from MX TEST repo)</bg>
   <bn>Liquorix Kernel (PAE, latest from MX TEST repo)</bn>
   <ca>Kernel Liquorix (PAE, més recent del dipòsit MX TEST)</ca>
   <cs>Liquorix Kernel (PAE, latest from MX TEST repo)</cs>
   <da>Liquorix Kernel (PAE, latest from MX TEST repo)</da>
   <de>Liquorix Kernel (PAE, der neueste im MX TEST Repo)</de>
   <el>Liquorix Kernel (PAE, τελευταία από το MX TEST repo)</el>
   <en>Liquorix Kernel (PAE, latest from MX TEST repo)</en>
   <es>Liquorix Kernel (PAE, último desde MX TEST repo)</es>
   <et>Liquorix Kernel (PAE, latest from MX TEST repo)</et>
   <eu>Liquorix Kernel (PAE, latest from MX TEST repo)</eu>
   <fa>Liquorix Kernel (PAE, latest from MX TEST repo)</fa>
   <fil_PH>Liquorix Kernel (PAE, latest from MX TEST repo)</fil_PH>
   <fi>Liquorix Kernel (PAE, latest from MX TEST repo)</fi>
   <fr>Liquorix Kernel (PAE, latest from MX TEST repo)</fr>
   <he_IL>Liquorix Kernel (PAE, latest from MX TEST repo)</he_IL>
   <hi>Liquorix Kernel (PAE, latest from MX TEST repo)</hi>
   <hr>Liquorix Kernel (PAE, latest from MX TEST repo)</hr>
   <hu>Liquorix Kernel (PAE, latest from MX TEST repo)</hu>
   <id>Liquorix Kernel (PAE, latest from MX TEST repo)</id>
   <is>Liquorix Kernel (PAE, latest from MX TEST repo)</is>
   <it>Kernel Liquorix (PAE, ultimo dal repo MX TEST)</it>
   <ja_JP>Liquorix Kernel (PAE, latest from MX TEST repo)</ja_JP>
   <ja>Liquorix Kernel (PAE, latest from MX TEST repo)</ja>
   <kk>Liquorix Kernel (PAE, latest from MX TEST repo)</kk>
   <ko>Liquorix Kernel (PAE, latest from MX TEST repo)</ko>
   <lt>Liquorix Kernel (PAE, latest from MX TEST repo)</lt>
   <mk>Liquorix Kernel (PAE, latest from MX TEST repo)</mk>
   <mr>Liquorix Kernel (PAE, latest from MX TEST repo)</mr>
   <nb>Liquorix Kernel (PAE, latest from MX TEST repo)</nb>
   <nl>Liquorix Kernel (PAE, latest from MX TEST repo)</nl>
   <pl>Liquorix Kernel (PAE, latest from MX TEST repo)</pl>
   <pt_BR>Liquorix Kernel (PAE, latest from MX TEST repo)</pt_BR>
   <pt>Núcleo (kernel) (PAE, versão mais recente do MX TEST repo)</pt>
   <ro>Liquorix Kernel (PAE, latest from MX TEST repo)</ro>
   <ru>Liquorix ядро, PAE (последняя версия из репозитория MX TEST)</ru>
   <sk>Liquorix Kernel (PAE, latest from MX TEST repo)</sk>
   <sl>Liquorix jedro (PAE, zadnje iz MX TEST repozitorija)</sl>
   <sq>Liquorix Kernel (PAE, latest from MX TEST repo)</sq>
   <sr>Liquorix Kernel (PAE, latest from MX TEST repo)</sr>
   <sv>Liquorix Kärna (PAE, senaste från MX TEST repo)</sv>
   <tr>Liquorix Kernel (PAE, latest from MX TEST repo)</tr>
   <uk>Liquorix Kernel (PAE, latest from MX TEST repo)</uk>
   <vi>Liquorix Kernel (PAE, latest from MX TEST repo)</vi>
   <zh_CN>Liquorix Kernel (PAE, latest from MX TEST repo)</zh_CN>
   <zh_TW>Liquorix Kernel (PAE, latest from MX TEST repo)</zh_TW>
</description>

<installable>
32
</installable>

<screenshot>none</screenshot>

<preinstall>
echo "deb http://mxrepo.com/mx/testrepo/ bullseye test">/etc/apt/sources.list.d/mxpitemp.list
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
