<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
Debian 32 bit w/pae (5.10)
</name>

<description>
   <am>Default Debian 32bit linux kernel w/PAE</am>
   <ar>Default Debian 32bit linux kernel w/PAE</ar>
   <bg>Default Debian 32bit linux kernel w/PAE</bg>
   <bn>Default Debian 32bit linux kernel w/PAE</bn>
   <ca>Kernel Linux Debian 32bit per omissió amb PAE</ca>
   <cs>Default Debian 32bit linux kernel w/PAE</cs>
   <da>Default Debian 32bit linux kernel w/PAE</da>
   <de>Default Debian 32bit Linux Kernel w/PAE</de>
   <el>Προεπιλεγμένο Debian 32bit linux kernel PAE</el>
   <en>Default Debian 32bit linux kernel w/PAE</en>
   <es>Kernel Debian predeterminado 32bit linux  w/PAE</es>
   <et>Default Debian 32bit linux kernel w/PAE</et>
   <eu>Default Debian 32bit linux kernel w/PAE</eu>
   <fa>Default Debian 32bit linux kernel w/PAE</fa>
   <fil_PH>Default Debian 32bit linux kernel w/PAE</fil_PH>
   <fi>Default Debian 32bit linux kernel w/PAE</fi>
   <fr>Default Debian 32bit linux kernel w/PAE</fr>
   <he_IL>Default Debian 32bit linux kernel w/PAE</he_IL>
   <hi>Default Debian 32bit linux kernel w/PAE</hi>
   <hr>Default Debian 32bit linux kernel w/PAE</hr>
   <hu>Default Debian 32bit linux kernel w/PAE</hu>
   <id>Default Debian 32bit linux kernel w/PAE</id>
   <is>Default Debian 32bit linux kernel w/PAE</is>
   <it>Kernel Debian di default 32bit w/PAE</it>
   <ja_JP>Default Debian 32bit linux kernel w/PAE</ja_JP>
   <ja>Default Debian 32bit linux kernel w/PAE</ja>
   <kk>Default Debian 32bit linux kernel w/PAE</kk>
   <ko>Default Debian 32bit linux kernel w/PAE</ko>
   <lt>Default Debian 32bit linux kernel w/PAE</lt>
   <mk>Default Debian 32bit linux kernel w/PAE</mk>
   <mr>Default Debian 32bit linux kernel w/PAE</mr>
   <nb>Default Debian 32bit linux kernel w/PAE</nb>
   <nl>Default Debian 32bit linux kernel w/PAE</nl>
   <pl>Default Debian 32bit linux kernel w/PAE</pl>
   <pt_BR>Default Debian 32bit linux kernel w/PAE</pt_BR>
   <pt>Núcleo (kernel) linux Debian predefinido 32bit c/PAE</pt>
   <ro>Default Debian 32bit linux kernel w/PAE</ro>
   <ru>Стандартное Debian ядро, 32 bit PAE</ru>
   <sk>Default Debian 32bit linux kernel w/PAE</sk>
   <sl>Privzeto Debian 32 bitno linux jedro s PAE</sl>
   <sq>Default Debian 32bit linux kernel w/PAE</sq>
   <sr>Default Debian 32bit linux kernel w/PAE</sr>
   <sv>Standard Debian 32bit linux kärna med PAE</sv>
   <tr>Default Debian 32bit linux kernel w/PAE</tr>
   <uk>Default Debian 32bit linux kernel w/PAE</uk>
   <vi>Default Debian 32bit linux kernel w/PAE</vi>
   <zh_CN>Default Debian 32bit linux kernel w/PAE</zh_CN>
   <zh_TW>Default Debian 32bit linux kernel w/PAE</zh_TW>
</description>

<installable>
32
</installable>

<screenshot>none</screenshot>

<preinstall>

</preinstall>

<install_package_names>
linux-image-686-pae
linux-headers-686-pae
</install_package_names>


<postinstall>
file=$(apt-cache show linux-image-686-pae=4.19 |grep -m1 Depends) 
file=${file#"Depends:"}
rebuild_dkms_packages.sh $file
</postinstall>


<uninstall_package_names>
linux-image-686-pae
linux-headers-686-pae
</uninstall_package_names>
</app>
