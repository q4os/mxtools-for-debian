#!/bin/bash

#make po files

RESOURCE="mx-idevice-mounter"

lang="am ar bg ca cs da de el es et eu fa fi fr he_IL hi hr hu id is it ja kk ko lt mk mr nb nl pl pt ro ru sk sl sq sr sv tr uk zh_CN zh_TW"

make_po()
{
for val in $lang; do
    if [ ! -e "po/$val/$RESOURCE.po" ]; then
        mkdir -p po/$val
        msginit --input=po/"$RESOURCE".pot --no-translator --locale=$val --output=po/$val/"$RESOURCE".po
    else
        msgmerge --update po/$val/$RESOURCE.po po/$RESOURCE.pot
    fi
done
}

make_pot()
{
if [ ! -d "pot" ]; then
    mkdir pot
fi
xgettext --language Shell  --add-comments --sort-output -o pot/$RESOURCE.pot ../idevice-mounter
}


make_mo()
{
    for val in $lang; do
        if [ ! -e "mo/$val/$RESOURCE.mo" ]; then
            mkdir -p mo/$val
            msgfmt --output-file=mo/$val/"$RESOURCE".mo po/"${RESOURCE}_${val}.po"
        fi
    done
}

po()
{
    make_pot
    make_po
}

mo()
{
    make_mo
}

main()
{
    $1 
    $2
}

main "$@"


