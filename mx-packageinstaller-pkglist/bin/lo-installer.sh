#!/bin/bash

# libreoffice installer
# to install a set of default libreoffice packages
# from default debian repo or up-/downgrade to/from debian backports
#  vers 2019-10-16.2
#  vers 2020-02-10.1
#  vers 2021-12-01 -  reworked for bullseye... - fehlix

VERSION="2021-12-01"

usage () {

cat <<USAGE

lo-installer.sh - LibreOffice installer

Helper script to install LibreOffice default set of writer, calc, draw,
math, impress and base modules from either Debian's main or backports repository.
Additional libreoffice packages like language or help packages can be added.

Usage:
    lo-installer.sh [-b | -m | -r  | -s | -d  | -h  | -v ]  [ libreoffice-packages ... ]

Options:
    -b, --backports    install or upgrade to Debian's backports provided version
    -m, --main         install or downgrade to Debian's main provided version
    -r, --reinstall    reinstall libroffice packages
    -s, --simulate     do not install, run simulation
    -d, --debug        print lot's of additonal info
    -h, --help         show help info
    -v, --version      print version

USAGE
  exit 0
}

if (($(id -u )==0)); then
    SUDO=""
else
    SUDO="sudo"
    sudo -k
    sudo -v
fi

main() {

    DEBIAN_CODENAME=$(debian_codename) || exit 1
    INSTALL_TARGET=""
    SIMULATE=
    MYDEBUG=
    REINSTALL=

    local CLP=()
    local a b c
    for a in "$@" ""; do
        c=$b
        case $b in
            -h|--help)  usage
                        ;;
            -b|--backports)
                        INSTALL_TARGET=backports
                        c=
                        ;;
            -m|--main)  INSTALL_TARGET=main
                        c=
                        ;;
            -r|--reinstall)
                        REINSTALL="--reinstall"
                        c=
                        ;;
            -s|--simulate)
                        SIMULATE="-s"
                        c=
                        ;;
            -d|--debug) MYDEBUG=true
                        c=
                        ;;
            -v|--version)
                        echo VERSION=$VERSION
                        exit 0
                        ;;
            -*)         echo "[WARN] unknown parameter ignored: $p";
                        ;;
        esac
        b=$a
        [[ $c ]] && CLP+=("$c")
    done

    LO_ADD_PKGS=()
    LO_ADD_HM_PKGS=()
    LO_HELPER=()
    for p in "${CLP[@]}"; do
        [[ $p =~ libreoffice-*       ]] && LO_ADD_PKGS+=( $p )
        [[ $p == lo-main-helper      ]] && LO_HELPER+=( $p )
        [[ $p == lo-backports-helper ]] && LO_HELPER+=( $p )
        [[ $p =~ hyphen-*            ]] && LO_ADD_HM_PKGS+=( $p )
        [[ $p =~ mythes-*            ]] && LO_ADD_HM_PKGS+=( $p )
        if [[ $p =~ libreoffice-l10n-*  ]]; then
           lp="${p#libreoffice-l10n-}"
           [ "$lp" = "pt" ] && lp="pt-pt"
           LO_ADD_HM_PKGS+=( hyphen-$lp mythes-$lp )
         fi
    done

    # default list of libreoffice installed packages
    #
    LO_INSTALL_PACKAGES_ALL=(
        libreoffice-base
        libreoffice-base-core
        libreoffice-base-drivers
        libreoffice-calc
        libreoffice-common
        libreoffice-core
        libreoffice-draw
        libreoffice-gnome
        libreoffice-gtk3
        libreoffice-help-common
        libreoffice-help-en-us
        libreoffice-impress
        libreoffice-java-common
        libreoffice-math
        libreoffice-report-builder-bin
        libreoffice-sdbc-hsqldb
        libreoffice-style-breeze
        libreoffice-style-colibre
        libreoffice-style-sifr
        libreoffice-writer
        libreoffice-avmedia-backend-gstreamer
        libjuh-java
        libjurt-java
        liblibreoffice-java
        libridl-java
        libuno-cppu3
        libuno-cppuhelpergcc3-3
        libuno-purpenvhelpergcc3-3
        libuno-sal3
        libuno-salhelpergcc3-3
        libunoil-java
        libunoloader-java
        uno-libs-private
        ure
        fonts-opensymbol
        python3-uno
        )

    LO_INSTALL_PACKAGES_MANUAL=(
        libreoffice-base
        libreoffice-base-core
        libreoffice-calc
        libreoffice-common
        libreoffice-core
        libreoffice-draw
        libreoffice-help-en-us
        libreoffice-impress
        libreoffice-java-common
        libreoffice-math
        libreoffice-report-builder-bin
        libreoffice-sdbc-hsqldb
        libreoffice-style-sifr
        libreoffice-writer
        ure
        fonts-opensymbol
        libreoffice-avmedia-backend-gstreamer
        python3-uno
        )

    LO_INSTALL_PACKAGES_MANUAL_KDE=(
        libreoffice-kde5
        libreoffice-kf5
        )

    LO_INSTALL_PACKAGES_MANUAL_GNOME=(
        libreoffice-gnome
        libreoffice-gtk3
        )

    if pidof -q /usr/bin/plasmashell; then
        LO_INSTALL_PACKAGES_MANUAL+=( ${LO_INSTALL_PACKAGES_MANUAL_KDE[@]} )
    else
        LO_INSTALL_PACKAGES_MANUAL+=( ${LO_INSTALL_PACKAGES_MANUAL_GNOME[@]} )
    fi

    LO_INSTALL_PACKAGES_EXTRA=(

#   libjuh-java
#   libjurt-java
        liblibreoffice-java
#       libreoffice-base-drivers
#       libreoffice-help-common
        libreoffice-style-breeze
        libreoffice-style-colibre
#   libridl-java
        libuno-cppu3
#       libuno-cppuhelpergcc3-3
        libuno-purpenvhelpergcc3-3
        libuno-sal3
        libuno-salhelpergcc3-3
#   libunoil-java
        libunoloader-java
#       uno-libs-private
        )


    LO_INSTALL_PACKAGES=( ${LO_INSTALL_PACKAGES_MANUAL[@]} )

    LO_INSTALL_PACKAGES+=( ${LO_INSTALL_PACKAGES_EXTRA[@]} )

    LO_INSTALLED_PACKAGES=()

    if lo_installed; then
        LO_INSTALLED_PACKAGES=( $(lo_installed_packages) )
        LO_INSTALL_PACKAGES+=( ${LO_INSTALLED_PACKAGES[@]} )
    fi

    LO_INSTALL_PACKAGES+=( ${LO_ADD_PKGS[@]} )

    if ! lo_installed; then
        [[ -z $INSTALL_TARGET ]] && INSTALL_TARGET=main
    fi


    [[ $MYDEBUG ]] && echo LO_INSTALLED_ARCH     $(lo_installed_arch)
    [[ $MYDEBUG ]] && echo LO_INSTALLED_VERSION  $(lo_installed_version)
    LO_CANDIDATE_VERSION=$(lo_candidate_version)
    [[ $MYDEBUG ]] && echo LO_CANDIDATE_VERSION=$LO_CANDIDATE_VERSION
    LO_CANDIDATE_SUITE=$(lo_candidate_suite)
    [[ $MYDEBUG ]] && echo LO_CANDIDATE_SUITE=$LO_CANDIDATE_SUITE
    [[ $MYDEBUG ]] && echo "****************************************"
    if [[ -z $INSTALL_TARGET ]]; then
        if  [[ -n ${LO_CANDIDATE_SUITE} &&  -z ${LO_CANDIDATE_SUITE##${DEBIAN_CODENAME}*} ]]; then
            INSTALL_TARGET=main
        fi
        if [[ ${LO_CANDIDATE_SUITE} == ${DEBIAN_CODENAME}-backports ]]; then
            INSTALL_TARGET=backports
        fi
    fi

    [[ $MYDEBUG ]] && echo INSTALL_TARGET=$INSTALL_TARGET

    if [[ -z $INSTALL_TARGET || $INSTALL_TARGET == backports ]]; then
        set_backports_apt_pref
        if ! backports_enabled; then
            APT_OPTS="-o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10"
            ${SUDO} apt-get $APT_OPTS $APT_CFG_BACKPORTS_SOURCE $APT_CFG_BACKPORTS_UPDATE update
        fi
    fi
    if [[ $INSTALL_TARGET == main ]]; then
        set_main_apt_pref
    fi
    [[ $MYDEBUG ]] && echo APT_CFG=$APT_CFG
    [[ $MYDEBUG ]] && echo "****************************************"
    LO_CANDIDATE_VERSION=$(lo_candidate_version)
    [[ $MYDEBUG ]] && echo LO_CANDIDATE_VERSION=$LO_CANDIDATE_VERSION
    LO_CANDIDATE_SUITE=$(lo_candidate_suite)
    [[ $MYDEBUG ]] && echo LO_CANDIDATE_SUITE=$LO_CANDIDATE_SUITE
    [[ $MYDEBUG ]] && echo "****************************************"

    #LO_INSTALLEDPKGS=( $(lo_installed_packages) )
    #printf '%-40s\t%-40s\t%-40s\n' ${LO_INSTALLEDPKGS[@]}

    [[ $MYDEBUG ]] && echo LO_BACKPORTS_VERSION  $(lo_backports_version)
    [[ $MYDEBUG ]] && echo APT_CFG=$APT_CFG

    #LO_ADD_HM_PKGS - hyphen mythes packages
    HMP=""
    for hmp in "${LO_ADD_HM_PKGS[@]}"; do
        HMP+=" ";
        HMP+=$(apt-cache $APT_CFG search -n "^$hmp(-[a-z]+)?" | cut -d " " -f1);
    done

    [[ $MYDEBUG ]] && set -xv
    $SUDO apt-get $SIMULATE $APT_CFG install $REINSTALL $(apt-cache $APT_CFG madison "${LO_INSTALL_PACKAGES[@]}" | grep "${LO_CANDIDATE_VERSION#*:}"  | grep "$(debian_architecture)" | grep "$(debian_codename)"  | cut -d'|' -f1) ${LO_HELPER[*]} $HMP

    #APT_OPTS="-o=Dpkg::Use-Pty=0 -o Acquire::http:Timeout=10 -o Acquire::https:Timeout=10 -o Acquire::ftp:Timeout=10"
    #$SUDO apt-get $APT_OPTS  update

    [[ $MYDEBUG ]] && printf '\n\nLO_INSTALL_PACKAGES[@]}\n'
    [[ $MYDEBUG ]] && printf '%s\n' "${LO_INSTALL_PACKAGES[@]}"
}

debian_codename() {
    local debian_version="/etc/debian_version"
    if ! [ -r $debian_version ]; then
        echo "[FATAL]: Debian version file '$debian_version' not found. exit" >&2
        exit 1
    fi
    local debian_id=$(cat $debian_version 2>/dev/null | cut -d. -f1)
    case "$debian_id" in
         9) echo stretch  ;;
        10) echo buster   ;;
        11) echo bullseye ;;
        12) echo bookworm ;;
        13) echo trixie   ;;
        "bookworm/sid") echo bookworm;;
         *) echo "[FATAL]: Debian [$debian_id] codename not found. exit " >&2
            exit 1
            ;;
    esac
}

