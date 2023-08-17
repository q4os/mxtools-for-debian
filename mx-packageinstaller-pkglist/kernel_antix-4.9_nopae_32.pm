<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
antiX 4.9 32 bit no-pae
</name>

<description>
   <am>antiX 4.9.326 kernel, 32 bit no-pae</am>
   <ar>antiX 4.9.326 kernel, 32 bit no-pae</ar>
   <bg>antiX 4.9.326 kernel, 32 bit no-pae</bg>
   <bn>antiX 4.9.326 kernel, 32 bit no-pae</bn>
   <ca>Kernel antiX 4.9.326, 32 bit no-PAE</ca>
   <cs>antiX 4.9.326 kernel, 32 bit no-pae</cs>
   <da>antiX 4.9.326 kernel, 32 bit no-pae</da>
   <de>antiX 4.9.326 kernel, 32 bit no-pae</de>
   <el>πυρήνα antiX 4.9.326, 32 bit no-pae</el>
   <en>antiX 4.9.326 kernel, 32 bit no-pae</en>
   <es>antiX 4.9.326 kernel, 32 bit no-pae</es>
   <et>antiX 4.9.326 kernel, 32 bit no-pae</et>
   <eu>antiX 4.9.326 kernel, 32 bit no-pae</eu>
   <fa>antiX 4.9.326 kernel, 32 bit no-pae</fa>
   <fil_PH>antiX 4.9.326 kernel, 32 bit no-pae</fil_PH>
   <fi>antiX 4.9.326 kernel, 32 bit no-pae</fi>
   <fr>antiX 4.9.326 kernel, 32 bit no-pae</fr>
   <he_IL>antiX 4.9.326 kernel, 32 bit no-pae</he_IL>
   <hi>antiX 4.9.326 kernel, 32 bit no-pae</hi>
   <hr>antiX 4.9.326 kernel, 32 bit no-pae</hr>
   <hu>antiX 4.9.326 kernel, 32 bit no-pae</hu>
   <id>antiX 4.9.326 kernel, 32 bit no-pae</id>
   <is>antiX 4.9.326 kernel, 32 bit no-pae</is>
   <it>Kernel antiX 4.9.326, 32 bit no-pae</it>
   <ja_JP>antiX 4.9.326 kernel, 32 bit no-pae</ja_JP>
   <ja>antiX 4.9.326 kernel, 32 bit no-pae</ja>
   <kk>antiX 4.9.326 kernel, 32 bit no-pae</kk>
   <ko>antiX 4.9.326 kernel, 32 bit no-pae</ko>
   <lt>antiX 4.9.326 kernel, 32 bit no-pae</lt>
   <mk>antiX 4.9.326 kernel, 32 bit no-pae</mk>
   <mr>antiX 4.9.326 kernel, 32 bit no-pae</mr>
   <nb>antiX 4.9.326 kernel, 32 bit no-pae</nb>
   <nl>antiX 4.9.326 kernel, 32 bit no-pae</nl>
   <pl>antiX 4.9.326 kernel, 32 bit no-pae</pl>
   <pt_BR>antiX 4.9.326 kernel, 32 bit no-pae</pt_BR>
   <pt>Núcleo (kernel) antiX 4.9.326, 32 bit no-pae</pt>
   <ro>antiX 4.9.326 kernel, 32 bit no-pae</ro>
   <ru>antiX 4.9.326 kernel, 32 bit no-pae</ru>
   <sk>antiX 4.9.326 kernel, 32 bit no-pae</sk>
   <sl>antiX 4.9.326 jedro, 32 bitno no-pae</sl>
   <sq>antiX 4.9.326 kernel, 32 bit no-pae</sq>
   <sr>antiX 4.9.326 kernel, 32 bit no-pae</sr>
   <sv>antiX 4.9.326 kärna 32 bit no-pae</sv>
   <tr>antiX 4.9.326 kernel, 32 bit no-pae</tr>
   <uk>antiX 4.9.326 kernel, 32 bit no-pae</uk>
   <vi>antiX 4.9.326 kernel, 32 bit no-pae</vi>
   <zh_CN>antiX 4.9.326 kernel, 32 bit no-pae</zh_CN>
   <zh_TW>antiX 4.9.326 kernel, 32 bit no-pae</zh_TW>
</description>

<installable>
32
</installable>

<screenshot>none</screenshot>

<preinstall>
apt-get install antix-archive-keyring
echo "deb http://la.mxrepo.com/antix/bookworm bookworm main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
</preinstall>

<install_package_names>
linux-image-4.9.0-326-antix.1-486-smp
linux-headers-4.9.0-326-antix.1-486-smp
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
apt-get update
rebuild_dkms_packages.sh linux-image-4.9.0-326-antix.1-486-smp
</postinstall>


<uninstall_package_names>
linux-image-4.9.0-326-antix.1-486-smp
linux-headers-4.9.0-326-antix.1-486-smp
</uninstall_package_names>
</app>
