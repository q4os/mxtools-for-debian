#!/bin/bash

#make po files

RESOURCE="preview-mx"

lang="am ar bg ca cs da de el es et eu fa fi fr he_IL hi hr hu id is it ja kk ko lt mk mr nb nl pl pt pt_BR ro ru sk sl sq sr sv tr uk zh_CN zh_TW"

make_po()
{
for val in $lang; do
    if [ ! -e "po/$val/$RESOURCE.po" ]; then
        mkdir -p po/$val
        msginit --input=pot/"$RESOURCE".pot --no-translator --locale=$val --output=po/$val/"$RESOURCE".po
    else
        msgmerge --update po/$val/$RESOURCE.po pot/$RESOURCE.pot
    fi
done
}

make_pot()
{
if [ ! -d "po" ]; then
    mkdir po
fi
xgettext --keyword=_ --language=C --add-comments --sort-output -o pot/$RESOURCE.pot src/main.c
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

pot(){
	make_pot
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


