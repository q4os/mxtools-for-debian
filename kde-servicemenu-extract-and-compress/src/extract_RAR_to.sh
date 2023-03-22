#!/bin/bash

nameext=$(basename "$1")
name=${nameext%.*}
name2=${name%.*}
nameext2=$(extract_dir_dialog.sh "$getdir")
dir=$(dirname "$1") 

cd "$nameext2" 
overw="0"

if ( [ -d "$name2" ] || [ -f "$name2" ] ); then
echo "file/folder exists"
overw=$(overwrite_dialog.sh "$ow")
fi
echo $overw

if [ "$overw" == "0" ];then
mkdir "$name2"
cd "$dir"
unrar x -o+ "$nameext" "$nameext2"/"$name2"
extract_dialog.sh
else
extract_cancel_dialog.sh
fi