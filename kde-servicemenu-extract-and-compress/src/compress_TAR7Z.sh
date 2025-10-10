#!/bin/bash

dir=$(dirname "$1")

cd "$dir"
files=""
y="0"

for i in "$@";do
file=$(basename "$i")
files="$files"" \"""$file""\""
y=$((y+1))
done

if [ $y -gt 1 ];then
name=$(archname_dialog.sh "$getname")
  if [ "$name" == "" ];then
  compress_cancel_dialog.sh
  exit
  fi
else
name=$(basename "$1")
fi

archname="$name"".tar"
archname2="$archname"".7z"
exec="tar -cf ""\"$archname\"""$files"" --overwrite"

overw="0"

if ( [ -d "$archname2" ] || [ -f "$archname2" ] ); then
echo "file/folder exists"
overw=$(overwrite_compress_dialog.sh "$ow")
fi
echo $overw

if [ "$overw" == "0" ];then
echo "$exec" > .comparchive.sh && chmod a=r+w+x .comparchive.sh
sh .comparchive.sh && rm .comparchive.sh
7z a -aoa "$archname2" "$archname"
rm "$archname"
compress_dialog.sh
else
compress_cancel_dialog.sh
fi