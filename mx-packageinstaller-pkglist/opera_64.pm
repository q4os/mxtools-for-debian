<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Browser
</category>

<name>
Opera
</name>

<description>
   <am>Opera browser</am>
   <ar>Opera browser</ar>
   <bg>Opera browser</bg>
   <bn>Opera browser</bn>
   <ca>Navegador Opera</ca>
   <cs>Opera browser</cs>
   <da>Opera-browser</da>
   <de>Opera-Browser</de>
   <el>Πρόγραμμα περιήγησης Opera</el>
   <en>Opera browser</en>
   <es>Navegador Opera</es>
   <et>Opera browser</et>
   <eu>Opera browser</eu>
   <fa>Opera browser</fa>
   <fil_PH>Opera browser</fil_PH>
   <fi>Opera-nettiselain</fi>
   <fr>Navigateur Opera</fr>
   <he_IL>Opera browser</he_IL>
   <hi>ओपेरा ब्राउज़र</hi>
   <hr>Opera browser</hr>
   <hu>Opera browser</hu>
   <id>Opera browser</id>
   <is>Opera browser</is>
   <it>Opera browser</it>
   <ja>Opera ブラウザ</ja>
   <kk>Opera browser</kk>
   <ko>Opera browser</ko>
   <lt>Opera naršyklė</lt>
   <mk>Opera browser</mk>
   <mr>Opera browser</mr>
   <nb>Nettleseren Opera</nb>
   <nl>Opera browser</nl>
   <pl>przeglądarka Opera</pl>
   <pt_BR>Opera - Navegador de internet </pt_BR>
   <pt>Navegador web Opera</pt>
   <ro>Opera browser</ro>
   <ru>Браузер Opera</ru>
   <sk>Opera browser</sk>
   <sl>Opera brskalnik</sl>
   <sq>Shfletuesi Opera</sq>
   <sr>Opera browser</sr>
   <sv>Opera webbläsare</sv>
   <tr>Opera tarayıcı</tr>
   <uk>Браузер Opera</uk>
   <vi>Opera browser</vi>
   <zh_CN>Opera browser</zh_CN>
   <zh_TW>Opera browser</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>http://www-static.operacdn.com/static-heap/e3/e3be37900045a27590062000a1e380e005644b8a/linux.png</screenshot>

<preinstall>

# get opera archive keying
KEYASC=/tmp/opera-archive.key
KEYURL=https://deb.opera.com/archive.key
DEBSRC="deb https://deb.opera.com/opera-stable/ stable non-free"
GPGCMD="gpg --no-default-keyring --keyring /dev/null"
rm $KEYASC 2>/dev/null
echo "Get Opera Archive Keyring"
curl '-#RLJ' -o $KEYASC $KEYURL
if [ ! -f $KEYASC ]; then
echo "[ERROR]: Failed to fetch Opera archive keyring '$KEYURL'"
echo "Exit"
exit 1
fi

SHOWKEY=$($GPGCMD --show-keys ${KEYASC})
if (($? ==0 )); then
echo  "$SHOWKEY" | sed 's/^uid\s*/uid   /'
cp $KEYASC /etc/apt/trusted.gpg.d/opera-archive.asc 2>/dev/null >/dev/null
else
echo "[ERROR]: Invalid Opera archive keyring"
echo "Exit"
exit 2
fi

# remove not needed lib_extra
LIB_EXTRA=/usr/lib/x86_64-linux-gnu/opera/lib_extra
if [ -d $LIB_EXTRA ]; then
rm $LIB_EXTRA/* 2>/dev/null
rmdir $LIB_EXTRA
fi

# disable existing Opera sources list entries
sed -i -r '/opera.com/d' /etc/apt/sources.list /etc/apt/sources.list.d/*.list 2>/dev/null
rm /etc/apt/sources.list.d/opera*.list 2>/dev/null
# temporary enable opera's default list  in /etc/apt/sources.list.d/mxpitmp.list
rm -f  /etc/apt/sources.list.d/mxpitmp.list 2>/dev/null
echo "$DEBSRC" > /etc/apt/sources.list.d/mxpitmp.list

# create empty /etc/apt/sources.list
if [ ! -f /etc/apt/sources.list ]; then
echo "## empty list " > /etc/apt/sources.list
fi
apt-get update

</preinstall>

<install_package_names>
opera-stable
</install_package_names>

<postinstall>
rm -f  /etc/apt/sources.list.d/mxpitmp.list 2>/dev/null
apt-get update
</postinstall>

<uninstall_package_names>
opera-stable
</uninstall_package_names>

<postuninstall>
case "$(dpkg-query -f '${db:Status-Abbrev}' -W opera-stable 2>/dev/null)" in
i*)
:
;;
*)
APTRNG=/etc/apt/trusted.gpg.d/opera-archive-keyring.gpg
OPERAKEYS=($(apt-key adv --keyid-format 0xlong --list-keys opera.com 2>/dev/null | grep -oE '[[:xdigit:]]{40}'))
if ((${#OPERAKEYS[*]} != 0 )); then
apt-key del ${OPERAKEYS[*]} 2>/dev/null >/dev/null
fi
rm $APTRNG 2>/dev/null
rm ${APTRNG}~ 2>/dev/null
rm /etc/apt/sources.list.d/opera*.list  2>/dev/null
rm /etc/cron.daily/opera-browser  2>/dev/null
echo " "
apt-get -y purge opera-stable
echo " "
apt-get update
;;
esac

</postuninstall>

</app>
