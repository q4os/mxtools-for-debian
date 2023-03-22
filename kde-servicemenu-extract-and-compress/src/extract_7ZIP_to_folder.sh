#!/bin/bash

nameext=$(basename "$1")
name=${nameext%.*}
name2=${name%.*}
dir=$(dirname "$1") 

cd "$dir" 
overw="0"

if ( [ -d "$name2" ] || [ -f "$name2" ] ); then
echo "file/folder exists"
overw=$(overwrite_dialog.sh "$ow")
fi
echo $overw

if [ "$overw" == "0" ];then
mkdir "$name2"
7z x -aoa -y "$nameext" -o"$name2"
extract_dialog.sh
else
extract_cancel_dialog.sh
fi