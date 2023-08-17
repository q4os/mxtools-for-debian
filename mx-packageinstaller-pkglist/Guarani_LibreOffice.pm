<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Guarani_Libreoffice
</name>

<description>
   <am>Guarani Language Meta-Package for LibreOffice</am>
   <ar>Guarani Language Meta-Package for LibreOffice</ar>
   <bg>Guarani Language Meta-Package for LibreOffice</bg>
   <bn>Guarani Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet per LibreOffice</ca>
   <cs>Guarani Language Meta-Package for LibreOffice</cs>
   <da>Guarani Language Meta-Package for LibreOffice</da>
   <de>Meta-Paket für LibreOffice</de>
   <el>Meta-Package για το LibreOffice</el>
   <en>Guarani Language Meta-Package for LibreOffice</en>
   <es>Meta-Package para LibreOffice</es>
   <et>Guarani Language Meta-Package for LibreOffice</et>
   <eu>Guarani Language Meta-Package for LibreOffice</eu>
   <fa>Guarani Language Meta-Package for LibreOffice</fa>
   <fil_PH>Guarani Language Meta-Package for LibreOffice</fil_PH>
   <fi>Metatieto-paketti LibreOffice:lle</fi>
   <fr>Meta-Paquet pour LibreOffice</fr>
   <he_IL>Guarani Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु मेटा-पैकेज</hi>
   <hr>Guarani Language Meta-Package for LibreOffice</hr>
   <hu>Guarani Language Meta-Package for LibreOffice</hu>
   <id>Guarani Language Meta-Package for LibreOffice</id>
   <is>Guarani Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto per LibreOffice</it>
   <ja>LibreOffice用メタパッケージ</ja>
   <kk>Guarani Language Meta-Package for LibreOffice</kk>
   <ko>Guarani Language Meta-Package for LibreOffice</ko>
   <lt>Guarani Language Meta-Package for LibreOffice</lt>
   <mk>Guarani Language Meta-Package for LibreOffice</mk>
   <mr>Guarani Language Meta-Package for LibreOffice</mr>
   <nb>LibreOffice metapakke</nb>
   <nl>Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-Pakiet dla LibreOffice</pl>
   <pt_BR>Meta-Pacote para o LibreOffice</pt_BR>
   <pt>Meta-Pacote para LibreOffice</pt>
   <ro>Guarani Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет для LibreOffice</ru>
   <sk>Guarani Language Meta-Package for LibreOffice</sk>
   <sl>Meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë për LibreOffice-in</sq>
   <sr>Guarani Language Meta-Package for LibreOffice</sr>
   <sv>Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Üst-Paket</tr>
   <uk>Guarani Language Meta-Package for LibreOffice</uk>
   <vi>Guarani Language Meta-Package for LibreOffice</vi>
   <zh_CN>Guarani Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Guarani Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-gug
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-gug
</uninstall_package_names>
</app>
