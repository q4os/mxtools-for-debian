<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Browser
</category>

<name>
LibreWolf
</name>

<description>
   <am>Latest LibreWolf browser (stable)</am>
   <ar>Latest LibreWolf browser (stable)</ar>
   <bg>Latest LibreWolf browser (stable)</bg>
   <bn>Latest LibreWolf browser (stable)</bn>
   <ca>Darrer navegador LibreWolf (estable)</ca>
   <cs>Latest LibreWolf browser (stable)</cs>
   <da>Seneste LibreWolf-browser (stabil)</da>
   <de>Aktueller LibreWolf-Browser (stabil)</de>
   <el>Τελευταίο πρόγραμμα περιήγησης LibreWolf (σταθερό)</el>
   <en>Latest LibreWolf browser (stable)</en>
   <es>El último LibreWolf (estable)</es>
   <et>Latest LibreWolf browser (stable)</et>
   <eu>Latest LibreWolf browser (stable)</eu>
   <fa>Latest LibreWolf browser (stable)</fa>
   <fil_PH>Latest LibreWolf browser (stable)</fil_PH>
   <fi>Latest LibreWolf browser (stable)</fi>
   <fr>La dernière version de LibreWolf (stable)</fr>
   <he_IL>Latest LibreWolf browser (stable)</he_IL>
   <hi>Latest LibreWolf browser (stable)</hi>
   <hr>Latest LibreWolf browser (stable)</hr>
   <hu>Latest LibreWolf browser (stable)</hu>
   <id>Latest LibreWolf browser (stable)</id>
   <is>Latest LibreWolf browser (stable)</is>
   <it>Ultima versione (stabile) del browser LibreWolf</it>
   <ja>Latest LibreWolf browser (stable)</ja>
   <kk>Latest LibreWolf browser (stable)</kk>
   <ko>Latest LibreWolf browser (stable)</ko>
   <lt>Latest LibreWolf browser (stable)</lt>
   <mk>Latest LibreWolf browser (stable)</mk>
   <mr>Latest LibreWolf browser (stable)</mr>
   <nb>Latest LibreWolf browser (stable)</nb>
   <nl>Meest recente LibreWolf browser (stable)</nl>
   <pl>najnowsza przeglądarka LibreWolf (stabilna)</pl>
   <pt_BR>Versão mais recente (estável) do navegador web LibreWolf</pt_BR>
   <pt>Versão mais recente (estável) do navegador web LibreWolf</pt>
   <ro>Latest LibreWolf browser (stable)</ro>
   <ru>Браузер LibreWolf (последняя стабильная версия)</ru>
   <sk>Posledný LibreWolf prehliadač (stabilný)</sk>
   <sl>Zadnja različica googlovega LibreWolf brskalnika</sl>
   <sq>Latest LibreWolf browser (stable)</sq>
   <sr>Latest LibreWolf browser (stable)</sr>
   <sv>SenasteLibreWolf webbläsare (stable)</sv>
   <tr>Latest LibreWolf browser (stable)</tr>
   <uk>Крайня стабільна версія браузера LibreWolf</uk>
   <vi>Latest LibreWolf browser (stable)</vi>
   <zh_CN>Latest LibreWolf browser (stable)</zh_CN>
   <zh_TW>Latest LibreWolf browser (stable)</zh_TW>
</description>

<installable>
 64
</installable>

<screenshot></screenshot>

<preinstall>
wget https://deb.librewolf.net/keyring.gpg -O /etc/apt/trusted.gpg.d/librewolf.gpg
echo "deb [arch=amd64] http://deb.librewolf.net bookworm main">/etc/apt/sources.list.d/librewolf.list
apt-get update
</preinstall>

<install_package_names>
librewolf
</install_package_names>


<postinstall>
if [ -d /usr/share/xfce4/helpers ]; then
if ! [ -f /usr/share/xfce4/helpers/librewolf.desktop ]; then

cat &lt;&lt;'EOF' &gt; /usr/share/xfce4/helpers/librewolf.desktop
[Desktop Entry]
Version=1.0
Encoding=UTF-8
Icon=librewolf
Name=LibreWolf
NoDisplay=true
StartupNotify=false
Terminal=false
Type=X-XFCE-Helper
X-XFCE-Binaries=librewolf;
X-XFCE-Category=WebBrowser
X-XFCE-Commands=%B;
X-XFCE-CommandsWithParameter=%B "%s";

EOF
fi
fi
</postinstall>

<uninstall_package_names>
librewolf
</uninstall_package_names>

<postuninstall>
if [ -f /usr/share/xfce4/helpers/librewolf.desktop ]; then
rm /usr/share/xfce4/helpers/librewolf.desktop
fi
rm /etc/apt/sources.list.d/librewolf.list
rm /etc/apt/trusted.gpg.d/librewolf.gpg
apt-get update
</postuninstall>
</app>
