<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
Debian 64 bit
</name>

<description>
<am>የዴቢያን ነባሪ የከርነል ራስ-ዝማኔዎች ጥቅል</am>
<ar>ﻥﺎﻴﺒﻳﺪﻟ ﺔﻴﺿﺍﺮﺘﻓﻻ﻿ﺍ ﺓﺍﻮﻨﻠﻟ ﺔﻴﺋﺎﻘﻠﺘﻟﺍ ﺕﺎﺜﻳﺪﺤﺘﻟﺍ ﺔﻣﺰﺣ</ar>
<bg>Пакет за автоматични актуализации на ядрото по подразбиране на Debian</bg>
<bn>ডেবিয়ান ডিফল্ট কার্নেল স্বয়ংক্রিয় আপডেট প্যাকেজ</bn>
<ca>Paquet d'actualitzacions automàtiques del nucli predeterminat de Debian</ca>
<cs>Výchozí balíček automatických aktualizací jádra Debianu</cs>
<da>Debians standard kerne autoopdateringspakke</da>
<de>Debian-Standardpaket für automatische Kernel-Updates</de>
<el>Το προεπιλεγμένο πακέτο αυτόματων ενημερώσεων πυρήνα του Debian</el>
<es>Paquete de actualizaciones automáticas del kernel predeterminado de Debian</es>
<et>Debiani vaikimisi kerneli automaatsete värskenduste pakett</et>
<eu>Debian kernel lehenetsitako eguneratze automatikoen paketea</eu>
<en>Debian default kernel auto updates package</en>
<fa> ﻥﺎﯿﺑﺩ ﺽﺮﻓu200cﺶﯿﭘ ﻪﺘﺴﻫ ﺭﺎﮐﺩﻮﺧ ﯽﻧﺎﺳﺭﺯﻭﺭu200cﻪﺑ ﻪﺘﺴﺑ</fa>
<fil_PH>Debian default na kernel auto updates package</fil_PH>
<fi>Debianin oletusytimen automaattiset päivityspaketit</fi>
<fr>Paquet de mises à jour automatiques du noyau Debian par défaut</fr>
<he_IL>לדחמה תרירב Debian תביל לש םייטמוטואה םינוכדעה תליבח</he_IL>
<hi>डेबियन डिफ़ॉल्ट कर्नेल ऑटो अपडेट पैकेज</hi>
<hr>Debianov zadani paket za automatsko ažuriranje jezgre</hr>
<hu>Debian alapértelmezett kernel automatikus frissítési csomagja</hu>
<id>Paket pembaruan otomatis kernel default Debian</id>
<is>Debian sjálfgefinn kjarna sjálfvirkur uppfærslupakki</is>
<it>Pacchetto di aggiornamenti automatici del kernel Debian predefinito</it>
<ja>Debian のデフォルトのカーネル自動更新パッケージ</ja>
<kk>Debian ядросының әдепкі автоматты жаңарту пакеті</kk>
<ko>Debian 기본 커널 자동 업데이트 패키지</ko>
<lt>Debian'o numatytasis branduolio automatinių naujinimų paketas</lt>
<mk>Стандардниот пакет за автоматско ажурирање на кернелот на Debian</mk>
<mr>डेबियन डीफॉल्ट कर्नल ऑटो अपडेट्स पॅकेज</mr>
<nb>Debians standard kjerneoppdateringspakke</nb>
<nl>Debian standaardpakket voor automatische kernelupdates</nl>
<pl>Domyślny pakiet automatycznych aktualizacji jądra Debiana</pl>
<pt_BR>Pacote de atualizações automáticas do kernel padrão do Debian</pt_BR>
<pt>Pacote de atualizações automáticas do kernel padrão do Debian</pt>
<ro>Pachetul Debian de actualizări automate a nucleului implicit</ro>
<ru>Пакет автоматических обновлений ядра Debian по умолчанию</ru>
<sk>Predvolený balík automatických aktualizácií jadra Debianu</sk>
<sl>Privzeti paket samodejnih posodobitev jedra Debian</sl>
<sq>Paketa e përditësimeve automatike të kernelit të paracaktuar të Debian</sq>
<sr>Дебиан подразумевани пакет аутоматских ажурирања кернела</sr>
<sv>Debians standardpaket för automatisk uppdatering av kärnan</sv>
<tr>Debian varsayılan çekirdek otomatik güncelleme paketi</tr>
<uk>Пакет автоматичних оновлень ядра Debian за замовчуванням</uk>
<vi>Gói cập nhật tự động kernel mặc định của Debian</vi>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>

</preinstall>

<install_package_names>
linux-image-amd64
linux-headers-amd64
</install_package_names>


<postinstall>
file=$(apt-cache show linux-image-amd64=4.19* |grep -m1 Depends) 
file=${file#"Depends:"}
rebuild_dkms_packages.sh $file
</postinstall>


<uninstall_package_names>
linux-image-amd64
linux-headers-amd64
</uninstall_package_names>
</app>
