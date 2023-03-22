
DESKTOP_LIST_FILE=/etc/default/desktop.list
DESKTOP_DEFAULT_FILE=/etc/default/desktop.default

#------------------------------------------------------------------------------
# Function: desktop_list [deskcode]
#
# Lists all data lines from the two files.  Ignores comments and empty lines.
# If a deskcode is given, only give the line that has the deskcode.
#------------------------------------------------------------------------------
desktop_list() {
    if [ "$1" ]; then
        desktop_list | grep "^[[:space:]]*$1 "
    else
        cat $DESKTOP_DEFAULT_FILE $DESKTOP_LIST_FILE 2>/dev/null \
            | grep -v "^[[:space:]]*#" | grep "[[:alpha:]]"
    fi
}

#------------------------------------------------------------------------------
# Function: desktop_abbrevs [sep] [rox-flag]
#
# Returns a list of cheatcodes for the installed window managers.  If <sep> is
# given then use <sep> as the separator (default " ").  If <rox-flag> is given
# then we include rox-$deskcode for wm's that support Rox. We use a little
# extra code to print the rox- versions first.
#------------------------------------------------------------------------------
desktop_abbrevs() {
    local sep=" "
    [ "$#" -ge "1" ] && sep="$1"
    local rox="$2"

    local not_first
    local abbrev cmnd remain
    desktop_list | while read abbrev cmnd remain; do
        [ "$cmnd" ] || continue
        which $cmnd &>/dev/null || continue
        if [ "$rox" -a "$remain" ]; then
            [ "$not_first" ] && echo -n "$sep"
            echo -n "rox-$abbrev"
            not_first=true
        fi
        [ "$not_first" ] && echo -n "$sep"
        echo -n "$abbrev"
        not_first=true
    done
}

#------------------------------------------------------------------------------
# Function: default_deskcode [deskcode]
#
# If deskcode is provided then echoes it and return true if it is installed
# and otherwise return false.  If no <deskcode> is given then echo the deskcode
# of the first installed wm from the lists and return true.  If none is found
# return false.
#------------------------------------------------------------------------------

#  default_deskcode() {
#      local deskcode=$(_default_deskcode "$@")
#      [ "$deskcode" ] || return 1
#      echo $deskcode
#      return 0
#  }
#  
#  # FIXME: see select_device_once_cli() for solution
#  # used by above to get around "while read" sub-shell limitations

default_deskcode() {
    local desktop
    [ "$1" = "-d" ] && desktop=true && shift
    local code=${1#rox-}
    while read abbrev cmnd remain; do
        [ "$cmnd" ]                         || continue
        [ "$code" -a "$code" != "$abbrev" ] && continue
        which $cmnd &>/dev/null             || continue
        if [ "$desktop" ]; then
            echo $cmnd
        else
            echo $abbrev
        fi
        return 0
    done <<End_of_Read_While
$(desktop_list)
End_of_Read_While

    return 1
}

is_valid_deskcode() {
    default_deskcode "$@" > /dev/null
    local retval=$?
    return $retval
}

#------------------------------------------------------------------------------
# Function: get_boot_param_deskcode
#
# Get desktop= boot parameter which can be a comma separated list of bootcodes.
# echo the first one that corresponds to an installed wm and return true.
# If there is no desktop= parameter or if none of the corresponding wm's are
# installed then return false.
#------------------------------------------------------------------------------
get_bootparam_deskcode() {
    local boot_param=$(get_boot_param desktop)
    [ "$boot_param" ] || return 1

    for code in $(echo "$boot_param" | tr "," " "); do
        default_deskcode $code && return 0
    done
    return 1
}

#------------------------------------------------------------------------------
# Function: get_default_deskcode [deskcode]
#
# Try as hard as possible to echo a valid deskcode.  If a <deskcode> parameter
# is given then try that first.  If that fails, try the desktop= boot parameter
# and if that fails, try every wm in the list until we find one that works.

#------------------------------------------------------------------------------
get_default_deskcode() {
    [ "$#" -gt 0 ] && default_deskcode $1 && return 0

    get_bootparam_deskcode && return 0
    default_deskcode       && return 0
    return 1
}

get_xinitrc_default() {
    # FIXME
    get_rox_default_deskcode "$@"
}

get_rox_default_deskcode() {
    local default=$(get_default_deskcode "$@")
    if has_rox $default &&  ! is_boot_param noRox; then
        default="rox-$default"
    fi
    echo $default
}

#------------------------------------------------------------------------------
# Function: has_rox deskcode
#
# Return true if deskcode is found and can use Rox.  Return false otherwise.
#------------------------------------------------------------------------------
has_rox() {
    [ "$1" ] || return 1
    desktop_list "$1" | grep -q "[[:alnum:]]\+.*[[:space:]]\+[[:alnum:]]\+.*[[:space:]]\+[[:alnum:]]"
    return $?
}

is_autogen_file() {
    file="$1"
    [ -f "$file" ] || return 0
    grep -q "^[[:space:]]*#[[:space:]]*AUTO-GENERATED" "$file" && return 0
    return 1
}

#------------------------------------------------------------------------------
# Function: make_xinitrc [options] [<default>]
#
# Print a .xinitrc file to stdout including entries for all the installed wm's.
#
# If <default> is given then it is used as the default entry.
#
# Options:
#    -a|--all   print an entry for every wm, not just for those installed.
#    -r|--rox   If <default> is given and it supports Rox then the Rox version
#               Is used as the default, otherwise the non-Rox version is used.
#------------------------------------------------------------------------------
make_xinitrc() {
    local all rox
    while [ "$#" -gt "0" ]; do
        case "$1" in
            -a|-all|--all)
                all=true
                shift
                ;;
            -r|-rox|--rox)
                rox=true
                shift;
                ;;
            *)
                break
                ;;
        esac
    done

    local deskcode="$1" && shift
    if expr match "$deskcode" "rox-" &>/dev/null; then
        rox=true
        deskcode=${deskcode#rox-}
    fi


    cat <<Xinitrc
#----------------------------------------------------------------------
# .xinitrc file
#
# AUTO-GENERATED by $(basename $0) on $(date)
#
# This file was auto-generated.  If you want to customize it you MUST
# DELETE the line above or your changes will be lost on the next boot.
#
# Alternatively, simply put your changes in .xinitrc-custom.
#----------------------------------------------------------------------

[ -x ~/.xinitrc-custom ] && ~/.xinitrc-custom

[ -f ~/.Xmodmap ] && xmodmap ~/.Xmodmap

case "\$1" in
Xinitrc

    desktop_list | while read abbrev cmnd start pinb; do
        [ "$cmnd" ] || continue
        [ "$all" ] || which $cmnd &>/dev/null || continue

        if [ "$start" ]; then
            make_rox    "Rox-$abbrev|rox-$abbrev" $cmnd $start $pinb $abbrev
            make_no_rox "$abbrev"                 $cmnd $start $pinb $abbrev
        else
            make_xinitrc_entry "$abbrev" $cmnd $abbrev
        fi

    done

    if [ "$deskcode" ]; then

        desktop_list $deskcode | while read abbrev cmnd start pinb; do
            if [ "$start" ]; then
                if [ "$rox" ]; then
                    echo -e "\n    # Default: rox-$abbrev"
                    make_rox    "*" $cmnd $start $pinb $abbrev
                else
                    echo -e "\n    # Default: $abbrev"
                    make_no_rox "*" $cmnd $start $pinb $abbrev
                fi
            else
                echo -e "\n    # Default: $abbrev"
                make_xinitrc_entry "*" $cmnd $start $pinb $abbrev
            fi
            break
        done

    fi
    echo "esac"
}

