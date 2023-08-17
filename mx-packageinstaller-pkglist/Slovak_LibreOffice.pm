<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Slovak_Libreoffice
</name>

<description>
   <am>Slovak Language Meta-Package for LibreOffice</am>
   <ar>Slovak Language Meta-Package for LibreOffice</ar>
   <bg>Slovak Language Meta-Package for LibreOffice</bg>
   <bn>Slovak Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Eslovac per LibreOffice</ca>
   <cs>Slovak Language Meta-Package for LibreOffice</cs>
   <da>Slovak Language Meta-Package for LibreOffice</da>
   <de>Slowakisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Σλοβακικά</el>
   <en>Slovak Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma eslovaco para LibreOffice</es>
   <et>Slovak Language Meta-Package for LibreOffice</et>
   <eu>Slovak Language Meta-Package for LibreOffice</eu>
   <fa>Slovak Language Meta-Package for LibreOffice</fa>
   <fil_PH>Slovak Language Meta-Package for LibreOffice</fil_PH>
   <fi>Slovakialainen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-paquet langue slovaque pour LibreOffice</fr>
   <he_IL>Slovak Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु स्लोवाकियाई भाषा मेटा-पैकेज</hi>
   <hr>Slovak Language Meta-Package for LibreOffice</hr>
   <hu>Slovak Language Meta-Package for LibreOffice</hu>
   <id>Slovak Language Meta-Package for LibreOffice</id>
   <is>Slovak Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua slovacca per LibreOffice</it>
   <ja>LibreOffice 用のスロバキア語メタパッケージ</ja>
   <kk>Slovak Language Meta-Package for LibreOffice</kk>
   <ko>Slovak Language Meta-Package for LibreOffice</ko>
   <lt>Slovak Language Meta-Package for LibreOffice</lt>
   <mk>Slovak Language Meta-Package for LibreOffice</mk>
   <mr>Slovak Language Meta-Package for LibreOffice</mr>
   <nb>Slovakisk språkpakke for LibreOffice</nb>
   <nl>Slovaaks Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Słowacki meta-pakiet językowy dla LibreOffice</pl>
   <pt_BR>Eslovaco Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Eslovaco Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Slovak Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет словацкого языка для LibreOffice</ru>
   <sk>Slovak Language Meta-Package for LibreOffice</sk>
   <sl>Slovaški jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në sllovakisht për LibreOffice-in</sq>
   <sr>Slovak Language Meta-Package for LibreOffice</sr>
   <sv>Slovakiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Slovakça Dili Üst-Paketi</tr>
   <uk>Slovak Language Meta-Package for LibreOffice</uk>
   <vi>Slovak Language Meta-Package for LibreOffice</vi>
   <zh_CN>Slovak Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Slovak Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-sk
libreoffice-help-sk
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-sk
libreoffice-help-sk
</uninstall_package_names>
</app>
