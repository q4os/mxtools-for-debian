<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Development
</category>

<name>
MS-VSCode
</name>

<description>
   <am>Microsoft Visual Studio Code</am>
   <ar>Microsoft Visual Studio Code</ar>
   <bg>Microsoft Visual Studio Code</bg>
   <bn>Microsoft Visual Studio Code</bn>
   <ca>Microsoft Visual Studio Code</ca>
   <cs>Microsoft Visual Studio Code</cs>
   <da>Microsoft Visual Studio Code</da>
   <de>Microsoft Visual Studio Code</de>
   <el>Microsoft Visual Studio Code</el>
   <en>Microsoft Visual Studio Code</en>
   <es>Microsoft Visual Studio Code</es>
   <et>Microsoft Visual Studio Code</et>
   <eu>Microsoft Visual Studio Code</eu>
   <fa>Microsoft Visual Studio Code</fa>
   <fil_PH>Microsoft Visual Studio Code</fil_PH>
   <fi>Microsoft Visual Studio Code</fi>
   <fr>Microsoft Visual Studio Code</fr>
   <he_IL>Microsoft Visual Studio Code</he_IL>
   <hi>Microsoft Visual Studio Code</hi>
   <hr>Microsoft Visual Studio Code</hr>
   <hu>Microsoft Visual Studio Code</hu>
   <id>Microsoft Visual Studio Code</id>
   <is>Microsoft Visual Studio Code</is>
   <it>Microsoft Visual Studio Code</it>
   <ja>Microsoft Visual Studio Code</ja>
   <kk>Microsoft Visual Studio Code</kk>
   <ko>Microsoft Visual Studio Code</ko>
   <lt>Microsoft Visual Studio Code</lt>
   <mk>Microsoft Visual Studio Code</mk>
   <mr>Microsoft Visual Studio Code</mr>
   <nb>Microsoft Visual Studio Code</nb>
   <nl>Microsoft Visual Studio Code</nl>
   <pl>Microsoft Visual Studio Code</pl>
   <pt_BR>Microsoft Visual Studio Code</pt_BR>
   <pt>Microsoft Visual Studio Code</pt>
   <ro>Microsoft Visual Studio Code</ro>
   <ru>Microsoft Visual Studio Code</ru>
   <sk>Microsoft Visual Studio Code</sk>
   <sl>Microsoft Visual Studio Code</sl>
   <sq>Microsoft Visual Studio Code</sq>
   <sr>Microsoft Visual Studio Code</sr>
   <sv>Microsoft Visual Studio Code</sv>
   <tr>Microsoft Visual Studio Code</tr>
   <uk>Microsoft Visual Studio Code</uk>
   <vi>Microsoft Visual Studio Code</vi>
   <zh_CN>Microsoft Visual Studio Code</zh_CN>
   <zh_TW>Microsoft Visual Studio Code</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>https://code.visualstudio.com/assets/home/home-screenshot-linux-lg.png</screenshot>

<preinstall>
curl https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > /tmp/microsoft.gpg
install -o root -g root -m 644 /tmp/microsoft.gpg /etc/apt/trusted.gpg.d/
echo "deb [arch=amd64] https://packages.microsoft.com/repos/vscode stable main">/etc/apt/sources.list.d/vscode.list
apt-get update
</preinstall>

<install_package_names>
apt-transport-https
code
</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
code
</uninstall_package_names>
</app>
