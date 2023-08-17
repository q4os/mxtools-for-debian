<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Dutch_Libreoffice
</name>

<description>
   <am>Dutch LibreOffice Language Meta-Package</am>
   <ar>Dutch LibreOffice Language Meta-Package</ar>
   <bg>Dutch LibreOffice Language Meta-Package</bg>
   <bn>Dutch LibreOffice Language Meta-Package</bn>
   <ca>Meta-paquet per LibreOffice en Holandès</ca>
   <cs>Dutch LibreOffice Language Meta-Package</cs>
   <da>Hollandsk LibreOffice sprog-metapakke</da>
   <de>Niederländisches LibreOffice Meta-Paket</de>
   <el>Libreoffice στα Ολλανδικά</el>
   <en>Dutch LibreOffice Language Meta-Package</en>
   <es>Meta-Paquete de Idioma Holandés LibreOffice</es>
   <et>Dutch LibreOffice Language Meta-Package</et>
   <eu>Dutch LibreOffice Language Meta-Package</eu>
   <fa>Dutch LibreOffice Language Meta-Package</fa>
   <fil_PH>Dutch LibreOffice Language Meta-Package</fil_PH>
   <fi>Hollanninkielinen metatieto-paketti LibreOffice:lle</fi>
   <fr>Méta-Paquet langue néerlandaise pour LibreOffice</fr>
   <he_IL>Dutch LibreOffice Language Meta-Package</he_IL>
   <hi>लिब्रे-ऑफिस हेतु डच भाषा मेटा-पैकेज</hi>
   <hr>Dutch LibreOffice Language Meta-Package</hr>
   <hu>Dutch LibreOffice Language Meta-Package</hu>
   <id>Dutch LibreOffice Language Meta-Package</id>
   <is>Dutch LibreOffice Language Meta-Package</is>
   <it>Meta-pacchetto della lingua olandese per LibreOffice</it>
   <ja>オランダ語の LibreOffice言語メタパッケージ</ja>
   <kk>Dutch LibreOffice Language Meta-Package</kk>
   <ko>Dutch LibreOffice Language Meta-Package</ko>
   <lt>Dutch LibreOffice Language Meta-Package</lt>
   <mk>Dutch LibreOffice Language Meta-Package</mk>
   <mr>Dutch LibreOffice Language Meta-Package</mr>
   <nb>Nederlandsk språkpakke for LibreOffice</nb>
   <nl>Nederlandse LibreOffice Taal Meta-Pakket</nl>
   <pl>Holenderski metapakiet językowy dla LibreOffice</pl>
   <pt_BR>Holandês Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Holandês Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Dutch LibreOffice Language Meta-Package</ro>
   <ru>Мета-пакет голландского языка для LibreOffice</ru>
   <sk>Dutch LibreOffice Language Meta-Package</sk>
   <sl>Nizozemski jezikovni metapaket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në holandisht për LibreOffice-in</sq>
   <sr>Dutch LibreOffice Language Meta-Package</sr>
   <sv>Holländska LibreOffice Språk Meta-Paket</sv>
   <tr>Flemenkçe LibreOffice Dil Üst-Paketi</tr>
   <uk>Dutch LibreOffice Language Meta-Package</uk>
   <vi>Dutch LibreOffice Language Meta-Package</vi>
   <zh_CN>Dutch LibreOffice Language Meta-Package</zh_CN>
   <zh_TW>Dutch LibreOffice Language Meta-Package</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-nl
libreoffice-help-nl
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-nl
libreoffice-help-nl
</uninstall_package_names>
</app>
