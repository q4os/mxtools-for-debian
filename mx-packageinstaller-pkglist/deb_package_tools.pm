<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Development
</category>

<name>
deb packaging tools
</name>

<description>
   <am>tools for creation of deb packages</am>
   <ar>tools for creation of deb packages</ar>
   <bg>tools for creation of deb packages</bg>
   <bn>tools for creation of deb packages</bn>
   <ca>eines per a la creació de paquets deb</ca>
   <cs>tools for creation of deb packages</cs>
   <da>værktøjer til at oprette deb-pakker</da>
   <de>Werkzeuge zur Erstellung von Deb-Paketen</de>
   <el>Εργαλεία για τη δημιουργία πακέτων deb</el>
   <en>tools for creation of deb packages</en>
   <es>Herramientas para la creación de paquetes deb</es>
   <et>tools for creation of deb packages</et>
   <eu>tools for creation of deb packages</eu>
   <fa>tools for creation of deb packages</fa>
   <fil_PH>tools for creation of deb packages</fil_PH>
   <fi>*deb pakettien luontityökalu</fi>
   <fr>Outils pour la création de paquets .deb</fr>
   <he_IL>tools for creation of deb packages</he_IL>
   <hi>Deb पैकेज सृजन हेतु साधन</hi>
   <hr>tools for creation of deb packages</hr>
   <hu>tools for creation of deb packages</hu>
   <id>tools for creation of deb packages</id>
   <is>tools for creation of deb packages</is>
   <it>Strumenti per la creazione di pacchetti deb</it>
   <ja>deb パッケージ作成ツール</ja>
   <kk>tools for creation of deb packages</kk>
   <ko>tools for creation of deb packages</ko>
   <lt>įrankiai, skirti deb paketų kūrimui</lt>
   <mk>tools for creation of deb packages</mk>
   <mr>tools for creation of deb packages</mr>
   <nb>verktøy for å lage deb-pakker</nb>
   <nl>gereedschappen voor het maken van deb pakketten</nl>
   <pl>narzędzia do tworzenia pakietów deb</pl>
   <pt_BR>Ferramentas para criação de pacotes deb</pt_BR>
   <pt>Ferramentas para criar pacotes deb</pt>
   <ro>tools for creation of deb packages</ro>
   <ru>Инструменты для создания deb пакетов</ru>
   <sk>Nástroje na vytváranie deb balíkov</sk>
   <sl>orodja za izdelavo deb paketov</sl>
   <sq>mjete për krijim paketash deb</sq>
   <sr>tools for creation of deb packages</sr>
   <sv>verktyg för att skapa deb paket</sv>
   <tr>deb paketleri oluşturma araçları</tr>
   <uk>утиліти для створення  deb пакунків</uk>
   <vi>tools for creation of deb packages</vi>
   <zh_CN>tools for creation of deb packages</zh_CN>
   <zh_TW>tools for creation of deb packages</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>

<preinstall>
#!/bin/bash
cp /etc/apt/sources.list.d/debian.list /etc/apt/pbuilder.list
</preinstall>

<install_package_names>
pbuilder
debhelper
libparse-debcontrol-perl
devscripts
dh-make
fakeroot
build-essential
dpkg-dev
</install_package_names>


<postinstall>
#!/bin/bash
rm -f /etc/apt/pbuilder.list
</postinstall>


<uninstall_package_names>
pbuilder
debhelper
libparse-debcontrol-perl
devscripts
dh-make
fakeroot
dpkg-dev
</uninstall_package_names>
</app>
