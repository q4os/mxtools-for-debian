<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
antix 4.9 32 bit pae
</name>

<description>
   <am>antiX 4.9.326 kernel, 32 bit pae</am>
   <ar>antiX 4.9.326 kernel, 32 bit pae</ar>
   <bg>antiX 4.9.326 kernel, 32 bit pae</bg>
   <bn>antiX 4.9.326 kernel, 32 bit pae</bn>
   <ca>Kernel antiX 4.9.326, 32 bit PAE</ca>
   <cs>antiX 4.9.326 kernel, 32 bit pae</cs>
   <da>antiX 4.9.326 kernel, 32 bit pae</da>
   <de>antiX 4.9.326 kernel, 32 bit pae</de>
   <el>πυρήνας antiX 4.9.326, 32 bit pae</el>
   <en>antiX 4.9.326 kernel, 32 bit pae</en>
   <es>antiX 4.9.326 kernel, 32 bit pae</es>
   <et>antiX 4.9.326 kernel, 32 bit pae</et>
   <eu>antiX 4.9.326 kernel, 32 bit pae</eu>
   <fa>antiX 4.9.326 kernel, 32 bit pae</fa>
   <fil_PH>antiX 4.9.326 kernel, 32 bit pae</fil_PH>
   <fi>antiX 4.9.326 kernel, 32 bit pae</fi>
   <fr>antiX 4.9.326 kernel, 32 bit pae</fr>
   <he_IL>antiX 4.9.326 kernel, 32 bit pae</he_IL>
   <hi>antiX 4.9.326 kernel, 32 bit pae</hi>
   <hr>antiX 4.9.326 kernel, 32 bit pae</hr>
   <hu>antiX 4.9.326 kernel, 32 bit pae</hu>
   <id>antiX 4.9.326 kernel, 32 bit pae</id>
   <is>antiX 4.9.326 kernel, 32 bit pae</is>
   <it>Kernel antiX 4.9.326, 32 bit pae</it>
   <ja_JP>antiX 4.9.326 kernel, 32 bit pae</ja_JP>
   <ja>antiX 4.9.326 kernel, 32 bit pae</ja>
   <kk>antiX 4.9.326 kernel, 32 bit pae</kk>
   <ko>antiX 4.9.326 kernel, 32 bit pae</ko>
   <lt>antiX 4.9.326 kernel, 32 bit pae</lt>
   <mk>antiX 4.9.326 kernel, 32 bit pae</mk>
   <mr>antiX 4.9.326 kernel, 32 bit pae</mr>
   <nb>antiX 4.9.326 kernel, 32 bit pae</nb>
   <nl>antiX 4.9.326 kernel, 32 bit pae</nl>
   <pl>antiX 4.9.326 kernel, 32 bit pae</pl>
   <pt_BR>antiX 4.9.326 kernel, 32 bit pae</pt_BR>
   <pt>Núcleo (kernel) antiX 4.9.326, 32 bit pae</pt>
   <ro>antiX 4.9.326 kernel, 32 bit pae</ro>
   <ru>antiX 4.9.326 kernel, 32 bit pae</ru>
   <sk>antiX 4.9.326 kernel, 32 bit pae</sk>
   <sl>antiX 4.9.326 jedro, 32 bitno pae</sl>
   <sq>antiX 4.9.326 kernel, 32 bit pae</sq>
   <sr>antiX 4.9.326 kernel, 32 bit pae</sr>
   <sv>antiX 4.9.326 kärna, 32 bit pae</sv>
   <tr>antiX 4.9.326 kernel, 32 bit pae</tr>
   <uk>antiX 4.9.326 kernel, 32 bit pae</uk>
   <vi>antiX 4.9.326 kernel, 32 bit pae</vi>
   <zh_CN>antiX 4.9.326 kernel, 32 bit pae</zh_CN>
   <zh_TW>antiX 4.9.326 kernel, 32 bit pae</zh_TW>
</description>

<installable>
32
</installable>

<screenshot>none</screenshot>

<preinstall>
apt-get install antix-archive-keyring
echo "deb http://la.mxrepo.com/antix/bullseye bullseye main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
</preinstall>

<install_package_names>
linux-image-4.9.0-326-antix.1-686-smp-pae
linux-headers-4.9.0-326-antix.1-686-smp-pae
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
apt-get update
rebuild_dkms_packages.sh linux-image-4.9.0-326-antix.1-686-smp-pae
</postinstall>


<uninstall_package_names>
linux-image-4.9.0-326-antix.1-686-smp-pae
linux-headers-4.9.0-326-antix.1-686-smp-pae
</uninstall_package_names>
</app>
