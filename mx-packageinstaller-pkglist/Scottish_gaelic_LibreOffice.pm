<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Scottish_gaelic_Libreoffice
</name>

<description>
   <am>Scottish_gaelic Language Meta-Package for LibreOffice</am>
   <ar>Scottish_gaelic Language Meta-Package for LibreOffice</ar>
   <bg>Scottish_gaelic Language Meta-Package for LibreOffice</bg>
   <bn>Scottish_gaelic Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Gaèlic Escocès per LibreOffice</ca>
   <cs>Scottish_gaelic Language Meta-Package for LibreOffice</cs>
   <da>Scottish_gaelic Language Meta-Package for LibreOffice</da>
   <de>Schottisch-gälisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Scottish Gaelic </el>
   <en>Scottish_gaelic Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma gallego escocés para LibreOffice</es>
   <et>Scottish_gaelic Language Meta-Package for LibreOffice</et>
   <eu>Scottish_gaelic Language Meta-Package for LibreOffice</eu>
   <fa>Scottish_gaelic Language Meta-Package for LibreOffice</fa>
   <fil_PH>Scottish_gaelic Language Meta-Package for LibreOffice</fil_PH>
   <fi>Skotlantilais-gaelilainen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet en langue gaélique écossaise pour LibreOffice</fr>
   <he_IL>Scottish_gaelic Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु स्कॉटिश_गेलिक भाषा मेटा-पैकेज</hi>
   <hr>Scottish_gaelic Language Meta-Package for LibreOffice</hr>
   <hu>Scottish_gaelic Language Meta-Package for LibreOffice</hu>
   <id>Scottish_gaelic Language Meta-Package for LibreOffice</id>
   <is>Scottish_gaelic Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua gaelica scozzese per LibreOffice</it>
   <ja>LibreOffice用の高地スコットランド・ゲール語メタパッケージ</ja>
   <kk>Scottish_gaelic Language Meta-Package for LibreOffice</kk>
   <ko>Scottish_gaelic Language Meta-Package for LibreOffice</ko>
   <lt>Scottish_gaelic Language Meta-Package for LibreOffice</lt>
   <mk>Scottish_gaelic Language Meta-Package for LibreOffice</mk>
   <mr>Scottish_gaelic Language Meta-Package for LibreOffice</mr>
   <nb>Skotsk-gælisk språkpakke for LibreOffice</nb>
   <nl>Schots-Gaelic Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-pakiet języka gaelickiego szkockiego dla LibreOffice</pl>
   <pt_BR>Gaélico Escocês Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Gaélico_escocês Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Scottish_gaelic Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет шотландского языка для LibreOffice</ru>
   <sk>Scottish_gaelic Language Meta-Package for LibreOffice</sk>
   <sl>Škotski gelik jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësire në skoceze gaelike për LibreOffice</sq>
   <sr>Scottish_gaelic Language Meta-Package for LibreOffice</sr>
   <sv>Skottsk_gaelic Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için İskoç Dili Üst-Paketi</tr>
   <uk>Scottish_gaelic Language Meta-Package for LibreOffice</uk>
   <vi>Scottish_gaelic Language Meta-Package for LibreOffice</vi>
   <zh_CN>Scottish_gaelic Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Scottish_gaelic Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-gd
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-gd
</uninstall_package_names>
</app>
