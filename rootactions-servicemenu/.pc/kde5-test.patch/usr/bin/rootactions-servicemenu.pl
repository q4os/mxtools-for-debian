#!/usr/bin/perl -w

eval 'exec /usr/bin/perl  -S $0 ${1+"$@"}'
    if 0; # not running under some shell

#----IMPORTANT--------------------------------------------------------

# This script may contain bugs so use with caution.
# If you find bugs or have improvement suggestions, please drop me an email :)

#----Information-------------------------------------------------------

#This script is part of the Root Actions -servicemenu for kde.
#Written by kubicle <mail.kubicle@gmail.com>


#----Disclaimer-------------------------------------------------------  

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

#----Instructions-----------------------------------------------------

#This script should be placed along your $PATH for the servicemenu to work, some possible locations are:
#  /usr/local/bin and /usr/bin.

#Also make sure the script is executable for the users that shall use the servicemenu.
#---------------------------------------------------------------------

########### USER CONFIGURABLE VARIABLES #######################

$CPDIALOGS="true" ; #Show dialogs when copying/moving, changing to "false" will suppress dialogs and *OVERWRITE WITHOUT WARNING* 



########### DIALOG MESSAGES, TRANSLATABLE STRINGS #############

sub copy_dialog_msgs {
	&get_kde_language;
	%COPYTITLE = (
	   "ca" => "Copia",
	   "cs" => "Kopírovat",
	   "el" => "Αντιγραφή",
	   "en_US" => "Copy",
	   "fi" => "Kopioi",
	   "fr" => "Copier",
	   "gl" => "Copiar",
	   "hu" => "Másolás",
	   "it" => "Copia",
	   "lt" => "Kopijuoti",
	   "nb" => "Kopier",
	   "nn" => "Kopier",
	   "pl" => "Kopiuj",
	   "pt" => "Copiar",
	   "pt_PT" => "Copiar",
	   "ru" => "Копировать",
	   "sl" => "Kopiraj",
	   "sr" => "Копирај",
	   "sr\@latin" => "Kopiraj",
	   "sv" => "Kopiera",
	   "tr" => "Kopyala",
	   "xx" => "Your string, xx is the country abbreviation"
	);
	%COPYMSG = (
	   "ca" => "Nom de fitxer:",
	   "cs" => "Název souboru:",
	   "el" => "Όνομα Αρχείου",
	   "en_US" => "Filename:",
	   "fi" => "Tiedoston nimi",
	   "fr" => "Nom du fichier:",
	   "gl" => "Nome do ficheiro:",
	   "hu" => "Fájl neve:",
	   "it" => "Nome del file:",
	   "lt" => "Failo pavadinimas:",
	   "nb" => "Filnavn:",
	   "nn" => "Filnamn:",
	   "pl" => "Nazwa pliku:",
	   "pt" => "Nome do ficheiro:",
	   "pt_PT" => "Nome do ficheiro:",
	   "ru" => "Имя файла:",
	   "sl" => "Ime datoteke",
	   "sr" => "Име фајла:",
	   "sr\@latin" => "Ime fajla:",
	   "sv" => "Filnamn:",
	   "tr" => "Dosya adı:",
	   "xx" => "Your string, xx is the country abbreviation"
	);
	$COPYTITLE = ($COPYTITLE{"$KDELANG"} or $COPYTITLE{"$KDELANGSHT"} or $COPYTITLE{"en_US"});
	$COPYMSG = ($COPYMSG{"$KDELANG"} or $COPYMSG{"$KDELANGSHT"} or $COPYMSG{"en_US"});
	return 0;
}

sub rename_dialog_msgs {
	&get_kde_language;
	%RENAMETITLE = (
	   "ca" => "Mou/Reanomena",
	   "cs" => "Přesunout/Přejmenovat",
	   "de" => "Umbenennen",
	   "el" => "Μετακίνηση/Μετονομασία",
	   "en_US" => "Move/Rename",
	   "es" => "Renombrar",
	   "fi" => "Nimeä uudelleen",
	   "fr" => "Renommer",
	   "gl" => "Cambiar o nome",
	   "hu" => "Mozgatás/Átnevezés",
	   "it" => "Muovi/Rinomina",
	   "lt" => "Perkelti/pervadinti",
	   "nb" => "Flytt/Omdøp",
	   "nl" => "Hernoemen",
	   "nn" => "Flytt/Omdøyp",
	   "pl" => "Przenieś/Zmień nazwę",
	   "pt" => "Mudar o nome",
	   "pt_PT" => "Mudar o nome",
	   "ru" => "Переместить/переименовать",
	   "sl" => "Premakni/preimenuj",
	   "sr" => "Премести/преименуј",
	   "sr\@latin" => "Premesti/preimenuj",
	   "sv" => "Flytta/Byt namn",
	   "tr" => "Taşı/Yeniden İsimlendir",
	   "xx" => "Your string, xx is the country abbreviation"
	);
	%RENAMEMSG = (
	   "ca" => "Nom nou:",
	   "cs" => "Nové jméno:",
	   "de" => "Neuer Name:",
	   "el" => "Νέο Όνομα:",
	   "en_US" => "New name:",
	   "es" => "Nombre nuevo:",
	   "fi" => "Uusi nimi:",
	   "fr" => "Nouveau nom :",
	   "gl" => "Novo nome:",
	   "hu" => "Új név:",
	   "it" => "Nuovo nome:",
	   "lt" => "Naujas pavadinimas:",
	   "nb" => "Nytt navn:",
	   "nl" => "Nieuwe naam:",
	   "nn" => "Nytt namn:",
	   "pl" => "Nowa nazwa:",
	   "pt" => "Novo nome:",
	   "pt_PT" => "Novo nome:",
	   "ru" => "Новое имя:",
	   "sl" => "Novo ime",
	   "sr" => "Ново име:",
	   "sr\@latin" => "Novo ime:",
	   "sv" => "Nytt filnamn:",
	   "tr" => "Yeni isim:",
	   "xx" => "Your string, xx is the country abbreviation"
	);
	$RENAMETITLE = ($RENAMETITLE{"$KDELANG"} or $RENAMETITLE{"$KDELANGSHT"} or $RENAMETITLE{"en_US"});
	$RENAMEMSG = ($RENAMEMSG{"$KDELANG"} or $RENAMEMSG{"$KDELANGSHT"} or $RENAMEMSG{"en_US"});
	return 0;
}

