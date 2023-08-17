<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Malayalam_Libreoffice
</name>

<description>
   <am>Malayalam Language Meta-Package for LibreOffice</am>
   <ar>Malayalam Language Meta-Package for LibreOffice</ar>
   <bg>Malayalam Language Meta-Package for LibreOffice</bg>
   <bn>Malayalam Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Malaià per LibreOffice</ca>
   <cs>Malayalam Language Meta-Package for LibreOffice</cs>
   <da>Malayalam Language Meta-Package for LibreOffice</da>
   <de>Malayalam Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Malayalam</el>
   <en>Malayalam Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma malayo para LibreOffice</es>
   <et>Malayalam Language Meta-Package for LibreOffice</et>
   <eu>Malayalam Language Meta-Package for LibreOffice</eu>
   <fa>Malayalam Language Meta-Package for LibreOffice</fa>
   <fil_PH>Malayalam Language Meta-Package for LibreOffice</fil_PH>
   <fi> Malajalamkielinen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet en langue malayalam pour LibreOffice</fr>
   <he_IL>Malayalam Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु मलयालम भाषा मेटा-पैकेज</hi>
   <hr>Malayalam Language Meta-Package for LibreOffice</hr>
   <hu>Malayalam Language Meta-Package for LibreOffice</hu>
   <id>Malayalam Language Meta-Package for LibreOffice</id>
   <is>Malayalam Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua malayalam per LibreOffice</it>
   <ja>LibreOffice 用のマレーシア語メタパッケージ</ja>
   <kk>Malayalam Language Meta-Package for LibreOffice</kk>
   <ko>Malayalam Language Meta-Package for LibreOffice</ko>
   <lt>Malayalam Language Meta-Package for LibreOffice</lt>
   <mk>Malayalam Language Meta-Package for LibreOffice</mk>
   <mr>Malayalam Language Meta-Package for LibreOffice</mr>
   <nb>Malayalam språkpakke for LibreOffice</nb>
   <nl>Malayalam Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-pakiet języka malajalam dla LibreOffice</pl>
   <pt_BR>Malaiala Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Malaio Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Malayalam Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет малаяльского языка для LibreOffice</ru>
   <sk>Malayalam Language Meta-Package for LibreOffice</sk>
   <sl>Malajlamski jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në malajalamase për LibreOffice-in</sq>
   <sr>Malayalam Language Meta-Package for LibreOffice</sr>
   <sv>Malayalam Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Malayalam Dili Üst-Paketi</tr>
   <uk>Malayalam Language Meta-Package for LibreOffice</uk>
   <vi>Malayalam Language Meta-Package for LibreOffice</vi>
   <zh_CN>Malayalam Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Malayalam Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ml
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ml
</uninstall_package_names>
</app>
