## Process this file with automake to produce Makefile.in

SUBDIRS = icons

install-data-local:

@INTLTOOL_DESKTOP_RULE@

desktopdir = $(datadir)/applications
desktop_in_files = disk-manager.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
CLEANFILES = $(desktop_DATA)
EXTRA_DIST = $(desktop_in_files)

pkgdata_DATA = disk-manager.glade \
               write-policy.fdi \
               ro-policy.fdi
EXTRA_DIST += $(pkgdata_DATA)

man_MANS = \
    disk-manager.8

EXTRA_DIST += $(man_MANS)

## Remove any fdi rule that could have been created by the program
uninstall-hook:
	rm -f $(DESTDIR)/etc/hal/fdi/policy/20-ntfs-config-write-policy.fdi || echo 0
	rm -f $(DESTDIR)/etc/hal/fdi/policy/20-ntfs-config-ro-policy.fdi || echo 0