# Helper used in make_xinitrc
make_no_rox() {
    local abbrev="$1" cmnd=$2 start=$3 pinb=$4 root=$5

    cat << No_Rox
    $abbrev)
        export DESKTOP_CODE="$root"
        echo "$root" > \$HOME/.wallpaper/session
        sed -i -e "s/rox --pinboard=antiX-$pinb &/#rox --pinboard=antiX-$pinb &/" \$HOME/$start
        exec $cmnd
        ;;
No_Rox
}

# Helper used in make_xinitrc
# FIXME: delete the sed line below to restore this to the original.
make_rox() {
    local abbrev="$1" cmnd=$2 start=$3 pinb=$4 root=$5

    cat << With_Rox
    $abbrev)
        export DESKTOP_CODE="rox-$root"
        echo "rox-$root" > \$HOME/.wallpaper/session
        rox --pinboard=antX-$pinb
        exec $cmnd
        ;;
With_Rox
}

make_xinitrc_entry() {
    local abbrev="$1" cmnd=$2 root=$3

    cat << Xinitrc_Entry
    $abbrev)
        export DESKTOP_CODE="$root"
        exec $cmnd
        ;;
Xinitrc_Entry
}

#------------------------------------------------------------------------------
# Function: slim_sessions [--rox] [deskcode]
#
#
#------------------------------------------------------------------------------
slim_sessions() {
    local rox deskcode default list code
    case "$1" in
        -r|-rox|--rox)
            rox=true
            shift
            ;;
    esac
    deskcode="$1"
    if expr match "$deskcode" "rox-" &>/dev/null; then
        rox=true
        deskcode=${deskcode#rox-}
    fi

    default=$(get_default_deskcode $deskcode)
    # FIXME: add SEVERE error message here if still no default_desk_code

    [ "$rox" ] && has_rox $default && default="rox-$default"
    list=$default

    for code in $(desktop_abbrevs " " rox); do
        case ",$list," in
            *,$code,*)
                ;;
            *)
                list=$list,$code
                ;;
        esac
    done
    echo $list
}

slim_deskcode_default() {
    grep ^"sessions " /etc/slim.conf | sed "s/^sessions *//" | cut -d, -f1
}

select_deskcode() {
    local default=$(slim_deskcode_default)
    [ "$#" -ge 1 ] && default="$1"

    local deskcode choice
    for deskcode in $(desktop_abbrevs " " rox); do
        [ "$deskcode" = "$default" ] && continue
        choice="$choice!$deskcode"
    done
    choice="current: $default$choice"
    combo_box "Select Default" "$choice" \
        "$TITLE"                         \
        ""

    UI_RESULT=${UI_RESULT#current: }
}

