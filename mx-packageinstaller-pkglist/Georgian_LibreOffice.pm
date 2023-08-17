<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Georgian_Libreoffice
</name>

<description>
   <am>Georgian Language Meta-Package for LibreOffice</am>
   <ar>Georgian Language Meta-Package for LibreOffice</ar>
   <bg>Georgian Language Meta-Package for LibreOffice</bg>
   <bn>Georgian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet per LibreOffice</ca>
   <cs>Georgian Language Meta-Package for LibreOffice</cs>
   <da>Georgian Language Meta-Package for LibreOffice</da>
   <de>Meta-Paket für LibreOffice</de>
   <el>Meta-Package για το LibreOffice</el>
   <en>Georgian Language Meta-Package for LibreOffice</en>
   <es>Meta-Package para LibreOffice</es>
   <et>Georgian Language Meta-Package for LibreOffice</et>
   <eu>Georgian Language Meta-Package for LibreOffice</eu>
   <fa>Georgian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Georgian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Metatieto-paketti LibreOffice:lle</fi>
   <fr>Meta-Paquet pour LibreOffice</fr>
   <he_IL>Georgian Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु मेटा-पैकेज</hi>
   <hr>Georgian Language Meta-Package for LibreOffice</hr>
   <hu>Georgian Language Meta-Package for LibreOffice</hu>
   <id>Georgian Language Meta-Package for LibreOffice</id>
   <is>Georgian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto per LibreOffice</it>
   <ja>LibreOffice用メタパッケージ</ja>
   <kk>Georgian Language Meta-Package for LibreOffice</kk>
   <ko>Georgian Language Meta-Package for LibreOffice</ko>
   <lt>Georgian Language Meta-Package for LibreOffice</lt>
   <mk>Georgian Language Meta-Package for LibreOffice</mk>
   <mr>Georgian Language Meta-Package for LibreOffice</mr>
   <nb>LibreOffice metapakke</nb>
   <nl>Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-Pakiet dla LibreOffice</pl>
   <pt_BR>Meta-Pacote para o LibreOffice</pt_BR>
   <pt>Meta-Pacote para LibreOffice</pt>
   <ro>Georgian Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет для LibreOffice</ru>
   <sk>Georgian Language Meta-Package for LibreOffice</sk>
   <sl>Meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë për LibreOffice-in</sq>
   <sr>Georgian Language Meta-Package for LibreOffice</sr>
   <sv>Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Üst-Paket</tr>
   <uk>Georgian Language Meta-Package for LibreOffice</uk>
   <vi>Georgian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Georgian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Georgian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ka
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ka
</uninstall_package_names>
</app>
