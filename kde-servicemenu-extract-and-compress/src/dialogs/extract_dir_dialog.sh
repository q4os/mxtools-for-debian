#!/bin/bash

#TRANSLATIONS
case $LANG in
	en* )
		TITLE="Enter path";;

	ca* )
		TEXT="Introduïu el camí";;

	da* )
		TITLE="Angiv sti";;

	de* )
		TITLE="Pfad eingeben";;

	cs* )
		TITLE="Zadejte cestu";;

	es* )
		TITLE="Introduzca la ruta";;

	fr* )
		TITLE="Entrez le chemin";;

	it* )
		TITLE="Inserisci percorso";;

	pl* )
		TITLE="Podaj ścieżkę";;

	pt_BR* )
		TITLE="Introduza o caminho";;

	ru* )
		TITLE="Выберите или введите путь для распаковки";;

	tr* )
		TITLE="Dizin giriniz";;

	zn_CN* )
		TITLE="输入路径";;

	* )
		TITLE="Enter path";;

esac

getdir=$(kdialog --title "$TITLE" --getexistingdirectory ~/)

echo "$getdir"