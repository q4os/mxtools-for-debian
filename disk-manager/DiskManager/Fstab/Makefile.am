## Process this file with automake to produce Makefile.in

appdir = $(pyexecdir)/DiskManager/Fstab
app_PYTHON = __init__.py \
			DiskInfo.py \
			EventHandler.py \
			Fstab.py \
			Fstabconfig.py \
			FstabData.py \
			FstabDialogs.py \
			FstabError.py \
			FstabHandler.py \
			FstabUtility.py \
			Mounter.py \
			SimpleGladeApp.py \
			ToolsBackend.py

CLEANFILES = Fstabconfig.py
EXTRA_DIST = Fstabconfig.py.in

all-local: Fstabconfig.py
