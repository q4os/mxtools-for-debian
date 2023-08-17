<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
antiX 4.19 pae 32 bit
</name>

<description>
   <am>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</am>
   <ar>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</ar>
   <bg>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</bg>
   <bn>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</bn>
   <ca>Kernel antiX 4.19.276 esmenat per Meltdown i Spectre, pae 32 bit</ca>
   <cs>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</cs>
   <da>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</da>
   <de>antiX 4.19.276 Kernel Meltdown und Spectre gepatcht, pae 32 bit</de>
   <el>antiX 4.19.276 πυρήνα Meltdown και Spectre patched, pae 32 bit</el>
   <en>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</en>
   <es>antiX 4.19.276 kernel Meltdown y Spectre parcheados, pae 32 bit</es>
   <et>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</et>
   <eu>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</eu>
   <fa>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</fa>
   <fil_PH>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</fil_PH>
   <fi>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</fi>
   <fr>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</fr>
   <he_IL>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</he_IL>
   <hi>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</hi>
   <hr>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</hr>
   <hu>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</hu>
   <id>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</id>
   <is>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</is>
   <it>Kernel antiX 4.19.276, con patch per Meltdown e Spectre, pae 32 bit</it>
   <ja_JP>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</ja_JP>
   <ja>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</ja>
   <kk>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</kk>
   <ko>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</ko>
   <lt>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</lt>
   <mk>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</mk>
   <mr>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</mr>
   <nb>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</nb>
   <nl>antiX 4.19.276 kernel Meltdown en Spectre patched, pae 32 bit</nl>
   <pl>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</pl>
   <pt_BR>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</pt_BR>
   <pt>Núcleo (kernel) antiX 4.19.276 com correcções para as falhas Meltdown e Spectre, pae 32 bit</pt>
   <ro>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</ro>
   <ru>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</ru>
   <sk>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</sk>
   <sl>antiX 4.19.276 Meltdownjedro in Spectre zakrpano, pae 32 bitno</sl>
   <sq>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</sq>
   <sr>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</sr>
   <sv>antiX 4.19.276 kärna Meltdown och Spectre fixad, pae 32 bit</sv>
   <tr>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</tr>
   <uk>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</uk>
   <vi>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</vi>
   <zh_CN>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</zh_CN>
   <zh_TW>antiX 4.19.276 kernel Meltdown and Spectre patched, pae 32 bit</zh_TW>
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
linux-image-4.19.276-antix.1-686-smp-pae
linux-headers-4.19.276-antix.1-686-smp-pae
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
rebuild_dkms_packages.sh linux-image-4.19.276-antix.1-686-smp-pae
apt-get update
</postinstall>


<uninstall_package_names>
linux-image-4.19.276-antix.1-686-smp-pae
linux-headers-4.19.276-antix.1-686-smp-pae
</uninstall_package_names>
</app>
