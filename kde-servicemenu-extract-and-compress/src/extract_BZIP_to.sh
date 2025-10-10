#!/bin/bash

nameext=$(basename "$1")
content=${nameext%.*}
nameext2=$(extract_dir_dialog.sh "$getdir")
dir=$(dirname "$1") 

cd "$dir" 
overw="0"

if ( [ -d "$content" ] || [ -f "$content" ] ); then
echo "file/folder exists"
overw=$(overwrite_dialog.sh "$ow")
fi
echo $overw

cd "$dir" 

if [ "$overw" == "0" ];then
cp "$nameext" "$nameext2"
cd "$nameext2"
bunzip2 -f "$nameext"
extract_dialog.sh
else
extract_cancel_dialog.sh
fi