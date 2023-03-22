#!/bin/bash

nameext=$(basename "$1")
content=${nameext%.*}
dir=$(dirname "$1") 

cd "$dir"
overw="0"

if ( [ -d "$content" ] || [ -f "$content" ] ); then
echo "file/folder exists"
overw=$(overwrite_dialog.sh "$ow")
fi
echo $overw

if [ "$overw" == "0" ];then
bunzip2 -f "$nameext"
extract_dialog.sh
else
extract_cancel_dialog.sh
fi