<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Remote Access
</category>

<name>
AnyDesk
</name>

<description>
   <am>AnyDesk remote access</am>
   <ar>AnyDesk remote access</ar>
   <bg>AnyDesk remote access</bg>
   <bn>AnyDesk remote access</bn>
   <ca>Accés remot AnyDesk</ca>
   <cs>AnyDesk remote access</cs>
   <da>AnyDesk remote access</da>
   <de>AnyDesk Fernzugriff</de>
   <el>Απομακρυσμένη πρόσβαση με το AnyDesk</el>
   <en>AnyDesk remote access</en>
   <es>Acceso remoto a cualquier escritorio.</es>
   <et>AnyDesk remote access</et>
   <eu>AnyDesk remote access</eu>
   <fa>AnyDesk remote access</fa>
   <fil_PH>AnyDesk remote access</fil_PH>
   <fi>AnyDesk-etäyhteys</fi>
   <fr>AnyDesk accès à distance</fr>
   <he_IL>AnyDesk remote access</he_IL>
   <hi>AnyDesk दूरस्थ अभिगम</hi>
   <hr>AnyDesk remote access</hr>
   <hu>AnyDesk remote access</hu>
   <id>AnyDesk remote access</id>
   <is>AnyDesk remote access</is>
   <it>AnyDesk accesso remoto</it>
   <ja>AnyDesk リモートアクセス</ja>
   <kk>AnyDesk remote access</kk>
   <ko>AnyDesk remote access</ko>
   <lt>AnyDesk remote access</lt>
   <mk>AnyDesk remote access</mk>
   <mr>AnyDesk remote access</mr>
   <nb>AnyDesk fjerntilgang</nb>
   <nl>AnyDesk afstand toegang</nl>
   <pl>dostęp zdalny AnyDesk</pl>
   <pt_BR>AnyDesk - Aplicativo de Acesso Remoto</pt_BR>
   <pt>Aplicação de acesso remoto</pt>
   <ro>AnyDesk remote access</ro>
   <ru>Удалённый доступ AnyDesk</ru>
   <sk>Vzdialený prýstup AnyDesk</sk>
   <sl>AnyDesk oddaljeni dostop</sl>
   <sq>Hyrje e largët me AnyDesk</sq>
   <sr>AnyDesk remote access</sr>
   <sv>AnyDesk fjärråtkomst </sv>
   <tr>AnyDesk uzaktan erişim</tr>
   <uk>AnyDesk remote access</uk>
   <vi>AnyDesk remote access</vi>
   <zh_CN>AnyDesk remote access</zh_CN>
   <zh_TW>AnyDesk remote access</zh_TW>
</description>

<installable>
64
</installable>

<screenshot>none</screenshot>

<preinstall>

<![CDATA[
# remove existing repo and keyring for anydesk
rm -f /etc/apt/keyrings/*anydesk*               2>/dev/null
rm -f /etc/apt/trusted.gpg.d/*anydesk*          2>/dev/null
rm -f /etc/apt/sources.list.d/*anydesk*.list    2>/dev/null
rm -f /etc/apt/sources.list.d/*anydesk*.sources 2>/dev/null
apt-get update
apt-get install extrepo --yes
# add "contrib" and "non-free" in extrepo's config
[ -f /etc/extrepo/config.yaml ] && sed  -i -r -e '$a- contrib' -e '$a- non-free' -e '/- (contrib|non-free)/d;'  /etc/extrepo/config.yaml

extrepo enable anydesk
# Fixing the LC_TYPE error in apt for certain locales such as tr_TR.UTF-8
for SRC in $(grep -sHi ^uris /etc/apt/sources.list.d/*.sources | grep -v ':URIs' | sed 's/:URIs.*//I' | sort -u); do
env LC_CTYPE=C.UTF-8 sed -i 's/^uris/URIs/I' "$SRC"
done
# removing not needed architectures
case "$(dpkg --print-architecture)" in
  amd64)
  [ -f /etc/apt/sources.list.d/extrepo_anydesk.sources ] && sed -i '/Architectures:/cArchitectures: amd64' /etc/apt/sources.list.d/extrepo_anydesk.sources
  ;;
esac
apt-get update
]]>

</preinstall>

<install_package_names>
anydesk
</install_package_names>

<postinstall>
<![CDATA[
echo "--------------------------------"
echo "...$(gettext -d apt -s ' Done')!"
echo "--------------------------------"
]]>
</postinstall>

<uninstall_package_names>
anydesk
</uninstall_package_names>

<postuninstall>
<![CDATA[
# remove any existing repo and keyring for anydesk
rm -f /etc/apt/keyrings/*anydesk*               2>/dev/null
rm -f /etc/apt/trusted.gpg.d/*anydesk*          2>/dev/null
rm -f /etc/apt/sources.list.d/*anydesk*.list    2>/dev/null
rm -f /etc/apt/sources.list.d/*anydesk*.sources 2>/dev/null

apt-get update
echo "--------------------------------"
echo "...$(gettext -d apt -s ' Done')!"
echo "--------------------------------"
]]>
</postuninstall>

</app>
