<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
US_English_Libreoffice
</name>

<description>
   <am>US English Help for LibreOffice</am>
   <ar>US English Help for LibreOffice</ar>
   <bg>US English Help for LibreOffice</bg>
   <bn>US English Help for LibreOffice</bn>
   <ca>Ajuda en anglès (USA) per LibreOffice</ca>
   <cs>US English Help for LibreOffice</cs>
   <da>Engelsk (USA) hjælp til LibreOffice</da>
   <de>Englische (US) Hilfe für LibreOffice</de>
   <el>Βοήθεια για το LibreOffice στα Αγγλικά (ΗΠΑ)</el>
   <en>US English Help for LibreOffice</en>
   <es>Ayuda Inglesa (EE.UU.) para LiberOffice</es>
   <et>US English Help for LibreOffice</et>
   <eu>US English Help for LibreOffice</eu>
   <fa>US English Help for LibreOffice</fa>
   <fil_PH>US English Help for LibreOffice</fil_PH>
   <fi>US English Help for LibreOffice</fi>
   <fr>Aide anglais_US pour LibreOffice</fr>
   <he_IL>US English Help for LibreOffice</he_IL>
   <hi>US English Help for LibreOffice</hi>
   <hr>US English Help for LibreOffice</hr>
   <hu>US English Help for LibreOffice</hu>
   <id>US English Help for LibreOffice</id>
   <is>US English Help for LibreOffice</is>
   <it>Guida in Inglese US per LibreOffice</it>
   <ja_JP>US English Help for LibreOffice</ja_JP>
   <ja>US English Help for LibreOffice</ja>
   <kk>US English Help for LibreOffice</kk>
   <ko>US English Help for LibreOffice</ko>
   <lt>US English Help for LibreOffice</lt>
   <mk>US English Help for LibreOffice</mk>
   <mr>US English Help for LibreOffice</mr>
   <nb>US English Help for LibreOffice</nb>
   <nl>US Engelse Hulp voor LibreOffice</nl>
   <pl>Angielski Amerykański pomoc dla LibreOffice</pl>
   <pt_BR>US Inglês Help para LibreOffice</pt_BR>
   <pt>Inglês US Help para LibreOffice</pt>
   <ro>US English Help for LibreOffice</ro>
   <ru>US English Help for LibreOffice</ru>
   <sk>US English Help for LibreOffice</sk>
   <sl>US English Help for LibreOffice</sl>
   <sq>US English Help for LibreOffice</sq>
   <sr>US English Help for LibreOffice</sr>
   <sv>US Engelsk Hjälp för LibreOffice</sv>
   <tr>LibreOffice için Amerikan İngilizcesi Yardımı</tr>
   <uk>US English довідка LibreOffice</uk>
   <vi>US English Help for LibreOffice</vi>
   <zh_CN>US English Help for LibreOffice</zh_CN>
   <zh_TW>US English Help for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-help-en-us
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-help-en-us
</uninstall_package_names>
</app>
