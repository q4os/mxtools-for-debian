<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
French_Libreoffice
</name>

<description>
   <am>French Language Meta-Package for LibreOffice</am>
   <ar>French Language Meta-Package for LibreOffice</ar>
   <bg>French Language Meta-Package for LibreOffice</bg>
   <bn>French Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet per LibreOffice</ca>
   <cs>French Language Meta-Package for LibreOffice</cs>
   <da>Fransk sprog-metapakke til LibreOffice</da>
   <de>Meta-Paket für LibreOffice</de>
   <el>Meta-Package για το LibreOffice</el>
   <en>French Language Meta-Package for LibreOffice</en>
   <es>Meta-Package para LibreOffice</es>
   <et>French Language Meta-Package for LibreOffice</et>
   <eu>French Language Meta-Package for LibreOffice</eu>
   <fa>French Language Meta-Package for LibreOffice</fa>
   <fil_PH>French Language Meta-Package for LibreOffice</fil_PH>
   <fi>Metatieto-paketti LibreOffice:lle</fi>
   <fr>Meta-Paquet pour LibreOffice</fr>
   <he_IL>French Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु मेटा-पैकेज</hi>
   <hr>French Language Meta-Package for LibreOffice</hr>
   <hu>French Language Meta-Package for LibreOffice</hu>
   <id>French Language Meta-Package for LibreOffice</id>
   <is>French Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto per LibreOffice</it>
   <ja>LibreOffice用メタパッケージ</ja>
   <kk>French Language Meta-Package for LibreOffice</kk>
   <ko>French Language Meta-Package for LibreOffice</ko>
   <lt>French Language Meta-Package for LibreOffice</lt>
   <mk>French Language Meta-Package for LibreOffice</mk>
   <mr>French Language Meta-Package for LibreOffice</mr>
   <nb>LibreOffice metapakke</nb>
   <nl>Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-Pakiet dla LibreOffice</pl>
   <pt_BR>Meta-Pacote para o LibreOffice</pt_BR>
   <pt>Meta-Pacote para LibreOffice</pt>
   <ro>French Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет для LibreOffice</ru>
   <sk>French Language Meta-Package for LibreOffice</sk>
   <sl>Meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë për LibreOffice-in</sq>
   <sr>French Language Meta-Package for LibreOffice</sr>
   <sv>Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Üst-Paket</tr>
   <uk>French Language Meta-Package for LibreOffice</uk>
   <vi>French Language Meta-Package for LibreOffice</vi>
   <zh_CN>French Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>French Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-fr
libreoffice-help-fr
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-fr
libreoffice-help-fr
</uninstall_package_names>
</app>
