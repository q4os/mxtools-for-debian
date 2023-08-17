<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
South African_Libreoffice
</name>

<description>
   <am>South African Language Meta-Package for LibreOffice</am>
   <ar>South African Language Meta-Package for LibreOffice</ar>
   <bg>South African Language Meta-Package for LibreOffice</bg>
   <bn>South African Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Sudafricà per LibreOffice</ca>
   <cs>South African Language Meta-Package for LibreOffice</cs>
   <da>South African Language Meta-Package for LibreOffice</da>
   <de>Südafrikanisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Νοτιοαφρικανικά</el>
   <en>South African Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma sudafricano para LibreOffice</es>
   <et>South African Language Meta-Package for LibreOffice</et>
   <eu>South African Language Meta-Package for LibreOffice</eu>
   <fa>South African Language Meta-Package for LibreOffice</fa>
   <fil_PH>South African Language Meta-Package for LibreOffice</fil_PH>
   <fi>Etelä-afrikkalainen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet en langue sud-africaine pour LibreOffice</fr>
   <he_IL>South African Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु दक्षिण अफ्रीकी भाषा मेटा-पैकेज</hi>
   <hr>South African Language Meta-Package for LibreOffice</hr>
   <hu>South African Language Meta-Package for LibreOffice</hu>
   <id>South African Language Meta-Package for LibreOffice</id>
   <is>South African Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua sud-africana per LibreOffice</it>
   <ja>LibreOffice 用の南アフリカ語メタパッケージ</ja>
   <kk>South African Language Meta-Package for LibreOffice</kk>
   <ko>South African Language Meta-Package for LibreOffice</ko>
   <lt>South African Language Meta-Package for LibreOffice</lt>
   <mk>South African Language Meta-Package for LibreOffice</mk>
   <mr>South African Language Meta-Package for LibreOffice</mr>
   <nb>Sørafrikansk språkpakke for LibreOffice</nb>
   <nl>Zuid-Afrikaanse Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Południowoafrykański meta-pakiet językowy dla LibreOffice</pl>
   <pt_BR>Sul-Africano Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Sul Africano Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>South African Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет южноафриканского языка для LibreOffice</ru>
   <sk>South African Language Meta-Package for LibreOffice</sk>
   <sl>Južnoafriški jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në jugafrikanase për LibreOffice-in</sq>
   <sr>South African Language Meta-Package for LibreOffice</sr>
   <sv>Sydafrikanskt Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Güney Afrika Dili Üst-Paketi</tr>
   <uk>South African Language Meta-Package for LibreOffice</uk>
   <vi>South African Language Meta-Package for LibreOffice</vi>
   <zh_CN>South African Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>South African Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-za
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-za
</uninstall_package_names>
</app>
