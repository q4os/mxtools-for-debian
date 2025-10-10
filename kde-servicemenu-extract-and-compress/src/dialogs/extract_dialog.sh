#!/bin/bash

#TRANSLATIONS
case $LANG in
	en* )
		TITLE="Extract"
		TEXT="Extraction done!";;

	ca* )
		TITLE="Extracció"
		TEXT="S'ha finalitzat l'extracció!";;

	da* )
		TITLE="Pak ud"
		TEXT="Udpakning gennemført!";;


	de* )
		TITLE="Entpacken"
		TEXT="Die Datei wurde entpackt!";;

	cs* )
		TITLE="Rozbalit"
		TEXT="Úspěšně rozbaleno!";;

	es* )
		TITLE="Extracción"
		TEXT="¡Extracción finalizada !";;

	fr* )
		TITLE="Extraire"
		TEXT="Extraction terminée!";;

	it* )
		TITLE="Estrazione"
		TEXT="Estrazione terminata!";;

	pl* )
		TITLE="Rozpakuj"
		TEXT="Rozpakowywanie zakończone!";;

	pt_BR* )
		TITLE="Extração"
		TEXT="Extração finalizada!";;

	ru* )
		TITLE="Распаковка"
		TEXT="Распаковка успешно выполнена!";;

	tr* )
		TITLE="Aç"
		TEXT="Açma işlemi tamamlandı!";;

	zn_CN* )
		TITLE="解压缩"
		TEXT="解压缩完成！";;

	* )
		TITLE="Extract"
		TEXT="Extraction done!";;

esac

kdialog --title "$TITLE" --msgbox "$TEXT"