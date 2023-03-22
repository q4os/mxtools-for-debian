<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
GB_English_Libreoffice
</name>

<description>
   <am>GB English Help for LibreOffice</am>
   <ar>GB English Help for LibreOffice</ar>
   <bg>GB English Help for LibreOffice</bg>
   <bn>GB English Help for LibreOffice</bn>
   <ca>Ajuda en anglès (UK) per LibreOffice</ca>
   <cs>GB English Help for LibreOffice</cs>
   <da>Engelsk (storbritannien) hjælp til LibreOffice</da>
   <de>English (GB) Help for LibreOffice</de>
   <el>Βοήθεια για το LibreOffice στα Αγγλικά (ΜΒ)</el>
   <en>GB English Help for LibreOffice</en>
   <es>Ayuda Inglesa (GB) para LibreOffice</es>
   <et>GB English Help for LibreOffice</et>
   <eu>GB English Help for LibreOffice</eu>
   <fa>GB English Help for LibreOffice</fa>
   <fil_PH>GB English Help for LibreOffice</fil_PH>
   <fi>GB English Help for LibreOffice</fi>
   <fr>Aide anglais_GB pour LibreOffice</fr>
   <he_IL>GB English Help for LibreOffice</he_IL>
   <hi>GB English Help for LibreOffice</hi>
   <hr>GB English Help for LibreOffice</hr>
   <hu>GB English Help for LibreOffice</hu>
   <id>GB English Help for LibreOffice</id>
   <is>GB English Help for LibreOffice</is>
   <it>Guida in Inglese GB per LibreOffice</it>
   <ja_JP>GB English Help for LibreOffice</ja_JP>
   <ja>GB English Help for LibreOffice</ja>
   <kk>GB English Help for LibreOffice</kk>
   <ko>GB English Help for LibreOffice</ko>
   <lt>GB English Help for LibreOffice</lt>
   <mk>GB English Help for LibreOffice</mk>
   <mr>GB English Help for LibreOffice</mr>
   <nb>GB English Help for LibreOffice</nb>
   <nl>GB Engelse Hulp voor LibreOffice</nl>
   <pl>Angielski Brytyjski pomoc dla LibreOffice</pl>
   <pt_BR>GB Inglês Help para LibreOffice</pt_BR>
   <pt>Inglês GB Help para LibreOffice</pt>
   <ro>GB English Help for LibreOffice</ro>
   <ru>GB English Help for LibreOffice</ru>
   <sk>GB English Help for LibreOffice</sk>
   <sl>GB English Help for LibreOffice</sl>
   <sq>GB English Help for LibreOffice</sq>
   <sr>GB English Help for LibreOffice</sr>
   <sv>GB Engelsk Hjälp för LibreOffice</sv>
   <tr>LibreOffice için Britanya ingilizcesi Yardımı</tr>
   <uk>GB English довідка LibreOffice</uk>
   <vi>GB English Help for LibreOffice</vi>
   <zh_CN>GB English Help for LibreOffice</zh_CN>
   <zh_TW>GB English Help for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-help-en-gb
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-help-en-gb
</uninstall_package_names>
</app>
