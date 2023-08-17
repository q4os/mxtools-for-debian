<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Serbian_Libreoffice
</name>

<description>
   <am>Serbian Language Meta-Package for LibreOffice</am>
   <ar>Serbian Language Meta-Package for LibreOffice</ar>
   <bg>Serbian Language Meta-Package for LibreOffice</bg>
   <bn>Serbian Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Serbi per LibreOffice</ca>
   <cs>Serbian Language Meta-Package for LibreOffice</cs>
   <da>Serbian Language Meta-Package for LibreOffice</da>
   <de>Serbisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Σερβικά</el>
   <en>Serbian Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma serbio para LibreOffice</es>
   <et>Serbian Language Meta-Package for LibreOffice</et>
   <eu>Serbian Language Meta-Package for LibreOffice</eu>
   <fa>Serbian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Serbian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Serbialainen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-paquet langue serbe pour LibreOffice</fr>
   <he_IL>Serbian Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु सर्बियाई भाषा मेटा-पैकेज</hi>
   <hr>Serbian Language Meta-Package for LibreOffice</hr>
   <hu>Serbian Language Meta-Package for LibreOffice</hu>
   <id>Serbian Language Meta-Package for LibreOffice</id>
   <is>Serbian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua serba per LibreOffice</it>
   <ja>LibreOffice用のセルビア語メタパッケージ</ja>
   <kk>Serbian Language Meta-Package for LibreOffice</kk>
   <ko>Serbian Language Meta-Package for LibreOffice</ko>
   <lt>Serbian Language Meta-Package for LibreOffice</lt>
   <mk>Serbian Language Meta-Package for LibreOffice</mk>
   <mr>Serbian Language Meta-Package for LibreOffice</mr>
   <nb>Serbisk språkpakke for LibreOffice</nb>
   <nl>Servisch Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-pakiet języka serbskiego dla LibreOffice</pl>
   <pt_BR>Sérvio Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Sérvio Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Serbian Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет сербского языка для LibreOffice</ru>
   <sk>Serbian Language Meta-Package for LibreOffice</sk>
   <sl>Srbski jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në serbisht për LibreOffice-in</sq>
   <sr>Serbian Language Meta-Package for LibreOffice</sr>
   <sv>Serbiskt Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Sırpça Dili Üst-Paketi</tr>
   <uk>Serbian Language Meta-Package for LibreOffice</uk>
   <vi>Serbian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Serbian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Serbian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-sr
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-sr
</uninstall_package_names>
</app>
