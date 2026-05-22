<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Office
</category>

<name>
SoftMaker FreeOffice 2021
</name>

<description>
   <am>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</am>
   <ar>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</ar>
   <bg>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</bg>
   <bn>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</bn>
   <ca>SoftMaker FreeOffice 2021 és una suite ofimàtica ràpida, compatible i fiable.</ca>
   <cs>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</cs>
   <da>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</da>
   <de>SoftMaker FreeOffice 2021 ist ein schnelles, kompatibles und zuverlässiges Paket der Standardbüroprogramme</de>
   <el>Το SoftMaker FreeOffice 2021 είναι μια γρήγορη, συμβατή και αξιόπιστη σουίτα γραφείου</el>
   <en>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</en>
   <es>SoftMaker FreeOffice 2021 es una suite ofimática rápida, compatible y confiable</es>
   <et>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</et>
   <eu>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</eu>
   <fa>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</fa>
   <fil_PH>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</fil_PH>
   <fi>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</fi>
   <fr>SoftMaker FreeOffice 2021 est une suite bureautique fiable, rapide et compatible</fr>
   <he_IL>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</he_IL>
   <hi>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</hi>
   <hr>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</hr>
   <hu>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</hu>
   <id>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</id>
   <is>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</is>
   <it>SoftMaker FreeOffice 2021 è una suite per ufficio veloce, affidabile e compatibile</it>
   <ja>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</ja>
   <kk>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</kk>
   <ko>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</ko>
   <lt>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</lt>
   <mk>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</mk>
   <mr>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</mr>
   <nb>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</nb>
   <nl>SoftMaker FreeOffice 2021 is een snel, geschikt en betrouwbaar officeprogramma</nl>
   <pl>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</pl>
   <pt_BR>O SoftMaker FreeOffice 2021 é uma suite de escritório rápida, compatível e confiável</pt_BR>
   <pt>O SoftMaker FreeOffice 2021 é uma suite de escritório rápida, compatível com outras e confiável</pt>
   <ro>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</ro>
   <ru>SoftMaker FreeOffice 2021 - быстрый, совместимый и надежный офисный пакет</ru>
   <sk>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</sk>
   <sl>SoftMaker FreeOffice 2021 je hiter in združljiv paket za pisarniško rabo</sl>
   <sq>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</sq>
   <sr>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</sr>
   <sv>SoftMaker FreeOffice 2021 är snabb, kompatibel och pålitlig kontors-uppsättning</sv>
   <tr>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</tr>
   <uk>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</uk>
   <vi>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</vi>
   <zh_CN>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</zh_CN>
   <zh_TW>SoftMaker FreeOffice 2021 is a fast, compatible and reliable office suite</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>

echo "---------------------"
echo "FreeOffice Installer "
echo "---------------------"

mkdir -p /etc/apt/keyrings
curl -fsSL https://shop.softmaker.com/repo/apt/softmaker-repo.asc | gpg --dearmor > /etc/apt/keyrings/softmaker.gpg
echo "deb [signed-by=/etc/apt/keyrings/softmaker.gpg] https://shop.softmaker.com/repo/apt stable non-free" > /etc/apt/sources.list.d/softmaker.list

apt-get update

</preinstall>

<install_package_names>
softmaker-freeoffice-2021
</install_package_names>

<postinstall>

</postinstall>

<uninstall_package_names>
softmaker-freeoffice-2021
</uninstall_package_names>

<postuninstall>

echo "removing Softmaker repository key"
rm /etc/apt/keybrings/softmaker.gpg

echo "removing Softmaker repository  /etc/apt/sources.list.d/softmaker.list"
rm /etc/apt/sources.list.d/softmaker.list
echo "DONE"

</postuninstall>

</app>
