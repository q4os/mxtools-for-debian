#!/bin/sh

mkdir -p "/etc/calamares/modules/"

#--- configure squashfs file location for calamares
# sed -i 's:/run/live/medium/live/filesystem.squashfs:/live/boot-dev/antiX/linuxfs:' /etc/calamares/modules/unpackfs.conf
rm -f /etc/calamares/modules/unpackfs.conf
cat > "/etc/calamares/modules/unpackfs.conf" <<EOF
---
unpack:
    -   source: "/live/boot-dev/antiX/linuxfs"
        sourcefs: "squashfs"
        destination: ""
EOF

#--- configure packages removal, fix https://www.q4os.org/forum/viewtopic.php?pid=23193#p23193
rm -f /etc/calamares/modules/packages.conf
cat > "/etc/calamares/modules/packages.conf" <<EOF
backend: apt
update_db: false
update_system: false
operations:
  - try_remove:
      - live-boot
      - live-config
      - live-tools
      - "^live-boot-*"
      - "^live-config-*"
      - "^live-task-*"
EOF

#--- swap choices
rm -f /etc/calamares/modules/partition.conf
cat > "/etc/calamares/modules/partition.conf" <<EOF
userSwapChoices:
    - none      # Create no swap, use no swap
    - small     # Up to 4GB
    - suspend   # At least main memory size
    - file      # To swap file instead of partition
initialSwapChoice: file
EOF

#--- users
rm -f /etc/calamares/modules/users.conf
cat > "/etc/calamares/modules/users.conf" <<EOF
---
userGroup:       users
defaultGroups:
    - name: cdrom
      must_exist: false
      system: true
    - name: floppy
      must_exist: false
      system: true
    - name: sudo
      must_exist: false
      system: true
    - name: audio
      must_exist: false
      system: true
    - name: dip
      must_exist: false
      system: true
    - name: video
      must_exist: false
      system: true
    - name: plugdev
      must_exist: false
      system: true
    - name: netdev
      must_exist: false
      system: true
    - name: lpadmin
      must_exist: false
      system: true
    - name: scanner
      must_exist: false
      system: true
    - name: bluetooth
      must_exist: false
      system: true
autologinGroup:  autologin
sudoersGroup:    sudo
setRootPassword: true
doReusePassword: true

passwordRequirements:
    nonempty: true
    minLength: 1  # Password at least this many characters
    maxLength: -1  # Password at most this many characters
    #libpwquality:
        #- minlen=0
        #- minclass=0
EOF

#--- disable "sources-final" job from calamares configuration, as it duplicates debian repositories
rm -f /usr/share/calamares/helpers/calamares-sources-final
cat > "/usr/share/calamares/helpers/calamares-sources-final" <<EOF
#!/bin/sh
exit 0
EOF
chmod a+x /usr/share/calamares/helpers/calamares-sources-final

if [ -f "/var/lib/mxdebian/.mxsnapshot_accounts_reset.stp" ] ; then
  #remove live user "demo"
  FRSTUSER="$( dash /usr/share/apps/q4os_system/bin/get_first_user.sh --name )"
  rm -f /etc/calamares/modules/removeuser.conf
  cat > "/etc/calamares/modules/removeuser.conf" <<EOF
username: $FRSTUSER
EOF
  sed -i '/ - unpackfs/a\  - removeuser' /etc/calamares/settings.conf #add line after unpackfs
else
  #don't proceed a user+keyboard+locale+displaymanager
  sed -i '/ - users/d' /etc/calamares/settings.conf
  sed -i '/ - keyboard/d' /etc/calamares/settings.conf
  # sed -i '/ - locale/d' /etc/calamares/settings.conf
  sed -i '/ - displaymanager/d' /etc/calamares/settings.conf
fi
