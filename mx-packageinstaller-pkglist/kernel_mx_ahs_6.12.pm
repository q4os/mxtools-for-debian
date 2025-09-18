<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Kernels
</category>

<name>
Debian 6.12.35+1 64 bit (AHS)
</name>

<description>
<am>ዴቢያን 6.12.35+1፣ 64 ቢት የቅርብ ጊዜ ከMX repo</am>
<ar>Debian 6.12.35+1 ، ﻦﻣ ﺙﺪﺣﻷ﻿ﺍ ﺖﺑ 64 ﺭﺍﺪﺻﻹ﻿ﺍ MX repo</ar>
<bg>Debian 6.12.35+1, 64-битов най-нов от MX repo</bg>
<bn>ডেবিয়ান 6.12.35+1, MX রেপো থেকে 64 বিট সর্বশেষ</bn>
<ca>Debian 6.12.35+1, 64 bits més recent de MX repo</ca>
<cs>Debian 6.12.35+1, 64 bit nejnovější z MX repo</cs>
<da>Debian 6.12.35+1, 64 bit seneste fra MX repo</da>
<de>Debian 6.12.35+1, 64 Bit, neueste Version aus dem MX-Repo</de>
<el>Debian 6.12.35+1, 64 bit πιο πρόσφατο από το repo MX</el>
<es>Debian 6.12.35+1, 64 bit más reciente del repositorio MX</es>
<et>Debian 6.12.35+1, 64-bitine uusim versioon MX repost</et>
<eu>Debian 6.12.35+1, 64 biteko azkena MX repo-tik</eu>
<en>Debian 6.12.35+1, 64 bit latest from MX repo</en>
<fa>Debian 6.12.35+1، 64 ﺯﺍ ﻦﯾﺮﺗﺪﯾﺪﺟ ﺖﯿﺑ MX repo</fa>
<fil_PH>Debian 6.12.35+1, 64 bit pinakabago mula sa MX repo</fil_PH>
<fi>Debian 6.12.35+1, 64-bittinen uusin MX reposta</fi>
<fr>Debian 6.12.35+1, dernière version 64 bits du référentiel MX</fr>
<he_IL>MX repo -מ ןורחאה תויביס 64 ,6.12.35+1 ןאיבד</he_IL>
<hi>डेबियन 6.12.35+1, एमएक्स रेपो से 64 बिट नवीनतम</hi>
<hr>Debian 6.12.35+1, 64-bitni najnoviji iz MX repoa</hr>
<hu>Debian 6.12.35+1, 64 bit legfrissebb az MX repóból</hu>
<id>Debian 6.12.35+1, 64 bit terbaru dari repo MX</id>
<is>Debian 6.12.35+1, 64 bita nýjasta frá MX repo</is>
<it>Debian 6.12.35+1, 64 bit più recente dal repository MX</it>
<ja>Debian 6.12.35+1、MX リポジトリからの最新の 64 ビット</ja>
<kk>Debian 6.12.35+1, MX репосынан ең соңғы 64 бит</kk>
<ko>Debian 6.12.35+1, MX 저장소의 최신 64비트</ko>
<lt>Debian 6.12.35+1, 64 bitų naujausia versija iš MX repo</lt>
<mk>Debian 6.12.35+1, најново 64 битно од MX repo</mk>
<mr>डेबियन 6.12.35+1, MX रेपो वरून 64 बिट नवीनतम</mr>
<nb>Debian 6.12.35+1, 64 bit nyeste fra MX repo</nb>
<nl>Debian 6.12.35+1, 64 bit laatste van MX repo</nl>
<pl>Debian 6.12.35+1, 64-bitowy najnowszy z repozytorium MX</pl>
<pt_BR>Debian 6.12.35+1, 64 bits mais recente do repositório MX</pt_BR>
<pt>Debian 6.12.35+1, 64 bits mais recente do repositório MX</pt>
<ro>Debian 6.12.35+1, cel mai recent 64 de biți de la MX repo</ro>
<ru>Debian 6.12.35+1, последняя 64-разрядная версия из репозитория MX</ru>
<sk>Debian 6.12.35+1, 64-bitová najnovšia verzia MX repo</sk>
<sl>Debian 6.12.35+1, 64-bitni najnovejši iz repoja MX</sl>
<sq>Debian 6.12.35+1, 64 bit më i fundit nga repo MX</sq>
<sr>Дебиан 6.12.35+1, 64-битни најновији из МКС репо</sr>
<sv>Debian 6.12.35+1, 64-bitars senaste från MX-repo</sv>
<tr>Debian 6.12.35+1, MX deposundan en son 64 bit</tr>
<uk>Debian 6.12.35+1, остання 64-розрядна версія з репозиторію MX</uk>
<vi>Debian 6.12.35+1, 64 bit mới nhất từ MX repo</vi>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>
if [ $(apt-get update --print-uris | grep -c -m1 -E "/mx/repo/dists/bookworm/ahs/") = 0 ]; then
MXREPO=$(apt-get update --print-uris | grep -oE "https?://.*/mx/repo/dists/bookworm/main" | tail -1 | sed "s:^:deb :; s:/repo/dists/:/repo/ :; s:/main: ahs:")
: ${MXREPO:=deb http://mxrepo.com/mx/repo/ bookworm ahs}
echo "$MXREPO" > /etc/apt/sources.list.d/mxpitemp.list
apt-get update
fi
DKMS_PKGS=($(dpkg-query -f '${db:Status-Abbrev}\t${Package}\n' -W  -- '*-dkms' | grep ^i | grep -- '-dkms$' | cut -d$'\t' -f2))
apt-get install linux-image-6.12.35+1-amd64-unsigned linux-headers-6.12.35+1-amd64 ${DKMS_PKGS[*]}
</preinstall>

<install_package_names>

</install_package_names>


<postinstall>
rebuild_dkms_packages.sh linux-image-6.12.35+1-amd64-unsigned
if [ -f /etc/apt/sources.list.d/mxpitemp.list ]; then
rm /etc/apt/sources.list.d/mxpitemp.list
apt-get update
fi
echo "...$(gettext -d apt -s ' Done')!"
</postinstall>


<uninstall_package_names>
linux-image-6.12.35+1-amd64-unsigned
linux-headers-6.12.35+1-amd64
</uninstall_package_names>
</app>
