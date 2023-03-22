
DDM_FILE=/etc/X11/default-display-manager

ddm_error() {
    local fmt="$(gettext "$1")" && shift
    DDM_ERROR=$(printf "$fmt" "$@")
}

get_default_display_manager() {
    unset DDM_SERVICE DDM_PROGRAM DDM_ERROR

    [ -r "$DDM_FILE" ] || return 1

    local prog=$(cat $DDM_FILE)
    if [ -z "$prog" ]; then
        ddm_error "The default display manager file %s was empty" "[f]$DDM_FILE[/]"
        return 2
    fi

    if ! [ -x "$prog" ]; then
        ddm_error "Could not find executable %s" "[f]$prog[/]"
        return 3
    fi

    if ! ps -o cmd -u root | grep -q ^$prog; then
        ddm_error "The default display manager, %s, is not running" "[f]$prog[/]"
        return 4
    fi

    local service=$(basename $prog)
    if ! [ -e "$INIT_DIR/$service" ]; then
        ddm_error "The default display manager service, %s, does not exist" "[f]$INIT_DIR/$service"
        return 5
    fi
    DDM_SERVICE=$service
    DDM_PROGRAM=$prog
    return 0
}

get_display_manager() {
    get_default_display_manager && return

    unset DDM_SERVICE DDM_PROGRAM
    local dm prog
    for dm in  $(grep -l DEFAULT_DISPLAY_MANAGER_FILE $INIT_DIR/*); do
        prog=$(grep ^DAEMON= $dm | head -n 1 | sed 's/^DAEMON=//')
        [ "$prog" ]    || continue
        [ -x "$prog" ] || continue
        ps -o cmd -u root | grep -q ^$prog || continue
        DDM_SERVICE=$(basename $dm)
        DDM_PROGRAM=$prog
        return 0
    done
    return 1
}

all_display_managers() {
    local dm prog
    for dm in  $(grep -l DEFAULT_DISPLAY_MANAGER_FILE $INIT_DIR/*); do
        prog=$(grep ^DAEMON= $dm | head -n 1 | sed 's/^DAEMON=//')
        [ "$prog" ] || continue
        echo "$(basename $dm):$prog"
    done
}

select_display_manager() {
    get_display_manager
    local ddm=$DDM_SERVICE
    local all_dms="$ddm:$DDM_PROGRAM $(all_display_managers | grep -v ^$ddm:)"
    local full choice
    for full in $all_dms; do
        choice="$choice$(echo $full | cut -d: -f1)!"
    done
    combo_box "$choice" \
        ""                                     \
        "Select a new default display manager" \
        ""                                     \
    UI_RESULT=$(echo "$all_dms" | grep ^$UI_RESULT: | cut -d: -f2)
}

set_default_display_manager(){
    prog="$1"
    echo "$prog" > $DDM_FILE
}

