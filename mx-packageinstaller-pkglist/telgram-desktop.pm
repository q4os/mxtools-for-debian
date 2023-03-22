<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Messaging
</category>

<name>
Telegram Desktop Client (debian backports)
</name>

<description>
   <am>Telegram Desktop Client (debian backports)</am>
   <ar>Telegram Desktop Client (debian backports)</ar>
   <bg>Telegram Desktop Client (debian backports)</bg>
   <bn>Telegram Desktop Client (debian backports)</bn>
   <ca>Telegram Desktop Client (debian backports)</ca>
   <cs>Telegram Desktop Client (debian backports)</cs>
   <da>Telegram Desktop Client (debian backports)</da>
   <de>Telegram Desktop Client (debian backports)</de>
   <el>Telegram Desktop Client (debian backports)</el>
   <en>Telegram Desktop Client (debian backports)</en>
   <es>Telegram Desktop Client (debian backports)</es>
   <et>Telegram Desktop Client (debian backports)</et>
   <eu>Telegram Desktop Client (debian backports)</eu>
   <fa>Telegram Desktop Client (debian backports)</fa>
   <fil_PH>Telegram Desktop Client (debian backports)</fil_PH>
   <fi>Telegram Desktop Client (debian backports)</fi>
   <fr>Telegram Desktop Client (debian backports)</fr>
   <he_IL>Telegram Desktop Client (debian backports)</he_IL>
   <hi>Telegram Desktop Client (debian backports)</hi>
   <hr>Telegram Desktop Client (debian backports)</hr>
   <hu>Telegram Desktop Client (debian backports)</hu>
   <id>Telegram Desktop Client (debian backports)</id>
   <is>Telegram Desktop Client (debian backports)</is>
   <it>Telegram Desktop Client (debian backports)</it>
   <ja_JP>Telegram Desktop Client (debian backports)</ja_JP>
   <ja>Telegram Desktop Client (debian backports)</ja>
   <kk>Telegram Desktop Client (debian backports)</kk>
   <ko>Telegram Desktop Client (debian backports)</ko>
   <lt>Telegram Desktop Client (debian backports)</lt>
   <mk>Telegram Desktop Client (debian backports)</mk>
   <mr>Telegram Desktop Client (debian backports)</mr>
   <nb>Telegram Desktop Client (debian backports)</nb>
   <nl>Telegram Desktop Client (debian backports)</nl>
   <pl>Telegram Desktop Client (debian backports)</pl>
   <pt_BR>Telegram Desktop Client (debian backports)</pt_BR>
   <pt>Telegram Desktop Client (debian backports)</pt>
   <ro>Telegram Desktop Client (debian backports)</ro>
   <ru>Telegram Desktop Client (debian backports)</ru>
   <sk>Telegram Desktop Client (debian backports)</sk>
   <sl>Telegram Desktop Client (debian backports)</sl>
   <sq>Telegram Desktop Client (debian backports)</sq>
   <sr>Telegram Desktop Client (debian backports)</sr>
   <sv>Telegram Desktop Client (debian backports)</sv>
   <tr>Telegram Desktop Client (debian backports)</tr>
   <uk>Telegram Desktop Client (debian backports)</uk>
   <vi>Telegram Desktop Client (debian backports)</vi>
   <zh_CN>Telegram Desktop Client (debian backports)</zh_CN>
   <zh_TW>Telegram Desktop Client (debian backports)</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>https://screenshots.debian.net/shrine/screenshot/16680/simage/large-0fd665fe199a1e24fcbe3390976edca6.png</screenshot>

<preinstall>
echo "deb http://deb.debian.org/debian bullseye-backports main contrib non-free">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
apt-get install telegram-desktop -t bullseye-backports
</preinstall>

<install_package_names>

</install_package_names>


<postinstall>
rm -f /etc/apt/sources.list.d/mxpitemp.list
apt-get update
</postinstall>


<uninstall_package_names>
telegram-desktop
</uninstall_package_names>

<postuninstall>

</postuninstall>
</app>
