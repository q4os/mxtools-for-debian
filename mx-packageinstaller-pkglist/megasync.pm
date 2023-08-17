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
   <el>Πακέτο για την υπηρεσία συγχρονισμού αρχείων MEGA</el>
   <en>Client for MEGA file syncing service</en>
   <es>Cliente para el servicio de sincronización de archivos de MEGA</es>
   <et>Client for MEGA file syncing service</et>
   <eu>Client for MEGA file syncing service</eu>
   <fa>Client for MEGA file syncing service</fa>
   <fil_PH>Client for MEGA file syncing service</fil_PH>
   <fi>Asiakasohjelma MEGA-tiedostonsynkronointipalvelulle</fi>
   <fr>Client pour le service de synchronisation de dossiers MEGA</fr>
   <he_IL>Client for MEGA file syncing service</he_IL>
   <hi>MEGA फाइल समकालीन सेवा हेतु साधन</hi>
   <hr>Client for MEGA file syncing service</hr>
   <hu>Client for MEGA file syncing service</hu>
   <id>Client for MEGA file syncing service</id>
   <is>Client for MEGA file syncing service</is>
   <it>Client per il servizio di sincronizzazione file di MEGA</it>
   <ja>MEGA ファイル同期サービス用クライアント</ja>
   <kk>Client for MEGA file syncing service</kk>
   <ko>Client for MEGA file syncing service</ko>
   <lt>Client for MEGA file syncing service</lt>
   <mk>Client for MEGA file syncing service</mk>
   <mr>Client for MEGA file syncing service</mr>
   <nb>Klient for filsynkroniseringstjenesten MEGA</nb>
   <nl>Programma voor MEGA bestandssynchronisatieservice</nl>
   <pl>klient dla usługi synchronizacji plików MEGA</pl>
   <pt_BR>Aplicativo cliente para o serviço de sincronização de arquivos do MEGA</pt_BR>
   <pt>Aplicação cliente para o serviço de sincronização de ficheiros da MEGA</pt>
   <ro>Client for MEGA file syncing service</ro>
   <ru>Клиент синхронизации файлов для MEGA</ru>
   <sk>Client for MEGA file syncing service</sk>
   <sl>Odjemalec za sinhronizacijo datotek iz MEGA oblaka</sl>
   <sq>Klient për shërbimin MEGA të njëkohësimit të kartelave</sq>
   <sr>Client for MEGA file syncing service</sr>
   <sv>Klient för MEGA filsynkroniserings-service</sv>
   <tr>MEGA dosya eşzamanlama hizmeti için istemci</tr>
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
echo "deb https://mega.nz/linux/repo/Debian_12/ ./">/etc/apt/sources.list.d/mxpitemp.list
curl -sS https://mega.nz/linux/repo/Debian_12/Release.key | gpg --dearmor --yes -o /etc/apt/trusted.gpg.d/megasync.gpg
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
