<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Messaging
</category>

<name>
1-to-1 Voice
</name>

<description>
   <am>voice chat between two pc's via encrypted mumble</am>
   <ar>voice chat between two pc's via encrypted mumble</ar>
   <bg>voice chat between two pc's via encrypted mumble</bg>
   <bn>voice chat between two pc's via encrypted mumble</bn>
   <ca>Xat de veu entre dos PCs amb mumble encriptat</ca>
   <cs>voice chat between two pc's via encrypted mumble</cs>
   <da>stemmechat mellem to pc'er via krypteret mumble</da>
   <de>Verschlüsselte Privatgespräche mittels Mumble über SSL</de>
   <el>φωνητική συνομιλία μεταξύ δύο ηλεκτρονικών υπολογιστών μέσω κρυπτογραφημένων mumble</el>
   <en>voice chat between two pc's via encrypted mumble</en>
   <es>Chat de voz entre dos PCs vía mumble encriptado</es>
   <et>voice chat between two pc's via encrypted mumble</et>
   <eu>voice chat between two pc's via encrypted mumble</eu>
   <fa>voice chat between two pc's via encrypted mumble</fa>
   <fil_PH>voice chat between two pc's via encrypted mumble</fil_PH>
   <fi>voice chat between two pc's via encrypted mumble</fi>
   <fr>Échanges vocaux chiffrés entre deux PC via mumble</fr>
   <he_IL>voice chat between two pc's via encrypted mumble</he_IL>
   <hi>voice chat between two pc's via encrypted mumble</hi>
   <hr>voice chat between two pc's via encrypted mumble</hr>
   <hu>voice chat between two pc's via encrypted mumble</hu>
   <id>voice chat between two pc's via encrypted mumble</id>
   <is>voice chat between two pc's via encrypted mumble</is>
   <it>Chat voce tra due pc attraverso mumble in forma criptata</it>
   <ja_JP>voice chat between two pc's via encrypted mumble</ja_JP>
   <ja>voice chat between two pc's via encrypted mumble</ja>
   <kk>voice chat between two pc's via encrypted mumble</kk>
   <ko>voice chat between two pc's via encrypted mumble</ko>
   <lt>voice chat between two pc's via encrypted mumble</lt>
   <mk>voice chat between two pc's via encrypted mumble</mk>
   <mr>voice chat between two pc's via encrypted mumble</mr>
   <nb>voice chat between two pc's via encrypted mumble</nb>
   <nl>voice chat tussen twee PCs via beveiligde mumble</nl>
   <pl>czat głosowy pomiędzy dwoma komputerami szyfrowany przez mumble</pl>
   <pt_BR>Chat de voz entre dois computadores via mumble encriptado</pt_BR>
   <pt>Chat de voz entre dois computadores via mumble encriptado</pt>
   <ro>voice chat between two pc's via encrypted mumble</ro>
   <ru>Голосовой чат между двумя PC через шифрованный канал</ru>
   <sk>voice chat between two pc's via encrypted mumble</sk>
   <sl>Glasovni pogovor med dvema računalnikoma preko šifriranega Mumble</sl>
   <sq>voice chat between two pc's via encrypted mumble</sq>
   <sr>voice chat between two pc's via encrypted mumble</sr>
   <sv>röstchatt mellan två pc's via encrypted mumble</sv>
   <tr>voice chat between two pc's via encrypted mumble</tr>
   <uk>зашифрований голосовий чат між двома ПК</uk>
   <vi>voice chat between two pc's via encrypted mumble</vi>
   <zh_CN>voice chat between two pc's via encrypted mumble</zh_CN>
   <zh_TW>voice chat between two pc's via encrypted mumble</zh_TW>
</description>

<installable>
32,64
</installable>

<screenshot>none</screenshot>

<preinstall>
echo "deb http://la.mxrepo.com/antix/buster buster main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
</preinstall>

<install_package_names>
1-to-1-voice-antix
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
apt-get update
</postinstall>


<uninstall_package_names>
1-to-1-voice-antix
</uninstall_package_names>
</app>
