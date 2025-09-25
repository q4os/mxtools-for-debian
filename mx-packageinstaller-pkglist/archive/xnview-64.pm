<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Graphics
</category>

<name>
XnView
</name>

<description>
   <am>picture viewer, browser and converter</am>
   <ar>picture viewer, browser and converter</ar>
   <bg>picture viewer, browser and converter</bg>
   <bn>picture viewer, browser and converter</bn>
   <ca>visor d'imatges, navegador i conversor</ca>
   <cs>picture viewer, browser and converter</cs>
   <da>billedfremviser, browser og konverter</da>
   <de>Bildbetrachter, -browser und -konverter</de>
   <el>Πρόγραμμα προβολής εικόνων, πρόγραμμα περιήγησης και μετατροπέα</el>
   <en>picture viewer, browser and converter</en>
   <es>visor de imágenes, navegador y conversor</es>
   <et>picture viewer, browser and converter</et>
   <eu>picture viewer, browser and converter</eu>
   <fa>picture viewer, browser and converter</fa>
   <fil_PH>picture viewer, browser and converter</fil_PH>
   <fi>kuvankatseluohjelma, selain ja muunnin</fi>
   <fr>Visionner, feuilleter et convertir des images</fr>
   <he_IL>picture viewer, browser and converter</he_IL>
   <hi>चित्र प्रदर्शक, ब्राउज़र व रूपांतरण साधन</hi>
   <hr>picture viewer, browser and converter</hr>
   <hu>picture viewer, browser and converter</hu>
   <id>picture viewer, browser and converter</id>
   <is>picture viewer, browser and converter</is>
   <it>visualizzatore e convertitore di immagini</it>
   <ja>画像ビューアやブラウザ、コンバーター</ja>
   <kk>picture viewer, browser and converter</kk>
   <ko>picture viewer, browser and converter</ko>
   <lt>picture viewer, browser and converter</lt>
   <mk>picture viewer, browser and converter</mk>
   <mr>picture viewer, browser and converter</mr>
   <nb>bildeframvisning, utforsking og konvertering</nb>
   <nl>afbeelding bekijker, verkenner en omzetter</nl>
   <pl>przeglądarka zdjęć i konwerter</pl>
   <pt_BR>Visualizador, navegador e conversor de imagens</pt_BR>
   <pt>Visualizador, navegador e conversor de imagens</pt>
   <ro>picture viewer, browser and converter</ro>
   <ru>Просмотрщик изображений, каталогизатор и конвертер</ru>
   <sk>picture viewer, browser and converter</sk>
   <sl>Pregledovalnik slik, brskalnik in pretvornik</sl>
   <sq>parës, shfletues dhe shndërrues figurash</sq>
   <sr>picture viewer, browser and converter</sr>
   <sv>bildvisare och konverterare</sv>
   <tr>resim görüntüleyici, tarayıcı ve dönüştürücü</tr>
   <uk>picture viewer, browser and converter</uk>
   <vi>picture viewer, browser and converter</vi>
   <zh_CN>picture viewer, browser and converter</zh_CN>
   <zh_TW>picture viewer, browser and converter</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>https://linux-cdn.softpedia.com/screenshots/XnView-MP_2.jpeg</screenshot>

<preinstall>
wget https://download.xnview.com/XnViewMP-linux-x64.deb
dpkg -i XnViewMP-linux-x64.deb
rm XnViewMP-linux-x64.deb
</preinstall>

<install_package_names>
xnview
</install_package_names>


<postinstall>
CMD='QT_PLATFORMTHEME=${QT_PLATFORMTHEME/gtk2/fusion} QT_PLATFORM_PLUGIN=${QT_PLATFORMTHEME/gtk2/fusion} QT_QPA_PLATFORMTHEME=${QT_QPA_PLATFORMTHEME/gtk2/fusion} exec xnview "$@"'
sed -i "s%^Exec=xnview%Exec=bash -c '$CMD' _ %" /usr/share/applications/XnView.desktop 

</postinstall>


<uninstall_package_names>
xnview
</uninstall_package_names>
</app>
