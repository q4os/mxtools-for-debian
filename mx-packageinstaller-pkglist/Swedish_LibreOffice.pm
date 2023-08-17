<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Swedish_Libreoffice
</name>

<description>
   <am>Swedish Language Meta-Package for LibreOffice</am>
   <ar>Swedish Language Meta-Package for LibreOffice</ar>
   <bg>Swedish Language Meta-Package for LibreOffice</bg>
   <bn>Swedish Language Meta-Package for LibreOffice</bn>
   <ca>Meta-paquet de llengua Sueca</ca>
   <cs>Swedish Language Meta-Package for LibreOffice</cs>
   <da>Svensk sprog-metapakke til LibreOffice</da>
   <de>Schwedisches Sprach-Meta-Paket</de>
   <el>Σουηδική γλώσσα</el>
   <en>Swedish Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Sueco</es>
   <et>Swedish Language Meta-Package for LibreOffice</et>
   <eu>Swedish Language Meta-Package for LibreOffice</eu>
   <fa>Swedish Language Meta-Package for LibreOffice</fa>
   <fil_PH>Swedish Language Meta-Package for LibreOffice</fil_PH>
   <fi>Ruotsinkielinen metatieto-paketti</fi>
   <fr>Méta-paquet langue pour suédoise</fr>
   <he_IL>Swedish Language Meta-Package for LibreOffice</he_IL>
   <hi>स्वीडिश भाषा मेटा-पैकेज</hi>
   <hr>Swedish Language Meta-Package for LibreOffice</hr>
   <hu>Swedish Language Meta-Package for LibreOffice</hu>
   <id>Swedish Language Meta-Package for LibreOffice</id>
   <is>Swedish Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua svedese</it>
   <ja>スウェーデン語 メタパッケージ</ja>
   <kk>Swedish Language Meta-Package for LibreOffice</kk>
   <ko>Swedish Language Meta-Package for LibreOffice</ko>
   <lt>Swedish Language Meta-Package for LibreOffice</lt>
   <mk>Swedish Language Meta-Package for LibreOffice</mk>
   <mr>Swedish Language Meta-Package for LibreOffice</mr>
   <nb>Svensk språkpakke</nb>
   <nl>Zweedse Taal Meta-Pakket</nl>
   <pl>Szwedzki metapakiet językowy</pl>
   <pt_BR>Sueco Meta-Pacote de Idioma</pt_BR>
   <pt>Sueco Meta-Pacote de Idioma</pt>
   <ro>Swedish Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет шведского языка</ru>
   <sk>Swedish Language Meta-Package for LibreOffice</sk>
   <sl>Švedski jezikovni meta-paket</sl>
   <sq>Meta-Paketë gjuhësore në suedisht</sq>
   <sr>Swedish Language Meta-Package for LibreOffice</sr>
   <sv>Svenskt Språk-Meta-Paket</sv>
   <tr>İsveççe Dili Üst-Paketi</tr>
   <uk>Swedish Language Meta-Package for LibreOffice</uk>
   <vi>Swedish Language Meta-Package for LibreOffice</vi>
   <zh_CN>Swedish Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Swedish Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-sv
libreoffice-help-sv
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-sv
libreoffice-help-sv
</uninstall_package_names>
</app>
