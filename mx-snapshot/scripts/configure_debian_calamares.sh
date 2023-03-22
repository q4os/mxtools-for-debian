#!/bin/sh

mkdir -p "/etc/calamares/modules/"

#configure squashfs file location for calamares
# sed -i 's:/run/live/medium/live/filesystem.squashfs:/live/boot-dev/antiX/linuxfs:' /etc/calamares/modules/unpackfs.conf
rm -f /etc/calamares/modules/unpackfs.conf
cat > "/etc/calamares/modules/unpackfs.conf" <<EOF
---
unpack:
    -   source: "/live/boot-dev/antiX/linuxfs"
        sourcefs: "squashfs"
        destination: ""
EOF

#configure packages removal, fix https://www.q4os.org/forum/viewtopic.php?pid=23193#p23193
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

#disable "sources-final" job from calamares configuration, as it duplicates debian repositories
# /bin/echo -e '#!/bin/sh\nexit 0' > /usr/sbin/sources-final
rm -f /usr/sbin/sources-final
cat > "/usr/sbin/sources-final" <<EOF
#!/bin/sh
exit 0
EOF
chmod a+x /usr/sbin/sources-final
