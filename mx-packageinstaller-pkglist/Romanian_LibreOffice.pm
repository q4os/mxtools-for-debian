<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Romanian_Libreoffice
</name>

<description>
   <am>Romanian Language Meta-Package for LibreOffice</am>
   <ar>Romanian Language Meta-Package for LibreOffice</ar>
   <bg>Romanian Language Meta-Package for LibreOffice</bg>
   <bn>Romanian Language Meta-Package for LibreOffice</bn>
   <ca>Meta-paquet de llengua en Romanès</ca>
   <cs>Romanian Language Meta-Package for LibreOffice</cs>
   <da>Rumænsk sprog-metapakke til LibreOffice</da>
   <de>Rumänisches Sprach-Meta-Paket</de>
   <el>Ρουμανική γλώσσα</el>
   <en>Romanian Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Rumano</es>
   <et>Romanian Language Meta-Package for LibreOffice</et>
   <eu>Romanian Language Meta-Package for LibreOffice</eu>
   <fa>Romanian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Romanian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Romanialainen metatieto-paketti</fi>
   <fr>Méta-paquet pour langue roumaine</fr>
   <he_IL>Romanian Language Meta-Package for LibreOffice</he_IL>
   <hi>रोमानियाई भाषा मेटा-पैकेज</hi>
   <hr>Romanian Language Meta-Package for LibreOffice</hr>
   <hu>Romanian Language Meta-Package for LibreOffice</hu>
   <id>Romanian Language Meta-Package for LibreOffice</id>
   <is>Romanian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua rumena</it>
   <ja>ルーマニア語メタパッケージ</ja>
   <kk>Romanian Language Meta-Package for LibreOffice</kk>
   <ko>Romanian Language Meta-Package for LibreOffice</ko>
   <lt>Romanian Language Meta-Package for LibreOffice</lt>
   <mk>Romanian Language Meta-Package for LibreOffice</mk>
   <mr>Romanian Language Meta-Package for LibreOffice</mr>
   <nb>Romansk språkpakke</nb>
   <nl>Roemeense Taal Meta-Pakket</nl>
   <pl>Rumuński metapakiet językowy</pl>
   <pt_BR>Romeno Meta-Pacote de Idioma</pt_BR>
   <pt>Romeno Meta-Pacote de Idioma</pt>
   <ro>Romanian Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет румынского языка</ru>
   <sk>Romanian Language Meta-Package for LibreOffice</sk>
   <sl>Romunski jezikovni meta-paket</sl>
   <sq>Meta-Paketë gjuhësore në rumanisht</sq>
   <sr>Romanian Language Meta-Package for LibreOffice</sr>
   <sv>Rumänskt Språk-Meta-Paket</sv>
   <tr>Rumence Dili Üst-Paketi</tr>
   <uk>Romanian Language Meta-Package for LibreOffice</uk>
   <vi>Romanian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Romanian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Romanian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ro
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ro
</uninstall_package_names>
</app>
