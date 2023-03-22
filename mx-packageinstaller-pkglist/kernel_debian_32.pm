<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
Debian 32 bit no-pae (5.10)
</name>

<description>
   <am>Default Debian 32bit linux kernel, no-PAE, single core</am>
   <ar>Default Debian 32bit linux kernel, no-PAE, single core</ar>
   <bg>Default Debian 32bit linux kernel, no-PAE, single core</bg>
   <bn>Default Debian 32bit linux kernel, no-PAE, single core</bn>
   <ca>Kernel Linux Debian 32bit no-PAE per omissió, 32 bit, single core</ca>
   <cs>Default Debian 32bit linux kernel, no-PAE, single core</cs>
   <da>Default Debian 32bit linux kernel, no-PAE, single core</da>
   <de>Default Debian 32bit Linux Kernel, no-PAE, single core</de>
   <el>Προκαθορισμένο Debian 32bit linux kernel, no-PAE, single core</el>
   <en>Default Debian 32bit linux kernel, no-PAE, single core</en>
   <es>Kernel Debian predeterminado 32bit linux, no-PAE, single core</es>
   <et>Default Debian 32bit linux kernel, no-PAE, single core</et>
   <eu>Default Debian 32bit linux kernel, no-PAE, single core</eu>
   <fa>Default Debian 32bit linux kernel, no-PAE, single core</fa>
   <fil_PH>Default Debian 32bit linux kernel, no-PAE, single core</fil_PH>
   <fi>Default Debian 32bit linux kernel, no-PAE, single core</fi>
   <fr>Default Debian 32bit linux kernel, no-PAE, single core</fr>
   <he_IL>Default Debian 32bit linux kernel, no-PAE, single core</he_IL>
   <hi>Default Debian 32bit linux kernel, no-PAE, single core</hi>
   <hr>Default Debian 32bit linux kernel, no-PAE, single core</hr>
   <hu>Default Debian 32bit linux kernel, no-PAE, single core</hu>
   <id>Default Debian 32bit linux kernel, no-PAE, single core</id>
   <is>Default Debian 32bit linux kernel, no-PAE, single core</is>
   <it>Kernel Debian di default 32bit, no-PAE, single core</it>
   <ja_JP>Default Debian 32bit linux kernel, no-PAE, single core</ja_JP>
   <ja>Default Debian 32bit linux kernel, no-PAE, single core</ja>
   <kk>Default Debian 32bit linux kernel, no-PAE, single core</kk>
   <ko>Default Debian 32bit linux kernel, no-PAE, single core</ko>
   <lt>Default Debian 32bit linux kernel, no-PAE, single core</lt>
   <mk>Default Debian 32bit linux kernel, no-PAE, single core</mk>
   <mr>Default Debian 32bit linux kernel, no-PAE, single core</mr>
   <nb>Default Debian 32bit linux kernel, no-PAE, single core</nb>
   <nl>Default Debian 32bit linux kernel, no-PAE, single core</nl>
   <pl>Default Debian 32bit linux kernel, no-PAE, single core</pl>
   <pt_BR>Default Debian 32bit linux kernel, no-PAE, single core</pt_BR>
   <pt>Núcleo (kernel) linux Debian predefinido 32bit, no-PAE, single core</pt>
   <ro>Default Debian 32bit linux kernel, no-PAE, single core</ro>
   <ru>Стандартное Debian ядро, 32 bit no-PAE с поддержкой 1-core</ru>
   <sk>Default Debian 32bit linux kernel, no-PAE, single core</sk>
   <sl>Privzeto Debian 32 bitno linux jedro brez PAE, eno jedro</sl>
   <sq>Default Debian 32bit linux kernel, no-PAE, single core</sq>
   <sr>Default Debian 32bit linux kernel, no-PAE, single core</sr>
   <sv>Standard Debian 32bit linux kärna, no-PAE, single core</sv>
   <tr>Öntanımlı Debian 32 bit linux çekirdeği, PAE olmayan, tek çekirdek</tr>
   <uk>Default Debian 32bit linux kernel, no-PAE, single core</uk>
   <vi>Default Debian 32bit linux kernel, no-PAE, single core</vi>
   <zh_CN>Default Debian 32bit linux kernel, no-PAE, single core</zh_CN>
   <zh_TW>Default Debian 32bit linux kernel, no-PAE, single core</zh_TW>
</description>

<installable>
32
</installable>

<screenshot>none</screenshot>

<preinstall>

</preinstall>

<install_package_names>
linux-image-686
linux-headers-686
</install_package_names>


<postinstall>
file=$(apt-cache show linux-image-686=4.19 |grep -m1 Depends) 
file=${file#"Depends:"}
rebuild_dkms_packages.sh $file
</postinstall>


<uninstall_package_names>
linux-image-686
linux-headers-686
</uninstall_package_names>
</app>
