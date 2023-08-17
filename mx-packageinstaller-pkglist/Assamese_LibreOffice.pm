<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Assamese_Libreoffice
</name>

<description>
   <am>Assamese Language Meta-Package for LibreOffice</am>
   <ar>Assamese Language Meta-Package for LibreOffice</ar>
   <bg>Assamese Language Meta-Package for LibreOffice</bg>
   <bn>Assamese Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet per LibreOffice</ca>
   <cs>Assamese Language Meta-Package for LibreOffice</cs>
   <da>Assamese Language Meta-Package for LibreOffice</da>
   <de>Meta-Paket für LibreOffice</de>
   <el>Meta-Package για το LibreOffice</el>
   <en>Assamese Language Meta-Package for LibreOffice</en>
   <es>Meta-Package para LibreOffice</es>
   <et>Assamese Language Meta-Package for LibreOffice</et>
   <eu>Assamese Language Meta-Package for LibreOffice</eu>
   <fa>Assamese Language Meta-Package for LibreOffice</fa>
   <fil_PH>Assamese Language Meta-Package for LibreOffice</fil_PH>
   <fi>Metatieto-paketti LibreOffice:lle</fi>
   <fr>Meta-Paquet pour LibreOffice</fr>
   <he_IL>Assamese Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु मेटा-पैकेज</hi>
   <hr>Assamese Language Meta-Package for LibreOffice</hr>
   <hu>Assamese Language Meta-Package for LibreOffice</hu>
   <id>Assamese Language Meta-Package for LibreOffice</id>
   <is>Assamese Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto per LibreOffice</it>
   <ja>LibreOffice用メタパッケージ</ja>
   <kk>Assamese Language Meta-Package for LibreOffice</kk>
   <ko>Assamese Language Meta-Package for LibreOffice</ko>
   <lt>Assamese Language Meta-Package for LibreOffice</lt>
   <mk>Assamese Language Meta-Package for LibreOffice</mk>
   <mr>Assamese Language Meta-Package for LibreOffice</mr>
   <nb>LibreOffice metapakke</nb>
   <nl>Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-Pakiet dla LibreOffice</pl>
   <pt_BR>Meta-Pacote para o LibreOffice</pt_BR>
   <pt>Meta-Pacote para LibreOffice</pt>
   <ro>Assamese Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет для LibreOffice</ru>
   <sk>Assamese Language Meta-Package for LibreOffice</sk>
   <sl>Meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë për LibreOffice-in</sq>
   <sr>Assamese Language Meta-Package for LibreOffice</sr>
   <sv>Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Üst-Paket</tr>
   <uk>Assamese Language Meta-Package for LibreOffice</uk>
   <vi>Assamese Language Meta-Package for LibreOffice</vi>
   <zh_CN>Assamese Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Assamese Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-as
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-as
</uninstall_package_names>
</app>
