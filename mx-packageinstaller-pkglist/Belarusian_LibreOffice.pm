<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Belarusian_Libreoffice
</name>

<description>
   <am>Belarusian Language Meta-Package for LibreOffice</am>
   <ar>Belarusian Language Meta-Package for LibreOffice</ar>
   <bg>Belarusian Language Meta-Package for LibreOffice</bg>
   <bn>Belarusian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet per LibreOffice</ca>
   <cs>Belarusian Language Meta-Package for LibreOffice</cs>
   <da>Belarusian Language Meta-Package for LibreOffice</da>
   <de>Meta-Paket für LibreOffice</de>
   <el>Meta-Package για το LibreOffice</el>
   <en>Belarusian Language Meta-Package for LibreOffice</en>
   <es>Meta-Package para LibreOffice</es>
   <et>Belarusian Language Meta-Package for LibreOffice</et>
   <eu>Belarusian Language Meta-Package for LibreOffice</eu>
   <fa>Belarusian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Belarusian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Metatieto-paketti LibreOffice:lle</fi>
   <fr>Meta-Paquet pour LibreOffice</fr>
   <he_IL>Belarusian Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु मेटा-पैकेज</hi>
   <hr>Belarusian Language Meta-Package for LibreOffice</hr>
   <hu>Belarusian Language Meta-Package for LibreOffice</hu>
   <id>Belarusian Language Meta-Package for LibreOffice</id>
   <is>Belarusian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto per LibreOffice</it>
   <ja>LibreOffice用メタパッケージ</ja>
   <kk>Belarusian Language Meta-Package for LibreOffice</kk>
   <ko>Belarusian Language Meta-Package for LibreOffice</ko>
   <lt>Belarusian Language Meta-Package for LibreOffice</lt>
   <mk>Belarusian Language Meta-Package for LibreOffice</mk>
   <mr>Belarusian Language Meta-Package for LibreOffice</mr>
   <nb>LibreOffice metapakke</nb>
   <nl>Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-Pakiet dla LibreOffice</pl>
   <pt_BR>Meta-Pacote para o LibreOffice</pt_BR>
   <pt>Meta-Pacote para LibreOffice</pt>
   <ro>Belarusian Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет для LibreOffice</ru>
   <sk>Belarusian Language Meta-Package for LibreOffice</sk>
   <sl>Meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë për LibreOffice-in</sq>
   <sr>Belarusian Language Meta-Package for LibreOffice</sr>
   <sv>Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Üst-Paket</tr>
   <uk>Belarusian Language Meta-Package for LibreOffice</uk>
   <vi>Belarusian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Belarusian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Belarusian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-be
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-be
</uninstall_package_names>
</app>
