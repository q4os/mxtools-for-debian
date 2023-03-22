<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Audio
</category>

<name>
Spotify
</name>

<description>
   <am>Spotify from Spotify Repo</am>
   <ar>Spotify from Spotify Repo</ar>
   <bg>Spotify from Spotify Repo</bg>
   <bn>Spotify from Spotify Repo</bn>
   <ca>Spotify del dipòsit Spotify </ca>
   <cs>Spotify from Spotify Repo</cs>
   <da>Spotify fra Spotify softwarekilde</da>
   <de>Spotify vom Spotify Repo</de>
   <el>Spotify από το Spotify Repo</el>
   <en>Spotify from Spotify Repo</en>
   <es>Spotify de los repositorios de Spotify</es>
   <et>Spotify from Spotify Repo</et>
   <eu>Spotify from Spotify Repo</eu>
   <fa>Spotify from Spotify Repo</fa>
   <fil_PH>Spotify from Spotify Repo</fil_PH>
   <fi>Spotify from Spotify Repo</fi>
   <fr>Spotify du dépôt de Spotify</fr>
   <he_IL>Spotify from Spotify Repo</he_IL>
   <hi>Spotify from Spotify Repo</hi>
   <hr>Spotify from Spotify Repo</hr>
   <hu>Spotify from Spotify Repo</hu>
   <id>Spotify from Spotify Repo</id>
   <is>Spotify from Spotify Repo</is>
   <it>Spotify dal repo di Spotify</it>
   <ja_JP>Spotify from Spotify Repo</ja_JP>
   <ja>Spotify from Spotify Repo</ja>
   <kk>Spotify from Spotify Repo</kk>
   <ko>Spotify from Spotify Repo</ko>
   <lt>Spotify from Spotify Repo</lt>
   <mk>Spotify from Spotify Repo</mk>
   <mr>Spotify from Spotify Repo</mr>
   <nb>Spotify from Spotify Repo</nb>
   <nl>Spotify uit Spotify Pakketbron</nl>
   <pl>Spotify z repozytorium Spotify</pl>
   <pt_BR>Spotify from Spotify Repo</pt_BR>
   <pt>Spotify do Spotify Repo</pt>
   <ro>Spotify from Spotify Repo</ro>
   <ru>Сервис потокового аудио включающий более 30 млн. </ru>
   <sk>Spotify from Spotify Repo</sk>
   <sl>Spotify iz Spotify repozitorija</sl>
   <sq>Spotify from Spotify Repo</sq>
   <sr>Spotify from Spotify Repo</sr>
   <sv>Spotify från Spotify Förråd</sv>
   <tr>Spotify from Spotify Repo</tr>
   <uk>Spotify from Spotify Repo</uk>
   <vi>Spotify from Spotify Testing Repo</vi>
   <zh_CN>Spotify from Spotify Repo</zh_CN>
   <zh_TW>Spotify from Spotify Repo</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
curl -sS https://download.spotify.com/debian/pubkey_7A3A762FAFD4A51F.gpg | sudo gpg --dearmor --yes -o /etc/apt/trusted.gpg.d/spotify.gpg
echo "deb http://repository.spotify.com stable non-free">/etc/apt/sources.list.d/spotify.list
apt-get update
</preinstall>

<install_package_names>
spotify-client
</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
spotify-client
</uninstall_package_names>

<postuninstall>
rm /etc/apt/trusted.gpg.d/repository-spotify-com-keyring.gpg
</postuninstall>
</app>
