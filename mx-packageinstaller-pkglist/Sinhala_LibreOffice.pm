<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Sinhala_Libreoffice
</name>

<description>
   <am>Sinhala Language Meta-Package for LibreOffice</am>
   <ar>Sinhala Language Meta-Package for LibreOffice</ar>
   <bg>Sinhala Language Meta-Package for LibreOffice</bg>
   <bn>Sinhala Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Sinhali per LibreOffice</ca>
   <cs>Sinhala Language Meta-Package for LibreOffice</cs>
   <da>Sinhala Language Meta-Package for LibreOffice</da>
   <de>Sinhala Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Sinhala</el>
   <en>Sinhala Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Lenguaje Cingalés para LibreOffice</es>
   <et>Sinhala Language Meta-Package for LibreOffice</et>
   <eu>Sinhala Language Meta-Package for LibreOffice</eu>
   <fa>Sinhala Language Meta-Package for LibreOffice</fa>
   <fil_PH>Sinhala Language Meta-Package for LibreOffice</fil_PH>
   <fi>Sinhalinkielinen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet en langue cingalaise pour LibreOffice</fr>
   <he_IL>Sinhala Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु सिंहली भाषा मेटा-पैकेज</hi>
   <hr>Sinhala Language Meta-Package for LibreOffice</hr>
   <hu>Sinhala Language Meta-Package for LibreOffice</hu>
   <id>Sinhala Language Meta-Package for LibreOffice</id>
   <is>Sinhala Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua singalese per LibreOffice</it>
   <ja>LibreOffice用のシンハラ語メタパッケージ</ja>
   <kk>Sinhala Language Meta-Package for LibreOffice</kk>
   <ko>Sinhala Language Meta-Package for LibreOffice</ko>
   <lt>Sinhala Language Meta-Package for LibreOffice</lt>
   <mk>Sinhala Language Meta-Package for LibreOffice</mk>
   <mr>Sinhala Language Meta-Package for LibreOffice</mr>
   <nb>Sinhala språkpakke for LibreOffice</nb>
   <nl>Sinhala Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-pakiet języka syngaleskiego dla LibreOffice</pl>
   <pt_BR>Cingalês Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Cingalês Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Sinhala Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет сингальского языка для LibreOffice</ru>
   <sk>Sinhala Language Meta-Package for LibreOffice</sk>
   <sl>Sinhalski jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në sinhaleze për LibreOffice-in</sq>
   <sr>Sinhala Language Meta-Package for LibreOffice</sr>
   <sv>Sinhala Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Sinhala Dili Üst-Paketi</tr>
   <uk>Sinhala Language Meta-Package for LibreOffice</uk>
   <vi>Sinhala Language Meta-Package for LibreOffice</vi>
   <zh_CN>Sinhala Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Sinhala Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-si
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-si
</uninstall_package_names>
</app>
