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
   <el>Το τελευταίο πρόγραμμα περιήγησης Google Chrome (σταθερό)</el>
   <en>Latest Google Chrome browser (stable)</en>
   <es>El último Google Chrome (estable)</es>
   <et>Latest Google Chrome browser (stable)</et>
   <eu>Latest Google Chrome browser (stable)</eu>
   <fa>Latest Google Chrome browser (stable)</fa>
   <fil_PH>Latest Google Chrome browser (stable)</fil_PH>
   <fi>Viimeisin Google Chrome-selain (vakaa)</fi>
   <fr>La dernière version de Google Chrome (stable)</fr>
   <he_IL>Latest Google Chrome browser (stable)</he_IL>
   <hi>नवीनतम गूगल क्रोम ब्राउज़र (स्थिर)</hi>
   <hr>Latest Google Chrome browser (stable)</hr>
   <hu>Latest Google Chrome browser (stable)</hu>
   <id>Latest Google Chrome browser (stable)</id>
   <is>Latest Google Chrome browser (stable)</is>
   <it>Ultima versione (stabile) del browser Google Chrome</it>
   <ja>最新のGoogle Chromeブラウザ（安定版）</ja>
   <kk>Latest Google Chrome browser (stable)</kk>
   <ko>Latest Google Chrome browser (stable)</ko>
   <lt>Latest Google Chrome browser (stable)</lt>
   <mk>Latest Google Chrome browser (stable)</mk>
   <mr>Latest Google Chrome browser (stable)</mr>
   <nb>Seneste Google Chrome-nettleser (stabil)</nb>
   <nl>Meest recente Chrome browser (stable)</nl>
   <pl>najnowsza przeglądarka Google Chrome (stabilna)</pl>
   <pt_BR>Google Chrome - Versão mais recente e estável/stable do navegador de internet</pt_BR>
   <pt>Versão mais recente (estável) do navegador web Google Chrome</pt>
   <ro>Latest Google Chrome browser (stable)</ro>
   <ru>Браузер Google Chrome (последняя стабильная версия)</ru>
   <sk>Posledný Google Chrome prehliadač (stabilný)</sk>
   <sl>Zadnja različica googlovega Chrome brskalnika</sl>
   <sq>Shfletuesi Google Chrome më i ri (versioni i qëndrueshëm)</sq>
   <sr>Latest Google Chrome browser (stable)</sr>
   <sv>SenasteGoogle Chrome webbläsare (stable)</sv>
   <tr>Enson Google Chrome tarayıcı (kararlı)</tr>
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
#cleanup
#remove any temporary google keys
rm -f /etc/apt/trusted.gpg.d/google-chrome.gpg
#remove old or temporary sources, install will replace them, refresh sources in this case
if [ -e "/etc/apt/sources.list.d/google-chrome.list" ] || [ -e "/etc/apt/sources.list.d/mxpitemp.sources" ] ; then
	rm -f /etc/apt/sources.list.d/google-chrome.list
	rm -f /etc/apt/sources.list.d/mxpitemp.sources
	apt-get update
fi
# only add a temporary repo if no Google Chrome apt repo is still enabled after cleanup
if apt-get indextargets --format '$(REPO_URI)' 2>/dev/null | grep -q 'dl\.google\.com/linux/chrome'; then
   echo "Google Chrome apt repo already enabled, not adding temporary repo"
else
   wget -q -O - https://dl.google.com/linux/linux_signing_key.pub | gpg --dearmor | tee /etc/apt/trusted.gpg.d/google-chrome.gpg >/dev/null
   echo "X-Repolib-Name: Google Chrome
Types: deb
URIs: https://dl.google.com/linux/chrome-stable/deb/
Suites: stable
Components: main
Architectures: amd64
Signed-By: /etc/apt/trusted.gpg.d/google-chrome.gpg" > /etc/apt/sources.list.d/mxpitemp.sources
fi
apt-get update
</preinstall>

<install_package_names>
google-chrome-stable
</install_package_names>


<postinstall>
# only clean up if the temporary repo was set up in preinstall
if [ -f /etc/apt/sources.list.d/mxpitemp.sources ]; then
   rm -f /etc/apt/sources.list.d/mxpitemp.sources
   rm -f /etc/apt/trusted.gpg.d/google-chrome.gpg
fi
</postinstall>


<uninstall_package_names>
google-chrome-stable
</uninstall_package_names>
<postuninstall>
rm -f /etc/apt/sources.list.d/google-chrome.sources
apt-get update
</postuninstall>
</app>
