<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Language
</category>

<name>
Tajik_Libreoffice
</name>

<description>
   <am>Tajik Language Meta-Package for LibreOffice</am>
   <ar>Tajik Language Meta-Package for LibreOffice</ar>
   <bg>Tajik Language Meta-Package for LibreOffice</bg>
   <bn>Tajik Language Meta-Package for LibreOffice</bn>
   <ca>Metapaquet d'idioma Tadjik per LibreOffice</ca>
   <cs>Tajik Language Meta-Package for LibreOffice</cs>
   <da>Tajik Language Meta-Package for LibreOffice</da>
   <de>Tadschikisches Meta-Paket für LibreOffice</de>
   <el>LibreOffice στα Tajik</el>
   <en>Tajik Language Meta-Package for LibreOffice</en>
   <es>Metapaquete de idioma tayiko para LibreOffice</es>
   <et>Tajik Language Meta-Package for LibreOffice</et>
   <eu>Tajik Language Meta-Package for LibreOffice</eu>
   <fa>Tajik Language Meta-Package for LibreOffice</fa>
   <fil_PH>Tajik Language Meta-Package for LibreOffice</fil_PH>
   <fi>Tadzikinkielinen kielipaketti LibreOffice:lle</fi>
   <fr>Méta-Paquet en langue tadjike pour LibreOffice</fr>
   <he_IL>Tajik Language Meta-Package for LibreOffice</he_IL>
   <hi>लिब्रे-ऑफिस हेतु ताजिक भाषा मेटा-पैकेज</hi>
   <hr>Tajik Language Meta-Package for LibreOffice</hr>
   <hu>Tajik Language Meta-Package for LibreOffice</hu>
   <id>Tajik Language Meta-Package for LibreOffice</id>
   <is>Tajik Language Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto della lingua tagica per LibreOffice</it>
   <ja>LibreOffice 用のタジク語メタパッケージ</ja>
   <kk>Tajik Language Meta-Package for LibreOffice</kk>
   <ko>Tajik Language Meta-Package for LibreOffice</ko>
   <lt>Tajik Language Meta-Package for LibreOffice</lt>
   <mk>Tajik Language Meta-Package for LibreOffice</mk>
   <mr>Tajik Language Meta-Package for LibreOffice</mr>
   <nb>Tajik språkpakke for LibreOffice</nb>
   <nl>Tajik Taal Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-pakiet języka tadżyckiego dla LibreOffice</pl>
   <pt_BR>Tajique Pacote de Idioma para o LibreOffice</pt_BR>
   <pt>Tajique Meta-Pacote de Idioma para LibreOffice</pt>
   <ro>Tajik Language Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет таджикского языка для LibreOffice</ru>
   <sk>Tajik Language Meta-Package for LibreOffice</sk>
   <sl>Tadžiški jezikovni meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë gjuhësore në taxhike për LibreOffice-in</sq>
   <sr>Tajik Language Meta-Package for LibreOffice</sr>
   <sv>Tajik Språk Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Tacikçe Dili Üst-Paketi</tr>
   <uk>Tajik Language Meta-Package for LibreOffice</uk>
   <vi>Tajik Language Meta-Package for LibreOffice</vi>
   <zh_CN>Tajik Language Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Tajik Language Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>


<preinstall>

package_list="
libreoffice-l10n-tg
"
lo-installer.sh $package_list
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
libreoffice-l10n-tg
</uninstall_package_names>
</app>
