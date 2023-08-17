<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Norwegian_Libreoffice
</name>

<description>
   <am>Norwegian Language Meta-Package for LibreOffice</am>
   <ar>Norwegian Language Meta-Package for LibreOffice</ar>
   <bg>Norwegian Language Meta-Package for LibreOffice</bg>
   <bn>Norwegian Language Meta-Package for LibreOffice</bn>
   <ca>Meta-paquet de llengua per a Noruec</ca>
   <cs>Norwegian Language Meta-Package for LibreOffice</cs>
   <da>Norsk sprog-metapakke til LibreOffice</da>
   <de>Norwegisches Sprach-Meta-Paket</de>
   <el>Νορβηγική γλώσσα</el>
   <en>Norwegian Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Noruego</es>
   <et>Norwegian Language Meta-Package for LibreOffice</et>
   <eu>Norwegian Language Meta-Package for LibreOffice</eu>
   <fa>Norwegian Language Meta-Package for LibreOffice</fa>
   <fil_PH>Norwegian Language Meta-Package for LibreOffice</fil_PH>
   <fi>Norjankielinen metatieto-paketti</fi>
   <fr>Méta-paquet pour langue norvégienne</fr>
   <he_IL>Norwegian Language Meta-Package for LibreOffice</he_IL>
   <hi>नॉर्वेजियाई भाषा मेटा-पैकेज</hi>
   <hr>Norwegian Language Meta-Package for LibreOffice</hr>
   <hu>Norwegian Language Meta-Package for LibreOffice</hu>
   <id>Norwegian Language Meta-Package for LibreOffice</id>
   <is>Norwegian Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua norvegese</it>
   <ja>ノルウェー語メタパッケージ</ja>
   <kk>Norwegian Language Meta-Package for LibreOffice</kk>
   <ko>Norwegian Language Meta-Package for LibreOffice</ko>
   <lt>Norwegian Language Meta-Package for LibreOffice</lt>
   <mk>Norwegian Language Meta-Package for LibreOffice</mk>
   <mr>Norwegian Language Meta-Package for LibreOffice</mr>
   <nb>Norsk bokmålsk språkpakke</nb>
   <nl>Noorse Taal Meta-Pakket</nl>
   <pl>Norweski metapakiet językowy</pl>
   <pt_BR>Norueguês Meta-Pacote de Idioma</pt_BR>
   <pt>Norueguês Meta-Pacote de Idioma</pt>
   <ro>Norwegian Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет норвежского языка</ru>
   <sk>Norwegian Language Meta-Package for LibreOffice</sk>
   <sl>Norveški jezikovni meta-paket</sl>
   <sq>Meta-Paketë gjuhësore në norvegjisht</sq>
   <sr>Norwegian Language Meta-Package for LibreOffice</sr>
   <sv>Norskt Språk Meta-Paket </sv>
   <tr>Norveççe Dili Üst-Paketi</tr>
   <uk>Norwegian Language Meta-Package for LibreOffice</uk>
   <vi>Norwegian Language Meta-Package for LibreOffice</vi>
   <zh_CN>Norwegian Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Norwegian Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-nb
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-nb
</uninstall_package_names>
</app>
