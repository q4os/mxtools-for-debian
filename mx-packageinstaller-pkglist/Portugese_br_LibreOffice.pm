<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Portugese(BR) Libreoffice
</name>

<description>
   <am>Portugese(BR) Language Meta-Package for LibreOffice</am>
   <ar>Portugese(BR) Language Meta-Package for LibreOffice</ar>
   <bg>Portugese(BR) Language Meta-Package for LibreOffice</bg>
   <bn>Portugese(BR) Language Meta-Package for LibreOffice</bn>
   <ca>Meta-paquet de llengua en Portuguès (BR)</ca>
   <cs>Portugese(BR) Language Meta-Package for LibreOffice</cs>
   <da>Portugisisk (brasilien) sprog-metapakke til LibreOffice</da>
   <de>Portugiesisches (BR) Sprach-Meta-Paket</de>
   <el>Πορτογαλικά(Βραζιλίας) Γλώσσα</el>
   <en>Portugese(BR) Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Idioma Portugués (BR)</es>
   <et>Portugese(BR) Language Meta-Package for LibreOffice</et>
   <eu>Portugese(BR) Language Meta-Package for LibreOffice</eu>
   <fa>Portugese(BR) Language Meta-Package for LibreOffice</fa>
   <fil_PH>Portugese(BR) Language Meta-Package for LibreOffice</fil_PH>
   <fi>Portugalinkielinen (BR) metatieto-paketti</fi>
   <fr>Méta-paquet pour langue portugaise (BR)</fr>
   <he_IL>Portugese(BR) Language Meta-Package for LibreOffice</he_IL>
   <hi>पुर्तगाली (ब्राज़ील) भाषा मेटा-पैकेज</hi>
   <hr>Portugese(BR) Language Meta-Package for LibreOffice</hr>
   <hu>Portugese(BR) Language Meta-Package for LibreOffice</hu>
   <id>Portugese(BR) Language Meta-Package for LibreOffice</id>
   <is>Portugese(BR) Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua portoghese(BR)</it>
   <ja>ポルトガル語(BR) 言語メタパッケージ</ja>
   <kk>Portugese(BR) Language Meta-Package for LibreOffice</kk>
   <ko>Portugese(BR) Language Meta-Package for LibreOffice</ko>
   <lt>Portugese(BR) Language Meta-Package for LibreOffice</lt>
   <mk>Portugese(BR) Language Meta-Package for LibreOffice</mk>
   <mr>Portugese(BR) Language Meta-Package for LibreOffice</mr>
   <nb>Brasiliansk portugisisk språkpakke</nb>
   <nl>Portugese(BR) Taal Meta-Pakket</nl>
   <pl>Portugalski (BR) metapakiet językowy</pl>
   <pt_BR>Português (BR) Meta-Pacote de Idioma</pt_BR>
   <pt>Português(BR) Meta-Pacote de Idioma</pt>
   <ro>Portugese(BR) Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет бразильского португальского языка</ru>
   <sk>Portugese(BR) Language Meta-Package for LibreOffice</sk>
   <sl>Portugalski (BR) jezikovni meta-paket</sl>
   <sq>Meta-Paketë gjuhësore në portugalishte (BR)</sq>
   <sr>Portugese(BR) Language Meta-Package for LibreOffice</sr>
   <sv>Portugisiskt(BR) Språk Meta-Paket </sv>
   <tr>Portekizca(BR) Dili Üst-Paketi</tr>
   <uk>Portugese(BR) Language Meta-Package for LibreOffice</uk>
   <vi>Portugese(BR) Language Meta-Package for LibreOffice</vi>
   <zh_CN>Portugese(BR) Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Portugese(BR) Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-pt-br
libreoffice-help-pt-br
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-pt-br
libreoffice-help-pt-br
</uninstall_package_names>
</app>
