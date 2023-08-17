<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Polish_Libreoffice
</name>

<description>
   <am>Polish Language Meta-Package for LibreOffice</am>
   <ar>Polish Language Meta-Package for LibreOffice</ar>
   <bg>Polish Language Meta-Package for LibreOffice</bg>
   <bn>Polish Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet de llengua polonesa</ca>
   <cs>Polish Language Meta-Package for LibreOffice</cs>
   <da>Polsk sprog-metapakke til LibreOffice</da>
   <de>Polnisches Sprach-Meta-Paket</de>
   <el>Πολωνική γλώσσα</el>
   <en>Polish Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Polaco</es>
   <et>Polish Language Meta-Package for LibreOffice</et>
   <eu>Polish Language Meta-Package for LibreOffice</eu>
   <fa>Polish Language Meta-Package for LibreOffice</fa>
   <fil_PH>Polish Language Meta-Package for LibreOffice</fil_PH>
   <fi>Puolankielinen metatieto-paketti</fi>
   <fr>Méta-paquet pour langue polonaise</fr>
   <he_IL>Polish Language Meta-Package for LibreOffice</he_IL>
   <hi>पोलिश भाषा मेटा-पैकेज</hi>
   <hr>Polish Language Meta-Package for LibreOffice</hr>
   <hu>Polish Language Meta-Package for LibreOffice</hu>
   <id>Polish Language Meta-Package for LibreOffice</id>
   <is>Polish Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua polacca</it>
   <ja>ポーランド語メタパッケージ</ja>
   <kk>Polish Language Meta-Package for LibreOffice</kk>
   <ko>Polish Language Meta-Package for LibreOffice</ko>
   <lt>Polish Language Meta-Package for LibreOffice</lt>
   <mk>Polish Language Meta-Package for LibreOffice</mk>
   <mr>Polish Language Meta-Package for LibreOffice</mr>
   <nb>Polsk språkpakke</nb>
   <nl>Poolse Taal Meta-Pakket</nl>
   <pl>Polski metapakiet językowy</pl>
   <pt_BR>Polonês Meta-Pacote de Idioma</pt_BR>
   <pt>Polaco Meta-Pacote de Idioma</pt>
   <ro>Polish Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет польского языка</ru>
   <sk>Polish Language Meta-Package for LibreOffice</sk>
   <sl>Poljski jezikovni meta-paket</sl>
   <sq>Meta-Paketë gjuhësore në polonisht</sq>
   <sr>Polish Language Meta-Package for LibreOffice</sr>
   <sv>Polskt Språk-Meta-Paket</sv>
   <tr>Lehçe Dili Üst-Paketi</tr>
   <uk>Polish Language Meta-Package for LibreOffice</uk>
   <vi>Polish Language Meta-Package for LibreOffice</vi>
   <zh_CN>Polish Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Polish Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-pl
libreoffice-help-pl
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-pl
libreoffice-help-pl
</uninstall_package_names>
</app>
