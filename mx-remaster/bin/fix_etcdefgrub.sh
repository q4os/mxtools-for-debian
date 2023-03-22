#!/bin/sh

#this script removes crypt config sections, for ex.: "cryptdevice=..." from /etc/default/grub file

# --------------------
# function: remove_section
# --------------------
remove_section ()
{
  local LINE_ARG="$1"
  local DEL_STR="$2"
  local LN1="$( echo "$LINE_ARG " | awk -F"$DEL_STR=" '{ print $1}' | awk '{$1=$1};1' )" #first part of the string
  local LN2="$( echo "$LINE_ARG " | awk -F"$DEL_STR=" '{ print $2}' | cut -d' ' -f2- | awk '{$1=$1};1' )" #second part of the string
  echo "$LN1 $LN2"
}

# --------------------
# script start
# --------------------
FILE1="$1"
echo "[I] file: $FILE1 - starting.."
if [ ! -f "$FILE1" ] ; then
  echo "No input file given, exiting ..."
  exit
fi
echo "[I] before: $( cat $FILE1 | grep '^GRUB_CMDLINE_LINUX_DEFAULT=' )"

LN_WK="$( cat $FILE1 | grep '^GRUB_CMDLINE_LINUX_DEFAULT=' | head -n1 | cut -d'=' -f2- | sed -e 's/^"//' -e 's/"$//' )"
if [ -z "$LN_WK" ] ; then
  echo "No action needed, exiting ..."
  exit
fi

LN_WK="$(remove_section "$LN_WK" "cryptdevice")"
LN_WK="$(remove_section "$LN_WK" "root")"
LN_WK="$(remove_section "$LN_WK" "resume")"

LN_OUT="GRUB_CMDLINE_LINUX_DEFAULT=\"$( echo "$LN_WK" | awk '{$1=$1};1' )\"" #awk '{$1=$1};1' .. remove leading and trailing spaces

sed -i "/^GRUB_ENABLE_CRYPTODISK/d" $FILE1
sed -i "s@^GRUB_CMDLINE_LINUX_DEFAULT=.*@$LN_OUT@" $FILE1

echo "[I]  after: $( cat $FILE1 | grep '^GRUB_CMDLINE_LINUX_DEFAULT=' )"
echo "[I] file: $FILE1 - completed."
