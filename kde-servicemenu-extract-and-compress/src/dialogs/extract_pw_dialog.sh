#!/bin/bash

#TRANSLATIONS
case $LANG in
	en* )
		TEXT="Please enter the password.";;

	ca* )
		TEXT="Si us plau, introduïu la contrasenya.";;

	da* )
		TEXT="Indtast kodeordet.";;

	de* )
		TEXT="Bitte geben Sie das Passwort ein.";;

	cs* )
		TEXT="Prosím, zadejte heslo.";;

	es* )
		TEXT="Por favor introduzca la contraseña.";;

	fr* )
		TEXT="Veuillez entrer le mot de passe.";;

	it* )
		TEXT="Per favore inserisci la parola chiave.";;

	pl* )
		TEXT="Proszę o podanie hasła.";;

	pt_BR* )
		TEXT="Por favor introduza a senha.";;

	ru* )
		TEXT="Пожалуйста, введите пароль.";;

	tr* )
		TEXT="Lütfen parolayı giriniz.";;

	zn_CN* )
		TEXT="请输入密码。";;

	* )
		TEXT="Please enter the password.";;

esac

getpw=$(kdialog --password "$TEXT")

echo "$getpw"