<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Browser
</category>

<name>
Opera
</name>

<description>
   <am>Opera browser (32 bit is beta-only)</am>
   <ar>Opera browser (32 bit is beta-only)</ar>
   <bg>Opera browser (32 bit is beta-only)</bg>
   <bn>Opera browser (32 bit is beta-only)</bn>
   <ca>Navegador Opera (32 bit només beta)</ca>
   <cs>Opera browser (32 bit is beta-only)</cs>
   <da>Opera-browser (32 bit er kun beta)</da>
   <de>Opera-Browser (32 Bit ist nur Beta-Version)</de>
   <el>Πρόγραμμα περιήγησης Opera (32 bit είναι μόνο beta)</el>
   <en>Opera browser (32 bit is beta-only)</en>
   <es>Navegador Opera (32 bit - solo versión beta)</es>
   <et>Opera browser (32 bit is beta-only)</et>
   <eu>Opera browser (32 bit is beta-only)</eu>
   <fa>Opera browser (32 bit is beta-only)</fa>
   <fil_PH>Opera browser (32 bit is beta-only)</fil_PH>
   <fi>Opera browser (32 bit is beta-only)</fi>
   <fr>Navigateur Opera (32 bit en beta uniquement)</fr>
   <he_IL>Opera browser (32 bit is beta-only)</he_IL>
   <hi>Opera browser (32 bit is beta-only)</hi>
   <hr>Opera browser (32 bit is beta-only)</hr>
   <hu>Opera browser (32 bit is beta-only)</hu>
   <id>Opera browser (32 bit is beta-only)</id>
   <is>Opera browser (32 bit is beta-only)</is>
   <it>Opera browser (a 32 bit è esclusivamente-beta)</it>
   <ja_JP>Opera browser (32 bit is beta-only)</ja_JP>
   <ja>Opera browser (32 bit is beta-only)</ja>
   <kk>Opera browser (32 bit is beta-only)</kk>
   <ko>Opera browser (32 bit is beta-only)</ko>
   <lt>Opera naršyklė (32 bitų yra tik beta versija)</lt>
   <mk>Opera browser (32 bit is beta-only)</mk>
   <mr>Opera browser (32 bit is beta-only)</mr>
   <nb>Opera browser (32 bit is beta-only)</nb>
   <nl>Opera browser (32 bit is enkel-beta)</nl>
   <pl>przeglądarka Opera (wersja 32-bitowa tylko w wersji beta)</pl>
   <pt_BR>Navegador web Opera (32 bit is beta-only)</pt_BR>
   <pt>Navegador web Opera (32 bit is beta-only)</pt>
   <ro>Opera browser (32 bit is beta-only)</ro>
   <ru>Браузер Opera (32 bit бета-версия)</ru>
   <sk>Opera browser (32 bit is beta-only)</sk>
   <sl>Opera brskalnik (32 bitni je zgolj beta)</sl>
   <sq>Opera browser (32 bit is beta-only)</sq>
   <sr>Opera browser (32 bit is beta-only)</sr>
   <sv>Opera webbläsare (32 bit är enbart i beta)</sv>
   <tr>Opera tarayıcı (32 bit yalnızca betadır)</tr>
   <uk>Браузер Opera (32 біт лише у бета-версії)</uk>
   <vi>Opera browser (32 bit is beta-only)</vi>
   <zh_CN>Opera browser (32 bit is beta-only)</zh_CN>
   <zh_TW>Opera browser (32 bit is beta-only)</zh_TW>
</description>

<installable>
32
</installable>

<screenshot>http://www-static.operacdn.com/static-heap/e3/e3be37900045a27590062000a1e380e005644b8a/linux.png</screenshot>

<preinstall>
sed -i -r '/opera.com/ s/^#+//' /etc/apt/sources.list.d/various.list
wget -O - http://deb.opera.com/archive.key | apt-key add -
apt-get update
</preinstall>

<install_package_names>
opera-beta
</install_package_names>


<postinstall>
if [ -f /etc/apt/sources.list.d/opera-stable.list ]; then sed -i -r '/opera.com/ s/^([^#])/#\1/' /etc/apt/sources.list.d/various.list;apt-get update; fi
</postinstall>


<uninstall_package_names>
opera-beta
</uninstall_package_names>
</app>
