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
   <hi>Vivaldi web browser</hi>
   <hr>Vivaldi web browser</hr>
   <hu>Vivaldi web browser</hu>
   <id>Vivaldi web browser</id>
   <is>Vivaldi web browser</is>
   <it>Vivaldi web browser</it>
   <ja_JP>Vivaldi web browser</ja_JP>
   <ja>Vivaldi web browser</ja>
   <kk>Vivaldi web browser</kk>
   <ko>Vivaldi web browser</ko>
   <lt>Vivaldi saityno naršyklė</lt>
   <mk>Vivaldi web browser</mk>
   <mr>Vivaldi web browser</mr>
   <nb>Vivaldi web browser</nb>
   <nl>Vivaldi web browser</nl>
   <pl>przeglądarka Vivaldi</pl>
   <pt_BR>Navegador web Vivaldi</pt_BR>
   <pt>Navegador web Vivaldi</pt>
   <ro>Vivaldi web browser</ro>
   <ru>Браузер Vivaldi</ru>
   <sk>Vivaldi web browser</sk>
   <sl>Vivaldi spletni brskalnik</sl>
   <sq>Vivaldi web browser</sq>
   <sr>Vivaldi web browser</sr>
   <sv>Vivaldi webbläsare</sv>
   <tr>Vivaldi internet tarayıcı</tr>
   <uk>Vivaldi web browser</uk>
   <vi>Vivaldi web browser</vi>
   <zh_CN>Vivaldi web browser</zh_CN>
   <zh_TW>Vivaldi web browser</zh_TW>
</description>

<installable>
64,armhf
</installable>

<screenshot></screenshot>

<preinstall>
# vivaldi apt-source enabled
REXP="^[[:space:]]*deb[[:space:]]+https?://repo.vivaldi.com/stable/deb/?[[:space:]]+stable[[:space:]]+main"
if ! grep -sq -E "$REXP" /etc/apt/sources.list{,.d/*.list}; then
   echo "Add Vivaldi apt-source-list..."
   echo "deb http://repo.vivaldi.com/stable/deb/ stable main" > /etc/apt/sources.list.d/vivaldi.list  
   echo /etc/apt/sources.list.d/vivaldi.list :
   cat  /etc/apt/sources.list.d/vivaldi.list
   echo " "
fi 

printf "Remove expired Vivaldi Package Composer KEY04 packager@vivaldi.com ... "
#pub   rsa4096/0x8D04CE49EFB20B23 2018-01-05 [SC] [expired: 2020-01-25]
#      68AEAE71F9FA158703C1CBBC8D04CE49EFB20B23
#uid   [ expired] Vivaldi Package Composer KEY04 packager@vivaldi.com
( apt-key del 0x8D04CE49EFB20B23 ) 2>/dev/null

printf "Remove expired Vivaldi Package Composer KEY05 packager@vivaldi.com ... "
#pub   rsa4096/0x9658E8044A3AA3D6 2018-12-04 [SC] [expires: 2021-01-22]
#      B44B85E3E1A6386BFC79D4119658E8044A3AA3D6
#uid    unknown] Vivaldi Package Composer KEY05 packager@vivaldi.com
#sub   rsa4096/0x21974F15BECAE0A9 2018-12-04 [E] [expires: 2021-01-22]
( apt-key del 0x9658E8044A3AA3D6 ) 2>/dev/null

rm /tmp/vivaldi.key 2>/dev/null
wget https://repo.vivaldi.com/stable/linux_signing_key.pub -O /tmp/vivaldi.key
printf "Add Vivaldi archive keyring ... "
( apt-key --keyring /etc/apt/trusted.gpg.d/vivaldi-archive-keyring.gpg add /tmp/vivaldi.key ) 2>/dev/null
rm /tmp/vivaldi.key 2>/dev/null

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
#pub   rsa4096/0x8D04CE49EFB20B23 2018-01-05 [SC] [expired: 2020-01-25]
#      68AEAE71F9FA158703C1CBBC8D04CE49EFB20B23
#uid   [ expired] Vivaldi Package Composer KEY04 packager@vivaldi.com
( apt-key del 0x8D04CE49EFB20B23 ) 2>/dev/null 1>/dev/null
#pub   rsa4096/0x9658E8044A3AA3D6 2018-12-04 [SC] [expires: 2021-01-22]
#      B44B85E3E1A6386BFC79D4119658E8044A3AA3D6
#uid    unknown] Vivaldi Package Composer KEY05 packager@vivaldi.com
#sub   rsa4096/0x21974F15BECAE0A9 2018-12-04 [E] [expires: 2021-01-22]
( apt-key del 0x9658E8044A3AA3D6 ) 2>/dev/null 1>/dev/null
echo "Done!"

</postuninstall>

</app>
