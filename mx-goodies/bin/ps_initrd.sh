#!/bin/sh
#
# initrd packing/unpacking tool
# petya@nigilist.ru
#
#

# <-- ps_functions.sh direct copy
#
#
echo_stderr() {
  if [ -n "$1" ]; then
    {
      echo $*
    } 1>&2
  else
    cat 1>&2
  fi
}

ps_eval() {
  local v="$*"
  eval echo \$$v
}

find_command() {
  local var_name="$1"
  shift
  local binary="$*"
  
  if [ "`type $binary > /dev/null 2>&1 ; echo $?`" = "0" ]; then
    export $var_name=$binary
    return
  else
    for path in /sbin /bin /usr/sbin /usr/bin /usr/local/sbin /usr/local/bin ; do
      if [ "`type $path/$binary > /dev/null 2>&1 ; echo $?`" = "0" ]; then
        export $var_name=$path/$binary
        return
      fi
    done
  fi
}

have_command() {
  local command="$1"
  local mandatory="$2"
  local e=""
  
  # simple command cache. might expire
  if [ -n "`ps_eval cmd_$command`" ]; then
    return 0
  fi
  
  local command_param=""
  if [ "$command" = "find" ] || [ "$command" = "depmod" ] ; then
    command_param=kot
  fi

  $command $command_param 0</dev/null 1>/dev/null 2>/dev/null ; e=$?
  
  if [ "$e" != "127" ]; then
    export cmd_$command=$command
    return 0
  else
    find_command cmd_$command $command
    if [ -n "`ps_eval cmd_$command`" ]; then
      return 0
    else
      if [ -n "$mandatory" ]; then
        echo_stderr "ERROR: binary [$command] is not available. Aborting..."
        exit 1
      else
        return 1
      fi
    fi
  fi
}

have_prereqs() {
  local prereqs="$*"
  local got_prereqs=y

  for p in $prereqs; do
    if ! have_command $p ; then
      got_prereqs=n
    fi
  done

  if [ "$got_prereqs" = "y" ]; then
    return 0
  else
    return 1
  fi
}

do_dirname() {
  local fullpath="$*"
  echo "$fullpath" | sed "s%[^/]*$%%"
}
#
#
# ps_functions.sh direct copy -->

debug() {
  local msg="$*"
  if [ -n "$DEBUG" ]; then
    echo "$msg"
  fi
}
warn() {
  local msg="$*"
  echo "WARNING: $msg"  
}

die() {
  local msg="$*"
  echo_stderr "$msg"
  exit 1
}

do_update_module_maps() {
  local initrd="$1"

  if [ ! -d "$initrd" ]; then
    die "do_update_module_maps: directory [$initrd] does not exist."
    exit 1
  fi

  local scan_modules_prereqs="find depmod sed"

  if have_prereqs $scan_modules_prereqs ; then
    for pcimap in `$cmd_find $initrd -name modules.pcimap` ; do
      local modules_dir=`do_dirname $pcimap | sed "s%/*$%%"`
      local kernel_version=`echo $modules_dir | sed "s%.*/%%"`
      local base_dir=`echo $modules_dir | $cmd_sed "s%/lib/modules/.*%%"`
      debug "found [modules.pcimap in $base_dir], running depmod -b $base_dir $kernel_version"
      $cmd_depmod -b $base_dir $kernel_version
    done
  else
    warn "do not all prereqs: $scan_modules_prereqs, modules.pcimap not updated"
  fi
}

do_open() {
  if [ -f "$INITRD" ]; then

    if [ -d "$TEMP_INITRD" ]; then
      die "$TEMP_INITRD already exists. run ps_initrd.sh close or cancel first (or remove invalid directory)"
    fi 

    echo "opening $INITRD"
    mkdir $TEMP_INITRD
    
    if [ ! -d $TEMP_INITRD ]; then
      echo "Can't create $TEMP_INITRD directory"
      exit 1
    fi
    
    gzip -cd $INITRD > $INITRD.unzipped ; e=$?
    if [ "$e" = "0" ]; then
      UNZIPPED=.unzipped
    else
      rm $INITRD.unzipped
      UNZIPPED=
    fi
    
    echo "Opening... Might take some time."

    # centos 3.8
    if [ "`file ${INITRD}${UNZIPPED} | grep ext2`" ]; then
      mount -o loop ${INITRD}${UNZIPPED} $TEMP_INITRD
    # centos 4.4
    elif [ "`file ${INITRD}${UNZIPPED} | grep cpio`" ]; then
      cat ${INITRD}${UNZIPPED} | (cd $TEMP_INITRD && cpio -i --make-directories)
    elif [ "`file ${INITRD}${UNZIPPED} | grep 'Linux Compressed ROM File System data'`" ]; then
      mkdir $TEMP_INITRD.tmp
      mount -o loop -t cramfs ${INITRD}${UNZIPPED} $TEMP_INITRD.tmp ; e=$?
      if [ "$e" != "0" ]; then
        echo "Unable to mount cramfs image. Is cramfs support enabled in kernel?"
        exit 1
      fi
      cd $TEMP_INITRD.tmp
      tar c -f - . | tar x -C $TEMP_INITRD -f -
      cd ..
      umount $TEMP_INITRD.tmp
      rm -r $TEMP_INITRD.tmp
    elif [ "`file ${INITRD}${UNZIPPED} | grep 'Squashfs'`" ]; then
      mkdir $TEMP_INITRD.tmp
      mount -o loop -t squashfs ${INITRD}${UNZIPPED} $TEMP_INITRD.tmp ; e=$?
      if [ "$e" != "0" ]; then
        echo "Unable to mount squashfs image. Is squashfs support enabled in kernel?"
        exit 1
      fi
      cd $TEMP_INITRD.tmp
      tar c -f - . | tar x -C ../$TEMP_INITRD -f -
      cd ..
      umount $TEMP_INITRD.tmp
      rm -r $TEMP_INITRD.tmp
    else
      echo "${INITRD}${UNZIPPED}: unknown image format `file ${INITRD}${UNZIPPED}`"
      exit 1
    fi
    
    if [ "`ls -l $TEMP_INITRD | wc | awk '{print $1}'`" = "1" ]; then
      echo "ERROR: empty $TEMP_INITRD. Aborting."
      exit 1
    fi
    
    if [ -f $TEMP_INITRD/modules/modules.cgz ]; then
      mkdir $TEMP_MODULES
      cp $TEMP_INITRD/modules/modules.cgz $TEMP_MODULES/modules.c.gz
      cd $TEMP_MODULES
      gzip -cd modules.c.gz | cpio -id
    fi
  else
    echo "$INITRD doesn't exist"
    exit 1
  fi
}

