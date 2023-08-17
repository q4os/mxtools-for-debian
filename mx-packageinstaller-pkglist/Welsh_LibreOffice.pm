<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Welsh_Libreoffice
</name>

<description>
   <am>Welsh Language Meta-Package for LibreOffice</am>
   <ar>Welsh Language Meta-Package for LibreOffice</ar>
   <bg>Welsh Language Meta-Package for LibreOffice</bg>
   <bn>Welsh Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Gal·lès per LibreOffice</ca>
   <cs>Welsh Language Meta-Package for LibreOffice</cs>
   <da>Welsh Language Meta-Package for LibreOffice</da>
   <de>Walisisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Ουαλικά</el>
   <en>Welsh Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma galés para LibreOffice</es>
   <et>Welsh Language Meta-Package for LibreOffice</et>
   <eu>Welsh Language Meta-Package for LibreOffice</eu>
   <fa>Welsh Language Meta-Package for LibreOffice</fa>
   <fil_PH>Welsh Language Meta-Package for LibreOffice</fil_PH>
   <fi>Kymrinkielinen (Wales) kielipaketti LibreOffice:lle</fi>
   <fr>Méta-paquet langue galloise pour LibreOffice</fr>
   <he_IL>Welsh Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु वेल्श भाषा मेटा-पैकेज</hi>
   <hr>Welsh Language Meta-Package for LibreOffice</hr>
   <hu>Welsh Language Meta-Package for LibreOffice</hu>
   <id>Welsh Language Meta-Package for LibreOffice</id>
   <is>Welsh Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua gallese per LibreOffice</it>
   <ja>LibreOffice 用 ウェールズ語メタパッケージ</ja>
   <kk>Welsh Language Meta-Package for LibreOffice</kk>
   <ko>Welsh Language Meta-Package for LibreOffice</ko>
   <lt>Welsh Language Meta-Package for LibreOffice</lt>
   <mk>Welsh Language Meta-Package for LibreOffice</mk>
   <mr>Welsh Language Meta-Package for LibreOffice</mr>
   <nb>Walisisk språkpakke for LibreOffice</nb>
   <nl>Welsh Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Walijski meta-pakiet językowy dla LibreOffice</pl>
   <pt_BR>Galês Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Galês Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Welsh Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет валлийского языка для LibreOffice</ru>
   <sk>Welsh Language Meta-Package for LibreOffice</sk>
   <sl>Velški jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në uellsisht për LibreOffice-in</sq>
   <sr>Welsh Language Meta-Package for LibreOffice</sr>
   <sv>Walesiska Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Galce Dili Üst-Paketi</tr>
   <uk>Welsh Language Meta-Package for LibreOffice</uk>
   <vi>Welsh Language Meta-Package for LibreOffice</vi>
   <zh_CN>Welsh Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Welsh Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-cy
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-cy
</uninstall_package_names>
</app>
