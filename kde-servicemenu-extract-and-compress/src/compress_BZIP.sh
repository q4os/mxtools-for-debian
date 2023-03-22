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
archname="$name"".tar.bz2"
exec="tar -cf ""\"$archname\"""$files"" --bzip2 --overwrite"
else
name=$(basename "$1")
archname="$name"".bz2"
exec="bzip2 -z -f ""$files"
fi

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