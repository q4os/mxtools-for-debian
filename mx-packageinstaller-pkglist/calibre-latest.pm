<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Office
</category>

<name>
Calibre (latest)
</name>

<description>
   <am>an e-book library management application</am>
   <ar>an e-book library management application</ar>
   <bg>an e-book library management application</bg>
   <ca>una aplicació de gestió de biblioteca de llibres electrònics</ca>
   <cs>an e-book library management application</cs>
   <da>et program til at håndtere e-bogsbibliotek</da>
   <de>Eine E-Book-Bibliotheksverwaltung</de>
   <el>Εφαρμογή διαχείρισης βιβλιοθήκης ηλεκτρονικών βιβλίων</el>
   <en>an e-book library management application</en>
   <es>Organizador de biblioteca de e-book</es>
   <et>an e-book library management application</et>
   <eu>an e-book library management application</eu>
   <fa>an e-book library management application</fa>
   <fi>e-kirjoja sisältävän kirjaston hallintasovellus</fi>
   <fr>Une application pour organiser sa bibliothèque e-book</fr>
   <he_IL>an e-book library management application</he_IL>
   <hi>ई-पुस्तक संग्रह प्रबंधन अनुप्रयोग </hi>
   <hr>an e-book library management application</hr>
   <hu>an e-book library management application</hu>
   <id>an e-book library management application</id>
   <is>an e-book library management application</is>
   <it>Applicazione per la gestione di librerie e-book</it>
   <ja>電子図書管理アプリケーション</ja>
   <kk>an e-book library management application</kk>
   <ko>an e-book library management application</ko>
   <lt>an e-book library management application</lt>
   <mk>an e-book library management application</mk>
   <mr>an e-book library management application</mr>
   <nb>program for å håndtere e-bøker</nb>
   <nl>een e-boek bibliotheek beheer applicatie</nl>
   <pl>aplikacja do zarządzania bibliotekami e-booków w systemie,  ich wyświetlania, edycji, tworzenia i konwersji</pl>
   <pt_BR>E-book - Aplicativo de gerenciamento de bibliotecas para livros eletrônicos</pt_BR>
   <pt>Aplicação de gestão de bibliotecas para livros electrónicos e-book</pt>
   <ro>an e-book library management application</ro>
   <ru>Менеджер библиотеки электронных книг</ru>
   <sk>an e-book library management application</sk>
   <sl>aplikacija za upravljanje knjižnic z elektronskimi knjigami</sl>
   <sq>një aplikacion administrimi biblioteke e-librash</sq>
   <sr>an e-book library management application</sr>
   <sv>en hanteringsapp för e-boksbibliotek</sv>
   <tr>bir e-kitap kütüphane yönetim uygulaması</tr>
   <uk>програма керування бібліотекою електронних книжок</uk>
   <zh_CN>an e-book library management application</zh_CN>
   <zh_TW>an e-book library management application</zh_TW>
</description>

<installable>
32,64
</installable>

<screenshot>https://screenshots.debian.net/shrine/screenshot/14971/simage/large-11a7b7b361bc4ac3a7b0488a61b6a157.png</screenshot>

<preinstall>

</preinstall>

<install_package_names>
calibre-helper
</install_package_names>

<postinstall>
echo "Installing calibre ..."
TMP=$(mktemp -d /tmp/tmpdir-calibre-installer.XXXXXXXXXXXXX)
# prepare tidy-up
tidy_up() { rm -r /tmp/tmpdir-calibre-installer.* 2>/dev/null ; }
trap tidy_up EXIT
INS=$TMP/linux-installer.sh
curl -o $INS -RLJ  https://download.calibre-ebook.com/linux-installer.sh
chmod +x $INS
$INS
echo "Fixing calibre-uninstaller to work with MXPI..."
# fix calibre uninstaller to work with MXPI
# by "removing" raw_input on /dev/tty to disable blocking keyboard
if [ -x /usr/bin/calibre-uninstall ]; then
    sed -i -r  -e "\:^sys.stdin = open[(]'/dev/tty'[)].*:s:^:#:" \
               -e "s/raw_input[(][^)]+[)]/'y'/" /usr/bin/calibre-uninstall
fi
echo "DONE!"
</postinstall>

<uninstall_package_names>
calibre-helper
</uninstall_package_names>

<postuninstall>
if [ -x /usr/bin/calibre-uninstall ]; then
   echo "Removing calibre ..."
   echo "y" | /usr/bin/calibre-uninstall
fi
echo "DONE!"
</postuninstall>

</app>
