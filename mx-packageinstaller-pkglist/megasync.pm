<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Network
</category>

<name>
Megasync
</name>

<description>
   <am>Client for MEGA file syncing service</am>
   <ar>Client for MEGA file syncing service</ar>
   <bg>Client for MEGA file syncing service</bg>
   <bn>Client for MEGA file syncing service</bn>
   <ca>Client per al servei de sincronització de MEGA</ca>
   <cs>Client for MEGA file syncing service</cs>
   <da>Klient til MEGA-filsynkroniseringstjeneste</da>
   <de>Client für MEGA-Dateisynchronisationsdienst</de>
   <el>Πελάτης για υπηρεσία συγχρονισμού αρχείων MEGA</el>
   <en>Client for MEGA file syncing service</en>
   <es>Cliente para el servicio de sincronización de archivos de MEGA</es>
   <et>Client for MEGA file syncing service</et>
   <eu>Client for MEGA file syncing service</eu>
   <fa>Client for MEGA file syncing service</fa>
   <fil_PH>Client for MEGA file syncing service</fil_PH>
   <fi>Client for MEGA file syncing service</fi>
   <fr>Client pour le service de synchronisation de dossiers MEGA</fr>
   <he_IL>Client for MEGA file syncing service</he_IL>
   <hi>Client for MEGA file syncing service</hi>
   <hr>Client for MEGA file syncing service</hr>
   <hu>Client for MEGA file syncing service</hu>
   <id>Client for MEGA file syncing service</id>
   <is>Client for MEGA file syncing service</is>
   <it>Client per il servizio di sincronizzazione file di MEGA</it>
   <ja_JP>Client for MEGA file syncing service</ja_JP>
   <ja>Client for MEGA file syncing service</ja>
   <kk>Client for MEGA file syncing service</kk>
   <ko>Client for MEGA file syncing service</ko>
   <lt>Client for MEGA file syncing service</lt>
   <mk>Client for MEGA file syncing service</mk>
   <mr>Client for MEGA file syncing service</mr>
   <nb>Client for MEGA file syncing service</nb>
   <nl>Programma voor MEGA bestandssynchronisatieservice</nl>
   <pl>klient dla usługi synchronizacji plików MEGA</pl>
   <pt_BR>Aplicativo cliente para o serviço de sincronização de arquivos da MEGA</pt_BR>
   <pt>Aplicação cliente para o serviço de sincronização de ficheiros da MEGA</pt>
   <ro>Client for MEGA file syncing service</ro>
   <ru>Клиент синхронизации файлов для MEGA</ru>
   <sk>Client for MEGA file syncing service</sk>
   <sl>Odjemalec za sinhronizacijo datotek iz MEGA oblaka</sl>
   <sq>Client for MEGA file syncing service</sq>
   <sr>Client for MEGA file syncing service</sr>
   <sv>Klient för MEGA filsynkroniserings-service</sv>
   <tr>Client for MEGA file syncing service</tr>
   <uk>клієнт для сервісу синхронізації файлів MEGA</uk>
   <vi>Client for MEGA file syncing service</vi>
   <zh_CN>Client for MEGA file syncing service</zh_CN>
   <zh_TW>Client for MEGA file syncing service</zh_TW>
</description>

<installable>
32,64
</installable>

<screenshot></screenshot>

<preinstall>
echo "deb https://mega.nz/linux/MEGAsync/Debian_11/ ./">/etc/apt/sources.list.d/mxpitemp.list
wget https://mega.nz/linux/MEGAsync/Debian_11/Release.key -O /tmp/megarelease.key
apt-key add /tmp/megarelease.key
rm /tmp/megarelease.key
apt-get update
</preinstall>

<install_package_names>
megasync
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
</postinstall>


<uninstall_package_names>
megasync
</uninstall_package_names>
</app>
