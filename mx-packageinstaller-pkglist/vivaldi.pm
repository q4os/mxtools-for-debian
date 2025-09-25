<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Browser
</category>

<name>
Vivaldi
</name>

<description>
   <am>Vivaldi web browser</am>
   <ar>Vivaldi web browser</ar>
   <bg>Vivaldi web browser</bg>
   <bn>Vivaldi web browser</bn>
   <ca>Navegador web Vivaldi</ca>
   <cs>Vivaldi web browser</cs>
   <da>Vivaldi-webbrowser</da>
   <de>Der Vivaldi Web-Browser</de>
   <el>Vivaldi web browser</el>
   <en>Vivaldi web browser</en>
   <es>Navegador de internet Vivaldi</es>
   <et>Vivaldi web browser</et>
   <eu>Vivaldi web browser</eu>
   <fa>Vivaldi web browser</fa>
   <fil_PH>Vivaldi web browser</fil_PH>
   <fi>Vivaldi-verkkoselain</fi>
   <fr>Navigateur Web Vivaldi</fr>
   <he_IL>Vivaldi web browser</he_IL>
   <hi>विवाल्डी वेब ब्राउज़र</hi>
   <hr>Vivaldi web browser</hr>
   <hu>Vivaldi web browser</hu>
   <id>Vivaldi web browser</id>
   <is>Vivaldi web browser</is>
   <it>Vivaldi web browser</it>
   <ja>Vivaldi ウェブブラウザ</ja>
   <kk>Vivaldi web browser</kk>
   <ko>Vivaldi web browser</ko>
   <lt>Vivaldi saityno naršyklė</lt>
   <mk>Vivaldi web browser</mk>
   <mr>Vivaldi web browser</mr>
   <nb>Nettleseren Vivaldi</nb>
   <nl>Vivaldi web browser</nl>
   <pl>przeglądarka Vivaldi</pl>
   <pt_BR>Vivaldi Navegador de Internet</pt_BR>
   <pt>Navegador web Vivaldi</pt>
   <ro>Vivaldi web browser</ro>
   <ru>Браузер Vivaldi</ru>
   <sk>Vivaldi web browser</sk>
   <sl>Vivaldi spletni brskalnik</sl>
   <sq>Shfletuesi Vivaldi</sq>
   <sr>Vivaldi web browser</sr>
   <sv>Vivaldi webbläsare</sv>
   <tr>Vivaldi internet tarayıcı</tr>
   <uk>Vivaldi web browser</uk>
   <vi>Vivaldi web browser</vi>
   <zh_CN>Vivaldi web browser</zh_CN>
   <zh_TW>Vivaldi web browser</zh_TW>
</description>

<installable>
64
</installable>

<screenshot></screenshot>

<preinstall>
# vivaldi apt-source enabled
REXP="^[[:space:]]*deb[[:space:]]+https?://repo.vivaldi.com/stable/deb/?[[:space:]]+stable[[:space:]]+main"
if ! grep -sq -E "$REXP" /etc/apt/sources.list{,.d/*.list}; then
   echo "Add Vivaldi apt-source-list..."
   echo "deb [arch=amd64]  http://repo.vivaldi.com/stable/deb/ stable main" > /etc/apt/sources.list.d/vivaldi.list  
   echo /etc/apt/sources.list.d/vivaldi.list :
   cat  /etc/apt/sources.list.d/vivaldi.list
   echo " "
fi 

printf "Add Vivaldi archive keyring ... "
wget -q -O - https://repo.vivaldi.com/stable/linux_signing_key.pub | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/vivaldi-archive-keyring.gpg >/dev/null
apt-get update

</preinstall>

<install_package_names>
vivaldi-stable
</install_package_names>

<postinstall>

echo ... $(gettext -d apt -s ' Done')'!'

</postinstall>


<uninstall_package_names>
vivaldi-stable
</uninstall_package_names>

<postuninstall>
echo " "
echo "Purge vivaldi-stable ..."
apt-get  -o=Dpkg::Use-Pty=0  --yes remove --purge  vivaldi-stable
echo "Remove Vivaldi source.list ... "
rm /etc/apt/sources.list.d/vivaldi.list 2>/dev/null
echo "Remove Vivaldi archive keyring ..."
rm /etc/apt/trusted.gpg.d/vivladi-archive-keyring.gpg 2>/dev/null
rm /etc/apt/trusted.gpg.d/vivaldi-archive-keyring.gpg 2>/dev/null
echo "Done!"

</postuninstall>

</app>
