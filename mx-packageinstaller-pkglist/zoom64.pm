<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Messaging
</category>

<name>
Zoom
</name>

<description>
   <am>Zoom Teleconference Client</am>
   <ar>Zoom Teleconference Client</ar>
   <bg>Zoom Teleconference Client</bg>
   <bn>Zoom Teleconference Client</bn>
   <ca>Zoom Teleconference CLient</ca>
   <cs>Zoom Teleconference Client</cs>
   <da>Zoom Teleconference Client</da>
   <de>Zoom Teleconference Client</de>
   <el>Zoom Teleconference Client</el>
   <en>Zoom Teleconference Client</en>
   <es>Zoom Teleconference Client</es>
   <et>Zoom Teleconference Client</et>
   <eu>Zoom Teleconference Client</eu>
   <fa>Zoom Teleconference Client</fa>
   <fil_PH>Zoom Teleconference Client</fil_PH>
   <fi>Zoom Teleconference Client</fi>
   <fr>Zoom Teleconference Client</fr>
   <he_IL>Zoom Teleconference Client</he_IL>
   <hi>Zoom Teleconference Client</hi>
   <hr>Zoom Teleconference Client</hr>
   <hu>Zoom Teleconference Client</hu>
   <id>Zoom Teleconference Client</id>
   <is>Zoom Teleconference Client</is>
   <it>Zoom Teleconference Client</it>
   <ja>Zoom Teleconference Client</ja>
   <kk>Zoom Teleconference Client</kk>
   <ko>Zoom Teleconference Client</ko>
   <lt>Zoom Teleconference Client</lt>
   <mk>Zoom Teleconference Client</mk>
   <mr>Zoom Teleconference Client</mr>
   <nb>Zoom Teleconference Client</nb>
   <nl>Zoom Teleconference Client</nl>
   <pl>Zoom Teleconference Client</pl>
   <pt_BR>Zoom Teleconference Client</pt_BR>
   <pt>Zoom Teleconference Client</pt>
   <ro>Zoom Teleconference Client</ro>
   <ru>Zoom Teleconference Client</ru>
   <sk>Zoom Teleconference Client</sk>
   <sl>Zoom Teleconference Client</sl>
   <sq>Zoom Teleconference Client</sq>
   <sr>Zoom Teleconference Client</sr>
   <sv>Zoom Teleconference Client</sv>
   <tr>Zoom Teleconference Client</tr>
   <uk>Zoom Teleconference Client</uk>
   <vi>Zoom Teleconference Client</vi>
   <zh_CN>Zoom Teleconference Client</zh_CN>
   <zh_TW>Zoom Teleconference Client</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
curl -RL https://zoom.us/client/latest/zoom_amd64.deb -o /tmp/zoom.deb
apt-get install /tmp/zoom.deb
</preinstall>

<install_package_names>

</install_package_names>


<postinstall>
rm /tmp/zoom.deb
</postinstall>


<uninstall_package_names>
zoom
</uninstall_package_names>
</app>
