<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Audio
</category>

<name>
XMMS
</name>

<description>
   <am>multimedia player modelled on winamp</am>
   <ar>multimedia player modelled on winamp</ar>
   <bg>multimedia player modelled on winamp</bg>
   <bn>multimedia player modelled on winamp</bn>
   <ca>Reproductor multimèdia modelat en winamp</ca>
   <cs>multimedia player modelled on winamp</cs>
   <da>multimedieafspiller som er udformet efter winamp</da>
   <de>Multimedia-Player nach dem Vorbild von Winamp</de>
   <el>αναπαραγωγή πολυμέσων με βάση το winamp</el>
   <en>multimedia player modelled on winamp</en>
   <es>Reproductor multimedia estilo Winamp</es>
   <et>multimedia player modelled on winamp</et>
   <eu>multimedia player modelled on winamp</eu>
   <fa>multimedia player modelled on winamp</fa>
   <fil_PH>multimedia player modelled on winamp</fil_PH>
   <fi>multimedia player modelled on winamp</fi>
   <fr>Lecteur multimédia inspiré de winamp</fr>
   <he_IL>multimedia player modelled on winamp</he_IL>
   <hi>multimedia player modelled on winamp</hi>
   <hr>multimedia player modelled on winamp</hr>
   <hu>multimedia player modelled on winamp</hu>
   <id>multimedia player modelled on winamp</id>
   <is>multimedia player modelled on winamp</is>
   <it>riproduttore multimediale simile a winamp</it>
   <ja_JP>multimedia player modelled on winamp</ja_JP>
   <ja>multimedia player modelled on winamp</ja>
   <kk>multimedia player modelled on winamp</kk>
   <ko>multimedia player modelled on winamp</ko>
   <lt>multimedia player modelled on winamp</lt>
   <mk>multimedia player modelled on winamp</mk>
   <mr>multimedia player modelled on winamp</mr>
   <nb>multimedia player modelled on winamp</nb>
   <nl>multimediaspeler gemodelleerd naar winamp</nl>
   <pl>odtwarzacz multimedialny wzorowany na Winampie</pl>
   <pt_BR>Reprodutor de multimídia semelhante ao winamp</pt_BR>
   <pt>Reprodutor de multimédia semelhante ao winamp</pt>
   <ro>multimedia player modelled on winamp</ro>
   <ru>Универсальный аудиоплеер наподобие WinAmp</ru>
   <sk>multimedia player modelled on winamp</sk>
   <sl>Večpredstavnostni predvajalnik, ki temelji na Winampu</sl>
   <sq>multimedia player modelled on winamp</sq>
   <sr>multimedia player modelled on winamp</sr>
   <sv>multimediaspelare formad efter winamp</sv>
   <tr>multimedia player modelled on winamp</tr>
   <uk>мультимедія програвач схожий на winamp</uk>
   <vi>multimedia player modelled on winamp</vi>
   <zh_CN>multimedia player modelled on winamp</zh_CN>
   <zh_TW>multimedia player modelled on winamp</zh_TW>
</description>

<installable>
32,64
</installable>

<screenshot>http://www.xmms.org/screenshots/main.gif</screenshot>

<preinstall>
apt-get install antix-archive-keyring
echo "deb http://la.mxrepo.com/antix/bullseye bullseye main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
</preinstall>

<install_package_names>
xmms
xmms-plugins-antix
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
apt-get update
</postinstall>


<uninstall_package_names>
xmms
xmms-plugins-antix
</uninstall_package_names>
</app>
