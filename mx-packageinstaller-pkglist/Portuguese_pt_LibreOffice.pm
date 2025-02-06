<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Portuguese(PT) Libreoffice
</name>

<description>
   <am>Portuguese(PT) Language Meta-Package for LibreOffice</am>
   <ar>Portuguese(PT) Language Meta-Package for LibreOffice</ar>
   <bg>Portuguese(PT) Language Meta-Package for LibreOffice</bg>
   <bn>Portuguese(PT) Language Meta-Package for LibreOffice</bn>
   <ca>Meta-paquet de llengua per Portuguès (PT)</ca>
   <cs>Portuguese(PT) Language Meta-Package for LibreOffice</cs>
   <da>Portugisisk (portugal) sprog-metapakke til LibreOffice</da>
   <de>Portugiesisches (PT) Sprach-Meta-Paket</de>
   <el>Πορτογαλική γλώσσα</el>
   <en>Portuguese(PT) Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Portugués (PT)</es>
   <et>Portuguese(PT) Language Meta-Package for LibreOffice</et>
   <eu>Portuguese(PT) Language Meta-Package for LibreOffice</eu>
   <fa>Portuguese(PT) Language Meta-Package for LibreOffice</fa>
   <fil_PH>Portuguese(PT) Language Meta-Package for LibreOffice</fil_PH>
   <fi>Portugalinkielinen (PT) metatieto-paketti</fi>
   <fr>Méta-paquet pour langue portugaise (PT)</fr>
   <he_IL>Portuguese(PT) Language Meta-Package for LibreOffice</he_IL>
   <hi>पुर्तगाली (पुर्तगाल) भाषा मेटा-पैकेज</hi>
   <hr>Portuguese(PT) Language Meta-Package for LibreOffice</hr>
   <hu>Portuguese(PT) Language Meta-Package for LibreOffice</hu>
   <id>Portuguese(PT) Language Meta-Package for LibreOffice</id>
   <is>Portuguese(PT) Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua portoghese(PT)</it>
   <ja>ポルトガル語 (PT) のメタパッケージ</ja>
   <kk>Portuguese(PT) Language Meta-Package for LibreOffice</kk>
   <ko>Portuguese(PT) Language Meta-Package for LibreOffice</ko>
   <lt>Portuguese(PT) Language Meta-Package for LibreOffice</lt>
   <mk>Portuguese(PT) Language Meta-Package for LibreOffice</mk>
   <mr>Portuguese(PT) Language Meta-Package for LibreOffice</mr>
   <nb>Portugisisk språkpakke</nb>
   <nl>Portuguese(PT) Taal Meta-Pakket</nl>
   <pl>Portugalski (PT) metapakiet językowy</pl>
   <pt_BR>Português (PT) Meta-Pacote de Idioma</pt_BR>
   <pt>Português(PT) Meta-Pacote de Idioma</pt>
   <ro>Portuguese(PT) Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет португальского языка</ru>
   <sk>Portuguese(PT) Language Meta-Package for LibreOffice</sk>
   <sl>Portugalski (PT) jezikovni meta-paket</sl>
   <sq>Meta-Paketë gjuhësore në portugalishte (PT)</sq>
   <sr>Portuguese(PT) Language Meta-Package for LibreOffice</sr>
   <sv>Portugisiskt(PT) Språk Meta-Paket </sv>
   <tr>Portekizca(PT) Dili Üst-Paketi</tr>
   <uk>Portuguese(PT) Language Meta-Package for LibreOffice</uk>
   <vi>Portuguese(PT) Language Meta-Package for LibreOffice</vi>
   <zh_CN>Portuguese(PT) Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Portuguese(PT) Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-pt
libreoffice-help-pt
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-pt
libreoffice-help-pt
</uninstall_package_names>
</app>
