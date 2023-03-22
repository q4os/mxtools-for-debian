#!/bin/bash

#make po files

RESOURCE="ddm-mx"

lang="am ar bg ca cs da de el es et eu fa fi fr fr_BE he_IL hi hr hu id is it ja kk ko lt mk mr nb nl pl pt pt_BR ro ru sk sl sq sr sv tr uk zh_CN zh_TW"

make_po()
{
for val in $lang; do
    if [ ! -e "po/$val/$RESOURCE.po" ]; then
        mkdir -p po/$val
        msginit --input=po/"$RESOURCE".pot --no-translator --locale=$val --output=po/${"RESOURCE"}_${val}.po
    else
        msgmerge --update po/"${RESOURCE}_${val}.po" po/$RESOURCE.pot
    fi
done
}

make_pot()
{
if [ ! -d "pot" ]; then
    mkdir pot
fi
xgettext --language Shell  --add-comments -o pot/$RESOURCE.pot ../ddm-mx ../nvidia-detect-mx
xgettext --language Desktop --join --add-comments -o pot/$RESOURCE.pot ../xdg/ddm-nvidia.desktop.in
}


make_mo()
{
    for val in $lang; do
        if [ ! -e "mo/$val/$RESOURCE.mo" ]; then
        	if [ -e po/"${RESOURCE}_${val}.po" ]; then
            	mkdir -p mo/$val
            	msgfmt --output-file=mo/$val/"$RESOURCE".mo po/"${RESOURCE}_${val}.po"
            fi
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

pot(){
	make_pot
}

main()
{
    $1 
    $2
}

main "$@"