do_close() {
  if [ -f "$INITRD" ]; then
    echo "closing $INITRD"

    if [ -f $INITRD.unzipped ]; then
      UNZIPPED=.unzipped
    else
      UNZIPPED=
    fi

    if [ "`mount | grep $INITRD`" != "" ]; then
      # centos 3.8 way (loop-mounted initrd)
      if [ -d $TEMP_MODULES ]; then
        cd $TEMP_MODULES
        DIR=`ls -1 | grep -v modules.c.gz`
        echo "packing modules.cgz from $DIR"
        find $DIR -type f | cpio -H crc -o | gzip -9 > modules.cgz || exit $?
        cp -f modules.cgz $TEMP_INITRD/modules/modules.cgz || exit $?
        cd .. || exit $?
        rm -rf $TEMP_MODULES || exit $?
      fi
      
      umount $TEMP_INITRD
    else
      # centos4 / debian4 way (cpio extracted though different cpio formats)

      do_update_module_maps "$TEMP_INITRD"

      if [ "`file ${INITRD}${UNZIPPED} | grep cpio`" ]; then
        CPIO_TYPE=`file $INITRD.unzipped`
        if echo $CPIO_TYPE | grep -q "SVR4 with no CRC" ; then
          CPIO_OPT="-H newc"
        elif echo $CPIO_TYPE | grep -q "pre-SVR4 or odc" ; then
          CPIO_OPT="-H odc"
        else
          echo "Unknown CPIO format: $CPIO_TYPE! Aborting..."
          exit 1
        fi
      
        cd $TEMP_INITRD
        find . | cpio -ov $CPIO_OPT > ${INITRD}${UNZIPPED} || exit $?
        cd ..
      elif [ "`file ${INITRD}${UNZIPPED} | grep 'Linux Compressed ROM File System data'`" ]; then
        mkcramfs $TEMP_INITRD ${INITRD}${UNZIPPED} ; e=$?
        if [ "$e" != "0" ]; then
          echo "Error running `mkcramfs $TEMP_INITRD ${INITRD}${UNZIPPED}`. Do you have mkcramfs in PATH?"
          exit 1
        fi
      elif [ "`file ${INITRD}${UNZIPPED} | grep 'Squashfs'`" ]; then
        mksquashfs $TEMP_INITRD ${INITRD}${UNZIPPED} -noappend ; e=$?
        if [ "$e" != "0" ]; then
          echo "Error running `mksquashfs $TEMP_INITRD ${INITRD}${UNZIPPED}`. Do you have mksquashfs in PATH?"
          exit 1
        fi
      else
        echo "${INITRD}${UNZIPPED}: unknown image format. Did we manage to open it?"
        exit 1
      fi
    fi

    if [ "${UNZIPPED}" != "" ]; then
      gzip -c ${INITRD}${UNZIPPED} > $INITRD || exit $?
      rm ${INITRD}${UNZIPPED} || exit $?
    fi

    rm -rf $TEMP_INITRD || exit $?
  else
    echo "$INITRD doesn't exist"
    exit 1
  fi
}

do_cancel() {
  if [ -f "$INITRD" ]; then
    echo "canceling $INITRD"
    if [ "`mount | grep $INITRD`" != "" ]; then
      umount $TEMP_INITRD
    fi
    if [ -d $TEMP_MODULES ]; then
      rm -rf $TEMP_MODULES
    fi

    rm -rf $TEMP_INITRD
    rm $INITRD.unzipped
  else
    echo "$INITRD doesn't exist"
    exit 1
  fi
}

do_usage() {
  echo "Usage: `basename $0` INITRD open|close|cancel"
}

DEBUG=1
INITRD=$1
ACTION=$2

# trying to guess INITRD name given any part of unpacked initrd
BOO=`echo $INITRD | sed -re 's%-initrd$%%'`
[ -f $BOO ] && INITRD=$BOO
BOO=`echo $INITRD | sed -re 's%-modules$%%'`
[ -f $BOO ] && INITRD=$BOO
BOO=`echo $INITRD | sed -re 's%\.unzipped$%%'`
[ -f $BOO ] && INITRD=$BOO

if [ "`echo $INITRD | sed "s%^/%%"`" = "$INITRD" ]; then
  INITRD=`pwd`/$INITRD
fi

TEMP_INITRD=$INITRD-image
TEMP_MODULES=$INITRD-modules

if [ "$ACTION" = "open" ]; then
  do_open
elif [ "$ACTION" = "close" ]; then
  do_close
elif [ "$ACTION" = "cancel" ]; then
  do_cancel
else
  do_usage
fi
