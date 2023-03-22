<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Misc
</category>

<name>
Google Earth Pro
</name>

<description>
   <am>Google Earth Globe</am>
   <ar>Google Earth Globe</ar>
   <bg>Google Earth Globe</bg>
   <bn>Google Earth Globe</bn>
   <ca>Mapamundi de Google Earth</ca>
   <cs>Google Earth Globe</cs>
   <da>Google Earth-globus</da>
   <de>Google Earth Globe</de>
   <el>Google Earth Globe</el>
   <en>Google Earth Globe</en>
   <es>Globo Terráqueo de Google Earth</es>
   <et>Google Earth Globe</et>
   <eu>Google Earth Globe</eu>
   <fa>Google Earth Globe</fa>
   <fil_PH>Google Earth Globe</fil_PH>
   <fi>Google Earth Globe</fi>
   <fr>Google Earth Globe</fr>
   <he_IL>Google Earth Globe</he_IL>
   <hi>Google Earth Globe</hi>
   <hr>Google Earth Globe</hr>
   <hu>Google Earth Globe</hu>
   <id>Google Earth Globe</id>
   <is>Google Earth Globe</is>
   <it>Google Earth Globe</it>
   <ja_JP>Google Earth Globe</ja_JP>
   <ja>Google Earth Globe</ja>
   <kk>Google Earth Globe</kk>
   <ko>Google Earth Globe</ko>
   <lt>Google Earth Globe</lt>
   <mk>Google Earth Globe</mk>
   <mr>Google Earth Globe</mr>
   <nb>Google Earth Globe</nb>
   <nl>Google Earth Wereldbol</nl>
   <pl>Globus Google Earth</pl>
   <pt_BR>Ver a Terra a partir do espaço</pt_BR>
   <pt>Ver a Terra a partir do espaço</pt>
   <ro>Google Earth Globe</ro>
   <ru>Google Планета Земля - спутниковые снимки земного шара</ru>
   <sk>Google Earth Globe</sk>
   <sl>Googlov globus Zemlje</sl>
   <sq>Google Earth Globe</sq>
   <sr>Google Earth Globe</sr>
   <sv>Google Earth Glob</sv>
   <tr>Google Earth Globe</tr>
   <uk>Google Earth Globe</uk>
   <vi>Google Earth Globe</vi>
   <zh_CN>Google Earth Globe</zh_CN>
   <zh_TW>Google Earth Globe</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>https://www.google.com/intl/en_us/earth/assets/static/images/versions/versions__pro__desktop-image.v2.png</screenshot>

<preinstall>
wget -q -O - https://dl.google.com/linux/linux_signing_key.pub | sudo apt-key add -
echo "deb [arch=amd64] http://dl.google.com/linux/earth/deb/ stable main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
</preinstall>

<install_package_names>
google-earth-pro-stable
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
</postinstall>


<uninstall_package_names>
google-earth-pro-stable
</uninstall_package_names>
</app>
