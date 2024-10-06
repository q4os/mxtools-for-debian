<?xml version="1.0"?>
<app>

<category>
Kernels
</category>

<name>
Liquorix 6.10.11-2 64 bit
</name>

<description>
   Liquorix 6.10.11-2
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
apt-get install linux-image-6.10.11-2-liquorix-amd64 linux-headers-6.10.11-2-liquorix-amd64 ${DKMS_PKGS[*]}
</preinstall>

<install_package_names>

</install_package_names>


<postinstall>
rebuild_dkms_packages.sh linux-image-6.10.11-2-liquorix-amd64
if [ -f /etc/apt/sources.list.d/mxpitemp.list ]; then
rm /etc/apt/sources.list.d/mxpitemp.list
apt-get update
fi
echo "...$(gettext -d apt -s ' Done')!"
</postinstall>


<uninstall_package_names>
linux-image-6.10.11-2-liquorix-amd64
linux-headers-6.10.11-2-liquorix-amd64
</uninstall_package_names>
</app>
