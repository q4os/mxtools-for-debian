#!/bin/sh

# This script builds MX tools for Debian
#
# Prerequisities: build dependencies preinstalled
#
# Change dir into this source directory and run this script:
# $ sh build_all.sh
#
# Find generated .deb packages in the build directory

MX_BUILD_DIR="/tmp/.build-mxtools-for-debian"

echo
read -p "Prepare for build, press Enter to continue ..." XYZ
rm -rf $MX_BUILD_DIR
rsync -a --exclude '.git/' . $MX_BUILD_DIR/
find $MX_BUILD_DIR/ -maxdepth 1 -type f -delete

echo
read -p "Build all packages, press Enter to continue ..." XYZ
cd $MX_BUILD_DIR/
mkdir -p $MX_BUILD_DIR/.logs/
set -e
for SRCDIR1 in * ; do
  echo
  echo "Building in : $SRCDIR1/"
  cd $MX_BUILD_DIR/$SRCDIR1/
  dpkg-buildpackage -rfakeroot -uc -b 2>&1 | tee $MX_BUILD_DIR/.logs/${SRCDIR1}.log
done
set +e
echo "Saving packages ..."
mkdir -p $MX_BUILD_DIR/.builtout/
mv $MX_BUILD_DIR/*.deb $MX_BUILD_DIR/.builtout/
echo "Creating lists ..."
mkdir -p $MX_BUILD_DIR/.lists/
ls -1 $MX_BUILD_DIR/.builtout/ | awk -F'_' '{ print $1" "$2 }' | sort > $MX_BUILD_DIR/.lists/list_pcks1.lst
ls -1 $MX_BUILD_DIR/.builtout/ | awk -F'_' '{ print $1 }' | sort > $MX_BUILD_DIR/.lists/list_pcks2.lst

echo
echo "Debian packages has been saved: \"$MX_BUILD_DIR/.builtout/\""
