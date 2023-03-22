## Process this file with automake to produce Makefile.in

SUBDIRS = Fstab

appdir = $(pyexecdir)/DiskManager
app_PYTHON = __init__.py \
			DiskManager.py \
			Dialogs.py \
			SimpleGladeApp.py \
			config.py \
			Config.py \
			Utility.py

CLEANFILES = config.py
EXTRA_DIST = config.py.in

all-local: config.py
