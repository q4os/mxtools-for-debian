<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Office
</category>

<name>
LibreOffice (backports version)
</name>

<description>
   <am>Meta-Package for LibreOffice (backports version)</am>
   <ar>Meta-Package for LibreOffice (backports version)</ar>
   <bg>Meta-Package for LibreOffice (backports version)</bg>
   <bn>Meta-Package for LibreOffice (backports version)</bn>
   <ca>Metapaquet per LibreOffice</ca>
   <cs>Meta-Package for LibreOffice (backports version)</cs>
   <da>Meta-Package for LibreOffice (backports version)</da>
   <de>Meta-Paket für LibreOffice</de>
   <el>Meta-Package για το LibreOffice</el>
   <en>Meta-Package for LibreOffice (backports version)</en>
   <es>Meta-Package para LibreOffice</es>
   <et>Meta-Package for LibreOffice (backports version)</et>
   <eu>Meta-Package for LibreOffice (backports version)</eu>
   <fa>Meta-Package for LibreOffice (backports version)</fa>
   <fil_PH>Meta-Package for LibreOffice (backports version)</fil_PH>
   <fi>Metatieto-paketti LibreOffice:lle</fi>
   <fr>Meta-Paquet pour LibreOffice</fr>
   <he_IL>Meta-Package for LibreOffice (backports version)</he_IL>
   <hi>लिब्रे-ऑफिस हेतु मेटा-पैकेज</hi>
   <hr>Meta-Package for LibreOffice (backports version)</hr>
   <hu>Meta-Package for LibreOffice (backports version)</hu>
   <id>Meta-Package for LibreOffice (backports version)</id>
   <is>Meta-Package for LibreOffice (backports version)</is>
   <it>Meta-pacchetto per LibreOffice</it>
   <ja>LibreOffice用メタパッケージ</ja>
   <kk>Meta-Package for LibreOffice (backports version)</kk>
   <ko>Meta-Package for LibreOffice (backports version)</ko>
   <lt>Meta-Package for LibreOffice (backports version)</lt>
   <mk>Meta-Package for LibreOffice (backports version)</mk>
   <mr>Meta-Package for LibreOffice (backports version)</mr>
   <nb>LibreOffice metapakke</nb>
   <nl>Meta-Pakket voor LibreOffice</nl>
   <pl>Meta-Pakiet dla LibreOffice</pl>
   <pt_BR>Meta-Pacote para o LibreOffice</pt_BR>
   <pt>Meta-Pacote para LibreOffice</pt>
   <ro>Meta-Package for LibreOffice (backports version)</ro>
   <ru>Мета-пакет для LibreOffice</ru>
   <sk>Meta-Package for LibreOffice (backports version)</sk>
   <sl>Meta-paket za LibreOffice</sl>
   <sq>Meta-Paketë për LibreOffice-in</sq>
   <sr>Meta-Package for LibreOffice (backports version)</sr>
   <sv>Meta-Paket för LibreOffice</sv>
   <tr>LibreOffice için Üst-Paket</tr>
   <uk>Meta-Package for LibreOffice (backports version)</uk>
   <vi>Meta-Package for LibreOffice (backports version)</vi>
   <zh_CN>Meta-Package for LibreOffice (backports version)</zh_CN>
   <zh_TW>Meta-Package for LibreOffice (backports version)</zh_TW>
</description>


<installable>
32,64
</installable>

<screenshot>https://screenshots.debian.net/shrine/screenshot/18671/simage/large-c8241f577413887dbe78e15ff7706863.png</screenshot>

<preinstall>
apt-get install default-jre
lo-installer.sh --reinstall --backports lo-backports-helper
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
lo-backports-helper
</uninstall_package_names>

<postuninstall>
apt-get --yes  -o=Dpkg::Use-Pty=0 remove 'libreoffice*' 'libuno*'
</postuninstall>

</app>
