#!/bin/grep ^#:

#-------------------------------------------------------------------------
# File: fancy-prompts.zsh
#
# Inspired by Phil!'s ZSH Prompt: http://aperiodic.net/phil/prompt/
# Control your fancy Zsh prompt from the command line.  Also available
# in a Bash version.
#
# by BitJam, Version 1.0-rc01, Thu Nov  1 11:52:14 MDT 2012
#-------------------------------------------------------------------------

#:------------------------------------------------------------------------
#:
#:  This script cannot be run as a program.  You must source it:
#:
#:     $ zsh
#:     $ source ./fancy-prompts.zsh
#:     $ prompt-usage
#:     $ prompt-fancy
#:
#: To install, source this file inside your .zshrc file and set the prompt.
#: If you already have a precmd() function defined then source this file
#: *before* precmd() and have precmd() include "fancy-prompts-precmd":
#:
#:    source /some/path/fancy-prompts.zsh
#:    precmd() {
#:        ...
#:        fancy-prompts-precmd
#:    }
#:    prompt-fancy -PDp "zsh> "
#:
#:------------------------------------------------------------------------

precmd() {
    fancy-prompts-precmd
}

alias prompt-tiny="_fancy-prompt tiny"
alias prompt-std="_fancy-prompt std"
alias prompt-color="_fancy-prompt color"
alias prompt-gentoo="_fancy-prompt gentoo"
alias prompt-dir="_fancy-prompt dir"
alias prompt-narrow="_fancy-prompt narrow"
alias prompt-med="_fancy-prompt med"
alias prompt-wide="_fancy-prompt wide"
alias prompt-fancy="_fancy-prompt fancy"
alias prompt-zee="_fancy-prompt zee"
alias prompt-date="_fancy-prompt date"
alias prompt-curl="_fancy-prompt curl"

prompt-usage() {
    cat <<End_Usage

There are twelve fancy prompt commands available:

    prompt-tiny        prompt-dir        prompt-fancy
    prompt-std         prompt-med        prompt-zee
    prompt-color       prompt-narrow     prompt-date
    prompt-gentoo      prompt-wide       prompt-curl

Each command changes your prompt ranging from prompt-tiny, which gives you
a very simple prompt, to prompt-zee which gives you a very fancy one.
They all have these options (but not all prompts are affected by all options):

    -a --ascii         Disable Unicode
    -b --bright        Use brighter colors
    -B --bold          Use bold lines
    -c --compact       Make the fancier prompts a little smaller
    -C --compact2      Make the fancier prompts smaller elsewhere
    -d --date=<fmt>    Set the date format
    -D --double        Use doubled lines
    -h --help          Display this help
    -l --lines=<int>   Add <int> extra new lines before the prompt
    -m --mute          Use muted/dimmer colors
    -n --nocolor       Turn off colors
    -p --prompt=<str>  The final character(s) displayed in the prompt
    -P --parens        Use parens instead of brackets in prompt
    -r --right=<int>   Leave <int> spaces on right side of wide prompts
    -t --time=<fmt>    Set the time format
    -T --title=<str>   Add a centered title above the prompt

Short options and numeric operands stack.  Non-numeric operands do not stack.
The following is valid:  -l2r3T "Title"

The default in a virtual console is --mute.  In a psuedo-terminal (inside of
X-windows) the default is --bright.

The username and the final prompt characters are red for the root user.

Config file: ~/.config/fancy-prompts-zsh.conf
End_Usage
}

fancy-prompts-precmd() {
    local width term_width=$COLUMNS
    unset _PROMPT_TITLE_FILL
    if [ -n "$_PROMPT_TITLE_LEN" ]; then
        width=$(( ($term_width - $_PROMPT_TITLE_LEN) /2))
        _PROMPT_TITLE_FILL=${(l.$width.. .)}
    fi

    unset _PROMPT_1_FILL _PROMPT_2_FILL _PROMPT_PWD
    if [ -n "$_PROMPT_1_LEN" ]; then

        _PROMPT_PWD=${(%):-%~}
        local pwd_len=${#_PROMPT_PWD}
        if [[ "$pwd_len + $_PROMPT_1_LEN" -gt $term_width ]]; then
            local pwd_max=$(( $term_width - $_PROMPT_1_LEN))
            _PROMPT_PWD=${(%):-%$pwd_max<...<%~%}
        else
            _PROMPT_1_FILL=$(_prompt-fill $(( $term_width - $_PROMPT_1_LEN - $pwd_len )))
        fi
    fi

    if [ -n "$_PROMPT_2_LEN" ]; then

        _PROMPT_PWD=${(%):-%~}
        local pwd_len=${#_PROMPT_PWD}
        if [[ "$pwd_len + $_PROMPT_2_LEN" -gt $term_width ]]; then
            local pwd_max=$(( $term_width - $_PROMPT_2_LEN))
            _PROMPT_PWD=${(%):-%$pwd_max<...<%~%}
        else
            width=$(( $term_width - $_PROMPT_2_LEN - $pwd_len ))
            _PROMPT_2_FILL=${(l.$width.. .)}
        fi
    fi

    unset _PROMPT_3_FILL
    if [ -n "$_PROMPT_3_LEN" ]; then
        _PROMPT_3_FILL=$(_prompt-fill $(( $term_width - $_PROMPT_3_LEN )))
    fi

    [ -n "$_PROMPT_TOP_LINE_CNT" ] || return
    for i in $(seq $_PROMPT_TOP_LINE_CNT); do
        echo
    done
}

_prompt-fill() {
    local width=$1
    case $_PROMPT_HBAR_TYPE in
        1) echo ${(l.$width..━.)};;
        2) echo ${(l.$width..═.)};;
        3) echo ${(l.$width..─.)};;
        4) echo ${(l.$width..=.)};;
        *) echo ${(l.$width..-.)};;
    esac
}

