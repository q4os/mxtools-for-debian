<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
Debian 32 bit no-pae single core
</name>

<description>
   <am>የዴቢያን ነባሪ ከርነል</am>
<ar>ﺔﻴﺿﺍﺮﺘﻓﻻ﻿ﺍ ﻥﺎﻴﺑﺩ ﺓﺍﻮﻧ</ar>
<bg>Ядрото по подразбиране на Debian</bg>
<bn>ডেবিয়ান ডিফল্ট কার্নেল</bn>
<ca>Nucli predeterminat de Debian</ca>
<cs>Výchozí jádro Debianu</cs>
<da>Debians standardkerne</da>
<de>Debian-Standardkernel</de>
<el>Προεπιλεγμένος πυρήνας του Debian</el>
<es>Núcleo predeterminado de Debian</es>
<et>Debiani vaiketuum</et>
<eu>Debian kernel lehenetsia</eu>
<en>Debian default kernel</en>
<fa>ﻥﺎﯿﺑﺩ ﺽﺮﻓ ﺶﯿﭘ ﻪﺘﺴﻫ</fa>
<fil_PH>Debian na default na kernel</fil_PH>
<fi>Debianin oletusydin</fi>
<fr>Noyau par défaut de Debian</fr>
<he_IL>ןאיבד לש לדחמה תרירב תביל</he_IL>
<hi>डेबियन डिफ़ॉल्ट कर्नेल</hi>
<hr>Zadana jezgra Debiana</hr>
<hu>Debian alapértelmezett kernel</hu>
<id>Kernel bawaan Debian</id>
<is>Debian sjálfgefinn kjarni</is>
<it>Kernel predefinito Debian</it>
<ja>Debian のデフォルトのカーネル</ja>
<kk>Debian әдепкі ядросы</kk>
<ko>데비안 기본 커널</ko>
<lt>Debian'o numatytasis branduolys</lt>
<mk>Стандардно јадро на Debian</mk>
<mr>डेबियन डीफॉल्ट कर्नल</mr>
<nb>Debians standard kjerne</nb>
<nl>Standaard kernel van Debian</nl>
<pl>Domyślne jądro Debiana</pl>
<pt_BR>Kernel padrão do Debian</pt_BR>
<pt>Kernel padrão do Debian</pt>
<ro>Nucleul implicit Debian</ro>
<ru>Ядро Debian по умолчанию</ru>
<sk>Predvolené jadro Debianu</sk>
<sl>Privzeto jedro Debian</sl>
<sq>Kerneli i paracaktuar i Debian</sq>
<sr>Дебиан подразумевано језгро</sr>
<sv>Debians standardkärna</sv>
<tr>Debian varsayılan çekirdeği</tr>
<uk>Стандартне ядро Debian</uk>
<vi>Hạt nhân mặc định của Debian</vi>
</description>

<installable>
32
</installable>

<screenshot>none</screenshot>

<preinstall>

</preinstall>

<install_package_names>
linux-image-686
linux-headers-686
</install_package_names>


<postinstall>
file=$(apt-cache show linux-image-686=4.19 |grep -m1 Depends) 
file=${file#"Depends:"}
rebuild_dkms_packages.sh $file
</postinstall>


<uninstall_package_names>
linux-image-686
linux-headers-686
</uninstall_package_names>
</app>
