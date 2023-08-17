<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Occitan_Libreoffice
</name>

<description>
   <am>Occitan Language Meta-Package for LibreOffice</am>
   <ar>Occitan Language Meta-Package for LibreOffice</ar>
   <bg>Occitan Language Meta-Package for LibreOffice</bg>
   <bn>Occitan Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Occità per LibreOffice</ca>
   <cs>Occitan Language Meta-Package for LibreOffice</cs>
   <da>Occitan Language Meta-Package for LibreOffice</da>
   <de>Occitanisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Occitan</el>
   <en>Occitan Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma occitano para LibreOffice</es>
   <et>Occitan Language Meta-Package for LibreOffice</et>
   <eu>Occitan Language Meta-Package for LibreOffice</eu>
   <fa>Occitan Language Meta-Package for LibreOffice</fa>
   <fil_PH>Occitan Language Meta-Package for LibreOffice</fil_PH>
   <fi>Oksitaaninen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-paquet langue occitane pour LibreOffice</fr>
   <he_IL>Occitan Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु ओसीटान भाषा मेटा-पैकेज</hi>
   <hr>Occitan Language Meta-Package for LibreOffice</hr>
   <hu>Occitan Language Meta-Package for LibreOffice</hu>
   <id>Occitan Language Meta-Package for LibreOffice</id>
   <is>Occitan Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua occitana per LibreOffice</it>
   <ja>LibreOffice 用のオック語メタパッケージ</ja>
   <kk>Occitan Language Meta-Package for LibreOffice</kk>
   <ko>Occitan Language Meta-Package for LibreOffice</ko>
   <lt>Occitan Language Meta-Package for LibreOffice</lt>
   <mk>Occitan Language Meta-Package for LibreOffice</mk>
   <mr>Occitan Language Meta-Package for LibreOffice</mr>
   <nb>Oksitansk språkpakke for LibreOffice</nb>
   <nl>Occitan Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-pakiet języka Occitan dla LibreOffice</pl>
   <pt_BR>Occitana Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Occitano Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Occitan Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет окситанского языка для LibreOffice</ru>
   <sk>Occitan Language Meta-Package for LibreOffice</sk>
   <sl>Okitanski jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në oçitanisht për LibreOffice-in</sq>
   <sr>Occitan Language Meta-Package for LibreOffice</sr>
   <sv>Occitan Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Oksitanca Dili Üst-Paketi</tr>
   <uk>Occitan Language Meta-Package for LibreOffice</uk>
   <vi>Occitan Language Meta-Package for LibreOffice</vi>
   <zh_CN>Occitan Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Occitan Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-oc
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-oc
</uninstall_package_names>
</app>
