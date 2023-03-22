#!/bin/bash

CURRENTD=$(dirname "$0")  	# looking at the current location of this file
CURRENTDIR=$CURRENTD"/"		# add a slash at the end of the located path for upcoming operations

if [ "$CURRENTD" == "." ];then	# looking if it was navigated into the dir of this file
CURRENTDIR=""			# current directory changes fron "." to ""
fi

# local kde4-folder
LOCALPREFIX=`kde4-config --localprefix`

# Installation type
insttype=$(kdialog --title "ExtractAndCompress 1.4" --radiolist "Where is your installation located?" 0 "local" on 1 "systemwide (requires root password)" off) 	# installation type dialog
echo $insttype			# messages which type was chosen

# local installation
if [ "$insttype" = "0" ]; then
echo "chosen installation type: local"

cd "$LOCALPREFIX"/share/kde4/services/ServiceMenus		# change to servicemenu-dir

# remove *.desktop-files from local ServiceMenu dir
rm -f compress.desktop
rm -f compress_bgz.desktop
rm -f extract7ZIP.desktop
rm -f extractACE.desktop
rm -f extractBZIP.desktop
rm -f extractGZIP.desktop
rm -f extractRAR.desktop
rm -f extractTAR.desktop
rm -f extractTARGZ.desktop
rm -f extractZIP.desktop
cd "$HOME"/bin
rm -f compress_7ZIP.sh
rm -f compress_BZIP.sh
rm -f compress_GZIP.sh
rm -f compress_RAR.sh
rm -f compress_TAR7Z.sh
rm -f compress_TARBZ2.sh
rm -f compress_TARGZ.sh
rm -f compress_TAR.sh
rm -f compress_TGZ.sh
rm -f compress_ZIP.sh
rm -f extract_7ZIP_pw.sh
rm -f extract_7ZIP_pw_to_folder.sh
rm -f extract_7ZIP_pw_to.sh
rm -f extract_7ZIP.sh
rm -f extract_7ZIP_to_folder.sh
rm -f extract_7ZIP_to.sh
rm -f extract_ACE_pw.sh
rm -f extract_ACE_pw_to_folder.sh
rm -f extract_ACE_pw_to.sh
rm -f extract_ACE.sh
rm -f extract_ACE_to_folder.sh
rm -f extract_ACE_to.sh
rm -f extract_BZIP.sh
rm -f extract_BZIP_to_folder.sh
rm -f extract_BZIP_to.sh
rm -f extract_GZIP.sh
rm -f extract_GZIP_to_folder.sh
rm -f extract_GZIP_to.sh
rm -f extract_RAR_pw.sh
rm -f extract_RAR_pw_to_folder.sh
rm -f extract_RAR_pw_to.sh
rm -f extract_RAR.sh
rm -f extract_RAR_to_folder.sh
rm -f extract_RAR_to.sh
rm -f extract_TARGZ.sh
rm -f extract_TARGZ_to_folder.sh
rm -f extract_TARGZ_to.sh
rm -f extract_TAR.sh
rm -f extract_TAR_to_folder.sh
rm -f extract_TAR_to.sh
rm -f extract_ZIP.sh
rm -f extract_ZIP_to_folder.sh
rm -f extract_ZIP_to.sh
rm -f archname_dialog.sh
rm -f compress_dialog.sh
rm -f compress_cancel_dialog.sh
rm -f extract_dialog.sh
rm -f extract_cancel_dialog.sh
rm -f extract_dir_dialog.sh
rm -f extract_pw_dialog.sh
rm -f overwrite_dialog.sh
rm -f overwrite_compress_dialog.sh

echo "Uninstall done!"	# message for successful uninstallation
kdialog --title "ExtractAndCompress 1.4" --msgbox "ExtractAndCompress was successfully removed!" 	# dialog for successful uninstallation

# systemwide installation
elif [ "$insttype" == "1" ]; then
echo "chosen installation type: systemwide"

kdesu -d --noignorebutton "$CURRENTDIR"removeroot

echo "Uninstall done!"	# message for successful uninstallation
kdialog --title "ExtractAndCompress 1.4" --msgbox "ExtractAndCompress was successfully removed!" 	# dialog for successful uninstallation

else
echo "Uninstall aborted!"	# error-message for aborted uninstallation
kdialog --title "ExtractAndCompress 1.4" --error "Uninstall aborted!"  # error-dialog for aborted uninstallation
exit

fi

