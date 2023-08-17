<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Indic_Libreoffice
</name>

<description>
   <am>Indic Language Meta-Package for LibreOffice</am>
   <ar>Indic Language Meta-Package for LibreOffice</ar>
   <bg>Indic Language Meta-Package for LibreOffice</bg>
   <bn>Indic Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet per LibreOffice</ca>
   <cs>Indic Language Meta-Package for LibreOffice</cs>
   <da>Indic Language Meta-Package for LibreOffice</da>
   <de>Meta-Paket für LibreOffice</de>
   <el>Meta-Package για το LibreOffice</el>
   <en>Indic Language Meta-Package for LibreOffice</en>
   <es>Meta-Package para LibreOffice</es>
   <et>Indic Language Meta-Package for LibreOffice</et>
   <eu>Indic Language Meta-Package for LibreOffice</eu>
   <fa>Indic Language Meta-Package for LibreOffice</fa>
   <fil_PH>Indic Language Meta-Package for LibreOffice</fil_PH>
   <fi>Metatieto-paketti LibreOffice:lle</fi>
   <fr>Meta-Paquet pour LibreOffice</fr>
   <he_IL>Indic Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु मेटा-पैकेज</hi>
   <hr>Indic Language Meta-Package for LibreOffice</hr>
   <hu>Indic Language Meta-Package for LibreOffice</hu>
   <id>Indic Language Meta-Package for LibreOffice</id>
   <is>Indic Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto per LibreOffice</it>
   <ja>LibreOffice用メタパッケージ</ja>
   <kk>Indic Language Meta-Package for LibreOffice</kk>
   <ko>Indic Language Meta-Package for LibreOffice</ko>
   <lt>Indic Language Meta-Package for LibreOffice</lt>
   <mk>Indic Language Meta-Package for LibreOffice</mk>
   <mr>Indic Language Meta-Package for LibreOffice</mr>
   <nb>LibreOffice metapakke</nb>
   <nl>Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-Pakiet dla LibreOffice</pl>
   <pt_BR>Meta-Pacote para o LibreOffice</pt_BR>
   <pt>Meta-Pacote para LibreOffice</pt>
   <ro>Indic Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет для LibreOffice</ru>
   <sk>Indic Language Meta-Package for LibreOffice</sk>
   <sl>Meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë për LibreOffice-in</sq>
   <sr>Indic Language Meta-Package for LibreOffice</sr>
   <sv>Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Üst-Paket</tr>
   <uk>Indic Language Meta-Package for LibreOffice</uk>
   <vi>Indic Language Meta-Package for LibreOffice</vi>
   <zh_CN>Indic Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Indic Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-in
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-in
</uninstall_package_names>
</app>
