<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Remote Access
</category>

<name>
1-to-1 Assistance
</name>

<description>
   <am>Remote access help app from antiX</am>
   <ar>Remote access help app from antiX</ar>
   <bg>Remote access help app from antiX</bg>
   <bn>Remote access help app from antiX</bn>
   <ca>Eina d'ajuda d'accés remot d'antiX</ca>
   <cs>Remote access help app from antiX</cs>
   <da>Fjernadgang hjælpeprogram til antiX</da>
   <de>Anwendungshilfe von antiX für die Fernwartung</de>
   <el>Εφαρμογή απομακρυσμένης πρόσβασης από το antiX</el>
   <en>Remote access help app from antiX</en>
   <es>Aplicación de ayuda de acceso remoto desde AntiX</es>
   <et>Remote access help app from antiX</et>
   <eu>Remote access help app from antiX</eu>
   <fa>Remote access help app from antiX</fa>
   <fil_PH>Remote access help app from antiX</fil_PH>
   <fi>Remote access help app from antiX</fi>
   <fr>Application d'aide pour l'accès à distance d'antiX</fr>
   <he_IL>Remote access help app from antiX</he_IL>
   <hi>Remote access help app from antiX</hi>
   <hr>Remote access help app from antiX</hr>
   <hu>Remote access help app from antiX</hu>
   <id>Remote access help app from antiX</id>
   <is>Remote access help app from antiX</is>
   <it>App da antiX, che permette un accesso remoto</it>
   <ja_JP>Remote access help app from antiX</ja_JP>
   <ja>Remote access help app from antiX</ja>
   <kk>Remote access help app from antiX</kk>
   <ko>Remote access help app from antiX</ko>
   <lt>Nuotolinės prieigos pagalbos programa iš antiX</lt>
   <mk>Remote access help app from antiX</mk>
   <mr>Remote access help app from antiX</mr>
   <nb>Remote access help app from antiX</nb>
   <nl>Afstand toegankelijke hulp app van antiX</nl>
   <pl>aplikacja zdalnej pomocy od antiX</pl>
   <pt_BR>Aplicativo do antiX para ajuda por acesso remoto</pt_BR>
   <pt>Aplicação do antiX para ajuda por acesso remoto</pt>
   <ro>Remote access help app from antiX</ro>
   <ru>Приложение для помощи через удаленный доступ</ru>
   <sk>Remote access help app from antiX</sk>
   <sl>antiX pomoč za oddaljni dostop</sl>
   <sq>Remote access help app from antiX</sq>
   <sr>Remote access help app from antiX</sr>
   <sv>Hjälp-app för fjärråtkomst från antiX</sv>
   <tr>AntiX'ten uzaktan erişim yardımı uygulaması</tr>
   <uk>Програма віддаленої підтримки від antiX</uk>
   <vi>Remote access help app from antiX</vi>
   <zh_CN>Remote access help app from antiX</zh_CN>
   <zh_TW>Remote access help app from antiX</zh_TW>
</description>

<installable>
32,64
</installable>

<screenshot>none</screenshot>

<preinstall>
apt-get install antix-archive-keyring
echo "deb http://la.mxrepo.com/antix/buster buster main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
</preinstall>

<install_package_names>
1-to-1-assistance-antix
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
apt-get update
</postinstall>


<uninstall_package_names>
1-to-1-assistance-antix
</uninstall_package_names>
</app>
