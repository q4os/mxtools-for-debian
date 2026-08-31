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
mv "$(pwd)" $MX_BUILD_DIR
cd $MX_BUILD_DIR
find . -maxdepth 1 -type f -delete
mkdir -p .logs
mkdir -p .builtout

echo
read -p "Build all packages, press Enter to continue ..." XYZ
set -e
for SRCDIR1 in * ; do
  echo
  echo "Building in : $SRCDIR1/"
  cd $MX_BUILD_DIR/$SRCDIR1/
  dpkg-buildpackage -rfakeroot -uc -b 2>&1 | tee $MX_BUILD_DIR/.logs/${SRCDIR1}.log
done
set +e
echo "Saving packages ..."
mv $MX_BUILD_DIR/*.deb $MX_BUILD_DIR/.builtout/
echo "Creating lists ..."
mkdir -p $MX_BUILD_DIR/.lists/
ls -1 $MX_BUILD_DIR/.builtout/ | awk -F'_' '{ print $1" "$2 }' | sort > $MX_BUILD_DIR/.lists/list_pcks1.lst
ls -1 $MX_BUILD_DIR/.builtout/ | awk -F'_' '{ print $1 }' | sort > $MX_BUILD_DIR/.lists/list_pcks2.lst

echo
echo "Debian packages has been saved: \"$MX_BUILD_DIR/.builtout/\""
