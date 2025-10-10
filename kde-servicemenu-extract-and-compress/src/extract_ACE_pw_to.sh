#!/bin/bash

nameext=$(basename "$1")
name=${nameext%.*}
name2=${name%.*}
nameext2=$(extract_dir_dialog.sh "$getdir")
password=$(extract_pw_dialog.sh "$getpw")
dir=$(dirname "$1") 

cd "$nameext2" 
overw="0"

if ( [ -d "$name2" ] || [ -f "$name2" ] ); then
echo "file/folder exists"
overw=$(overwrite_dialog.sh "$ow")
fi
echo $overw

cd "$dir"

if [ "$overw" == "0" ];then
mkdir "$nameext2"/"$name2"
cp "$nameext" "$nameext2"/"$name2"
cd "$nameext2"/"$name2"
unace x -o -y -p"$password" "$nameext"
rm "$nameext"
extract_dialog.sh
else
extract_cancel_dialog.sh
fi