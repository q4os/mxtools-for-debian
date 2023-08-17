<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Lithuanian_Libreoffice
</name>

<description>
   <am>Lithuanian Language Meta-Package for LibreOffice</am>
   <ar>Lithuanian Language Meta-Package for LibreOffice</ar>
   <bg>Lithuanian Language Meta-Package for LibreOffice</bg>
   <bn>Lithuanian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Lituà per LibreOffice</ca>
   <cs>Lithuanian Language Meta-Package for LibreOffice</cs>
   <da>Lithuanian Language Meta-Package for LibreOffice</da>
   <de>Litauisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Λιθουανικά</el>
   <en>Lithuanian Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma lituano para LibreOffice</es>
   <et>Lithuanian Language Meta-Package for LibreOffice</et>
   <eu>Lithuanian Language Meta-Package for LibreOffice</eu>
   <fa>Lithuanian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Lithuanian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Liettualainen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet en langue lituanienne pour LibreOffice</fr>
   <he_IL>Lithuanian Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु लिथुआनियाई भाषा मेटा-पैकेज</hi>
   <hr>Lithuanian Language Meta-Package for LibreOffice</hr>
   <hu>Lithuanian Language Meta-Package for LibreOffice</hu>
   <id>Lithuanian Language Meta-Package for LibreOffice</id>
   <is>Lithuanian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua lituana per LibreOffice</it>
   <ja>LibreOffice用のリトアニア語メタパッケージ</ja>
   <kk>Lithuanian Language Meta-Package for LibreOffice</kk>
   <ko>Lithuanian Language Meta-Package for LibreOffice</ko>
   <lt>Lithuanian Language Meta-Package for LibreOffice</lt>
   <mk>Lithuanian Language Meta-Package for LibreOffice</mk>
   <mr>Lithuanian Language Meta-Package for LibreOffice</mr>
   <nb>Litauisk språkpakke for LibreOffice</nb>
   <nl>Litouws Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Litewski meta-pakiet językowy dla LibreOffice</pl>
   <pt_BR>Lituano Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Lituano Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Lithuanian Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет литовского языка для LibreOffice</ru>
   <sk>Lithuanian Language Meta-Package for LibreOffice</sk>
   <sl>Litvanski jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në lituanisht për LibreOffice-in</sq>
   <sr>Lithuanian Language Meta-Package for LibreOffice</sr>
   <sv>Litauiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Litvanyaca Dili Üst-Paketi</tr>
   <uk>Lithuanian Language Meta-Package for LibreOffice</uk>
   <vi>Lithuanian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Lithuanian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Lithuanian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-lt
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-lt
</uninstall_package_names>
</app>
