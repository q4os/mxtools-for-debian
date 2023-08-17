<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Galician_Libreoffice
</name>

<description>
   <am>Galician Language Meta-Package for LibreOffice</am>
   <ar>Galician Language Meta-Package for LibreOffice</ar>
   <bg>Galician Language Meta-Package for LibreOffice</bg>
   <bn>Galician Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet per LibreOffice</ca>
   <cs>Galician Language Meta-Package for LibreOffice</cs>
   <da>Galician Language Meta-Package for LibreOffice</da>
   <de>Meta-Paket für LibreOffice</de>
   <el>Meta-Package για το LibreOffice</el>
   <en>Galician Language Meta-Package for LibreOffice</en>
   <es>Meta-Package para LibreOffice</es>
   <et>Galician Language Meta-Package for LibreOffice</et>
   <eu>Galician Language Meta-Package for LibreOffice</eu>
   <fa>Galician Language Meta-Package for LibreOffice</fa>
   <fil_PH>Galician Language Meta-Package for LibreOffice</fil_PH>
   <fi>Metatieto-paketti LibreOffice:lle</fi>
   <fr>Meta-Paquet pour LibreOffice</fr>
   <he_IL>Galician Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु मेटा-पैकेज</hi>
   <hr>Galician Language Meta-Package for LibreOffice</hr>
   <hu>Galician Language Meta-Package for LibreOffice</hu>
   <id>Galician Language Meta-Package for LibreOffice</id>
   <is>Galician Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto per LibreOffice</it>
   <ja>LibreOffice用メタパッケージ</ja>
   <kk>Galician Language Meta-Package for LibreOffice</kk>
   <ko>Galician Language Meta-Package for LibreOffice</ko>
   <lt>Galician Language Meta-Package for LibreOffice</lt>
   <mk>Galician Language Meta-Package for LibreOffice</mk>
   <mr>Galician Language Meta-Package for LibreOffice</mr>
   <nb>LibreOffice metapakke</nb>
   <nl>Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-Pakiet dla LibreOffice</pl>
   <pt_BR>Meta-Pacote para o LibreOffice</pt_BR>
   <pt>Meta-Pacote para LibreOffice</pt>
   <ro>Galician Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет для LibreOffice</ru>
   <sk>Galician Language Meta-Package for LibreOffice</sk>
   <sl>Meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë për LibreOffice-in</sq>
   <sr>Galician Language Meta-Package for LibreOffice</sr>
   <sv>Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Üst-Paket</tr>
   <uk>Galician Language Meta-Package for LibreOffice</uk>
   <vi>Galician Language Meta-Package for LibreOffice</vi>
   <zh_CN>Galician Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Galician Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-gl
libreoffice-help-gl
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-gl
libreoffice-help-gl
</uninstall_package_names>
</app>