debian_architecture() {
    dpkg --print-architecture
}

debian_uri() {
    local arch=$(debian_architecture)
    local codename=$(debian_codename)
    local release="o=Debian,a=[a-z.-]*,n=$codename,l=Debian,c=main,b=$arch"
    apt-cache policy | grep -B1  "$release" | head -1 | cut -d' ' -f3
}

deb_line_backports() {
    local arch=$(debian_architecture)
    local uri=$(debian_uri)
    local components="main contrib non-free"
    local backports="$(debian_codename)-backports"
    local deb_line="deb [arch=$arch] $uri $backports $components"
    echo "$deb_line"
}
lo_installed() {
    dpkg-query -W -f='${db:Status-Abbrev}\n'  libreoffice-core 2>/dev/null | grep -sq ^i
}
lo_installed_version() {
    if lo_installed; then
       dpkg-query -W -f='${Version}\n'  libreoffice-core  2>/dev/null
    fi
}
lo_installed_arch() {
    if lo_installed; then
       dpkg-query -W -f='${Architecture}\n'  libreoffice-core  2>/dev/null
    fi
}
lo_candidate_version() {
    unset LC_MESSAGES
    unset LC_ALL

    [[ $MYDEBUG ]] && echo "LANG=C.UTF-8 apt-cache policy $APT_CFG  libreoffice-core | grep -oP '^[[:space:]]*Candidate[[:space:]]*:[[:space:]]*\K.*' 2>/dev/null" >&2
    LANG=C.UTF-8 apt-cache policy $APT_CFG  libreoffice-core | grep -oP '^[[:space:]]*Candidate[[:space:]]*:[[:space:]]*\K.*' 2>/dev/null
}
lo_backports_version() {
    local arch=$(lo_installed_arch)
    local codename=$(debian_codename)
    local pkg=${1:-libreoffice-core}
    echo $(apt-cache madison $CFG_BACKPORTS_SOURCE $pkg | grep -F "debian ${codename}-backports" | grep $arch | cut -d'|' -f2)
}
lo_candidate_suite() {
    local codename=$(debian_codename)
    [[ $MYDEBUG ]] && echo "apt-cache madison $APT_CFG libreoffice-core | grep $(lo_candidate_version) | grep -Po $codename(-(security|backports))?(?=/main)" >&2
    apt-cache madison $APT_CFG libreoffice-core | grep "$(lo_candidate_version)" | grep -Po "$codename(-(security|backports))?(?=/main)"
}

