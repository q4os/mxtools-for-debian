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

archname="$name"".zip"
exec="zip -r ""\"$archname\"""$files"

overw="0"

if ( [ -d "$archname" ] || [ -f "$archname" ] ); then
echo "file/folder exists"
overw=$(overwrite_compress_dialog.sh "$ow")
fi
echo $overw

if [ "$overw" == "0" ];then
echo "$exec" > .comparchive.sh && chmod a=r+w+x .comparchive.sh
sh .comparchive.sh && rm .comparchive.sh
compress_dialog.sh
else
compress_cancel_dialog.sh
fi