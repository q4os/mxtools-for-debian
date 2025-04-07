<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
antiX 5.10 32 bit pae
</name>

<description>
   <am>antiX 5.10.234 kernel pae 32 bit</am>
   <ar>antiX 5.10.234 kernel pae 32 bit</ar>
   <bg>antiX 5.10.234 kernel pae 32 bit</bg>
   <bn>antiX 5.10.234 kernel pae 32 bit</bn>
   <ca>Kernel antiX 5.10.234 PAE 32 bit</ca>
   <cs>antiX 5.10.234 kernel pae 32 bit</cs>
   <da>antiX 5.10.234 kernel pae 32 bit</da>
   <de>antiX 5.10.234 Kernel pae 32 bit</de>
   <el>antiX 5.10.234 πυρήνα pae 32 bit</el>
   <en>antiX 5.10.234 kernel pae 32 bit</en>
   <es>antiX 5.10.234 kernel pae 32 bit</es>
   <et>antiX 5.10.234 kernel pae 32 bit</et>
   <eu>antiX 5.10.234 kernel pae 32 bit</eu>
   <fa>antiX 5.10.234 kernel pae 32 bit</fa>
   <fil_PH>antiX 5.10.234 kernel pae 32 bit</fil_PH>
   <fi>antiX 5.10.234 kernel pae 32 bit</fi>
   <fr>antiX 5.10.234 kernel pae 32 bit</fr>
   <he_IL>antiX 5.10.234 kernel pae 32 bit</he_IL>
   <hi>antiX 5.10.234 kernel pae 32 bit</hi>
   <hr>antiX 5.10.234 kernel pae 32 bit</hr>
   <hu>antiX 5.10.234 kernel pae 32 bit</hu>
   <id>antiX 5.10.234 kernel pae 32 bit</id>
   <is>antiX 5.10.234 kernel pae 32 bit</is>
   <it>Kernel antiX 5.10.234, 32 bit pae</it>
   <ja_JP>antiX 5.10.234 kernel pae 32 bit</ja_JP>
   <ja>antiX 5.10.234 kernel pae 32 bit</ja>
   <kk>antiX 5.10.234 kernel pae 32 bit</kk>
   <ko>antiX 5.10.234 kernel pae 32 bit</ko>
   <lt>antiX 5.10.234 kernel pae 32 bit</lt>
   <mk>antiX 5.10.234 kernel pae 32 bit</mk>
   <mr>antiX 5.10.234 kernel pae 32 bit</mr>
   <nb>antiX 5.10.234 kernel pae 32 bit</nb>
   <nl>antiX 5.10.234 kernel pae 32 bit</nl>
   <pl>antiX 5.10.234 kernel pae 32 bit</pl>
   <pt_BR>antiX 5.10.234 kernel pae 32 bit</pt_BR>
   <pt>Núcleo (kernel) antiX 5.10.234 pae 32 bit</pt>
   <ro>antiX 5.10.234 kernel pae 32 bit</ro>
   <ru>antiX 5.10.234 kernel pae 32 bit</ru>
   <sk>antiX 5.10.234 kernel pae 32 bit</sk>
   <sl>antiX 5.10.234 jedro pae 32 bitno</sl>
   <sq>antiX 5.10.234 kernel pae 32 bit</sq>
   <sr>antiX 5.10.234 kernel pae 32 bit</sr>
   <sv>antiX 5.10.234 kärna pae 32 bit</sv>
   <tr>antiX 5.10.234 kernel pae 32 bit</tr>
   <uk>antiX 5.10.234 kernel pae 32 bit</uk>
   <vi>antiX 5.10.234 kernel pae 32 bit</vi>
   <zh_CN>antiX 5.10.234 kernel pae 32 bit</zh_CN>
   <zh_TW>antiX 5.10.234 kernel pae 32 bit</zh_TW>
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
linux-image-5.10.234-antix.2-686-smp-pae
linux-headers-5.10.234-antix.2-686-smp-pae
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
apt-get update
rebuild_dkms_packages.sh linux-image-5.10.234-antix.2-686-smp-pae
</postinstall>


<uninstall_package_names>
linux-image-5.10.234-antix.2-686-smp-pae
linux-headers-5.10.234-antix.2-686-smp-pae
</uninstall_package_names>
</app>
