#!/bin/bash

#TRANSLATIONS
case $LANG in
	en* )
		TEXT="This folder already includes an archive with the same name. Do you want to overwrite it?";;

	ca* )
		TEXT="Aquesta carpeta ja té un fitxer amb el mateix nom. Voleu sobreescriure'l?";;

	da* )
		TEXT="Denne mappe indeholder allerede et arkiv med samme navn. Vil du overskrive det?";;

	de* )
		TEXT="Im Ordner existiert bereits ein Archiv mit dem selben Namen. Möchten Sie dieses überschreiben?";;

	cs* )
		TEXT="Tato složka již obsahuje archiv se stejným jménem. Chcete ho přepsat?";;

	es* )
		TEXT="Esta carpeta ya tiene un archivo con el mismo nombre. ¿Desea sobreescribirlo?";;

	fr* )
		TEXT="Ce dossier contient déjà une archive ayant le même nom. Voulez-vous l'écraser?";;

	it* )
		TEXT="Questa cartella contiene già un archivio con lo stesso nome. Volete sovrascriverlo?";;

	pl* )
		TEXT="Ten folder zawiera już archiwum o tej samej nazwie. Czy chcesz je nadpisać?";;

	pt_BR* )
		TEXT="Esta pasta já inclui um arquivo com o mesmo nome. Deseja sobrescrevê-lo?";;

	ru* )
		TEXT="Эта папка уже содержит архив с таким именем. Хотите его перезаписать?";;

	tr* )
		TEXT="Bu dizin zaten aynı isimde bir arşiv içeriyor. Üzerine yazılsın mı?";;

	zn_CN* )
		TEXT="此文件夹已经包含相同名称的归档。您想要覆盖它吗？";;

	* )
		TEXT="This folder already includes an archive with the same name. Do you want to overwrite it?";;

esac

kdialog --warningyesno "$TEXT"

case $? in
	0) ow="0";;
	1) ow="1";;
esac

echo "$ow"