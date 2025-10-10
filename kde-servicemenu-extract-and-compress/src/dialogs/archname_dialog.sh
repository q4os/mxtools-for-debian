#!/bin/bash

#TRANSLATIONS
case $LANG in
	en* )
		TITLE="Name of archive"
		TEXT="Please enter a name for this archive:"
		INIT="archive";;

	ca* )
		TITLE="Nom de l'arxiu"
		TEXT="Si us plau, introduïu un nom per aquest arxiu:"
		INIT="arxiu";;

	da* )
		TITLE="Arkivets navn"
		TEXT="Indtast venligst et navn for dette arkiv:"
		INIT="arkiv";;

	de* )
		TITLE="Archivname"
		TEXT="Bitte geben Sie einen Namen für dieses Archiv ein:"
		INIT="archiv";;

	cs* )
		TITLE="Název archivu"
		TEXT="Prosím zadejte název archivu:"
		INIT="archivu";;

	es* )
		TITLE="Nombre del archivo"
		TEXT="Por favor introduzca un nombre para este archivo:"
		INIT="archivo";;

	fr* )
		TITLE="Nom de l'archive"
		TEXT="Veuillez entrer un nom pour cette archive:"
		INIT="archive";;

	it* )
		TITLE="Nome dell'archivio"
		TEXT="Inserisci un nome per questo archivio:"
		INIT="archivio";;

	pl* )
		TITLE="Nazwa archiwum"
		TEXT="Proszę o podanie nazwy dla tego archiwum:"
		INIT="archiwum";;

	pt_BR* )
		TITLE="Nome do arquivo"
		TEXT="Por favor, digite um nome para este arquivo:"
		INIT="arquivo";;

	ru* )
		TITLE="Имя архива"
		TEXT="Пожалуйста, введите имя архива:"
		INIT="архив";;

	tr* )
		TITLE="Arşiv Adı"
		TEXT="Bu arşiv için lütfen bir ad girin:"
		INIT="arşiv";;

	zn_CN* )
		TITLE="Name of archive"
		TEXT="Please enter a name for this archive:"
		INIT="archive";;

	* )
		TITLE="Name of archive"
		TEXT="Please enter a name for this archive:"
		INIT="archive";;

esac

name=$(kdialog --title "$TITLE" --inputbox "$TEXT" "$INIT")

case $? in
	0) getname="$name";;
	1) getname="";;
esac

echo "$getname"