sub delete_dialog_msgs {
	&get_kde_language;
	%DELETETITLE = (
	   "ca" => "Avís",
	   "cs" => "Varování",
	   "de" => "Achtung",
	   "el" => "Προσοχή",
	   "en_US" => "Warning",
	   "es" => "Aviso",
	   "fi" => "Varoitus",
	   "fr" => "Attention",
	   "gl" => "Aviso",
	   "hu" => "Figyelem",
	   "it" => "Attenzione",
	   "lt" => "Perspėjimas",
	   "nb" => "Advarsel",
	   "nl" => "Waarschuwing",
	   "nn" => "Advarsel",
	   "pl" => "Ostrzeżenie",
	   "pt" => "Atenção",
	   "pt_PT" => "Atenção",
	   "ru" => "Внимание",
	   "sl" => "Opozorilo",
	   "sr" => "Упозорење",
	   "sr\@latin" => "Upozorenje",
	   "sv" => "Varning",
	   "tr" => "Uyarı",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%DELETEMSG = (
	   "ca" => "Esteu segur que voleu esborrar-lo completament?: ",
	   "cs" => "Jste si jisti, že chcete nenávratně smazat: ",
	   "de" => "Möchten Sie die folgenden Dateien dauerhaft löschen: ",
	   "el" => "Είστε σίγουροι ότι επιθυμείτε την οριστική διαγραφή: ", 
	   "en_US" => "Are you sure you wish to permanently delete: ", 
	   "es" => "¿Estas seguro de querer borrarlo por completo?",
	   "fi" => "Haluatko varmasti kokonaan poistaa: ",
	   "fr" => "Êtes-vous sûr de vouloir supprimer définitivement :",
	   "gl" => "Está seguro de que o quere borrar definitivamente?",
	   "hu" => "Biztosan törölni akarod?",
	   "it" => "Sei sicuro di voler eliminare in modo definitivo: ",
	   "lt" => "Ar tikrai norite galutinai ištrinti: ",
	   "nb" => "Er du sikker på at du vil slette permanent: ",
	   "nl" => "Bent u zeker dat u permanent wil verwijderen: ",
	   "nn" => "Er du sikker på at du vil slette permanent: ",
	   "pl" => "Czy na pewno chcesz nieodwracalnie usunąć: ",
	   "pt" => "Tem a certeza que quer apagar definitivamente? ",
	   "pt_PT" => "Tem a certeza que quer apagar definitivamente? ",
	   "ru" => "Вы уверены, что хотите удалить:",
	   "sl" => "Ali ste prepričani, da želite izbrisati: ",
	   "sr" => "Желите ли заиста да обришете: ",
	   "sr\@latin" => "Želite li zaista da obrišete: ",
	   "sv" => "Är du säker på att du vill radera permanent?: ",
	   "tr" => "Bu dosyayı kalıcı olarak silmek istediğinize emin misiniz: ",
	   "xx" => "Your string, xx is the country abbreviation"
	);
	$DELETETITLE = ($DELETETITLE{"$KDELANG"} or $DELETETITLE{"$KDELANGSHT"} or $DELETETITLE{"en_US"});
	$DELETEMSG = ($DELETEMSG{"$KDELANG"} or $DELETEMSG{"$KDELANGSHT"} or $DELETEMSG{"en_US"});
	return 0;
}

sub ownership_dialog_msgs {
	&get_kde_language;
	%OWNERTITLE = (
	   "ca" => "Estableix UID:GID",
	   "cs" => "Nastavit UID:GIG",
	   "de" => "Eigentümer ändern",
	   "el" => "Ορίστε UID:GID", 
	   "en_US" => "Set UID:GID",
	   "es" => "Establecer UID:GID",
	   "fi" => "Anna UID:GID",
	   "fr" => "Modifier UID:GID",
	   "gl" => "Establecer UID:GID",
	   "hu" => "UID:GID beállítása",
	   "it" => "Cambia UID:GID",
	   "lt" => "Nustatyti UID:GID",
	   "nb" => "Sett UID:GID",
	   "nl" => "Wijzig UID:GID",
	   "nn" => "Set UID:GID",
	   "pl" => "Ustawienie UID:GID",
	   "pt" => "Atribuir UID:GID",
	   "pt_PT" => "Atribuir UID:GID",
	   "ru" => "Установить UID:GID",
	   "sl" => "Nastavi UID:GID",
	   "sr" => "Постави УИД:ГИД",
	   "sr\@latin" => "Postavi UID:GID",
	   "sv" => "Ändra UID:GID",
	   "tr" => "UID:GID'i ayarla",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%OWNERMSG = (
	   "ca" => "Canvia propietari a: ",
	   "cs" => "Změnit vlastníka na: ",
	   "de" => "Neuer Eigentümer: ",
	   "el" => "Αλλαγή Ιδιοκτησίας στον: ",
	   "en_US" => "Change ownership to: ", 
	   "es" => "Cambiar propietario a: ",
	   "fi" => "Vaihda omistajaksi: ",
	   "fr" => "Attribuer à : ",
	   "gl" => "Facer dono a: ",
	   "hu" => "Fájl tulajdonosának megváltoztatása: ",
	   "it" => "Cambia proprietario: ",
	   "lt" => "Pakeisti nuosavybę į: ",
	   "nb" => "Sett eierskap til: ",
	   "nl" => "Wijzig eigenaar in: ",
	   "nn" => "Set eigarskap til: ",
	   "pl" => "Zmień własność na: ",
	   "pt" => "Alterar dono para: ",
	   "pt_PT" => "Alterar dono para: ",
	   "ru" => "Сменить владельца на:",
	   "sl" => "Spremeni lastnika: ",
	   "sr" => "Промени власника: ",
	   "sr\@latin" => "Promeni vlasnika: ",
	   "sv" => "Ändra ägare till: ",
	   "tr" => "Dosyanın sahibini değiştir: ",
	   "xx" => "Your string, xx is the country abbreviation"
	);
	$OWNERTITLE = ($OWNERTITLE{"$KDELANG"} or $OWNERTITLE{"$KDELANGSHT"} or $OWNERTITLE{"en_US"});
	$OWNERMSG = ($OWNERMSG{"$KDELANG"} or $OWNERMSG{"$KDELANGSHT"} or $OWNERMSG{"en_US"});
	return 0;
}

sub recursion_dialog_msgs {
	&get_kde_language;
	%RECURSIONTITLE = (
	   "ca" => "Aplico els canvis recursivament?",
	   "cs" => "Aplikovat změny na všechny?",
	   "el" => "Εφαρμογή αλλαγών επαναλαμβανόμενα;",
	   "en_US" => "Apply changes recursively?",
	   "fi" => "Toteutetaanko muutokset rekursiivisesti?",
	   "fr" => "Appliquer récursivement les modifications ?",
	   "gl" => "Aplicar os cambios de forma recursiva?",
	   "hu" => "A változások az alkönyvtár(ak)ra is kiterjedjenek?",
	   "it" => "Applico i cambiamenti?",
	   "lt" => "Taikyti pakeitimus rekursyviai?",
	   "nb" => "Endre rekursivt?",
	   "nn" => "Endra rekursivt?",
	   "pl" => "Wprowadzić zmiany rekursywnie?",
	   "pt" => "Aplicar alterações recursivamente?",
	   "pt_PT" => "Aplicar alterações recursivamente?",
	   "ru" => "Применить изменения рекурсивно?",
	   "sl" => "Uveljavim spremembe rekurzivno?",
	   "sr" => "Да применим измене рекурзивно?",
	   "sr\@latin" => "Da primenim izmene rekurzivno?",
	   "sv" => "Verkställ ändringar rekursivt?",
	   "tr" => "Değişiklikler özyinelemeli olarak uygulansın mı?",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%RECURSIONMSG = (
	   "ca" => "Voleu aplicar els canvis recursivament a totes les subcarpetes i fitxers? \n(Useu-lo amb precaució)",
	   "cs" => "Jste si jisti, že chcete změny aplikovat na všechny soubory a podsložky? \n(Používejte z rozmyslem)",
	   "el" => "Επιθυμείτε την εφαρμογή των αλλαγών επαναλαμβανόμενα σε όλους τους υπο-καταλόγους και αρχεία; \n (Χρησιμοποιείστε το προσεκτικά)",
	   "en_US" => "Do you wish to apply the changes recursively to all sub-folders and files? \n(Use with caution)",
	   "fi" => "Haluatko ulottaa muutokset rekursiivisesti alikansioihin ja tiedostoihin? \n(Käytä varoen)",
	   "fr" => "Voulez-vous appliquer les modifications récursivement à tous les répertoires et fichiers ? \n(À utiliser avec précaution)",
	   "gl" => "Quere aplicar os cambios de forma recursiva a todos os cartafoles e ficheiros contidos?\n(Úsese con coidado)",
	   "hu" => "Szeretnéd, ha a változtatások minden alkönyvtárra és az ott lévő fájlokra is kiterjedjenek? \n(Légy óvatos)",
	   "it" => "Vuoi applicare i cambiamenti a tutte le sottocartelle e ai files? \n(Da utilizzare con cautela)",
	   "lt" => "Ar norite pritaikyti pakeitums rekursyviai visiems paaplankiams ir failams? \n(Naudoti atsargiai)",
	   "nb" => "Vil du gjøre endringene gjeldende rekursivt i alle undermapper og filer? \n(Vær forsiktig)",
	   "nn" => "Vil du gjere endringane gjeldande rekursivt i alle undermapper og filer? \n(Vér forsiktig)",
	   "pl" => "Czy chcesz wprowadzić zmiany rekursywnie dla wszystkich podfolderów i plików? \n(Używaj z ostrożnością)",
	   "pt" => "Deseja aplicar as alterações recursivamente a todos os sub-directórios e ficheiros? \n(Usar com cuidado)",
	   "pt_PT" => "Deseja aplicar as alterações recursivamente a todos os sub-directórios e ficheiros? \n(Usar com cuidado)",
	   "ru" => "Вы хотите применить изменения рекурсивно для всех подпапок и файлов",
	   "sl" => "Ali želite uveljaviti spremembe rekurzivno na vse podmape in datoteke? \n (Uporabite pazljivo)",
	   "sr" => "Желите ли да примените измене рекурзивно на све потфасцикле и фајлове? \n(Користите ово пажљиво)",
	   "sr\@latin" => "Želite li da primenite izmene rekurzivno na sve potfascikle i fajlove? \n(Koristite ovo pažljivo)",
	   "sv" => "Verkställ ändringar rekursivt för alla underkataloger och deras innehåll? \n(Var försiktig)",
	   "tr" => "Değişiklikleri tüm altdizinlere ve dosyalara özyinelemeli olarak uygulamak istiyor musunuz? \n(Dikkatli kullanın",
	   "xx" => "Your string, xx is the country abbreviation"
	);
	$RECURSIONTITLE = ($RECURSIONTITLE{"$KDELANG"} or $RECURSIONTITLE{"$KDELANGSHT"} or $RECURSIONTITLE{"en_US"});
	$RECURSIONMSG = ($RECURSIONMSG{"$KDELANG"} or $RECURSIONMSG{"$KDELANGSHT"} or $RECURSIONMSG{"en_US"});
	return 0;
}

sub root_reminder_msgs {
	&get_kde_language;
	%ROOTREMINDER = (
	   "ca" => "OBERT COM A ROOT",
	   "cs" => "OTEVŘENO JAKO ROOT",
	   "el" => "ΑΝΟΙΧΤΟ ΣΑΝ ΥΠΕΡΧΡΗΣΤΗΣ",
	   "en_US" => "OPENED AS ROOT",
	   "fi" => "AVATTU PÄÄKÄYTTÄJÄNÄ",
	   "fr" => "OUVERT EN ROOT",
	   "gl" => "ABERTO COMA ROOT",
	   "hu" => "RENDSZERGAZDAKÉNT MEGNYITVA",
	   "it" => "APERTO COME ROOT",
	   "lt" => "ATVERTA ROOT TEISĖMIS",
	   "nb" => "ÅPNET SOM ROOT",
	   "nn" => "OPNA SOM ROOT",
	   "pl" => "OTWARTO JAKO ADMINISTRATOR",
	   "pt" => "ABERTO COMO ROOT",
	   "pt_PT" => "ABERTO COMO ROOT",
	   "ru" => "Открыто как root",
	   "sl" => "ODPRTO KOT ADMINISTRATOR",
	   "sr" => "ОТВОРЕНО КАО КОРЕН",
	   "sr\@latin" => "OTVORENO KAO KOREN",
	   "sv" => "ÖPPNAD SOM ROOT",
	   "tr" => "YETKİLİ KULLANICI OLARAK AÇILDI",
	   "xx" => "Your string, xx is the country abbreviation"
	);
	$ROOTREMINDER = ($ROOTREMINDER{"$KDELANG"} or $ROOTREMINDER{"$KDELANGSHT"} or $ROOTREMINDER{"en_US"});
	return 0;
}


sub open_dialog_msgs {
	&get_kde_language;
	%OPENTITLE = (
	   "ca" => "Programa",
	   "cs" => "Program",
	   "de" => "Programm",
	   "el" => "Πρόγραμμα",
	   "en_US" => "Program",
	   "es" => "Programa",
	   "fi" => "Ohjelma",
	   "fr" => "Programme",
	   "gl" => "Programa",
	   "hu" => "Program",
	   "it" => "Programma",
	   "lt" => "Programa",
	   "nb" => "Program",
	   "nl" => "Programma",
	   "nn" => "Program",
	   "pl" => "Program",
	   "pt" => "Programa",
	   "pt_PT" => "Programa",
	   "ru" => "Программа",
	   "sl" => "Program",
	   "sr" => "Програм",
	   "sr\@latin" => "Program",
	   "sv" => "Program",
	   "tr" => "Program",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%OPENMSG = (
	   "ca" => "Obre amb: ",
	   "cs" => "Otevřít s: ",
	   "de" => "Öffnen mit: ",
	   "el" => "Άνοιγμα με: ",
	   "en_US" => "Open with: ",
	   "es" => "Abrir con: ",
	   "fi" => "Avaa ohjelmalla: ",
	   "fr" => "Ouvrir avec : ",
	   "gl" => "Abrir con: ",
	   "hu" => "Megnyitás ezzel: ",
	   "it" => "Apri con: ",
	   "lt" => "Atverti su: ",
	   "nb" => "Åpne med: ",
	   "nl" => "Openen met: ",
	   "nn" => "Opna med: ",
	   "pl" => "Otwórz z: ",
	   "pt" => "Abrir com: ",
	   "pt_PT" => "Abrir com: ",
	   "ru" => "Открыть с помощью: ",
	   "sl" => "Odpri z: ",
	   "sr" => "Отвори помоћу: ",
	   "sr\@latin" => "Otvori pomoću: ",
	   "sv" => "Öppna med: ",
	   "tr" => "Birlikte aç: ",
	   "xx" => "Your string, xx is the country abbreviation"
	);
	$OPENTITLE = ($OPENTITLE{"$KDELANG"} or $OPENTITLE{"$KDELANGSHT"} or $OPENTITLE{"en_US"});
	$OPENMSG = ($OPENMSG{"$KDELANG"} or $OPENMSG{"$KDELANGSHT"} or $OPENMSG{"en_US"});
	return 0;
}

sub permission_dialog_msgs {
	&get_kde_language;
	%PERMTITLE = (
	   "ca" => "Permisos",
	   "cs" => "Oprávnění",
	   "de" => "Berechtigungen",
	   "el" => "Άδειες",
	   "en_US" => "Permissions",
	   "es" => "Permisos",
	   "fi" => "Käyttöoikeudet",
	   "fr" => "Droits d'accès",
	   "gl" => "Permisos",
	   "hu" => "Jogosultságok",
	   "it" => "Permessi",
	   "lt" => "Leidimai",
	   "nb" => "Rettigheter",
	   "nl" => "Toegengsrechten",
	   "nn" => "Rettigheiter",
	   "pl" => "Uprawnienia",
	   "pt" => "Permissões",
	   "pt_PT" => "Permissões",
	   "ru" => "Резрешения",
	   "sl" => "Dovoljenja",
	   "sr" => "Дозволе",
	   "sr\@latin" => "Dozvole",
	   "sv" => "Rättigheter",
	   "tr" => "Yetkiler",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%PERMMSG = (
	   "ca" => "Seleccioneu els permisos: ",
	   "cs" => "Zvolit oprávnění",
	   "de" => "Berechtigungen auswählen: ",
	   "el" => "Επιλέξτε άδειες: ",
	   "en_US" => "Choose permissions: ",
	   "es" => "Elegir permisos: ",
	   "fi" => "Valitse käyttöoikeudet: ",
	   "fr" => "Choisissez les droits d'accès : ",
	   "gl" => "Escoller os permisos: ",
	   "hu" => "Add meg a jogosultságokat: ",
	   "it" => "Permessi di accesso: ",
	   "lt" => "Pasirinkite leidimus: ",
	   "nb" => "Velg rettigheter: ",
	   "nl" => "Toegangsrechten instellen: ",
	   "nn" => "Velg rettigheiter: ",
	   "pl" => "Wybierz uprawnienia: ",
	   "pt" => "Escolher permissões: ",
	   "pt_PT" => "Escolher permissões: ",
	   "ru" => "Выбрать разрешения: ",
	   "sl" => "Izberite dovoljenja: ",
	   "sr" => "Изаберите дозволе: ",
	   "sr\@latin" => "Izaberite dozvole: ",
	   "sv" => "Välj rättigheter: ",
	   "tr" => "Yetkileri belirle: ",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%OWN = (
	   "ca" => "Propietari:",
	   "cs" => "Vlastník:",
	   "de" => "Besitzer:",
	   "el" => "Ιδιοκτήτης:",
	   "en_US" => "Owner:",
	   "es" => "Propietario:",
	   "fi" => "Omistaja:",
	   "fr" => "Propriétaire :",
	   "gl" => "Dono:",
	   "hu" => "Tulajdonos:",
	   "it" => "Proprietario:",
	   "lt" => "Savininkas:",
	   "nb" => "Eier:",
	   "nl" => "Eigenaar:",
	   "nn" => "Eigar:",
	   "pl" => "Właściciel:",
	   "pt" => "Dono:",
	   "pt_PT" => "Dono:",
	   "ru" => "Владелец:",
	   "sl" => "Lastnik: ",
	   "sr" => "Власник:",
	   "sr\@latin" => "Vlasnik:",
	   "sv" => "Ägare:",
	   "tr" => "Sahip:",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%GRP = (
	   "ca" => "Grup:",
	   "cs" => "Skupina:",
	   "de" => "Gruppe:",
	   "el" => "Ομάδα:",
	   "en_US" => "Group:",
	   "es" => "Grupo:",
	   "fi" => "Ryhmä:",
	   "fr" => "Groupe :",
	   "gl" => "Grupo:",
	   "hu" => "Csoport:",
	   "it" => "Gruppo:",
	   "lt" => "Grupė:",
	   "nb" => "Gruppe:",
	   "nl" => "Groep:",
	   "nn" => "Gruppe:",
	   "pl" => "Grupa:",
	   "pt" => "Grupo:",
	   "pt_PT" => "Grupo:",
	   "ru" => "Группа:",
	   "sl" => "Skupina:",
	   "sr" => "Група:",
	   "sr\@latin" => "Grupa:",
	   "sv" => "Grupp:",
	   "tr" => "Grup",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%ALL = (
	   "ca" => "Altres:",
	   "cs" => "Ostatní:",
	   "de" => "Alle Benutzer:",
	   "el" => "Άλλοι:",
	   "en_US" => "Others:",
	   "es" => "Todos:",
	   "fi" => "Kaikki:",
	   "fr" => "Autres :",
	   "gl" => "Outros:",
	   "hu" => "Mindenki:",
	   "it" => "Tutti:",
	   "lt" => "Kiti:",
	   "nb" => "Andre:",
	   "nl" => "Anderen:",
	   "nn" => "Andre:",
	   "pl" => "Wszyscy:",
	   "pt" => "Todos:",
	   "pt_PT" => "Todos:",
	   "ru" => "Другие:",
	   "sl" => "Ostali:",
	   "sr" => "Остали:",
	   "sr\@latin" => "Ostali:",
	   "sv" => "Övriga:",
	   "tr" => "Diğerleri:",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%SPC = (
	   "ca" => "Especial:",
	   "cs" => "Speciální:",
	   "de" => "Spezial:",
	   "el" => "Ειδικά:",
	   "en_US" => "Special:",
	   "es" => "Especial:",
	   "fi" => "Erityiset:",
	   "fr" => "Spécial :",
	   "gl" => "Especial:",
	   "hu" => "Különleges:",
	   "it" => "Avanzati:",
	   "lt" => "Specialūs:",
	   "nb" => "Spesielt:",
	   "nl" => "Speciaal:",
	   "nn" => "Spesielt:",
	   "pl" => "Specjalne:",
	   "pt" => "Especial:",
	   "pt_PT" => "Especial:",
	   "ru" => "Специальный:",
	   "sl" => "Posebno:",
	   "sr" => "Посебно:",
	   "sr\@latin" => "Posebno:",
	   "sv" => "Special:",
	   "tr" => "Özel:",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%READ = (
	   "ca" => "lectura",
	   "cs" => "čtení",
	   "de" => "Lesen",
	   "el" => "αναγνώσιμο",
	   "en_US" => "readable",
	   "es" => "Lectura",
	   "fi" => "lukuoikeus",
	   "fr" => "Lecture",
	   "gl" => "lectura",
	   "hu" => "Olvasható",
	   "it" => "leggibile",
	   "lt" => "skaitomas",
	   "nb" => "lesbar",
	   "nl" => "Leesbaar",
	   "nn" => "lesbar",
	   "pl" => "tylko do odczytu",
	   "pt" => "Permissão de leitura",
	   "pt_PT" => "Permissão de leitura",
	   "ru" => "Разрешён для чтения",
	   "sl" => "branje",
	   "sr" => "читање",
	   "sr\@latin" => "čitanje",
	   "sv" => "läsbar",
	   "tr" => "okunabilir",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%WRITE = (
	   "ca" => "escriptura",
	   "cs" => "zápis",
	   "de" => "Schreiben",
	   "el" => "εγγράψιμο",
	   "en_US" => "writable",
	   "es" => "Escritura",
	   "fi" => "kirjoitusoikeus",
	   "fr" => "Écriture",
	   "gl" => "escritura",
	   "hu" => "Írható",
	   "it" => "scrivibile",
	   "lt" => "rašomas",
	   "nb" => "skrivbar",
	   "nl" => "Schrijfbaar",
	   "nn" => "skrivbar",
	   "pl" => "do odczytu i zapisu",
	   "pt" => "Permissão de escrita",
	   "pt_PT" => "Permissão de escrita",
	   "ru" => "Разрешён для записи",
	   "sl" => "pisanje",
	   "sr" => "упис",
	   "sr\@latin" => "upis",
	   "sv" => "skrivbar",
	   "tr" => "yazılabilir",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%EXEC = (
	   "ca" => "executable",
	   "cs" => "spuštění",
	   "de" => "Ausführen",
	   "el" => "εκτελέσιμο",
	   "en_US" => "executable",
	   "es" => "Ejecutable",
	   "fi" => "suoritusoikeus",
	   "fr" => "Exécution",
	   "gl" => "executable",
	   "hu" => "Futtatható",
	   "it" => "eseguibile",
	   "lt" => "vykdomas",
	   "nb" => "kjørbar",
	   "nl" => "Uitvoerbaar",
	   "nn" => "køyrbar",
	   "pl" => "wykonywalny",
	   "pt" => "Executável",
	   "pt_PT" => "Executável",
	   "ru" => "Исполняемый",
	   "sl" => "izvajanje",
	   "sr" => "извршна",
	   "sr\@latin" => "izvršna",
	   "sv" => "körbar",
	   "tr" => "çalıştırılabilir",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%SUID = (
	   "ca" => "estableix bit d'ID d'usuari",
	   "cs" => "nastavit uživatelské ID",
	   "de" => "User-ID-Bit setzen",
	   "el" => "ορίστε το ID bit του χρήστη",
	   "en_US" => "set user ID bit",
	   "es" => "Establecer el bit-ID de usuario",
	   "fi" => "aseta SUID-bitti",
	   "fr" => "Donner l'UID",
	   "gl" => "establecer o bit de identificación do usuario",
	   "hu" => "UID beállítása",
	   "it" => "inserisci bit UID",
	   "lt" => "nustatyti vartotojo UID bitą",
	   "nb" => "sett bruker-ID",
	   "nl" => "User ID instellen",
	   "nn" => "set brukar-ID",
	   "pl" => "ustaw ID użytkownika",
	   "pt" => "Definir o bit ID de utilizador",
	   "pt_PT" => "Definir o bit ID de utilizador",
	   "ru" => "Установить UID пользователя",
	   "sl" => "nastvi UID",
	   "sr" => "стави УИД",
	   "sr\@latin" => "stavi UID",
	   "sv" => "sätt UID",
	   "tr" => "kullanıcı ID bitini ayarla",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%SGID = (
	   "ca" => "estableix bit d'ID de grup",
	   "cs" => "nastavit skupinové ID",
	   "de" => "Gruppen-ID-Bit setzen",
	   "el" => "ορίστε το ID bit της ομάδας",
	   "en_US" => "set group ID bit",
	   "es" => "Establecer el bit-ID de grupo",
	   "fi" => "aseta SGID-bitti",
	   "fr" => "Donner le GID",
	   "gl" => "establecer o bit de identificación do grupo",
	   "hu" => "GID beállítása",
	   "it" => "inserisci bit GID",
	   "lt" => "nustatyti grupės GID bitą",
	   "nb" => "sett gruppe-ID",
	   "nl" => "Groep ID instellen",
	   "nn" => "set gruppe-ID",
	   "pl" => "ustaw ID grupy",
	   "pt" => "Definir o bit ID de grupo",
	   "pt_PT" => "Definir o bit ID de grupo",
	   "ru" => "Установить GID группы",
	   "sl" => "nastavi GID",
	   "sr" => "стави ГИД",
	   "sr\@latin" => "stavi GID",
	   "sv" => "sätt GID",
	   "tr" => "grup ID bitini ayarla",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%STICK = (
	   "ca" => "estableix bit d'adherit",
	   "cs" => "Sticky",
	   "de" => "Sticky-Bit setzen",
	   "el" => "ορίστε το sticky bit",
	   "en_US" => "set sticky bit",
	   "es" => "Establecer el bit-Sticky",
	   "fi" => "aseta sticky-bitti",
	   "fr" => "Allouer le bit collant",
	   "gl" => "establecer o bit de adhesión",
	   "hu" => "Sticky bit beállítása",
	   "it" => "inserisci bit Sticky",
	   "lt" => "nustatyti lipnų bitą",
	   "nb" => "klebrig",
	   "nl" => "Vastgezet",
	   "nn" => "klebrig",
	   "pl" => "ustaw parametr 'sticky'",
	   "pt" => "Definir o bit 'sticky'",
	   "pt_PT" => "Definir o bit 'sticky'",
	   "ru" => "Установить липкий бит",
	   "sl" => "nastavi lepljivo",
	   "sr" => "стави лепљиву",
	   "sr\@latin" => "stavi lepljivu",
	   "sv" => "klistrig",
	   "tr" => "yapışkan biti ayarla",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%SPCTITLE = (
	   "ca" => "Avís",
	   "cs" => "Varování",
	   "de" => "Achtung",
	   "el" => "Προσοχή",
	   "en_US" => "Warning",
	   "es" => "Aviso",
	   "fi" => "Varoitus",
	   "fr" => "Attention",
	   "gl" => "Aviso",
	   "hu" => "Figyelem",
	   "it" => "Attenzione",
	   "lt" => "Dėmesio",
	   "nb" => "Advarsel",
	   "nl" => "Waarschuwing",
	   "nn" => "Advarsel",
	   "pl" => "Ostrzeżenie",
	   "pt" => "Atenção",
	   "pt_PT" => "Atenção",
	   "ru" => "Внимание",
	   "sl" => "Opozorilo",
	   "sr" => "Упозорење",
	   "sr\@latin" => "Upozorenje",
	   "sv" => "Varning",
	   "tr" => "Dikkat",
	   "xx" => "Your string, xx is the country abbreviation"
	);

	%SPCMSG = (
	   "ca" => "Heu seleccionat establir permisos especials, esteu segur que voleu continuar?",
	   "cs" => "Zvolili jste speciální práva, jste si jisti že chcete pokračovat?",
	   "de" => "Sie haben eine spezielle Berechtigung ausgewählt. Möchten Sie wirklich fortfahren?",
	   "el" => "Έχετε επιλέξει να ορίσετε ειδικές άδειες, είστε σίγουροι ότι επιθυμείτε να προχωρήσετε;",
	   "en_US" => "You have chosen to set special permissions, are you sure you wish to continue?",
	   "es" => "Has seleccionado establecer permisos especiales, ¿estas seguro de continuar?",
	   "fi" => "Valitsit asetettavaksi erityisiä käyttöoikeuksia. Oletko varma, että haluat jatkaa?",
	   "fr" => "Vous avez choisi de modifier les droits étendus, êtes-vous sûr de vouloir continuer ?",
	   "gl" => "Escolleu establecer permisos especiais, quere continuar?",
	   "hu" => "Biztos, hogy meg akarod változtatni ezeket a különleges felhasználói jogosultságokat?",
	   "it" => "Hai scelto di utilizzare dei permessi avanzati, sei sicuro di voler continuare?",
	   "lt" => "Jūs pasirinkote nustatyti ypatingus leidimus, ar tikrai norite tęsti?",
	   "nb" => "Du har valgt spesielle bits. Vil du virkelig fortsette?",
	   "nl" => "U heeft ervoor gekozen speciale toegangsrechten in te stellen, bent u zeker dat u verder wilt gaan?",
	   "nn" => "Du har valgt spesielle bits. Vil du verkeleg fortsette?",
	   "pl" => "Wybrałeś ustawienie specjalnych uprawnień. Czy na pewno kontynuować?",
	   "pt" => "Escolheu definir as permissões especiais, tem a certeza que quer continuar?",
	   "pt_PT" => "Escolheu definir as permissões especiais, tem a certeza que quer continuar?",
	   "ru" => "Вы решили установить особые разрешения, вы уверены, что хотите продолжить?",
	   "sl" => "Nastavili ste posebna dovoljenja, želite nadaljevati?",
	   "sr" => "Поставили сте посебне дозволе, желите ли заиста да наставите?",
	   "sr\@latin" => "Postavili ste posebne dozvole, želite li zaista da nastavite?",
	   "sv" => "Specialrättigheter är valda. Vill du verkligen fortsätta?",
	   "tr" => "Özel yetki ayarları yapmayı seçtiniz, devam etmek istediğinize emin misiniz?",
	   "xx" => "Your string, xx is the country abbreviation"
	);

########### DIALOG MESSAGES END #####################################

	$PERMTITLE = ($PERMTITLE{"$KDELANG"} or $PERMTITLE{"$KDELANGSHT"} or $PERMTITLE{"en_US"});
	$PERMMSG = ($PERMMSG{"$KDELANG"} or $PERMMSG{"$KDELANGSHT"} or $PERMMSG{"en_US"});
	$OWN = ($OWN{"$KDELANG"} or $OWN{"$KDELANGSHT"} or $OWN{"en_US"});
	$GRP = ($GRP{"$KDELANG"} or $GRP{"$KDELANGSHT"} or $GRP{"en_US"});
	$ALL = ($ALL{"$KDELANG"} or $ALL{"$KDELANGSHT"} or $ALL{"en_US"});
	$SPC = ($SPC{"$KDELANG"} or $SPC{"$KDELANGSHT"} or $SPC{"en_US"});
	$READ = ($READ{"$KDELANG"} or $READ{"$KDELANGSHT"} or $READ{"en_US"});
	$WRITE = ($WRITE{"$KDELANG"} or $WRITE{"$KDELANGSHT"} or $WRITE{"en_US"});
	$EXEC = ($EXEC{"$KDELANG"} or $EXEC{"$KDELANGSHT"} or $EXEC{"en_US"});
	$SUID = ($SUID{"$KDELANG"} or $SUID{"$KDELANGSHT"} or $SUID{"en_US"});
	$SGID = ($SGID{"$KDELANG"} or $SGID{"$KDELANGSHT"} or $SGID{"en_US"});
	$STICK = ($STICK{"$KDELANG"} or $STICK{"$KDELANGSHT"} or $STICK{"en_US"});
	$SPCTITLE = ($SPCTITLE{"$KDELANG"} or $SPCTITLE{"$KDELANGSHT"} or $SPCTITLE{"en_US"});
	$SPCMSG = ($SPCMSG{"$KDELANG"} or $SPCMSG{"$KDELANGSHT"} or $SPCMSG{"en_US"});
	return 0;
}


#Find out the language used in kde
sub get_kde_language {
        $KDELANG = $ENV{LANG};
	chomp($KDELANGSHT = substr("$KDELANG",0,2));
	return 0;
}

sub get_kde_path {
	if ( exists $ENV{KDEDIR} ) {
		$KDEBINPATH = "$ENV{KDEDIR}/bin/";
	}

	else {
		$KDEBINPATH = "";
	}
	
}

# Print error message if called without arguments (if the script is run directly)
if ( $#ARGV lt 0 ) {
   die "This script is not meant to be run directly. It will run with the necessary arguments when called from the .desktop files when a root action is selected by right-clicking item(s) in konqueror/dolphin.\n" ;
}

# Application checks
&get_kde_path ;

# Check whether kdesudo is installed, and use it instead of kdesu to get root privileges.
#if ( `which kdesudo` ) {
#	$SUCOMMAND = "$KDEBINPATH"."kdesudo -d --noignorebutton --"}
#elsif ( `which kdesu` ) {
#	$SUCOMMAND = "$KDEBINPATH"."kdesu -d -c"}
if ( `which pkexec` ) {
	$SUCOMMAND = "pkexec"}
else { $SUCOMMAND = "xdg-su -c"}

$DIALOGCOMMAND = "$KDEBINPATH"."kdialog" ;

# Check whether dbus-run-session (dbus 1.8.x) is available
$DBUSRUN = "" ;
if ( `which dbus-run-session` ) {
	$DBUSRUN = "dbus-run-session"}


#---Start root actions---
$EXECNAME = $0 ;
$ACTION = shift @ARGV ;
$TARGET = join("' '", @ARGV) ;
&{$ACTION} or die ;

#---Root konsole subroutines---
sub root_konsole_here {
	$APPNAME = shift @ARGV ;
	$WORKDIR = shift @ARGV ;
	exec "$SUCOMMAND \'$EXECNAME\' do_root_konsole \'$APPNAME\' \'$WORKDIR\'";
	exit $?;
}

sub do_root_konsole {
	if ( $> eq 0 ) {
	   #get root reminder string
	   &root_reminder_msgs;
	   #Get the terminal and working directory
	   $APPNAME = shift @ARGV ;
	   
	   if ( -x $KDEBINPATH.$APPNAME ) {
              $TERMINAL = $KDEBINPATH.$APPNAME ; }
           else {
              $TERMINAL = $APPNAME ; }
	   
	   $APPNAME =~ tr/a-z/A-Z/ ;
	   $WORKDIR = shift @ARGV ;
	   exec "$TERMINAL --nofork --workdir \'$WORKDIR\'" ;
	   exit $?;
	}
}
#---End root konsole subroutines---

#---Open with subroutines---
sub custom_open_with {
	#Get program
	&open_dialog_msgs;
	$APPNAME = `$DIALOGCOMMAND --title "$OPENTITLE" --inputbox "$OPENMSG" program` ;
	if ( $? eq 0 ) {
	   chomp $APPNAME ;
      	   exec "$SUCOMMAND \'$EXECNAME\' do_open_with \'$APPNAME\' \'$TARGET\'";
	   exit $?;
	}
}

sub open_with {
	$APPNAME = shift @ARGV ;
	$TARGET = join("' '", @ARGV) ;

	if ( $APPNAME eq "defaultfm" ) {
	     chomp($APPNAME = `xdg-mime query default inode/directory | sed s/.desktop// | sed s/kde4-//`);
	     if ( $APPNAME eq "kfmclient_dir" ) { $APPNAME = "konqueror"; }
	     #Check that file manager is available
	     system "which $APPNAME" ;
	     if (( $? ne 0 ) || ( $APPNAME eq "" )) { 
		if ( `which dolphin` ) { $APPNAME = "dolphin"; } #dolphin as fallback if available
		else { $APPNAME = "konqueror"; } 
	     }
	}
	elsif ( $APPNAME eq "defaultte" ) {
	     chomp($APPNAME = `xdg-mime query default text/plain | sed s/.desktop// | sed s/kde4-//`);
	     system "which $APPNAME" ;
	     if (( $? ne 0 ) || ( $APPNAME eq "" )) { 
		if ( `which kate` ) { $APPNAME = "kate"; } #kate as fallback if available
		else { $APPNAME = "kwrite"; } 
             }   
	}
	#Redundant check in KDE4 ("defaultte" should handle choosing the text editor), kept for KDE3 compatibility 
	elsif ( $APPNAME eq "kate" ) {
	      system "which kate" ;
	      if ( $? ne 0 ) {
		  $APPNAME = "kwrite"; }
	}

	exec "$SUCOMMAND \'$EXECNAME\' do_open_with \'$APPNAME\' \'$TARGET\'";
	exit $?; 
}

sub do_open_with {
	#If running as root, open the files
	if ( $> eq 0 ) {
	   #get root reminder string
	   &root_reminder_msgs;
	   #Which program? (first argument)
	   $APPNAME = shift @ARGV ;
	   
	   if ( -x $KDEBINPATH.$APPNAME ) {
              $CPROGRAM = $KDEBINPATH.$APPNAME ; }
           else {
              $CPROGRAM = $APPNAME ;}

	   $APPNAME =~ tr/a-z/A-Z/ ;
	   shift;
	   #Create Target file string
	   $TARGET = join("' '", @ARGV) ;
	   
	   # Fixed bug with qt5 applications not opening
           # because they don't provide the kde-options like "--caption" any more
           $OPTTEST = system "$CPROGRAM --help-kde" ;
           if ( $OPTTEST eq 0 ) {
              exec "$DBUSRUN $CPROGRAM --caption \"$APPNAME $ROOTREMINDER\" \'$TARGET\'" ; }
           else {
              exec "KDE_SESSION_VERSION=5 KDE_FULL_SESSION=true $DBUSRUN $CPROGRAM \'$TARGET\'" ; }
           exit $?;
        }
}
#---End open with subroutines---

#---Copy subroutines---
sub root_copy {
	
	   &copy_dialog_msgs;
	   $OLDNAME = $ARGV[0];
	   $NEWNAME = `$DIALOGCOMMAND --title "$COPYTITLE" --inputbox "$COPYMSG" \'$OLDNAME\'` ;
	   chop $NEWNAME;
	   if ( $? eq 0 && $OLDNAME ne $NEWNAME ) {
	      exec "$SUCOMMAND \'$EXECNAME\' do_copy \'$OLDNAME\' \'$NEWNAME\'";
	      exit $?;

	   }
}

sub do_copy {
	#If running as root, rename the file
	if ( $> eq 0 ) {
	   #get the arguments)
	   $OLDNAME = $ARGV[0] ;
	   $NEWNAME = $ARGV[1] ;
	#Check for kde-cp
	if ( `which kde-cp` ) {
	     if ( $CPDIALOGS ) { $COPYCOMMAND = "$KDEBINPATH"."kde-cp --"; }
	     else { $COPYCOMMAND = "$KDEBINPATH"."kde-cp --overwrite --noninteractive --"; }
	}
        else { $COPYCOMMAND = "cp -r"; }
	   `$COPYCOMMAND \'$OLDNAME\' \'$NEWNAME\'`;
	}
}
#---End copy subroutines---

#---Rename subroutines---
sub root_rename {
	# If more than one file is selected, and a batch rename application is installed, open it
	$BATCHRENAMER = shift @ARGV ;
	$TARGET = join("' '", @ARGV) ;
	chomp($RENAMERPATH = `which $BATCHRENAMER`);	
	if ( $#ARGV > 0 && -x $RENAMERPATH ) {
	   exec "$SUCOMMAND \'$EXECNAME\' do_open_with \'$BATCHRENAMER\' \'$TARGET\'";
	   exit $?;
	}
	# else we'll use a simple rename script
	else {
	   &rename_dialog_msgs;
	   $OLDNAME = $ARGV[0];
	   $NEWNAME = `$DIALOGCOMMAND --title "$RENAMETITLE" --inputbox "$RENAMEMSG" \'$OLDNAME\'` ;
	   chop $NEWNAME;
	   if ( $? eq 0 && $OLDNAME ne $NEWNAME ) {
	      exec "$SUCOMMAND \'$EXECNAME\' do_rename \'$OLDNAME\' \'$NEWNAME\'";
	      exit $?;
	   }
	}
}

sub do_rename {
	#If running as root, rename the file
	if ( $> eq 0 ) {
	   #get the arguments)
	   $OLDNAME = $ARGV[0] ;
	   $NEWNAME = $ARGV[1] ;
	#Check for kde-mv
	if ( `which kde-mv` ) {
	     if ( $CPDIALOGS ) { $MOVECOMMAND = "$KDEBINPATH"."kde-mv --"; }
	     else { $MOVECOMMAND = "$KDEBINPATH"."kde-mv --overwrite --noninteractive --"; }
	}
        else { $MOVECOMMAND = "mv"}
	   `$MOVECOMMAND \'$OLDNAME\' \'$NEWNAME\'`;
	}
}
#---End rename subroutines---

#---Compress subroutines---
sub root_compress {
	exec "$SUCOMMAND \'$EXECNAME\' do_compress \'$TARGET\'";
}

sub do_compress {
	#If running as root, compress
	if ( $> eq 0 ) {
	   `ark --add --changetofirstpath --dialog \'$TARGET\'` ;
	   exit 0;
	}

}
#---End compress subroutines---

#---Deletion subroutines---
sub root_delete {
	#Create a viewer friendly list of files to be deleted for the warning dialog
	$TARGETLIST = join("\\n", @ARGV);
	
	#Show warning dialog
	&delete_dialog_msgs;
	system "$DIALOGCOMMAND --title \'$DELETETITLE\' --warningcontinuecancel \'$DELETEMSG\\n$TARGETLIST\'" ;

	#Is deletion confirmed?
	if ( $? eq 0 ) {
	   # kdesu will run the command as regular user if 'Ignore' is chosen from kdesu dialog.
	   # To prevent unwanted deletion of files, we'll run 'do_delete' instead of 'rm -r', 'do delete'
	   # will exit if it's run as normal user, therefore the files writable for user are safe when clicking 'Ignore'
	   exec "$SUCOMMAND \'$EXECNAME\' do_delete \'$TARGET\'";
	}
}

sub do_delete {
	#If running as root, remove the files targeted
	if ( $> eq 0 ) {
	   `rm -r --preserve-root \'$TARGET\'` ;
	}
	exit 0;
}
#---End deletion subroutines---

#---Ownership subroutines---
sub root_ownership {
	#chown target files to root

	# If only one directory selected, ask whether to apply the changes recursively
	$RECURSIVE = "0" ;
	if ( $#ARGV < 1 && -d $ARGV[0]) {
		&recursion_dialog_msgs ;
	   	system "$DIALOGCOMMAND --title \'$RECURSIONTITLE\' --yesno \'$RECURSIONMSG\'";
	   	if ( $? eq 0 ) {
	   		$RECURSIVE = "1" ;
	   	}
        }

	# Same as with delete, we don't want kdesu to run 'chown' when 'Ignore' is pressed in the kdesu dialog, so we use 'do_ownership' instead
	exec "$SUCOMMAND \'$EXECNAME\' do_ownership \'$RECURSIVE\' 0:0 \'$TARGET\'";
	exit $?;
}

sub user_ownership {

	# If only one directory selected, ask whether to apply the changes recursively
	$RECURSIVE = "0" ;
	if ( $#ARGV < 1 && -d $ARGV[0]) {
		&recursion_dialog_msgs ;
	   	system "$DIALOGCOMMAND --title \'$RECURSIONTITLE\' --yesno \'$RECURSIONMSG\'";
	   	if ( $? eq 0 ) {
	   		$RECURSIVE = "1" ;
	   	}
        }

	#Create a list of user GIDs, so we can pick only the primary group
	@GROUPS = split ' ', $);
	exec "$SUCOMMAND \'$EXECNAME\' do_ownership \'$RECURSIVE\' $>:$GROUPS[0] \'$TARGET\'";
	exit $?;
}

sub custom_ownership {
	#Get custom UID:GID
	$RECURSIVE = "0" ;
	if ( $#ARGV < 1 && -d $ARGV[0]) {
		&recursion_dialog_msgs ;
	   	system "$DIALOGCOMMAND --title \'$RECURSIONTITLE\' --yesno \'$RECURSIONMSG\'";
	   	if ( $? eq 0 ) {
	   		$RECURSIVE = "1" ;
	   	}
		$? = "0" ;
        }

	&ownership_dialog_msgs;
	$UIDGID=`$DIALOGCOMMAND --title "$OWNERTITLE" --inputbox "$OWNERMSG" user:group` ;

	if ( $? eq 0 ) {
	   chop $UIDGID;
	   exec "$SUCOMMAND \'$EXECNAME\' do_ownership \'$RECURSIVE\' \'$UIDGID\' \'$TARGET\'";
	   exit $?;
	}
}

sub do_ownership {
	#If running as root, make the ownership changes to targeted files
	if ( $> eq 0 ) {
	   #get recursion choice (first argument)
	   $RECURSIVE = shift @ARGV ;
	   #get the UID:GID numbers (second argument)
	   $UIDGID = shift @ARGV ;
	   shift;
	   #Create Target list
	   $TARGET = join("' '", @ARGV) ;
	   if ( $RECURSIVE eq 1 ) {
		`chown --preserve-root -R \'$UIDGID\' \'$TARGET\'` ;
	   }
	   else {
		`chown --preserve-root \'$UIDGID\' \'$TARGET\'` ;
	   }
	}
}
#---End ownership subroutines---

#---Permissions subroutines---
sub root_permissions {
	#chmod target files to root
	$STOTAL = 0 ;
	$RECURSIVE = "0" ;
	# Get current permissions of target files
	@CURPERM = `ls -ld \'$TARGET\' | cut -d ' ' -f1`;
	$ORCUR = $OWCUR = $OXCUR = $GRCUR = $GWCUR= $GXCUR = $ARCUR = $AWCUR = $AXCUR = $SSCUR = $SGCUR = $STCUR = "on";
	# Analyze permissions, set to off if permission is not set, so only permissions that are present for all chosen files are on by default 
	foreach $CUR (@CURPERM) {
	    if (substr($CUR, 1, 1) eq "-") { $ORCUR = "off"; }
	    if (substr($CUR, 2, 1) eq "-") { $OWCUR = "off"; }
	    if (substr($CUR, 3, 1) eq "-") { $OXCUR = $SSCUR = "off"; }
	    elsif (substr($CUR, 3, 1) eq "x") { $SSCUR = "off"; }
	    elsif (substr($CUR, 3, 1) eq "S") { $OXCUR = "off"; }
	    if (substr($CUR, 4, 1) eq "-") { $GRCUR = "off"; }
	    if (substr($CUR, 5, 1) eq "-") { $GWCUR = "off"; }
	    if (substr($CUR, 6, 1) eq "-") { $GXCUR = $SGCUR = "off"; }
	    elsif (substr($CUR, 6, 1) eq "x") { $SGCUR = "off"; }
	    elsif (substr($CUR, 6, 1) eq "S") { $GXCUR = "off"; }
	    if (substr($CUR, 7, 1) eq "-") { $ARCUR = "off"; }
	    if (substr($CUR, 8, 1) eq "-") { $AWCUR = "off"; }
	    if (substr($CUR, 9, 1) eq "-") { $AXCUR = $STCUR = "off"; }
	    elsif (substr($CUR, 9, 1) eq "x") { $STCUR = "off"; }
	    elsif (substr($CUR, 9, 1) eq "T") { $AXCUR = "off"; }
	}

	#Check whether user wishes to change permissions recursively
	if ( $#ARGV < 1 && -d $ARGV[0]) {
		&recursion_dialog_msgs ;
	   	system "$DIALOGCOMMAND --title \'$RECURSIONTITLE\' --yesno \'$RECURSIONMSG\'";
	   	if ( $? eq 0 ) {
	   		$RECURSIVE = "1" ; }
	}

	if ( $RECURSIVE eq 1 ) {
		&permission_dialog_msgs;
		chomp($PERMLIST=`$DIALOGCOMMAND --title "$PERMTITLE" --checklist "$PERMMSG" u:r "$OWN $READ" $ORCUR u:w "$OWN $WRITE" $OWCUR u:X "$OWN $EXEC" $OXCUR g:r "$GRP $READ" $GRCUR g:w "$GRP $WRITE" $GWCUR g:X "$GRP $EXEC" $GXCUR o:r "$ALL $READ" $ARCUR o:w "$ALL $WRITE" $AWCUR o:X "$ALL $EXEC" $AXCUR u:s "$SPC $SUID" $SSCUR g:s "$SPC $SGID" $SGCUR o:t "$SPC $STICK" $STCUR`);

		if ( $? eq 0 ) {
		   #Calculate permission set for chmod
		   @PERMLIST = split(" ", $PERMLIST);
		   $USTR = "u=";
		   $GSTR = "g=";
		   $OSTR = "o=";
		   $STOTAL = "0" ;
		   foreach $PERM (@PERMLIST) {
			$UGO = substr($PERM, 1, 1);
			$VAL = substr($PERM, 3, 1);
			if ($VAL eq "s" || $VAL eq "t") {
				$STOTAL = "1" ; }
			if ($UGO eq "u") {
				$USTR = $USTR . $VAL; }
			elsif ($UGO eq "g") {
				$GSTR = $GSTR . $VAL; }
			else { $OSTR = $OSTR . $VAL; }
		   }
		   $CHMOD = "$USTR,$GSTR,$OSTR";
		}
	}
	else {		
		&permission_dialog_msgs;
		chomp($PERMLIST=`$DIALOGCOMMAND --title "$PERMTITLE" --checklist "$PERMMSG" u:4 "$OWN $READ" $ORCUR u:2 "$OWN $WRITE" $OWCUR u:1 "$OWN $EXEC" $OXCUR g:4 "$GRP $READ" $GRCUR g:2 "$GRP $WRITE" $GWCUR g:1 "$GRP $EXEC" $GXCUR o:4 "$ALL $READ" $ARCUR o:2 "$ALL $WRITE" $AWCUR o:1 "$ALL $EXEC" $AXCUR s:4 "$SPC $SUID" $SSCUR s:2 "$SPC $SGID" $SGCUR s:1 "$SPC $STICK" $STCUR`);

		if ( $? eq 0 ) {
		   #Calculate permission set for chmod
		   @PERMLIST = split(" ", $PERMLIST);
		   $UTOTAL = 0;
		   $GTOTAL = 0;
		   $OTOTAL = 0;
		   $STOTAL = 0;
		   foreach $PERM (@PERMLIST) {
			$UGO = substr($PERM, 1, 1);
			$VAL = substr($PERM, 3, 1);
			if ($UGO eq "u") {
			$UTOTAL = $UTOTAL + $VAL; }
			elsif ($UGO eq "g") {
			$GTOTAL = $GTOTAL + $VAL; }
			elsif ($UGO eq "o") {
			$OTOTAL = $OTOTAL + $VAL; }
			else { $STOTAL = $STOTAL + $VAL; }
		   }
		   $CHMOD = "$STOTAL$UTOTAL$GTOTAL$OTOTAL";
		}
	}

	if ($STOTAL ne 0 ) {
		system "$DIALOGCOMMAND --title \'$SPCTITLE\' --warningcontinuecancel \'$SPCMSG\'";
	}
	if ( $? eq 0 ) {
	      exec "$SUCOMMAND \'$EXECNAME\' do_permissions \'$RECURSIVE\' \'$CHMOD\' \'$TARGET\'";
	      exit $?;
	}
}
sub do_permissions {
	#If running as root, make the permission changes to targeted files
	if ( $> eq 0 ) {
	   #get the chmod number (first argument)
	   $RECURSIVE = shift @ARGV ;
	   $CHMOD = shift @ARGV ;
	   shift;
	   #Create Target list
	   $TARGET = join("' '", @ARGV) ;

	   if ( $RECURSIVE eq 1 ) {
		`chmod --preserve-root -R \'$CHMOD\' \'$TARGET\'` ;
	   }
	   else {
		`chmod --preserve-root \'$CHMOD\' \'$TARGET\'` ;
	   }
	}
}
#---End permissions subroutines---
