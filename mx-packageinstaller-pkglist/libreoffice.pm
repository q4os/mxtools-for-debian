<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Office
</category>

<name>
LibreOffice
</name>

<description>
   <am>Meta-Package for LibreOffice</am>
   <ar>Meta-Package for LibreOffice</ar>
   <bg>Meta-Package for LibreOffice</bg>
   <bn>Meta-Package for LibreOffice</bn>
   <ca>Metapaquet per LibreOffice</ca>
   <cs>Meta-Package for LibreOffice</cs>
   <da>Meta-Package for LibreOffice</da>
   <de>Meta-Paket für LibreOffice</de>
   <el>Meta-Package για το LibreOffice</el>
   <en>Meta-Package for LibreOffice</en>
   <es>Meta-Package para LibreOffice</es>
   <et>Meta-Package for LibreOffice</et>
   <eu>Meta-Package for LibreOffice</eu>
   <fa>Meta-Package for LibreOffice</fa>
   <fil_PH>Meta-Package for LibreOffice</fil_PH>
   <fi>Meta-Package for LibreOffice</fi>
   <fr>Meta-Package for LibreOffice</fr>
   <he_IL>Meta-Package for LibreOffice</he_IL>
   <hi>Meta-Package for LibreOffice</hi>
   <hr>Meta-Package for LibreOffice</hr>
   <hu>Meta-Package for LibreOffice</hu>
   <id>Meta-Package for LibreOffice</id>
   <is>Meta-Package for LibreOffice</is>
   <it>Meta-pacchetto per LibreOffice</it>
   <ja_JP>Meta-Package for LibreOffice</ja_JP>
   <ja>Meta-Package for LibreOffice</ja>
   <kk>Meta-Package for LibreOffice</kk>
   <ko>Meta-Package for LibreOffice</ko>
   <lt>Meta-Package for LibreOffice</lt>
   <mk>Meta-Package for LibreOffice</mk>
   <mr>Meta-Package for LibreOffice</mr>
   <nb>Meta-Package for LibreOffice</nb>
   <nl>Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-Pakiet dla LibreOffice</pl>
   <pt_BR>Meta-Package for LibreOffice</pt_BR>
   <pt>Meta-Pacote para LibreOffice</pt>
   <ro>Meta-Package for LibreOffice</ro>
   <ru>Мета-пакет для LibreOffice</ru>
   <sk>Meta-Package for LibreOffice</sk>
   <sl>Meta-paket za LibreOffice</sl>
   <sq>Meta-Package for LibreOffice</sq>
   <sr>Meta-Package for LibreOffice</sr>
   <sv>Meta-Paket för LibreOffice</sv>
   <tr>Meta-Package for LibreOffice</tr>
   <uk>Meta-Package for LibreOffice</uk>
   <vi>Meta-Package for LibreOffice</vi>
   <zh_CN>Meta-Package for LibreOffice</zh_CN>
   <zh_TW>Meta-Package for LibreOffice</zh_TW>
</description>

<installable>
32,64
</installable>

<screenshot>https://screenshots.debian.net/shrine/screenshot/18671/simage/large-c8241f577413887dbe78e15ff7706863.png</screenshot>

<preinstall>

lo-installer.sh --reinstall --main lo-main-helper
apt-get update -o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>

<uninstall_package_names>
libreoffice-base
libreoffice-calc
libreoffice-draw
libreoffice-impress
libreoffice-math
libreoffice-writer
ure
lo-main-helper
</uninstall_package_names>

<postuninstall>
apt-get --yes  -o=Dpkg::Use-Pty=0 remove 'libreoffice*' 'libuno*'
</postuninstall>

</app>