_prompt-err() {
    echo "Error:" "$@" >&2
    echo "Use -h or --help to see available options" >&2
    return 0
}

_fancy-prompt() {
    [ -z "$ZSH_VERSION" ] && _prompt-err "This does not look like a Zsh shell" && return 2

    setopt prompt_subst
    [ $# -lt 1 ] && _prompt-err "_fancy-prompt() requires >= one command line argument." && return 2

    local prompt_type="$1"
    shift;

    #-- Adjust default values before reading in command line args

    # Adjust line_cnt and return on unknown prompt_type
    case "$prompt_type" in
        tiny|std|color|zee|date)
            local line_cnt=0 ;;

        dir|med|narrow|gentoo|wide|fancy|curl)
            local line_cnt=1 ;;

       *)    _prompt-err "_fancy_prompt() Unknown prompt type: $prompt_type" && return 3;;
    esac

    # Automatically brighten display in psuedo-terms
    local bright
    case "$( tty )" in */pts/*) bright=true;; esac

    local arg ascii compact compact2 double nocolor title cmd_date_fmt
    local cmd_time_fmt cmd_prompt bold parens mpad margin=0

    #-- Process command line.  Args must start with a "-"
    while [ $# -gt 0 -a -z "${1##-*}" ]; do
        arg="${1#-}"; shift

        # Stack short options
        case "$arg" in
            [a-zA-Z][a-zA-Z0-9]*)
                if echo "$arg" | grep -q "^[abcdhlmnprtBCDPT0-9]\+$"; then
                    set -- $(echo $arg | sed 's/\([0-9]\+\)/ \1 /g' | sed 's/\([a-zA-Z]\)/ -\1 /g') "$@"
                    continue
                fi;;
        esac
        # Check for missing and suspicious operands
        case "$arg" in
            -date|-lines|-prompt|-right|-time|-title|[dlprtT])
                if [ $# -lt 1 ]; then
                    _prompt-err "Expected an operand after -$arg" && return 4

                elif [ -z "${1##-[a-zA-Z]*}" -o -z "${1##--[a-zA-Z]*}" ]; then
                    echo "Skipping -$arg $1 due to suspicious operand."
                    shift
                    continue;
                fi;;
        esac

        case "$arg" in

           -ascii|a)  ascii=true                        ;;
          -bright|b)  bright=true                       ;;
            -bold|B)  bold=true                         ;;
         -compact|c)  compact=true                      ;;
        -compact2|C)  compact2=true                     ;;
            -date|d)  cmd_date_fmt=$1        ; shift    ;;
            -date=*)  cmd_date_fmt=${arg#*=}            ;;
          -double|D)  double=true                       ;;
            -help|h)  prompt-usage           ; return 0 ;;
           -lines|l)  line_cnt=$1            ; shift    ;;
           -lines=*)  line_cnt=${arg#*=}                ;;
            -mute|m)  unset bright                      ;;
         -nocolor|n)  nocolor=true                      ;;
          -prompt|p)  cmd_prompt=$1          ; shift    ;;
          -prompt=*)  cmd_prompt=${arg#*=}              ;;
          -paren*|P)  parens=true                       ;;
           -right|r)  margin=$1              ; shift    ;;
           -right=*)  margin=${arg#*=}                  ;;
            -time|t)  cmd_time_fmt=$1        ; shift    ;;
            -time=*)  cmd_time_fmt=${arg#*=}            ;;
           -title|T)  title=$1               ; shift    ;;
           -title=*)  title=${arg#*=}                   ;;

                  *)  _prompt-err "prompt-$prompt_type: Unexpected arg: -$arg" && return 5 ;;
        esac
    done
    [ $# -gt 0 ] && _prompt-err "Extra command line args:" "$@" && return 6

    [ -n "${margin#[0-9]}" -a -n "${margin#[0-9][0-9]}" ] && \
        _prompt-err "Expected a reasonable --right (margin), not $margin" && return 7

    [ -n "${line_cnt##[0-9]}" -a -n "${line_cnt##[0-9][0-9]}" ] && \
        _prompt-err "Expected a reasonable --lines, not $line_cnt" && return 8

    local end_prompt="\$ "
    # Adjust end_prompt by type
    case "$prompt_type" in fancy|zee|date|curl)
        if [ -n "$ascii" ]; then
            end_prompt="> "
        else
            end_prompt="> "
        fi ;;
    esac

    end_prompt=${cmd_prompt:-$end_prompt}

    #-- clear out globals
    unset _PROMPT_TOP_LINE_CNT
    unset _PROMPT_TITLE_LEN _PROMPT_TITLE_FILL _PROMPT_1_LEN
    unset _PROMPT_1_FILL _PROMPT_2_LEN _PROMPT_2_FILL _PROMPT_3_LEN _PROMPT_3_FILL

    [ "$line_cnt" -gt 0 ] && _PROMPT_TOP_LINE_CNT=$line_cnt

    #-- Set up line style and colors
    if [ -n "$ascii" ]; then

        if [ -n "$double" ]; then
            _PROMPT_HBAR_TYPE=4
            local hbar="=" vbar="|" tl_corn="#" bl_corn="#" tr_corn="#" br_corn="#"

        else
            _PROMPT_HBAR_TYPE=5
            local hbar="-" vbar="|" tl_corn="+" bl_corn="+" tr_corn="+" br_corn="+"
        fi
    else
        if [ -n "$bold" ]; then
            _PROMPT_HBAR_TYPE=1
            local hbar="━" vbar="┃" tl_corn="┏" tr_corn="┓" bl_corn="┗" br_corn="┛"

        elif [ -n "$double" ]; then
            _PROMPT_HBAR_TYPE=2
            local hbar="═" vbar="║" tl_corn="╔" tr_corn="╗" bl_corn="╚" br_corn="╝"

        else
            _PROMPT_HBAR_TYPE=3
            local hbar="─" vbar="│" tl_corn="┌" tr_corn="┐" bl_corn="└" br_corn="┘"
        fi
     fi

    local e=$(printf "\e[")

    if [ -z "$nocolor" ]; then
        local   black="%{${e}0;30m%}"     blue="%{${e}0;34m%}"      green="%{${e}0;32m%}"
        local    cyan="%{${e}0;36m%}"      red="%{${e}0;31m%}"     purple="%{${e}0;35m%}"
        local   brown="%{${e}0;33m%}"  lt_gray="%{${e}0;37m%}"    dk_gray="%{${e}1;30m%}"
        local lt_blue="%{${e}1;34m%}" lt_green="%{${e}1;32m%}"    lt_cyan="%{${e}1;36m%}"
        local  lt_red="%{${e}1;31m%}"  magenta="%{${e}1;35m%}"     yellow="%{${e}1;33m%}"
        local   white="%{${e}1;37m%}"  rev_red="%{${e}0;7;31m%}"

        # no color
        local nc="%{${e}0m%}"
    else
        local black blue green cyan red purple brown lt_gray dk_gray lt_blue lt_green
        local lt_cyan lt_red magenta yellow white rev_red nc
    fi

    if [ -n "$bright" ]; then
        local name_co=$lt_green prom_co=$lt_green line_co=$lt_cyan  date_co=$yellow
        local time_co=$yellow   host_co=$lt_blue  path_co=$magenta title_co=$lt_red at_co=$magenta
    else
        local name_co=$green    prom_co=$green    line_co=$cyan    date_co=$yellow
        local time_co=$brown    host_co=$lt_blue  path_co=$purple title_co=$red     at_co=$purple
    fi

    # *** sigh ***
    local new_line="
"
    # Override some colors for root user
    [ "$UID" -eq 0 ] && name_co=$rev_red
    [ "$UID" -eq 0 ] && prom_co=$red

    local time_fmt="%I:%M:%S %P"
    local date_fmt="%a, %B %d"

    # These look better if the final prompt is the same color as the lines
    case "$prompt_type" in
        fancy|zee|date|curl) prom_co=$line_co ;;
    esac

    #-- Allow user to change colors and date/time formats
    local conf_file=$HOME/.config/fancy-prompts-zsh.conf
    if [ -r "$conf_file" ]; then
        if zsh -n $conf_file; then
            source $conf_file
        else
            echo "Config file ignored due to errors. Command prompt-$prompt_type continuing." >&2
        fi
    fi

    # Priority: cmdline, config file, defaults
    time_fmt=${cmd_time_fmt:-$time_fmt}
    date_fmt=${cmd_date_fmt:-$date_fmt}

    [ "$margin" -gt 0 ] && mpad=${(l.$margin..-.)}

    # A centered title above the prompt
    if [ -n "$title" ]; then
        _PROMPT_TITLE_LEN=$((${#title} + $margin + 4 ))
        title=\${(e)_PROMPT_TITLE_FILL}$title_co$title$new_line
    fi

    [ -z "$compact" ] && local pad=$hbar
    [ -n "$compact" ] && local pad=

    [ -z "$compact2" ] && local pad2=$hbar
    [ -n "$compact2" ] && local pad2=

    [ -n "$parens" ] && local lb="(" rb=")"
    [ -z "$parens" ] && local lb="[" rb="]"

    # abbrevity is the soul of wit
    local fill_1="\${(e)_PROMPT_1_FILL}"
    local fill_2="\${(e)_PROMPT_2_FILL}"
    local fill_3="\${(e)_PROMPT_3_FILL}"
    local p_pwd="\${(e)_PROMPT_PWD}"

    local host=%m
    local name=%n

    #-- Now start putting all the pieces together

    # Basic reusable elements of the prompts
    local name_block="$line_co$lb$name_co$name$nc$at_co@$host_co$host$line_co$rb"
    local time_block="$line_co$lb$time_co%D{$time_fmt}$line_co$rb"
    local date_block="$line_co$lb$date_co%D{$date_fmt}$line_co$rb"
    local path_block="$line_co$lb$path_co$p_pwd$line_co$rb"

    # p2_bare is similar to p2 but has colors stripped out
    local p2="$name_block$pad2$time_block"
    local p2_bare="(%n@%m)$pad2(%D{$time_fmt})"
    local date_bare="(%D{$date_fmt})"

    #-- Final assembly
    local p3
    case "$prompt_type" in

      tiny) p3="" ;;

       std) p3="$nc$name@$host" ;;

     color) p3="$name_block" ;;

       dir) p3="$path_block$new_line"
           _PROMPT_1_LEN=$((2 + $margin)) ;;

       med) p3="$p2$pad$path_block$new_line"
           _PROMPT_1_LEN=${#${(%):-$p2_bare$pad()$mpad}} ;;

    narrow) p3="$p2$new_line$path_block$new_line"
            _PROMPT_1_LEN=$((2 + $margin));;

    gentoo) p3="$red%n $magenta%D{$time_fmt} $green$p_pwd$new_line"
            _PROMPT_1_LEN=${#${(%):-%n=%D{$time_fmt}=$mpad}} ;;

      wide) p3="$p2$pad$fill_1$path_block$new_line"
            _PROMPT_1_LEN=${#${(%):-$p2_bare$pad()$mpad}} ;;

     fancy) p3="$line_co$tl_corn$pad$p2$pad$fill_1$path_block$new_line"
            p3="$p3$line_co$bl_corn$pad"
            _PROMPT_1_LEN=${#${(%):-x$pad$p2_bare$pad()$mpad}} ;;

       zee) p3="$fill_2$path_block$pad$tr_corn$new_line"
            p3="$p3$line_co$tl_corn$pad$p2$fill_3$br_corn$new_line"
            p3="$p3$line_co$bl_corn$pad"
            _PROMPT_2_LEN=${#${(%):-()$pad-$mpad}}
            _PROMPT_3_LEN=${#${(%):-x$pad$p2_bare-$mpad}} ;;

      date) p3="$fill_2$path_block$pad$tr_corn$new_line"
            p3="$p3$line_co$tl_corn$pad$p2$fill_3$pad2$date_block$pad$br_corn$new_line"
            p3="$p3$line_co$bl_corn$pad"
            _PROMPT_2_LEN=${#${(%):-()$pad-$mpad}}
            _PROMPT_3_LEN=${#${(%):-x$pad$p2_bare$pad2$date_bare$pad-$mpad}} ;;

      curl) p3="$line_co$tl_corn$pad$p2$fill_3$pad2$date_block$pad$tr_corn$new_line"
            p3="$p3$vbar$fill_2$path_block$pad$br_corn$new_line"
            p3="$p3$line_co$bl_corn$pad"
            _PROMPT_2_LEN=${#${(%):-|()$pad-$mpad}}
            _PROMPT_3_LEN=${#${(%):-x$pad$p2_bare$pad2$date_bare$pad-$mpad}} ;;

    esac

    PROMPT="$title$p3$prom_co$end_prompt$nc"
}
