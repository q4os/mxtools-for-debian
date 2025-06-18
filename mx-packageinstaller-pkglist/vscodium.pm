<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Development
</category>

<name>
vscodium
</name>

<description>
   <am>de-Microsofted Visual Studio Code</am>
   <ar>de-Microsofted Visual Studio Code</ar>
   <bg>de-Microsofted Visual Studio Code</bg>
   <bn>de-Microsofted Visual Studio Code</bn>
   <ca>de-Microsofted Visual Studio Code</ca>
   <cs>de-Microsofted Visual Studio Code</cs>
   <da>de-Microsofted Visual Studio Code</da>
   <de>de-Microsofted Visual Studio Code</de>
   <el>de-Microsofted Visual Studio Code</el>
   <en>de-Microsofted Visual Studio Code</en>
   <es>de-Microsofted Visual Studio Code</es>
   <et>de-Microsofted Visual Studio Code</et>
   <eu>de-Microsofted Visual Studio Code</eu>
   <fa>de-Microsofted Visual Studio Code</fa>
   <fil_PH>de-Microsofted Visual Studio Code</fil_PH>
   <fi>de-Microsofted Visual Studio Code</fi>
   <fr>de-Microsofted Visual Studio Code</fr>
   <he_IL>de-Microsofted Visual Studio Code</he_IL>
   <hi>de-Microsofted Visual Studio Code</hi>
   <hr>de-Microsofted Visual Studio Code</hr>
   <hu>de-Microsofted Visual Studio Code</hu>
   <id>de-Microsofted Visual Studio Code</id>
   <is>de-Microsofted Visual Studio Code</is>
   <it>de-Microsofted Visual Studio Code</it>
   <ja>de-Microsofted Visual Studio Code</ja>
   <kk>de-Microsofted Visual Studio Code</kk>
   <ko>de-Microsofted Visual Studio Code</ko>
   <lt>de-Microsofted Visual Studio Code</lt>
   <mk>de-Microsofted Visual Studio Code</mk>
   <mr>de-Microsofted Visual Studio Code</mr>
   <nb>de-Microsofted Visual Studio Code</nb>
   <nl>de-Microsofted Visual Studio Code</nl>
   <pl>de-Microsofted Visual Studio Code</pl>
   <pt_BR>de-Microsofted Visual Studio Code</pt_BR>
   <pt>de-Microsofted Visual Studio Code</pt>
   <ro>de-Microsofted Visual Studio Code</ro>
   <ru>de-Microsofted Visual Studio Code</ru>
   <sk>de-Microsofted Visual Studio Code</sk>
   <sl>de-Microsofted Visual Studio Code</sl>
   <sq>de-Microsofted Visual Studio Code</sq>
   <sr>de-Microsofted Visual Studio Code</sr>
   <sv>de-Microsofted Visual Studio Code</sv>
   <tr>de-Microsofted Visual Studio Code</tr>
   <uk>de-Microsofted Visual Studio Code</uk>
   <vi>de-Microsofted Visual Studio Code</vi>
   <zh_CN>de-Microsofted Visual Studio Code</zh_CN>
   <zh_TW>de-Microsofted Visual Studio Code</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>https://code.visualstudio.com/assets/home/home-screenshot-linux-lg.png</screenshot>

<preinstall>
wget -qO - https://gitlab.com/paulcarroty/vscodium-deb-rpm-repo/raw/master/pub.gpg |gpg --dearmor -o /usr/share/keyrings/vscodium-archive-keyring.gpg
echo "deb [arch=amd64,arm64 signed-by=/usr/share/keyrings/vscodium-archive-keyring.gpg ] https://paulcarroty.gitlab.io/vscodium-deb-rpm-repo/debs vscodium main">/etc/apt/sources.list.d/vscodium.list
apt-get update
</preinstall>

<install_package_names>
apt-transport-https
codium
</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
codium
</uninstall_package_names>

<postuninstall>
rm /usr/share/keyrings/vscodium-archive-keyring.gpg
rm /etc/apt/sources.list.d/vscodium.list
apt-get update
</postuninstall>
</app>
