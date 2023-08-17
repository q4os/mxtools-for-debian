<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Network
</category>

<name>
PIAVPN - Private Internet Access VPN
</name>

<description>
   <am>Private Internet Access VPN</am>
   <ar>Private Internet Access VPN</ar>
   <bg>Private Internet Access VPN</bg>
   <bn>Private Internet Access VPN</bn>
   <ca>Accés privat a internet VPN</ca>
   <cs>Private Internet Access VPN</cs>
   <da>Private Internet Access VPN</da>
   <de>Privater Internetzugang VPN</de>
   <el>Private Internet Access VPN</el>
   <en>Private Internet Access VPN</en>
   <es>Acceso Privado a Internet VPN</es>
   <et>Private Internet Access VPN</et>
   <eu>Private Internet Access VPN</eu>
   <fa>Private Internet Access VPN</fa>
   <fil_PH>Private Internet Access VPN</fil_PH>
   <fi>PIA (Private Internet Access) tuo virtuaalisen VPN-erillisverkon</fi>
   <fr>Accès Internet Privé par VPN</fr>
   <he_IL>Private Internet Access VPN</he_IL>
   <hi>प्राइवेट इंटरनेट एक्सेस पीआईए</hi>
   <hr>Private Internet Access VPN</hr>
   <hu>Private Internet Access VPN</hu>
   <id>Private Internet Access VPN</id>
   <is>Private Internet Access VPN</is>
   <it>Accesso internet privato VPN</it>
   <ja>プライベートインターネットアクセスVPN</ja>
   <kk>Private Internet Access VPN</kk>
   <ko>Private Internet Access VPN</ko>
   <lt>Private Internet Access VPN</lt>
   <mk>Private Internet Access VPN</mk>
   <mr>Private Internet Access VPN</mr>
   <nb>Privat internett-tilgang, VPN</nb>
   <nl>Privé Internet Toegang VPN</nl>
   <pl>Private Internet Access VPN</pl>
   <pt_BR>Acesso Privado à Internet VPN</pt_BR>
   <pt>Acesso Privado à Internet VPN</pt>
   <ro>Private Internet Access VPN</ro>
   <ru>Частный VPN для выхода в интернет</ru>
   <sk>Private Internet Access VPN</sk>
   <sl>Zasebni spletni dostop preko VPN</sl>
   <sq>VPN Hyrjeje Private në Internet</sq>
   <sr>Private Internet Access VPN</sr>
   <sv>Private Internet Access VPN</sv>
   <tr>Özel İnternet Erişimi VPN</tr>
   <uk>Private Internet Access VPN</uk>
   <vi>Private Internet Access VPN</vi>
   <zh_CN>Private Internet Access VPN</zh_CN>
   <zh_TW>Private Internet Access VPN</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
</preinstall>

<install_package_names>
piavpn-downloader-installer
</install_package_names>

<postinstall>

if [ -x /usr/share/pia-downloader-installer/install_piavpn.sh ]; then
/usr/share/pia-downloader-installer/install_piavpn.sh
fi

</postinstall>

<uninstall_package_names>
piavpn-downloader-installer
</uninstall_package_names>
</app>
