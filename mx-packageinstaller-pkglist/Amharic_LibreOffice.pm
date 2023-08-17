<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Amharic_Libreoffice
</name>

<description>
   <am>Amharic Language Meta-Package for LibreOffice</am>
   <ar>Amharic Language Meta-Package for LibreOffice</ar>
   <bg>Amharic Language Meta-Package for LibreOffice</bg>
   <bn>Amharic Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet per LibreOffice</ca>
   <cs>Amharic Language Meta-Package for LibreOffice</cs>
   <da>Amharic Language Meta-Package for LibreOffice</da>
   <de>Meta-Paket für LibreOffice</de>
   <el>Meta-Package για το LibreOffice</el>
   <en>Amharic Language Meta-Package for LibreOffice</en>
   <es>Meta-Package para LibreOffice</es>
   <et>Amharic Language Meta-Package for LibreOffice</et>
   <eu>Amharic Language Meta-Package for LibreOffice</eu>
   <fa>Amharic Language Meta-Package for LibreOffice</fa>
   <fil_PH>Amharic Language Meta-Package for LibreOffice</fil_PH>
   <fi>Metatieto-paketti LibreOffice:lle</fi>
   <fr>Meta-Paquet pour LibreOffice</fr>
   <he_IL>Amharic Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु मेटा-पैकेज</hi>
   <hr>Amharic Language Meta-Package for LibreOffice</hr>
   <hu>Amharic Language Meta-Package for LibreOffice</hu>
   <id>Amharic Language Meta-Package for LibreOffice</id>
   <is>Amharic Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto per LibreOffice</it>
   <ja>LibreOffice用メタパッケージ</ja>
   <kk>Amharic Language Meta-Package for LibreOffice</kk>
   <ko>Amharic Language Meta-Package for LibreOffice</ko>
   <lt>Amharic Language Meta-Package for LibreOffice</lt>
   <mk>Amharic Language Meta-Package for LibreOffice</mk>
   <mr>Amharic Language Meta-Package for LibreOffice</mr>
   <nb>LibreOffice metapakke</nb>
   <nl>Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-Pakiet dla LibreOffice</pl>
   <pt_BR>Meta-Pacote para o LibreOffice</pt_BR>
   <pt>Meta-Pacote para LibreOffice</pt>
   <ro>Amharic Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет для LibreOffice</ru>
   <sk>Amharic Language Meta-Package for LibreOffice</sk>
   <sl>Meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë për LibreOffice-in</sq>
   <sr>Amharic Language Meta-Package for LibreOffice</sr>
   <sv>Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Üst-Paket</tr>
   <uk>Amharic Language Meta-Package for LibreOffice</uk>
   <vi>Amharic Language Meta-Package for LibreOffice</vi>
   <zh_CN>Amharic Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Amharic Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-am
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-am
</uninstall_package_names>
</app>
