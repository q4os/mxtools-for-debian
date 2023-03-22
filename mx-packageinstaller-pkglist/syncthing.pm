<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Network
</category>

<name>
Syncthing
</name>

<description>
   <am>Syncthing file sync utility</am>
   <ar>Syncthing file sync utility</ar>
   <bg>Syncthing file sync utility</bg>
   <bn>Syncthing file sync utility</bn>
   <ca>Utilitat de sincronització de fitxers Syncthing</ca>
   <cs>Syncthing file sync utility</cs>
   <da>Syncthing file sync utility</da>
   <de>Syncthing Dateisynchronisationsprogramm</de>
   <el>Συγχρονισμός χρησιμότητας συγχρονισμού αρχείων</el>
   <en>Syncthing file sync utility</en>
   <es>Utilidad de sincronización de archivos de sincronización</es>
   <et>Syncthing file sync utility</et>
   <eu>Syncthing file sync utility</eu>
   <fa>Syncthing file sync utility</fa>
   <fil_PH>Syncthing file sync utility</fil_PH>
   <fi>Syncthing file sync utility</fi>
   <fr>Utilitaire de synchronisation de fichier Syncthing</fr>
   <he_IL>Syncthing file sync utility</he_IL>
   <hi>Syncthing file sync utility</hi>
   <hr>Syncthing file sync utility</hr>
   <hu>Syncthing file sync utility</hu>
   <id>Syncthing file sync utility</id>
   <is>Syncthing file sync utility</is>
   <it>Syncthing applicazione di sincronizzazione di file</it>
   <ja_JP>Syncthing file sync utility</ja_JP>
   <ja>Syncthing file sync utility</ja>
   <kk>Syncthing file sync utility</kk>
   <ko>Syncthing file sync utility</ko>
   <lt>Syncthing file sync utility</lt>
   <mk>Syncthing file sync utility</mk>
   <mr>Syncthing file sync utility</mr>
   <nb>Syncthing file sync utility</nb>
   <nl>Syncthing file synchronisatie hulpprogramma</nl>
   <pl>Syncthing file sync utility</pl>
   <pt_BR>Utilitário de sincronização de arquivos</pt_BR>
   <pt>Utilitário de sincronização de ficheiros</pt>
   <ro>Syncthing file sync utility</ro>
   <ru>Syncthing - приложение распределенной синхронизации файлов</ru>
   <sk>Syncthing file sync utility</sk>
   <sl>Syncthing orodje za sinhronizacijo datotek</sl>
   <sq>Syncthing file sync utility</sq>
   <sr>Syncthing file sync utility</sr>
   <sv>Syncthing filsynkroniserings-redskap</sv>
   <tr>Syncthing file sync utility</tr>
   <uk>Syncthing file sync utility</uk>
   <vi>Syncthing file sync utility</vi>
   <zh_CN>Syncthing file sync utility</zh_CN>
   <zh_TW>Syncthing file sync utility</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>

<preinstall>
curl -s https://syncthing.net/release-key.txt | apt-key add -
echo "deb https://apt.syncthing.net/ syncthing stable">/etc/apt/sources.list.d/syncthing.list
apt-get update
</preinstall>

<install_package_names>
syncthing
</install_package_names>


<postinstall>
cp /usr/share/applications/syncthing-start.desktop /etc/xdg/autostart
</postinstall>


<uninstall_package_names>
syncthing
</uninstall_package_names>
</app>
