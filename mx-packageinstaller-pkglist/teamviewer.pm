<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Remote Access
</category>

<name>
TeamViewer
</name>

<description>
   <am>Teamviewer remote access</am>
   <ar>Teamviewer remote access</ar>
   <bg>Teamviewer remote access</bg>
   <bn>Teamviewer remote access</bn>
   <ca>Accés remot Teamviewer</ca>
   <cs>Teamviewer remote access</cs>
   <da>Teamviewer fjernadgang</da>
   <de>Fernzugriff mit Teamviewer </de>
   <el>Απομακρυσμένη πρόσβαση με το Teamviewer</el>
   <en>Teamviewer remote access</en>
   <es>Teamviewer acceso remoto</es>
   <et>Teamviewer remote access</et>
   <eu>Teamviewer remote access</eu>
   <fa>Teamviewer remote access</fa>
   <fil_PH>Teamviewer remote access</fil_PH>
   <fi>Teamviewer-etäyhteys</fi>
   <fr>Teamviewer accès à distance</fr>
   <he_IL>Teamviewer remote access</he_IL>
   <hi>Teamviewer दूरस्थ अभिगम</hi>
   <hr>Teamviewer remote access</hr>
   <hu>Teamviewer remote access</hu>
   <id>Teamviewer remote access</id>
   <is>Teamviewer remote access</is>
   <it>Teamviewer accesso remoto</it>
   <ja>TeamViewer リモートアクセス</ja>
   <kk>Teamviewer remote access</kk>
   <ko>Teamviewer remote access</ko>
   <lt>Teamviewer nuotolinė prieiga</lt>
   <mk>Teamviewer remote access</mk>
   <mr>Teamviewer remote access</mr>
   <nb>Teamviewer fjerntilgang</nb>
   <nl>Teamviewer afstand toegang</nl>
   <pl>dostęp zdalny TeamViewer</pl>
   <pt_BR>Teamviewer - Aplicativo de acesso remoto</pt_BR>
   <pt>Aplicação de acesso remoto</pt>
   <ro>Teamviewer remote access</ro>
   <ru>Удалённый доступ Teamviewer</ru>
   <sk>Teamviewer remote access</sk>
   <sl>Teamviewer oddaljeni dostop</sl>
   <sq>Hyrje e largët me Teamviewer</sq>
   <sr>Teamviewer remote access</sr>
   <sv>Teamviewer fjärråtkomst</sv>
   <tr>Teamviewer uzaktan erişim</tr>
   <uk>Teamviewer remote access</uk>
   <vi>Teamviewer remote access</vi>
   <zh_CN>Teamviewer remote access</zh_CN>
   <zh_TW>Teamviewer remote access</zh_TW>
</description>

<installable>
32,64
</installable>

<screenshot>none</screenshot>

<preinstall>

( wget -q https://linux.teamviewer.com/pubkey/currentkey.asc -O- \
| gpg --dearmor | sudo tee /usr/share/keyrings/teamviewer-keyring.gpg ) 2>/dev/null 1>/dev/null
echo "deb [signed-by=/usr/share/keyrings/teamviewer-keyring.gpg] https://linux.teamviewer.com/deb stable main" > /etc/apt/sources.list.d/mxtemp.list
apt-get update

</preinstall>

<install_package_names>
teamviewer
</install_package_names>

<postinstall>
rm /etc/apt/sources.list.d/mxtemp.list
apt-get update
</postinstall>

<uninstall_package_names>
teamviewer
</uninstall_package_names>
</app>
