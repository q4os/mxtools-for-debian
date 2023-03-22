<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Messaging
</category>

<name>
MS-Teams
</name>

<description>
   <am>Microsoft Teams</am>
   <ar>Microsoft Teams</ar>
   <bg>Microsoft Teams</bg>
   <bn>Microsoft Teams</bn>
   <ca>Microsoft Teams</ca>
   <cs>Microsoft Teams</cs>
   <da>Microsoft Teams</da>
   <de>Microsoft Teams</de>
   <el>Microsoft Teams</el>
   <en>Microsoft Teams</en>
   <es>Microsoft Teams</es>
   <et>Microsoft Teams</et>
   <eu>Microsoft Teams</eu>
   <fa>Microsoft Teams</fa>
   <fil_PH>Microsoft Teams</fil_PH>
   <fi>Microsoft Teams</fi>
   <fr>Microsoft Teams</fr>
   <he_IL>Microsoft Teams</he_IL>
   <hi>Microsoft Teams</hi>
   <hr>Microsoft Teams</hr>
   <hu>Microsoft Teams</hu>
   <id>Microsoft Teams</id>
   <is>Microsoft Teams</is>
   <it>Microsoft Teams</it>
   <ja_JP>Microsoft Teams</ja_JP>
   <ja>Microsoft Teams</ja>
   <kk>Microsoft Teams</kk>
   <ko>Microsoft Teams</ko>
   <lt>Microsoft Teams</lt>
   <mk>Microsoft Teams</mk>
   <mr>Microsoft Teams</mr>
   <nb>Microsoft Teams</nb>
   <nl>Microsoft Teams</nl>
   <pl>Microsoft Teams</pl>
   <pt_BR>Microsoft Teams</pt_BR>
   <pt>Microsoft Teams</pt>
   <ro>Microsoft Teams</ro>
   <ru>Microsoft Teams</ru>
   <sk>Microsoft Teams</sk>
   <sl>Microsoft Teams</sl>
   <sq>Microsoft Teams</sq>
   <sr>Microsoft Teams</sr>
   <sv>Microsoft Teams</sv>
   <tr>Microsoft Teams</tr>
   <uk>Microsoft Teams</uk>
   <vi>Microsoft Teams</vi>
   <zh_CN>Microsoft Teams</zh_CN>
   <zh_TW>Microsoft Teams</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>https://cdn-dynmedia-1.microsoft.com/is/image/microsoftcorp/KeyMessage3:VP5-1920x720</screenshot>

<preinstall>
curl https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > /tmp/microsoft.gpg
install -o root -g root -m 644 /tmp/microsoft.gpg /etc/apt/trusted.gpg.d/
echo "deb [arch=amd64] https://packages.microsoft.com/repos/ms-teams stable main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
</preinstall>

<install_package_names>
apt-transport-https
teams
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
</postinstall>


<uninstall_package_names>
teams
</uninstall_package_names>
</app>
