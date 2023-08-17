<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Italian_Libreoffice
</name>

<description>
   <am>Italian LibreOffice Language Meta-Package</am>
   <ar>Italian LibreOffice Language Meta-Package</ar>
   <bg>Italian LibreOffice Language Meta-Package</bg>
   <bn>Italian LibreOffice Language Meta-Package</bn>
   <ca>Metapaquet de llenguatge italià per a LibreOffice</ca>
   <cs>Italian LibreOffice Language Meta-Package</cs>
   <da>Italiensk LibreOffice sprog-metapakke</da>
   <de>Italienisches LibreOffice Meta-Paket</de>
   <el>LibreOffice στα Ιταλικά</el>
   <en>Italian LibreOffice Language Meta-Package</en>
   <es>Meta-Paquete de Idioma Italiano LibreOffice</es>
   <et>Italian LibreOffice Language Meta-Package</et>
   <eu>Italian LibreOffice Language Meta-Package</eu>
   <fa>Italian LibreOffice Language Meta-Package</fa>
   <fil_PH>Italian LibreOffice Language Meta-Package</fil_PH>
   <fi>Italialainen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-paquet langue italienne pour LibreOffice</fr>
   <he_IL>Italian LibreOffice Language Meta-Package</he_IL>
   <hi>लिब्रे-ऑफिस हेतु इतालवी भाषा मेटा-पैकेज</hi>
   <hr>Italian LibreOffice Language Meta-Package</hr>
   <hu>Italian LibreOffice Language Meta-Package</hu>
   <id>Italian LibreOffice Language Meta-Package</id>
   <is>Italian LibreOffice Language Meta-Package</is>
   <it>Meta-pacchetto della lingua italiana per LibreOffice</it>
   <ja>イタリア語 LibreOffice 言語メタパッケージ</ja>
   <kk>Italian LibreOffice Language Meta-Package</kk>
   <ko>Italian LibreOffice Language Meta-Package</ko>
   <lt>Italian LibreOffice Language Meta-Package</lt>
   <mk>Italian LibreOffice Language Meta-Package</mk>
   <mr>Italian LibreOffice Language Meta-Package</mr>
   <nb>Italiensk språkpakke for LibreOffice</nb>
   <nl>Italiaanse LibreOffice Taal Meta-Pakket</nl>
   <pl>Włoski metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Italiano Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Italiano Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Italian LibreOffice Language Meta-Package</ro>
   <ru>Мета-пакет итальянского языка для LibreOffice</ru>
   <sk>Italian LibreOffice Language Meta-Package</sk>
   <sl>Itlaijanski jezikovni metapaket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në italisht për LibreOffice-in</sq>
   <sr>Italian LibreOffice Language Meta-Package</sr>
   <sv>Italienskt LibreOffice Språk-Meta-Paket</sv>
   <tr>LibreOffice için İtalyanca Dili Üst-Paketi</tr>
   <uk>Italian LibreOffice Language Meta-Package</uk>
   <vi>Italian LibreOffice Language Meta-Package</vi>
   <zh_CN>Italian LibreOffice Language Meta-Package</zh_CN>
   <zh_TW>Italian LibreOffice Language Meta-Package</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-it
libreoffice-help-it
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-it
libreoffice-help-it
</uninstall_package_names>
</app>
