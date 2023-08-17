<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Misc
</category>

<name>
Google Earth
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
   <fi>Google Earth Maapallo</fi>
   <fr>Google Earth Globe</fr>
   <he_IL>Google Earth Globe</he_IL>
   <hi>गूगल पृथ्वी ग्लोब</hi>
   <hr>Google Earth Globe</hr>
   <hu>Google Earth Globe</hu>
   <id>Google Earth Globe</id>
   <is>Google Earth Globe</is>
   <it>Google Earth Globe</it>
   <ja>Google Earth 地球儀</ja>
   <kk>Google Earth Globe</kk>
   <ko>Google Earth Globe</ko>
   <lt>Google Earth Globe</lt>
   <mk>Google Earth Globe</mk>
   <mr>Google Earth Globe</mr>
   <nb>Globusen Google Earth</nb>
   <nl>Google Earth Wereldbol</nl>
   <pl>Globus Google Earth</pl>
   <pt_BR>Google Earth - Visualizar o planeta Terra a partir do espaço</pt_BR>
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
32
</installable>

<screenshot>https://www.google.com/intl/en_us/earth/assets/static/images/versions/versions__pro__desktop-image.v2.png</screenshot>

<preinstall>
wget http://dl.google.com/dl/earth/client/current/google-earth-stable_current_i386.deb
dpkg -i google-earth-stable*.deb
apt-get -f install
apt-get install pulseaudio pulseaudio-module-x11 gstreamer0.10-pulseaudio rtkit pavucontrol
rm google-earth-stable*.deb
</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
google-earth-stable
</uninstall_package_names>
</app>
