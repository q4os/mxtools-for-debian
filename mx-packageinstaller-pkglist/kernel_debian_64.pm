<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
Debian 64 bit (5.10)
</name>

<description>
   <am>Default Debian kernel Meltdown patched, 64bit</am>
   <ar>Default Debian kernel Meltdown patched, 64bit</ar>
   <bg>Default Debian kernel Meltdown patched, 64bit</bg>
   <bn>Default Debian kernel Meltdown patched, 64bit</bn>
   <ca>Kernel Debian per omissió, esmenat per Meltdown, 64 bit</ca>
   <cs>Default Debian kernel Meltdown patched, 64bit</cs>
   <da>Default Debian kernel Meltdown patched, 64bit</da>
   <de>Default Debian Kernel Meltdown gepatcht, 64bit</de>
   <el>Προκαθορισμένο Debian πυρήνα Meltdown patched, 64bit</el>
   <en>Default Debian kernel Meltdown patched, 64bit</en>
   <es>Kernel Debian predeterminado Meltdown parcheado, 64bit</es>
   <et>Default Debian kernel Meltdown patched, 64bit</et>
   <eu>Default Debian kernel Meltdown patched, 64bit</eu>
   <fa>Default Debian kernel Meltdown patched, 64bit</fa>
   <fil_PH>Default Debian kernel Meltdown patched, 64bit</fil_PH>
   <fi>Default Debian kernel Meltdown patched, 64bit</fi>
   <fr>Default Debian kernel Meltdown patched, 64bit</fr>
   <he_IL>Default Debian kernel Meltdown patched, 64bit</he_IL>
   <hi>Default Debian kernel Meltdown patched, 64bit</hi>
   <hr>Default Debian kernel Meltdown patched, 64bit</hr>
   <hu>Default Debian kernel Meltdown patched, 64bit</hu>
   <id>Default Debian kernel Meltdown patched, 64bit</id>
   <is>Default Debian kernel Meltdown patched, 64bit</is>
   <it>Kernel Debian di default, con la patch per Meltdown, 64bit</it>
   <ja_JP>Default Debian kernel Meltdown patched, 64bit</ja_JP>
   <ja>Default Debian kernel Meltdown patched, 64bit</ja>
   <kk>Default Debian kernel Meltdown patched, 64bit</kk>
   <ko>Default Debian kernel Meltdown patched, 64bit</ko>
   <lt>Default Debian kernel Meltdown patched, 64bit</lt>
   <mk>Default Debian kernel Meltdown patched, 64bit</mk>
   <mr>Default Debian kernel Meltdown patched, 64bit</mr>
   <nb>Default Debian kernel Meltdown patched, 64bit</nb>
   <nl>Default Debian kernel Meltdown patched, 64bit</nl>
   <pl>Default Debian kernel Meltdown patched, 64bit</pl>
   <pt_BR>Default Debian kernel Meltdown patched, 64bit</pt_BR>
   <pt>Núcleo (kernel) Debian predefinido com correcções para as falhas Meltdown e Spectre, 64bit</pt>
   <ro>Default Debian kernel Meltdown patched, 64bit</ro>
   <ru>Стандартное Debian ядро с патчем от Meltdown, 64 bit</ru>
   <sk>Default Debian kernel Meltdown patched, 64bit</sk>
   <sl>Privzeto Debian Meltdown jedro zakrpano, 64 bitno</sl>
   <sq>Default Debian kernel Meltdown patched, 64bit</sq>
   <sr>Default Debian kernel Meltdown patched, 64bit</sr>
   <sv>Standard Debian kärna Meltdown fixad, 64bit</sv>
   <tr>Default Debian kernel Meltdown patched, 64bit</tr>
   <uk>Default Debian kernel Meltdown patched, 64bit</uk>
   <vi>Default Debian kernel Meltdown patched, 64bit</vi>
   <zh_CN>Default Debian kernel Meltdown patched, 64bit</zh_CN>
   <zh_TW>Default Debian kernel Meltdown patched, 64bit</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>

</preinstall>

<install_package_names>
linux-image-amd64
linux-headers-amd64
</install_package_names>


<postinstall>
file=$(apt-cache show linux-image-amd64=4.19* |grep -m1 Depends) 
file=${file#"Depends:"}
rebuild_dkms_packages.sh $file
</postinstall>


<uninstall_package_names>
linux-image-amd64
linux-headers-amd64
</uninstall_package_names>
</app>
