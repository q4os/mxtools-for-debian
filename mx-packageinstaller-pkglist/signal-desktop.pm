<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Messaging
</category>

<name>
Signal Desktop Client
</name>

<description>
   <am>Signal Desktop Client</am>
   <ar>Signal Desktop Client</ar>
   <bg>Signal Desktop Client</bg>
   <bn>Signal Desktop Client</bn>
   <ca>Signal Desktop Client</ca>
   <cs>Signal Desktop Client</cs>
   <da>Signal Desktop Client</da>
   <de>Signal Desktop Client</de>
   <el>Signal Desktop Client</el>
   <en>Signal Desktop Client</en>
   <es>Signal Desktop Client</es>
   <et>Signal Desktop Client</et>
   <eu>Signal Desktop Client</eu>
   <fa>Signal Desktop Client</fa>
   <fil_PH>Signal Desktop Client</fil_PH>
   <fi>Signal Desktop Client</fi>
   <fr>Signal Desktop Client</fr>
   <he_IL>Signal Desktop Client</he_IL>
   <hi>Signal Desktop Client</hi>
   <hr>Signal Desktop Client</hr>
   <hu>Signal Desktop Client</hu>
   <id>Signal Desktop Client</id>
   <is>Signal Desktop Client</is>
   <it>Signal Desktop Client</it>
   <ja_JP>Signal Desktop Client</ja_JP>
   <ja>Signal Desktop Client</ja>
   <kk>Signal Desktop Client</kk>
   <ko>Signal Desktop Client</ko>
   <lt>Signal Desktop Client</lt>
   <mk>Signal Desktop Client</mk>
   <mr>Signal Desktop Client</mr>
   <nb>Signal Desktop Client</nb>
   <nl>Signal Desktop Client</nl>
   <pl>Signal Desktop Client</pl>
   <pt_BR>Signal Desktop Client</pt_BR>
   <pt>Signal Desktop Client</pt>
   <ro>Signal Desktop Client</ro>
   <ru>Signal Desktop Client</ru>
   <sk>Signal Desktop Client</sk>
   <sl>Signal Desktop Client</sl>
   <sq>Signal Desktop Client</sq>
   <sr>Signal Desktop Client</sr>
   <sv>Signal Desktop Client</sv>
   <tr>Signal Desktop Client</tr>
   <uk>Signal Desktop Client</uk>
   <vi>Signal Desktop Client</vi>
   <zh_CN>Signal Desktop Client</zh_CN>
   <zh_TW>Signal Desktop Client</zh_TW>
</description>

<installable>
64
</installable>

<screenshot></screenshot>

<preinstall>
wget -O- https://updates.signal.org/desktop/apt/keys.asc | apt-key add -
echo "deb [arch=amd64] https://updates.signal.org/desktop/apt xenial main">/etc/apt/sources.list.d/signal-xenial-added-by-mxpi.list
apt-get update
</preinstall>

<install_package_names>
signal-desktop
</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
signal-desktop
</uninstall_package_names>

<postuninstall>
rm /etc/apt/sources.list.d/signal-xenial-added-by-mxpi.list
</postuninstall>
</app>
