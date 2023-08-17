<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Northern_sotho_Libreoffice
</name>

<description>
   <am>Northern_sotho Language Meta-Package for LibreOffice</am>
   <ar>Northern_sotho Language Meta-Package for LibreOffice</ar>
   <bg>Northern_sotho Language Meta-Package for LibreOffice</bg>
   <bn>Northern_sotho Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet de Northern_sotho per LibreOffice</ca>
   <cs>Northern_sotho Language Meta-Package for LibreOffice</cs>
   <da>Northern_sotho Language Meta-Package for LibreOffice</da>
   <de>Nord-Sotho Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Northern_sotho </el>
   <en>Northern_sotho Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de Lenguaje Northern_sotho para LibreOffice</es>
   <et>Northern_sotho Language Meta-Package for LibreOffice</et>
   <eu>Northern_sotho Language Meta-Package for LibreOffice</eu>
   <fa>Northern_sotho Language Meta-Package for LibreOffice</fa>
   <fil_PH>Northern_sotho Language Meta-Package for LibreOffice</fil_PH>
   <fi>Pohjoissothonkielinen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet de la langue sotho du Nord pour LibreOffice</fr>
   <he_IL>Northern_sotho Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु उत्तरी_सोथो भाषा मेटा-पैकेज</hi>
   <hr>Northern_sotho Language Meta-Package for LibreOffice</hr>
   <hu>Northern_sotho Language Meta-Package for LibreOffice</hu>
   <id>Northern_sotho Language Meta-Package for LibreOffice</id>
   <is>Northern_sotho Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua sotho del nord per LibreOffice</it>
   <ja>LibreOffice 用北ソト語メタパッケージ</ja>
   <kk>Northern_sotho Language Meta-Package for LibreOffice</kk>
   <ko>Northern_sotho Language Meta-Package for LibreOffice</ko>
   <lt>Northern_sotho Language Meta-Package for LibreOffice</lt>
   <mk>Northern_sotho Language Meta-Package for LibreOffice</mk>
   <mr>Northern_sotho Language Meta-Package for LibreOffice</mr>
   <nb>Nordsotho språkpakke for LibreOffice</nb>
   <nl>Noord_sotho Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-pakiet języka Northern_sotho dla LibreOffice</pl>
   <pt_BR>Sepedi Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Sotho setentrional Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Northern_sotho Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет языка северный сото для LibreOffice</ru>
   <sk>Northern_sotho Language Meta-Package for LibreOffice</sk>
   <sl>Severni sotovski jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në soto veriore për LibreOffice-in</sq>
   <sr>Northern_sotho Language Meta-Package for LibreOffice</sr>
   <sv>Nordlig_sotho Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Kuzey sotho Dili Üst-Paketi</tr>
   <uk>Northern_sotho Language Meta-Package for LibreOffice</uk>
   <vi>Northern_sotho Language Meta-Package for LibreOffice</vi>
   <zh_CN>Northern_sotho Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Northern_sotho Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-nso
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-nso
</uninstall_package_names>
</app>
