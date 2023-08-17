<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Media Center
</category>

<name>
Plex
</name>

<description>
   <am>Plex Mediaserver (includes web client)(requires reboot after install)</am>
   <ar>Plex Mediaserver (includes web client)(requires reboot after install)</ar>
   <bg>Plex Mediaserver (includes web client)(requires reboot after install)</bg>
   <bn>Plex Mediaserver (includes web client)(requires reboot after install)</bn>
   <ca>Mediaserver Plex (inclou client web; cal rearrencar després d'instal·lar)</ca>
   <cs>Plex Mediaserver (includes web client)(requires reboot after install)</cs>
   <da>Plex-medieserver (inkluderer webklient)(kræver genstart efter installation)</da>
   <de>Plex Mediaserver (inkl. Web-Client)(erfordert Neustart nach der Installation)</de>
   <el>Plex Media Server (περιλαμβάνει web client) (απαιτεί επανεκκίνηση μετά την εγκατάσταση)</el>
   <en>Plex Mediaserver (includes web client)(requires reboot after install)</en>
   <es>Plex Mediaserver (incluye cliente web)(requiere reiniciar después de instalar)</es>
   <et>Plex Mediaserver (includes web client)(requires reboot after install)</et>
   <eu>Plex Mediaserver (includes web client)(requires reboot after install)</eu>
   <fa>Plex Mediaserver (includes web client)(requires reboot after install)</fa>
   <fil_PH>Plex Mediaserver (includes web client)(requires reboot after install)</fil_PH>
   <fi>Plex - mediapalvelin (sisältää verkkoasiakasohjelman) (vaatii uudelleenkäynnistyksen asennuksen jälkeen)</fi>
   <fr>Plex Mediaserver (client web inclus)(nécessite un re-démarrage après installation)</fr>
   <he_IL>Plex Mediaserver (includes web client)(requires reboot after install)</he_IL>
   <hi>प्लेक्स मीडिया सर्वर (वेब साधन युक्त)(इंस्टॉल उपरांत बूट आवश्यक)</hi>
   <hr>Plex Mediaserver (includes web client)(requires reboot after install)</hr>
   <hu>Plex Mediaserver (includes web client)(requires reboot after install)</hu>
   <id>Plex Mediaserver (includes web client)(requires reboot after install)</id>
   <is>Plex Mediaserver (includes web client)(requires reboot after install)</is>
   <it>Plex Mediaserver (include un client web)(richiede un reboot dopo l'installazione)</it>
   <ja>Plex メディアサーバー (ウェブクライアントを含む) (インストール後に再起動が必要)</ja>
   <kk>Plex Mediaserver (includes web client)(requires reboot after install)</kk>
   <ko>Plex Mediaserver (includes web client)(requires reboot after install)</ko>
   <lt>Plex Mediaserver (includes web client)(requires reboot after install)</lt>
   <mk>Plex Mediaserver (includes web client)(requires reboot after install)</mk>
   <mr>Plex Mediaserver (includes web client)(requires reboot after install)</mr>
   <nb>Plex Mediaserver (inkluderer nettklient) (maskinen må omstartes etter installasjon)</nb>
   <nl>Plex Mediaserver (inclusief webcliënt) (moet na installatie opnieuw worden opgestart)</nl>
   <pl>Plex Media Server (zawiera klienta WWW) (wymaga ponownego uruchomienia po instalacji)</pl>
   <pt_BR>Plex - Servidor multimídia (inclui cliente de internet/web, requer reinicialização do computador após a instalação)</pt_BR>
   <pt>Servidor multimédia Plex - inclui cliente de web (requere reinício do computador após a instalação)</pt>
   <ro>Plex Mediaserver (includes web client)(requires reboot after install)</ro>
   <ru>Медиасервер Plex (включает веб клиент) (после установки требуется перезагрузка)</ru>
   <sk>Plex Mediaserver (includes web client)(requires reboot after install)</sk>
   <sl>Plex medijski strežnik (vključuje web odjemalec) (zahteva ponovni zagon po namestitvi)</sl>
   <sq>Shërbyesi Plex për media (përfshin klient web)(lyp rinisje pas instalimi)</sq>
   <sr>Plex Mediaserver (includes web client)(requires reboot after install)</sr>
   <sv>Plex Mediaserver (inkluderar webbklient)(kräver omstart efter installation)</sv>
   <tr>Plex Mediaserver  (web istemcisi içerir) (kurulumdan sonra yeniden başlatma gerektirir)</tr>
   <uk>Plex Mediaserver (includes web client)(requires reboot after install)</uk>
   <vi>Plex Mediaserver (includes web client)(requires reboot after install)</vi>
   <zh_CN>Plex Mediaserver (includes web client)(requires reboot after install)</zh_CN>
   <zh_TW>Plex Mediaserver (includes web client)(requires reboot after install)</zh_TW>
</description>

<installable>
32,64
</installable>

<screenshot>https://www.plex.tv/wp-content/uploads/2021/05/image-avod-devices-all-may-2021-1440x872.png</screenshot>

<preinstall>
apt-get install apt-transport-https
curl https://downloads.plex.tv/plex-keys/PlexSign.key |tee /etc/apt/trusted.gpg.d/plex.asc
echo "deb https://downloads.plex.tv/repo/deb public main">/etc/apt/sources.list.d/plex-mx.list
apt-get update
</preinstall>

<install_package_names>
plexmediaserver
</install_package_names>


<postinstall>
if [ -x /usr/sbin/start_pms ]; then
apt-get install plex-sysvinit-compat
fi
</postinstall>

<postuninstall>
apt-get remove plex-sysvinit-compat
</postuninstall>

<uninstall_package_names>
plexmediaserver
</uninstall_package_names>
</app>
