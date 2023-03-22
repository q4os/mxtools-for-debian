#!/bin/bash

#TRANSLATIONS
case $LANG in
	en* )
		TITLE="Compress"
		TEXT="Compression aborted!";;

	ca* )
		TITLE="Compressió"
		TEXT="S'ha interromput la compressió!";;

	da* )
		TITLE="Komprimér"
		TEXT="Komprimering afbrudt!";;

	de* )
		TITLE="Komprimieren"
		TEXT="Komprimierung wurde nicht durchgeführt!";;

	cs* )
		TITLE="Komprimovat"
		TEXT="Komprese přerušena!";;

	es* )
		TITLE="Compresión"
		TEXT="¡Compresión abortada!";;

	fr* )
		TITLE="Compresser"
		TEXT="Compression annulée!";;

	it* )
		TITLE="Compressione"
		TEXT="Compressione interrotta!";;

	pl* )
		TITLE="Spakuj"
		TEXT="Pakowanie przerwane!";;

	pt_BR* )
		TITLE="Compressão"
		TEXT="A compressão foi cancelada!";;

	ru* )
		TITLE="Архивация"
		TEXT="Архивация прекращена!";;

	tr* )
		TITLE="Sıkıştır"
		TEXT="Sıkıştırma iptal edildi!";;

	zn_CN* )
		TITLE="压缩"
		TEXT="压缩已中断！";;

	* )
		TITLE="Compress"
		TEXT="Compression aborted!";;

esac

	

kdialog --title "$TITLE" --error "$TEXT"

