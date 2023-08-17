<?xml version="1.0" encoding="UTF-8"?>
<app>


<category>
Virtualization
</category>

<name>
VirtualBox
</name>

<description>
   <am>Virtualbox</am>
   <ar>Virtualbox</ar>
   <bg>Virtualbox</bg>
   <bn>Virtualbox</bn>
   <ca>Virtualbox</ca>
   <cs>Virtualbox</cs>
   <da>Virtualbox</da>
   <de>Die Virtualisierungssoftware VirtualBox</de>
   <el>Virtualbox</el>
   <en>Virtualbox</en>
   <es>Máquina virtual</es>
   <et>Virtualbox</et>
   <eu>Virtualbox</eu>
   <fa>Virtualbox</fa>
   <fil_PH>Virtualbox</fil_PH>
   <fi>Virtualbox - käyttöjärjestelmien ja muiden ohjelmien virtualisoimiseen tarkoitettu tietokoneohjelma.</fi>
   <fr>Virtualbox</fr>
   <he_IL>Virtualbox</he_IL>
   <hi>वर्चुअल बॉक्स</hi>
   <hr>Virtualbox</hr>
   <hu>Virtualbox</hu>
   <id>Virtualbox</id>
   <is>Virtualbox</is>
   <it>Virtualbox</it>
   <ja>バーチャルボックス</ja>
   <kk>Virtualbox</kk>
   <ko>Virtualbox</ko>
   <lt>Virtualbox</lt>
   <mk>Virtualbox</mk>
   <mr>Virtualbox</mr>
   <nb>VirtualBox</nb>
   <nl>Virtualbox</nl>
   <pl>menedżer maszyn wirtualnych</pl>
   <pt_BR>VirtualBox - Máquina virtual que permite instalar e executar diferentes sistemas operacionais em um único computador</pt_BR>
   <pt>Virtualbox</pt>
   <ro>Virtualbox</ro>
   <ru>Приложение виртуализации VirtualBox</ru>
   <sk>Virtualbox</sk>
   <sl>Virtualbox</sl>
   <sq>Virtualbox</sq>
   <sr>Virtualbox</sr>
   <sv>Virtualbox</sv>
   <tr>Virtualbox</tr>
   <uk>Virtualbox</uk>
   <vi>Virtualbox</vi>
   <zh_CN>Virtualbox</zh_CN>
   <zh_TW>Virtualbox</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>https://screenshots.debian.net/shrine/screenshot/9108/simage/large-e5809fdad2f4fa256e63549c1d1117fa.png</screenshot>

<preinstall>

</preinstall>

<install_package_names>
virtualbox
virtualbox-ext-pack
virtualbox-guest-additions-iso
</install_package_names>


<postinstall>
gpasswd -M $(getent group users | cut -d: -f4) vboxusers
</postinstall>


<uninstall_package_names>
virtualbox
virtualbox-ext-pack
virtualbox-guest-additions-iso
</uninstall_package_names>

</app>
