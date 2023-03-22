<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Browser
</category>

<name>
Google Chrome
</name>

<description>
   <am>Latest Google Chrome browser (stable)</am>
   <ar>Latest Google Chrome browser (stable)</ar>
   <bg>Latest Google Chrome browser (stable)</bg>
   <bn>Latest Google Chrome browser (stable)</bn>
   <ca>Darrer navegador Chrome (estable)</ca>
   <cs>Latest Google Chrome browser (stable)</cs>
   <da>Seneste Google Chrome-browser (stabil)</da>
   <de>Aktueller Google Chrome-Browser (stabil)</de>
   <el>Τελευταίο πρόγραμμα περιήγησης Google Chrome (σταθερό)</el>
   <en>Latest Google Chrome browser (stable)</en>
   <es>El último Google Chrome (estable)</es>
   <et>Latest Google Chrome browser (stable)</et>
   <eu>Latest Google Chrome browser (stable)</eu>
   <fa>Latest Google Chrome browser (stable)</fa>
   <fil_PH>Latest Google Chrome browser (stable)</fil_PH>
   <fi>Latest Google Chrome browser (stable)</fi>
   <fr>La dernière version de Google Chrome (stable)</fr>
   <he_IL>Latest Google Chrome browser (stable)</he_IL>
   <hi>Latest Google Chrome browser (stable)</hi>
   <hr>Latest Google Chrome browser (stable)</hr>
   <hu>Latest Google Chrome browser (stable)</hu>
   <id>Latest Google Chrome browser (stable)</id>
   <is>Latest Google Chrome browser (stable)</is>
   <it>Ultima versione (stabile) del browser Google Chrome</it>
   <ja_JP>Latest Google Chrome browser (stable)</ja_JP>
   <ja>Latest Google Chrome browser (stable)</ja>
   <kk>Latest Google Chrome browser (stable)</kk>
   <ko>Latest Google Chrome browser (stable)</ko>
   <lt>Latest Google Chrome browser (stable)</lt>
   <mk>Latest Google Chrome browser (stable)</mk>
   <mr>Latest Google Chrome browser (stable)</mr>
   <nb>Latest Google Chrome browser (stable)</nb>
   <nl>Meest recente Chrome browser (stable)</nl>
   <pl>najnowsza przeglądarka Google Chrome (stabilna)</pl>
   <pt_BR>Versão mais recente (estável) do navegador web Google Chrome</pt_BR>
   <pt>Versão mais recente (estável) do navegador web Google Chrome</pt>
   <ro>Latest Google Chrome browser (stable)</ro>
   <ru>Браузер Google Chrome (последняя стабильная версия)</ru>
   <sk>Posledný Google Chrome prehliadač (stabilný)</sk>
   <sl>Zadnja različica googlovega Chrome brskalnika</sl>
   <sq>Latest Google Chrome browser (stable)</sq>
   <sr>Latest Google Chrome browser (stable)</sr>
   <sv>SenasteGoogle Chrome webbläsare (stable)</sv>
   <tr>Latest Google Chrome browser (stable)</tr>
   <uk>Крайня стабільна версія браузера Google Chrome</uk>
   <vi>Latest Google Chrome browser (stable)</vi>
   <zh_CN>Latest Google Chrome browser (stable)</zh_CN>
   <zh_TW>Latest Google Chrome browser (stable)</zh_TW>
</description>

<installable>
 64
</installable>

<screenshot>https://www.google.com/chrome/assets/common/images/marquee/chrome-new.jpg</screenshot>

<preinstall>
wget -q -O - https://dl.google.com/linux/linux_signing_key.pub | sudo apt-key add -
echo "deb [arch=amd64] http://dl.google.com/linux/chrome/deb/ stable main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
</preinstall>

<install_package_names>
google-chrome-stable
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
</postinstall>


<uninstall_package_names>
google-chrome-stable
</uninstall_package_names>
</app>
