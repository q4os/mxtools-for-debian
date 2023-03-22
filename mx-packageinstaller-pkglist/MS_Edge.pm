<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Browser
</category>

<name>
MS-Edge
</name>

<description>
   <am>Microsoft Edge</am>
   <ar>Microsoft Edge</ar>
   <bg>Microsoft Edge</bg>
   <bn>Microsoft Edge</bn>
   <ca>Microsoft Edge</ca>
   <cs>Microsoft Edge</cs>
   <da>Microsoft Edge</da>
   <de>Microsoft Edge</de>
   <el>Microsoft Edge</el>
   <en>Microsoft Edge</en>
   <es>Microsoft Edge</es>
   <et>Microsoft Edge</et>
   <eu>Microsoft Edge</eu>
   <fa>Microsoft Edge</fa>
   <fil_PH>Microsoft Edge</fil_PH>
   <fi>Microsoft Edge</fi>
   <fr>Microsoft Edge</fr>
   <he_IL>Microsoft Edge</he_IL>
   <hi>Microsoft Edge</hi>
   <hr>Microsoft Edge</hr>
   <hu>Microsoft Edge</hu>
   <id>Microsoft Edge</id>
   <is>Microsoft Edge</is>
   <it>Microsoft Edge</it>
   <ja_JP>Microsoft Edge</ja_JP>
   <ja>Microsoft Edge</ja>
   <kk>Microsoft Edge</kk>
   <ko>Microsoft Edge</ko>
   <lt>Microsoft Edge</lt>
   <mk>Microsoft Edge</mk>
   <mr>Microsoft Edge</mr>
   <nb>Microsoft Edge</nb>
   <nl>Microsoft Edge</nl>
   <pl>Microsoft Edge</pl>
   <pt_BR>Microsoft Edge</pt_BR>
   <pt>Microsoft Edge</pt>
   <ro>Microsoft Edge</ro>
   <ru>Microsoft Edge</ru>
   <sk>Microsoft Edge</sk>
   <sl>Microsoft Edge</sl>
   <sq>Microsoft Edge</sq>
   <sr>Microsoft Edge</sr>
   <sv>Microsoft Edge</sv>
   <tr>Microsoft Edge</tr>
   <uk>Microsoft Edge</uk>
   <vi>Microsoft Edge</vi>
   <zh_CN>Microsoft Edge</zh_CN>
   <zh_TW>Microsoft Edge</zh_TW>
</description>

<installable>
64
</installable>

<screenshot></screenshot>

<preinstall>
curl https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > /tmp/microsoft.gpg
install -o root -g root -m 644 /tmp/microsoft.gpg /etc/apt/trusted.gpg.d/
echo "deb [arch=amd64] https://packages.microsoft.com/repos/edge/ stable main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
</preinstall>

<install_package_names>
apt-transport-https
microsoft-edge-stable
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
</postinstall>


<uninstall_package_names>
microsoft-edge-stable
</uninstall_package_names>
</app>
