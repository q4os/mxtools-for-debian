<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Messaging
</category>

<name>
Skype (64bit only)
</name>

<description>
   <am>Peer-to-peer video, voice, messaging</am>
   <ar>Peer-to-peer video, voice, messaging</ar>
   <bg>Peer-to-peer video, voice, messaging</bg>
   <bn>Peer-to-peer video, voice, messaging</bn>
   <ca>Missatgeria peer-to-peer de vídeo i veu</ca>
   <cs>Peer-to-peer video, voice, messaging</cs>
   <da>Modpart-til-modpart video, stemme, samtale</da>
   <de>Peer-to-Peer-Video, Sprache, Messaging</de>
   <el>Peer-to-peer βίντεο, φωνή, μηνύματα</el>
   <en>Peer-to-peer video, voice, messaging</en>
   <es>Vídeo entre pares, voz, mensajería</es>
   <et>Peer-to-peer video, voice, messaging</et>
   <eu>Peer-to-peer video, voice, messaging</eu>
   <fa>Peer-to-peer video, voice, messaging</fa>
   <fil_PH>Peer-to-peer video, voice, messaging</fil_PH>
   <fi>Peer-to-peer video, voice, messaging</fi>
   <fr>Messagerie vocale, vidéo, peer-to-peer</fr>
   <he_IL>Peer-to-peer video, voice, messaging</he_IL>
   <hi>Peer-to-peer video, voice, messaging</hi>
   <hr>Peer-to-peer video, voice, messaging</hr>
   <hu>Peer-to-peer video, voice, messaging</hu>
   <id>Peer-to-peer video, voice, messaging</id>
   <is>Peer-to-peer video, voice, messaging</is>
   <it>Peer-to-peer video, voce e messaggi</it>
   <ja_JP>Peer-to-peer video, voice, messaging</ja_JP>
   <ja>Peer-to-peer video, voice, messaging</ja>
   <kk>Peer-to-peer video, voice, messaging</kk>
   <ko>Peer-to-peer video, voice, messaging</ko>
   <lt>Peer-to-peer video, voice, messaging</lt>
   <mk>Peer-to-peer video, voice, messaging</mk>
   <mr>Peer-to-peer video, voice, messaging</mr>
   <nb>Peer-to-peer video, voice, messaging</nb>
   <nl>Peer-to-peer video, voice, berichten</nl>
   <pl>wideo, głos, przesyłanie wiadomości typu peer-to-peer</pl>
   <pt_BR>Peer-to-peer video, voice, messaging</pt_BR>
   <pt>Peer-to-peer video, voice, messaging</pt>
   <ro>Peer-to-peer video, voice, messaging</ro>
   <ru>Голосовая и видеосвязь (IP-телефония)</ru>
   <sk>Peer-to-peer video, voice, messaging</sk>
   <sl>Peer-to-peer video, zvok, sporočila</sl>
   <sq>Peer-to-peer video, voice, messaging</sq>
   <sr>Peer-to-peer video, voice, messaging</sr>
   <sv>Peer-to-peer video, röst, meddelandetjänst</sv>
   <tr>Peer-to-peer video, voice, messaging</tr>
   <uk>Peer-to-peer video, voice, messaging</uk>
   <vi>Peer-to-peer video, voice, messaging</vi>
   <zh_CN>Peer-to-peer video, voice, messaging</zh_CN>
   <zh_TW>Peer-to-peer video, voice, messaging</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
wget https://repo.skype.com/latest/skypeforlinux-64.deb
dpkg -i skypeforlinux-64.deb
apt-get -f install
rm skypeforlinux-64.deb
</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
skypeforlinux
</uninstall_package_names>
</app>