backports_enabled() {
    apt-cache policy | grep -sq -F "o=Debian Backports,a=${codename}-backports,n=${codename}-backports"
}
lo_installed_packages() {
    dpkg -l | grep ^i | grep "$(lo_installed_version)" | awk '{print $2}'
}
set_backports_apt_pref() {
    local codename=$(debian_codename)
    APT_BACKPORTS_PREF="$(mktemp -t apt_backports_pref.XXXXXXXXXXXX)"
    chmod 644 ${APT_BACKPORTS_PREF}
    printf  "Package: *\nPin: release o=Debian Backports,n=${codename}-backports\nPin-Priority: 1001\n" > ${APT_BACKPORTS_PREF}

    APT_CFG_BACKPORTS_PREF="-o Dir::Etc::preferences=${APT_BACKPORTS_PREF}"

    if backports_enabled; then
       [[ $MYDEBUG ]] && echo backports_enabled: True
       BACKPORTS_SOURCELIST=""
       APT_CFG_BACKPORTS_SOURCE=""
       APT_CFG_BACKPORTS_UPDATE=
    else
        [[ $MYDEBUG ]] && echo backports_enabled: False
        BACKPORTS_SOURCELIST=$(backports_sourcelist)
        [[ $MYDEBUG ]] && echo BACKPORTS_SOURCELIST $BACKPORTS_SOURCELIST
        [[ $MYDEBUG ]] && cat $BACKPORTS_SOURCELIST
        APT_CFG_BACKPORTS_SOURCE="-o Dir::Etc::sourcelist=$BACKPORTS_SOURCELIST"
        APT_CFG_BACKPORTS_UPDATE="-o Dir::Etc::sourceparts=- -o APT::Get::List-Cleanup=0"
    fi

    APT_CFG="-o=Dpkg::Use-Pty=0 ${APT_CFG_BACKPORTS_SOURCE} ${APT_CFG_BACKPORTS_PREF}"
    }

