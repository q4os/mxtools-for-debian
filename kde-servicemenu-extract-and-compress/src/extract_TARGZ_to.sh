#!/bin/bash

nameext=$(basename "$1")
nameext2=$(extract_dir_dialog.sh "$getdir")
dir=$(dirname "$1") 

cd "$nameext2"

tar -t -f "$dir"/"$nameext" > ".""$nameext""_files"
rowcount=$(cat ".""$nameext""_files" | wc -l)

i="1"
hits="0"

while ( [ $i -lt $((rowcount+1)) ] && [ $hits -lt 1 ] );do 

content=$(sed -n "$i{p;q}" ".""$nameext""_files")
cont=${content%*/*}
  if ( [ -d "$cont" ] || [ -f "$cont" ] ); then
  echo "file/folder exists"
  hits=$((hits+1))
  fi

i=$((i+1))
done

rm ".""$nameext""_files"
overw="0"

if [ $hits -gt 0 ]; then
echo "file/folder exists"
overw=$(overwrite_dialog.sh "$ow")
fi
echo $overw

cd "$dir" 

if [ "$overw" == "0" ];then
tar -xzf "$nameext" -C "$nameext2" --overwrite
extract_dialog.sh
else
extract_cancel_dialog.sh
fi