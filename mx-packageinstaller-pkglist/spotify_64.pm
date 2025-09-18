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
   <ca>Spotify del dipòsit Spotify Testing</ca>
   <cs>Spotify from Spotify Repo</cs>
   <da>Spotify fra Spotify softwarekilde</da>
   <de>Spotify vom Spotify Testing Repo</de>
   <el>Spotify από το Spotify Test Repo</el>
   <en>Spotify from Spotify Repo</en>
   <es>Spotify de los repositorios Testing de Spotify</es>
   <et>Spotify from Spotify Repo</et>
   <eu>Spotify from Spotify Repo</eu>
   <fa>Spotify from Spotify Repo</fa>
   <fil_PH>Spotify from Spotify Repo</fil_PH>
   <fi>Spotify ohjelmavarastosta Spotify testaus</fi>
   <fr>Spotify du dépôt de test Spotify </fr>
   <he_IL>Spotify from Spotify Repo</he_IL>
   <hi>Spotify परीक्षण पैकेज-संग्रह से Spotify</hi>
   <hr>Spotify from Spotify Repo</hr>
   <hu>Spotify from Spotify Repo</hu>
   <id>Spotify from Spotify Repo</id>
   <is>Spotify from Spotify Repo</is>
   <it>Spotify dal repo testing di Spotify</it>
   <ja>Spotify テストレポより Spotify</ja>
   <kk>Spotify from Spotify Repo</kk>
   <ko>Spotify from Spotify Repo</ko>
   <lt>Spotify from Spotify Repo</lt>
   <mk>Spotify from Spotify Repo</mk>
   <mr>Spotify from Spotify Repo</mr>
   <nb>Spotify fra Spotify Testing-pakkearkiv</nb>
   <nl>Spotify uit Spotify Testing Pakketbron</nl>
   <pl>Spotify z repozytorium testowego Spotify</pl>
   <pt_BR>Spotify (serviços de streaming de músicas) do Repositório de Teste/Test</pt_BR>
   <pt>Spotify do repositório Spotify Testing Repo</pt>
   <ro>Spotify from Spotify Repo</ro>
   <ru>Spotify из тестового репозитория Spotify</ru>
   <sk>Spotify from Spotify Repo</sk>
   <sl>Spotify iz Spotify testnega repozitorija</sl>
   <sq>Spotify nga Depoja e Testimit të Spotify-it</sq>
   <sr>Spotify from Spotify Repo</sr>
   <sv>Spotify från Spotify Testing Förråd</sv>
   <tr>Spotify Test Deposundan Spotify</tr>
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
curl -sS curl -sS https://download.spotify.com/debian/pubkey_C85668DF69375001.gpg | gpg --dearmor --yes -o /etc/apt/trusted.gpg.d/spotify.gpg
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
