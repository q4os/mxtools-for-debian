#!/bin/bash

#TRANSLATIONS
case $LANG in
	en* )
		TEXT="Existing data must be overwritten to extract this archive. Do you want to overwrite them?";;

	ca* )
		TEXT="Les dades existents han de sobreescriure's per extreure aquest arxiu. Voleu sobreescriure'ls?";;

	da* )
		TEXT="Eksisterende data skal overskrives for at udpakke dette arkiv. Vil du overskrive dem?";;

	de* )
		TEXT="Um das Archiv zu entpacken müssen bereits existierende Daten überschrieben werden. Möchten Sie diese überschreiben?";;

	cs* )
		TEXT="Pro rozbalení archivu je nutné přepsat stávající data. Chcete je přepsat?";;

	es* )
		TEXT="Los datos existen deben sobreescribirse para extraer este archivo. ¿Desea sobreescribirlos?";;

	fr* )
		TEXT="Des données existantes doivent être écrasées pour extraire cette archive. Voulez-vous l'écraser?";;

	it* )
		TEXT="Attenzione: i dati esistenti saranno sovrascritti. Volete continuare?";;

	pl* )
		TEXT="Istniejące dane muszą zostać nadpisane, aby rozpakować to archiwum. Czy chcesz, aby zostały one nadpisane?";;

	pt_BR* )
		TEXT="Os dados existentes serão sobrescrevidos quando extrair este arquivo. Deseja sobrescrevê-los?";;

	ru* )
		TEXT="Существующие данные будут удалены при распаковке этого архива. Вы хотите перезаписать их?";;

	tr* )
		TEXT="Bu arşivin açılabilmesi için var olan verinin üzerine yazolmak zorunda. Üzerine yazılsın mı?";;

	zn_CN* )
		TEXT="存在的数据必须被覆盖写入以解压缩此归档。您想要覆盖它们吗？";;

	* )
		TEXT="Existing data must be overwritten to extract this archive. Do you want to overwrite them?";;

esac

kdialog --warningyesno "$TEXT"

case $? in
	0) ow="0";;
	1) ow="1";;
esac

echo "$ow"