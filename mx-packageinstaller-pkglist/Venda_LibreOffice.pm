<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Venda_Libreoffice
</name>

<description>
   <am>Venda Language Meta-Package for LibreOffice</am>
   <ar>Venda Language Meta-Package for LibreOffice</ar>
   <bg>Venda Language Meta-Package for LibreOffice</bg>
   <bn>Venda Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Venda per LibreOffice</ca>
   <cs>Venda Language Meta-Package for LibreOffice</cs>
   <da>Venda Language Meta-Package for LibreOffice</da>
   <de>Venda Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Venda</el>
   <en>Venda Language Meta-Package for LibreOffice</en>
   <es>Meta-Paquete de idioma Venda para LibreOffice</es>
   <et>Venda Language Meta-Package for LibreOffice</et>
   <eu>Venda Language Meta-Package for LibreOffice</eu>
   <fa>Venda Language Meta-Package for LibreOffice</fa>
   <fil_PH>Venda Language Meta-Package for LibreOffice</fil_PH>
   <fi>Vendankielinen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet de langues Venda pour LibreOffice</fr>
   <he_IL>Venda Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु वेंदा भाषा मेटा-पैकेज</hi>
   <hr>Venda Language Meta-Package for LibreOffice</hr>
   <hu>Venda Language Meta-Package for LibreOffice</hu>
   <id>Venda Language Meta-Package for LibreOffice</id>
   <is>Venda Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua venda per LibreOffice</it>
   <ja>LibreOffice 用 ヴェンダ語メタパッケージ</ja>
   <kk>Venda Language Meta-Package for LibreOffice</kk>
   <ko>Venda Language Meta-Package for LibreOffice</ko>
   <lt>Venda Language Meta-Package for LibreOffice</lt>
   <mk>Venda Language Meta-Package for LibreOffice</mk>
   <mr>Venda Language Meta-Package for LibreOffice</mr>
   <nb>Venda språkpakke for LibreOffice</nb>
   <nl>Venda Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-pakiet języka Venda dla Libre Office</pl>
   <pt_BR>Venda Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Venda Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Venda Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет языка венда для LibreOffice</ru>
   <sk>Venda Language Meta-Package for LibreOffice</sk>
   <sl>Vendski jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në vendaisht për LibreOffice-in</sq>
   <sr>Venda Language Meta-Package for LibreOffice</sr>
   <sv>Venda Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Venda Dili Üst-Paketi</tr>
   <uk>Venda Language Meta-Package for LibreOffice</uk>
   <vi>Venda Language Meta-Package for LibreOffice</vi>
   <zh_CN>Venda Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Venda Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-ve
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-ve
</uninstall_package_names>
</app>
