#!/bin/bash

#TRANSLATIONS
case $LANG in
	en* )
		TITLE="Compress"
		TEXT="Compression done!";;

	ca* )
		TITLE="Compressió"
		TEXT="S'ha finalitzat la compressió!";;

	da* )
		TITLE="Komprimér"
		TEXT="Komprimering gennemført!";;

	de* )
		TITLE="Komprimieren"
		TEXT="Komprimierung wurde durchgeführt!";;

	cs* )
		TITLE="Komprimovat"
		TEXT="Úspěšně komprimováno!";;

	es* )
		TITLE="Compresión"
		TEXT="¡Compresión finalizada!";;

	fr* )
		TITLE="Compresser"
		TEXT="Compression terminée!";;

	it* )
		TITLE="Compressione"
		TEXT="Compressione terminata!";;

	pl* )
		TITLE="Spakuj"
		TEXT="Pakowanie zakończone!";;

	pt_BR* )
		TITLE="Compressão"
		TEXT="Compressão finalizada!";;

	tr* )
		TITLE="Sıkıştır"
		TEXT="Sıkıştırma tamamlandı!";;

	ru* )
		TITLE="Архивация"
		TEXT="Архивация успешно выполнена!";;

	zn_CN* )
		TITLE="压缩"
		TEXT="压缩完成！";;

	* )
		TITLE="Compress"
		TEXT="Compression done!";;

esac

kdialog --title "$TITLE" --msgbox "$TEXT"

