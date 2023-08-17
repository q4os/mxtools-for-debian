<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Office
</category>

<name>
Master PDF Editor 4 (Free Version)
</name>

<description>
   <am>Edit PDF documents.</am>
   <ar>Edit PDF documents.</ar>
   <bg>Edit PDF documents.</bg>
   <bn>Edit PDF documents.</bn>
   <ca>Editor de documents PDF</ca>
   <cs>Edit PDF documents.</cs>
   <da>Rediger PDF-dokumenter.</da>
   <de>PDF-Dokumente bearbeiten</de>
   <el>Επεξεργασία εγγράφων PDF.</el>
   <en>Edit PDF documents.</en>
   <es>Editor de documentos PDF</es>
   <et>Edit PDF documents.</et>
   <eu>Edit PDF documents.</eu>
   <fa>Edit PDF documents.</fa>
   <fil_PH>Edit PDF documents.</fil_PH>
   <fi>Muokkaa PDF-asiakirjoja.</fi>
   <fr>Éditer des documents PDF.</fr>
   <he_IL>Edit PDF documents.</he_IL>
   <hi>पीडीएफ प्रलेख संपादन।</hi>
   <hr>Edit PDF documents.</hr>
   <hu>Edit PDF documents.</hu>
   <id>Edit PDF documents.</id>
   <is>Edit PDF documents.</is>
   <it>Modifica i documenti in PDF.</it>
   <ja>PDF 文書の編集</ja>
   <kk>Edit PDF documents.</kk>
   <ko>Edit PDF documents.</ko>
   <lt>Taisyti PDF dokumentus.</lt>
   <mk>Edit PDF documents.</mk>
   <mr>Edit PDF documents.</mr>
   <nb>Rediger PDF-dokumenter.</nb>
   <nl>Bewerk PDF documenten</nl>
   <pl>edytuje dokumenty PDF</pl>
   <pt_BR>Editor de documentos em formato .pdf.</pt_BR>
   <pt>Editar documentos em formato .pdf.</pt>
   <ro>Edit PDF documents.</ro>
   <ru>Редактор PDF документов</ru>
   <sk>Edit PDF documents.</sk>
   <sl>Urejanje PDF dokumentov</sl>
   <sq>Përpunoni dokumente PDF.</sq>
   <sr>Edit PDF documents.</sr>
   <sv>Redigera PDF-dokument</sv>
   <tr>PDF belgeleri düzenle.</tr>
   <uk>редагування PDF документів.</uk>
   <vi>Edit PDF documents.</vi>
   <zh_CN>Edit PDF documents.</zh_CN>
   <zh_TW>Edit PDF documents.</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>https://code-industry.net/wp-content/uploads/2016/05/pdfa-edit.png</screenshot>

<preinstall>

<![CDATA[
set -e
ARC=$(dpkg --print-architecture)
DEB="master-pdf-editor-4.3.89_${ARC/amd64/qt5.amd64}.deb"
URL="https://code-industry.net/public"
TMPDEB=/tmp/master_pdf_4.deb
TMPDIR=/tmp/master_pdf_4_dir

test -e $TMPDEB && rm -r $TMPDEB
test -e $TMPDIR && rm -r $TMPDIR
mkdir $TMPDIR 
echo "Get: $DEB"
echo ""
curl --progress-bar -RLJ -o $TMPDEB $URL/$DEB
pushd $TMPDIR >/dev/null
ar p  $TMPDEB control.tar.gz | tar -xz
sed -i s/master-pdf-editor/master-pdf-editor-4/ control
tar -czf control.tar.gz *[!z]
ar r $TMPDEB  control.tar.gz
popd >/dev/null
echo "Install $DEB"
dpkg -i $TMPDEB
apt-get install -yf
rm $TMPDEB
rm -r $TMPDIR
echo "...$(gettext -d apt -s ' Done')!"
]]>

</preinstall>

<install_package_names>

</install_package_names>


<postinstall>

</postinstall>


<uninstall_package_names>
master-pdf-editor-4
</uninstall_package_names>
</app>
