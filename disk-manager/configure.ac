dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.57])
AC_INIT(disk-manager,[24.03.01],[https://github.com/MX-Linux/disk-manager/])
AM_INIT_AUTOMAKE()
AM_MAINTAINER_MODE

# su handler
AC_CHECK_PROGS([SUHANDLER], [su-to-root gksu kdesudo kdesu])

if test ${SUHANDLER} = "su-to-root"; then
    SUHANDLER="su-to-root -X -c"
fi

# Check path of some program
AC_PATH_PROGS([MODPROBE], [modprobe], /sbin/modprobe, [/sbin /usr/sbin $PATH])
AC_PATH_PROGS([MOUNT], [mount], /bin/mount, [/sbin /usr/sbin $PATH])
AC_PATH_PROGS([UMOUNT], [umount], /bin/umount, [/sbin /usr/sbin $PATH])
AC_PATH_PROGS([BLKID], [blkid], /sbin/blkid, [/sbin /usr/sbin $PATH])
AC_PATH_PROGS([DMSETUP], [dmsetup], /sbin/dmsetup, [/sbin /usr/sbin $PATH])

AM_PATH_PYTHON
AC_SUBST([PYOPTIONS], [-OOt])

# Internationalization
GETTEXT_PACKAGE=disk-manager
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], "$GETTEXT_PACKAGE", [Gettext package])
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])

AC_CONFIG_FILES([
        Makefile
        disk-manager
        po/Makefile.in
        DiskManager/Makefile
        DiskManager/config.py
        DiskManager/Fstab/Makefile
        DiskManager/Fstab/Fstabconfig.py
        data/Makefile
        data/disk-manager.desktop.in
        data/icons/Makefile
        data/icons/16x16/Makefile
        data/icons/22x22/Makefile
        data/icons/24x24/Makefile
        data/icons/scalable/Makefile
        ])
AC_OUTPUT

