<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Remote Access
</category>

<name>
RustDesk
</name>

<description>
<am>የርቀት መዳረሻ ሶፍትዌር በዝገት የተፃፈ</am>
<ar>برنامج الوصول عن بعد مكتوب بالصدأ</ar>
<bg>софтуер за отдалечен достъп, написан на rust</bg>
<bn>মরিচা লেখা রিমোট অ্যাক্সেস সফ্টওয়্যার</bn>
<ca>programari d'accés remot escrit en rust</ca>
<cs>software pro vzdálený přístup napsaný v rezu</cs>
<da>fjernadgangssoftware skrevet i rust</da>
<de>In Rost geschriebene Fernzugriffssoftware</de>
<el>λογισμικό απομακρυσμένης πρόσβασης γραμμένο σε σκουριά</el>
<es>software de acceso remoto escrito en óxido</es>
<et>kaugjuurdepääsu tarkvara, mis on kirjutatud roostes</et>
<eu>urruneko sarbide softwarea herdoilduta idatzita</eu>
<en>remote access software written in rust</en>
<fa>نرم افزار دسترسی از راه دور که با زنگ زدگی نوشته شده است</fa>
<fil_PH>remote access software na nakasulat sa kalawang</fil_PH>
<fi>ruosteella kirjoitettu etäkäyttöohjelmisto</fi>
<fr>logiciel d'accès à distance écrit en rust</fr>
<he_IL> תוכנת גישה מרחוק כתובה בחלודה</he_IL>
<hi>रिमोट एक्सेस सॉफ़्टवेयर जंग में लिखा हुआ है</hi>
<hr>softver za daljinski pristup napisan u rustu</hr>
<hu>távelérési szoftver rozsdával írva</hu>
<id>perangkat lunak akses jarak jauh yang ditulis dengan karat</id>
<is>fjaraðgangshugbúnaður skrifaður í ryð</is>
<it>software di accesso remoto scritto in ruggine</it>
<ja>Rustで書かれたリモートアクセスソフトウェア</ja>
<kk>тотпен жазылған қашықтан қол жеткізу бағдарламалық құралы</kk>
<ko>Rust로 작성된 원격 액세스 소프트웨어</ko>
<lt>nuotolinės prieigos programinė įranga, parašyta rūdžių spalva</lt>
<mk>софтвер за далечински пристап напишан во 'рѓа</mk>
<mr>गंज मध्ये लिहिलेले दूरस्थ प्रवेश सॉफ्टवेअर</mr>
<nb>fjerntilgangsprogramvare skrevet i rust</nb>
<nl>software voor externe toegang geschreven in roest</nl>
<pl>oprogramowanie do zdalnego dostępu napisane w rdzy</pl>
<pt_BR>software de acesso remoto escrito em ferrugem</pt_BR>
<pt>software de acesso remoto escrito em ferrugem</pt>
<ro>software de acces la distanță scris în rugină</ro>
<ru>программное обеспечение удаленного доступа, написанное на ржавчине</ru>
<sk>softvér pre vzdialený prístup napísaný v hrdze</sk>
<sl>programska oprema za oddaljeni dostop, napisana v rustu</sl>
<sq>softuer i aksesit në distancë i shkruar në ndryshk</sq>
<sr>софтвер за даљински приступ написан у рђи</sr>
<sv>fjärråtkomstprogramvara skriven i rost</sv>
<tr>pasla yazılmış uzaktan erişim yazılımı</tr>
<uk>програмне забезпечення віддаленого доступу, написане на rust</uk>
<vi>phần mềm truy cập từ xa được viết bằng rỉ sét</vi>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>

tempfile=$(mktemp);
version=$(wget https://github.com/rustdesk/rustdesk/releases -O $tempfile; awk -F'/download/' '{if (NF>1) {print $2; exit}}' $tempfile | cut -d'/' -f1);echo $version; rm $tempfile
wget https://github.com/rustdesk/rustdesk/releases/download/$version/rustdesk-$version-x86_64.deb -O /tmp/rustdesk-$version-x86_64.deb; apt-get install /tmp/rustdesk-$version-x86_64.deb; rm /tmp/rustdesk-$version-x86_64.deb
</preinstall>

<install_package_names>

</install_package_names>

<postinstall>

</postinstall>

<uninstall_package_names>
rustdesk
</uninstall_package_names>
</app>