set_main_apt_pref() {
    local codename=$(debian_codename)
    APT_MAIN_PREF="$(mktemp -t apt_main_pref.XXXXXXXXXXXX)"
    chmod 644 ${APT_MAIN_PREF}
    printf  "Package: *\nPin: release o=Debian,n=${codename}\nPin-Priority: 1001\n\n" > ${APT_MAIN_PREF}
    printf  "Package: *\nPin: release o=Debian,n=${codename}-security\nPin-Priority: 1001\n\n" >> ${APT_MAIN_PREF}
    APT_CFG_MAIN_PREF="-o Dir::Etc::preferences=${APT_MAIN_PREF}";
    APT_CFG="-o=Dpkg::Use-Pty=0 ${APT_CFG_MAIN_PREF}"
    }

tidy_up() {
    [[ $MYDEBUG ]] && set -x
    [[ -f $APT_MAIN_PREF        ]] && rm  $APT_MAIN_PREF
    [[ -f $APT_BACKPORTS_PREF   ]] && rm  $APT_BACKPORTS_PREF
    [[ -f $BACKPORTS_SOURCELIST ]] && rm  $BACKPORTS_SOURCELIST

    }

trap tidy_up EXIT


LO_CANDIDATE_PKGS=

lo_backports_packages() {
    local arch=$(lo_installed_arch)
    local P=( $@ )
    local B=()
    for p in $@; do
        echo $p $(lo_backports_version $p)
    done

}

#lo_backports_packages ${LO_INSTALLEDPKGS[@]}

backports_sourcelist() {
    tmp_sourcelist="$(mktemp -t apt_backports_sourcelist.XXXXXXXXXXXX)"
    chmod 644 ${tmp_sourcelist}
    echo "$(deb_line_backports)" >> ${tmp_sourcelist}
    echo $tmp_sourcelist
}

main "$@"

