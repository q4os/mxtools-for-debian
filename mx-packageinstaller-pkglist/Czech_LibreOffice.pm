<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Czech_Libreoffice
</name>

<description>
   <am>Czech LibreOffice Language Meta-Package</am>
   <ar>Czech LibreOffice Language Meta-Package</ar>
   <bg>Czech LibreOffice Language Meta-Package</bg>
   <bn>Czech LibreOffice Language Meta-Package</bn>
   <ca>Localització de LibreOffice en Txec</ca>
   <cs>Czech LibreOffice Language Meta-Package</cs>
   <da>Tjekkisk LibreOffice sprog-metapakke</da>
   <de>Tschechisches LibreOffice Meta-Paket</de>
   <el>Libreoffice στα Τσέχικα</el>
   <en>Czech LibreOffice Language Meta-Package</en>
   <es>Meta-Paquete de Idioma Checo LibreOffice</es>
   <et>Czech LibreOffice Language Meta-Package</et>
   <eu>Czech LibreOffice Language Meta-Package</eu>
   <fa>Czech LibreOffice Language Meta-Package</fa>
   <fil_PH>Czech LibreOffice Language Meta-Package</fil_PH>
   <fi>Tsekkiläinen metatieto-paketti LibreOffice:lle</fi>
   <fr>Méta-Paquet langue tchèque pour LibreOffice</fr>
   <he_IL>Czech LibreOffice Language Meta-Package</he_IL>
   <hi>लिब्रे-ऑफिस हेतु चेक भाषा मेटा-पैकेज</hi>
   <hr>Czech LibreOffice Language Meta-Package</hr>
   <hu>Czech LibreOffice Language Meta-Package</hu>
   <id>Czech LibreOffice Language Meta-Package</id>
   <is>Czech LibreOffice Language Meta-Package</is>
   <it>Meta-pacchetto della lingua ceca per LibreOffice</it>
   <ja>LibreOffice のチェコ語用の言語メタパッケージ</ja>
   <kk>Czech LibreOffice Language Meta-Package</kk>
   <ko>Czech LibreOffice Language Meta-Package</ko>
   <lt>Czech LibreOffice Language Meta-Package</lt>
   <mk>Czech LibreOffice Language Meta-Package</mk>
   <mr>Czech LibreOffice Language Meta-Package</mr>
   <nb>Tsjekkisk språkpakke for LibreOffice</nb>
   <nl>Tjechische LibreOffice Taal Meta-Pakket</nl>
   <pl>Czeski metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Checo Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Checo Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Czech LibreOffice Language Meta-Package</ro>
   <ru>Мета-пакет чешского языка для LibreOffice</ru>
   <sk>Czech LibreOffice Language Meta-Package</sk>
   <sl>Češki jezikovni metapaket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në çekisht për LibreOffice-in</sq>
   <sr>Czech LibreOffice Language Meta-Package</sr>
   <sv>Tjeckisk LibreOffice Språk Meta-Paket</sv>
   <tr>LibreOffice Çekçe Dili Üst-Paketi</tr>
   <uk>Czech LibreOffice Language Meta-Package</uk>
   <vi>Czech LibreOffice Language Meta-Package</vi>
   <zh_CN>Czech LibreOffice Language Meta-Package</zh_CN>
   <zh_TW>Czech LibreOffice Language Meta-Package</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-cs
libreoffice-help-cs
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-cs
libreoffice-help-cs
</uninstall_package_names>
</app>
