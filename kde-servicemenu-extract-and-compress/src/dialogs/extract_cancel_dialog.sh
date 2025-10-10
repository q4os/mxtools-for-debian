#!/bin/bash

#TRANSLATIONS
case $LANG in
	en* )
		TITLE="Extract"
		TEXT="Extraction aborted!";;

	ca* )
		TITLE="Extracció"
		TEXT="S'ha interromput l'extracció!";;

	da* )
		TITLE="Pak ud"
		TEXT="Udpakning afbrudt!";;

	de* )
		TITLE="Entpacken"
		TEXT="Die Datei wurde nicht entpackt!";;

	cs* )
		TITLE="Rozbalit"
		TEXT="Rozbalení přerušeno!";;

	es* )
		TITLE="Extracción"
		TEXT="¡Extracción abortada!";;

	fr* )
		TITLE="Extraire"
		TEXT="Extraction annulée!";;

	it* )
		TITLE="Estrazione"
		TEXT="Estrazione interrotta!";;

	pl* )
		TITLE="Rozpakuj"
		TEXT="Rozpakowywanie przerwane!";;

	pt_BR* )
		TITLE="Extração"
		TEXT="A extração foi cancelada!";;

	ru* )
		TITLE="Распаковка"
		TEXT="Распаковка прекращена!";;

	tr* )
		TITLE="Aç"
		TEXT="Açma işlemi iptal edildi!";;

	zn_CN* )
		TITLE="解压缩"
		TEXT="解压缩已中断！";;

	* )
		TITLE="Extract"
		TEXT="Extraction aborted!";;

esac

kdialog --title "$TITLE" --error "$TEXT"