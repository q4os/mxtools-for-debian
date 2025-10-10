#!/bin/bash

CURRENTD=$(dirname "$0")  	# looking at the current location of this file
CURRENTDIR=$CURRENTD"/"		# add a slash at the end of the located path for upcoming operations

if [ "$CURRENTD" == "." ];then	# looking if it was navigated into the dir of this file
CURRENTDIR=""			# current directory changes fron "." to ""
fi

# local kde4-folder
LOCALPREFIX=`kde4-config --localprefix`

# Installation type
insttype=$(kdialog --title "ExtractAndCompress 1.4" --radiolist "Choose your installation type" 0 "local" on 1 "systemwide (requires root password)" off) 	# installation type dialog
echo $insttype			# messages which type was chosen

# local installation
if [ "$insttype" = "0" ]; then
echo "chosen installation type: local"

# looking if local bin-folder exists
  if [ -d "$HOME"/bin ]; then
  echo "directory" "$HOME"/bin "exists"
  else
  mkdir "$HOME"/bin	# create local bin-folder
  echo "created directory" "$HOME"/bin
  fi

# looking if local servicemenu-folder exists
  if [ -d "$LOCALPREFIX"/share/kde4/services/ServiceMenus ]; then
  echo "directory" "$LOCALPREFIX"/share/kde4/services/ServiceMenus "exists"
  else
  mkdir "$LOCALPREFIX"/share/kde4/services/ServiceMenus	# create local servicemenu-folder
  echo "created directory" "$LOCALPREFIX"/share/kde4/services/ServiceMenus
  fi

cd "$CURRENTDIR"src		# change to src-dir inside the installation folder

# copies *.desktop-files to local ServiceMenu dir
cp compress.desktop "$LOCALPREFIX"/share/kde4/services/ServiceMenus
cp compress_bgz.desktop "$LOCALPREFIX"/share/kde4/services/ServiceMenus
cp extract7ZIP.desktop "$LOCALPREFIX"/share/kde4/services/ServiceMenus
cp extractACE.desktop "$LOCALPREFIX"/share/kde4/services/ServiceMenus
cp extractBZIP.desktop "$LOCALPREFIX"/share/kde4/services/ServiceMenus
cp extractGZIP.desktop "$LOCALPREFIX"/share/kde4/services/ServiceMenus
cp extractRAR.desktop "$LOCALPREFIX"/share/kde4/services/ServiceMenus
cp extractTAR.desktop "$LOCALPREFIX"/share/kde4/services/ServiceMenus
cp extractTARGZ.desktop "$LOCALPREFIX"/share/kde4/services/ServiceMenus
cp extractZIP.desktop "$LOCALPREFIX"/share/kde4/services/ServiceMenus
# copies all scripts to local bin dir
cp compress_7ZIP.sh "$HOME"/bin
cp compress_BZIP.sh "$HOME"/bin
cp compress_GZIP.sh "$HOME"/bin
cp compress_RAR.sh "$HOME"/bin
cp compress_TAR7Z.sh "$HOME"/bin
cp compress_TARBZ2.sh "$HOME"/bin
cp compress_TARGZ.sh "$HOME"/bin
cp compress_TAR.sh "$HOME"/bin
cp compress_TGZ.sh "$HOME"/bin
cp compress_ZIP.sh "$HOME"/bin
cp extract_7ZIP_pw.sh "$HOME"/bin
cp extract_7ZIP_pw_to_folder.sh "$HOME"/bin
cp extract_7ZIP_pw_to.sh "$HOME"/bin
cp extract_7ZIP.sh "$HOME"/bin
cp extract_7ZIP_to_folder.sh "$HOME"/bin
cp extract_7ZIP_to.sh "$HOME"/bin
cp extract_ACE_pw.sh "$HOME"/bin
cp extract_ACE_pw_to_folder.sh "$HOME"/bin
cp extract_ACE_pw_to.sh "$HOME"/bin
cp extract_ACE.sh "$HOME"/bin
cp extract_ACE_to_folder.sh "$HOME"/bin
cp extract_ACE_to.sh "$HOME"/bin
cp extract_BZIP.sh "$HOME"/bin
cp extract_BZIP_to_folder.sh "$HOME"/bin
cp extract_BZIP_to.sh "$HOME"/bin
cp extract_GZIP.sh "$HOME"/bin
cp extract_GZIP_to_folder.sh "$HOME"/bin
cp extract_GZIP_to.sh "$HOME"/bin
cp extract_RAR_pw.sh "$HOME"/bin
cp extract_RAR_pw_to_folder.sh "$HOME"/bin
cp extract_RAR_pw_to.sh "$HOME"/bin
cp extract_RAR.sh "$HOME"/bin
cp extract_RAR_to_folder.sh "$HOME"/bin
cp extract_RAR_to.sh "$HOME"/bin
cp extract_TARGZ.sh "$HOME"/bin
cp extract_TARGZ_to_folder.sh "$HOME"/bin
cp extract_TARGZ_to.sh "$HOME"/bin
cp extract_TAR.sh "$HOME"/bin
cp extract_TAR_to_folder.sh "$HOME"/bin
cp extract_TAR_to.sh "$HOME"/bin
cp extract_ZIP.sh "$HOME"/bin
cp extract_ZIP_to_folder.sh "$HOME"/bin
cp extract_ZIP_to.sh "$HOME"/bin
cd dialogs			# change to dialogs-dir
# copies all dialog-scripts to local bin dir
cp archname_dialog.sh "$HOME"/bin
cp compress_dialog.sh "$HOME"/bin
cp compress_cancel_dialog.sh "$HOME"/bin
cp extract_dialog.sh "$HOME"/bin
cp extract_cancel_dialog.sh "$HOME"/bin
cp extract_dir_dialog.sh "$HOME"/bin
cp extract_pw_dialog.sh "$HOME"/bin
cp overwrite_dialog.sh "$HOME"/bin
cp overwrite_compress_dialog.sh "$HOME"/bin

# following lines will be removed in next release
rm "$LOCALPREFIX"/share/kde4/services/ServiceMenus/extract.desktop
rm "$LOCALPREFIX"/share/kde4/services/ServiceMenus/extract_pw.desktop
rm "$HOME"/bin/compress_7ZIP2.sh
rm "$HOME"/bin/extract.sh
rm "$HOME"/bin/extract_to_folder.sh
rm "$HOME"/bin/extract_to.sh
rm "$HOME"/bin/extract_pw.sh
rm "$HOME"/bin/extract_pw_to_folder.sh
rm "$HOME"/bin/extract_pw_to.sh

echo "Installation done!"	# message for successful installation
kdialog --title "ExtractAndCompress 1.4" --msgbox "Installation done!" 	# dialog for successful installation

# systemwide installation
elif [ "$insttype" == "1" ]; then
echo "chosen installation type: systemwide"

# creating temporary installation folder inside the home dir to prevent installation issues
mkdir "$HOME"/.eandatemp
cp -r "$CURRENTDIR"src "$HOME"/.eandatemp && cp "$CURRENTDIR"copyroot "$HOME"/.eandatemp
echo "root password required"
kdesu -d --noignorebutton "$HOME"/.eandatemp/copyroot
rm -r -f "$HOME"/.eandatemp

echo "Installation done!" 	# message for successful installation
kdialog --title "ExtractAndCompress 1.4" --msgbox "Installation done!" 	# dialog for successful installation

else
echo "Installation aborted!"	# error-message for aborted installation
kdialog --title "ExtractAndCompress 1.4" --error "Installation aborted!"  # error-dialog for aborted installation
exit

fi

