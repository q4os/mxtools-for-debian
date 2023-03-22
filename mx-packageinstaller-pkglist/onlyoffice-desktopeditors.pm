<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Office
</category>

<name>
OnlyOffice Desktop Editors
</name>

<description>
   <am>OnlyOffice Desktop Editors - a free software office suite</am>
   <ar>OnlyOffice Desktop Editors - a free software office suite</ar>
   <bg>OnlyOffice Desktop Editors - a free software office suite</bg>
   <bn>OnlyOffice Desktop Editors - a free software office suite</bn>
   <ca>OnlyOffice Desktop Editors - a free software office suite</ca>
   <cs>OnlyOffice Desktop Editors - a free software office suite</cs>
   <da>OnlyOffice Desktop Editors - a free software office suite</da>
   <de>OnlyOffice Desktop Editors - a free software office suite</de>
   <el>OnlyOffice Desktop Editors - a free software office suite</el>
   <en>OnlyOffice Desktop Editors - a free software office suite</en>
   <es>OnlyOffice Desktop Editors - a free software office suite</es>
   <et>OnlyOffice Desktop Editors - a free software office suite</et>
   <eu>OnlyOffice Desktop Editors - a free software office suite</eu>
   <fa>OnlyOffice Desktop Editors - a free software office suite</fa>
   <fil_PH>OnlyOffice Desktop Editors - a free software office suite</fil_PH>
   <fi>OnlyOffice Desktop Editors - a free software office suite</fi>
   <fr>OnlyOffice Desktop Editors - a free software office suite</fr>
   <he_IL>OnlyOffice Desktop Editors - a free software office suite</he_IL>
   <hi>OnlyOffice Desktop Editors - a free software office suite</hi>
   <hr>OnlyOffice Desktop Editors - a free software office suite</hr>
   <hu>OnlyOffice Desktop Editors - a free software office suite</hu>
   <id>OnlyOffice Desktop Editors - a free software office suite</id>
   <is>OnlyOffice Desktop Editors - a free software office suite</is>
   <it>OnlyOffice Desktop Editors - a free software office suite</it>
   <ja_JP>OnlyOffice Desktop Editors - a free software office suite</ja_JP>
   <ja>OnlyOffice Desktop Editors - a free software office suite</ja>
   <kk>OnlyOffice Desktop Editors - a free software office suite</kk>
   <ko>OnlyOffice Desktop Editors - a free software office suite</ko>
   <lt>OnlyOffice Desktop Editors - a free software office suite</lt>
   <mk>OnlyOffice Desktop Editors - a free software office suite</mk>
   <mr>OnlyOffice Desktop Editors - a free software office suite</mr>
   <nb>OnlyOffice Desktop Editors - a free software office suite</nb>
   <nl>OnlyOffice Desktop Editors - a free software office suite</nl>
   <pl>OnlyOffice Desktop Editors - a free software office suite</pl>
   <pt_BR>OnlyOffice Desktop Editors - a free software office suite</pt_BR>
   <pt>OnlyOffice Desktop Editors - a free software office suite</pt>
   <ro>OnlyOffice Desktop Editors - a free software office suite</ro>
   <ru>OnlyOffice Desktop Editors - a free software office suite</ru>
   <sk>OnlyOffice Desktop Editors - a free software office suite</sk>
   <sl>OnlyOffice Desktop Editors - a free software office suite</sl>
   <sq>OnlyOffice Desktop Editors - a free software office suite</sq>
   <sr>OnlyOffice Desktop Editors - a free software office suite</sr>
   <sv>OnlyOffice Desktop Editors - a free software office suite</sv>
   <tr>OnlyOffice Desktop Editors - a free software office suite</tr>
   <uk>OnlyOffice Desktop Editors - a free software office suite</uk>
   <vi>OnlyOffice Desktop Editors - a free software office suite</vi>
   <zh_CN>OnlyOffice Desktop Editors - a free software office suite</zh_CN>
   <zh_TW>OnlyOffice Desktop Editors - a free software office suite</zh_TW>
</description>

<installable>
64
</installable>

<screenshot></screenshot>

<preinstall>
echo " "
echo "Preparing installation of 'OnlyOffice Desktop Editors'..."
TMP=$(mktemp -d /tmp/mxpi-tmpdir-onlyoffice-desktop-editors.XXXXXXXXXXXXX)
chmod 700 $TMP
tidy_up() { rm -r /tmp/mxpi-tmpdir-onlyoffice-desktop-editors.* 2>/dev/null ; }
trap tidy_up EXIT
#echo "Fetching release signature..."
pushd $TMP >/dev/null
# OnlyOffice long key-id:
KEY="0x8320CA65CB2DE8E5"  # OnlyOffice signing long key-id

popd >/dev/null
echo "Fetching OnlyOffice release signing key 0x$KEY ..."
# fetch signing key
GHD="--homedir $TMP"
KBX=$TMP/onlyoffice-archive-keyring.kbx
RNG="--no-default-keyring  --keyring $KBX"
OPT="--import-options self-sigs-only,import-clean"
HKP="--keyserver hkp://keyserver.ubuntu.com:80"  
GPG=/etc/apt/trusted.gpg.d/onlyoffice-archive-keyring.gpg
if gpg -v $GHD $RNG $OPT $HKP --recv-keys $KEY; then
   echo "Add onlyoffice-archive-keyring.gpg ..."
   if [ -f $GPG ]; then
       rm $GPG 
   fi
   gpg -v $GHD $RNG --output $GPG --export $KEY
else
   echo "Error:****"
   echo "Fatal: Cannot fetch OnlyOffice release signing key 0x$KEY ..."
   if [ -f /etc/apt/sources.list.d/onlyoffice.list ]; then
      echo "Removing OnlyOffice apt-source-list..."
      rm /etc/apt/sources.list.d/onlyoffice.list
   fi 
   echo "Error: exit 1"
   exit 1 
fi

# add OnlyOffice source ĺist if not allready added
#
REXP="^[[:space:]]*deb[[:space:]]+https://download.onlyoffice.com/repo/debian[[:space:]]+squeeze[[:space:]]+main"
if ! grep -sq -E "$REXP" /etc/apt/sources.list{,.d/*.list}; then
   echo "Add OnlyOffice apt-source-list..."
   echo "deb https://download.onlyoffice.com/repo/debian squeeze main" > /etc/apt/sources.list.d/onlyoffice.list  
   echo /etc/apt/sources.list.d/onlyoffice.list :
   cat  /etc/apt/sources.list.d/onlyoffice.list
   echo " "
fi 
echo " "
echo "Refreshing package lists ..."
apt-get  -o=Dpkg::Use-Pty=0  update
echo " "
echo "Preparations finished ..."
echo "Installing onlyoffice-desktopeditors ..."
echo " "
</preinstall>

<install_package_names>
onlyoffice-desktopeditors
</install_package_names>

<postinstall>
echo "Done!"
</postinstall>

<uninstall_package_names>
onlyoffice-desktopeditors
</uninstall_package_names>

<postuninstall>
echo " "
echo "Purge onlyoffice-desktopeditors ..."
apt-get  -o=Dpkg::Use-Pty=0  --yes remove --purge  onlyoffice-desktopeditors
 
if [  -f /etc/apt/sources.list.d/onlyoffice.list ]; then 
   rm -f /etc/apt/sources.list.d/onlyoffice.list; 
fi
if [  -f /etc/apt/trusted.gpg.d/onlyoffice-archive-keyring.gpg ]; then 
   rm -f /etc/apt/trusted.gpg.d/onlyoffice-archive-keyring.gpg; 
fi
echo "Done!"
</postuninstall>

</app>
