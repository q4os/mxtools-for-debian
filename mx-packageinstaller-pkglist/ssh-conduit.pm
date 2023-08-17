<?xml version="1.0" encoding="UTF-8"?>
<app>

<category>
Remote Access
</category>

<name>
ssh Conduit
</name>

<description>
   <am>A simple way to use remote resources via an ssh encypted connection</am>
   <ar>A simple way to use remote resources via an ssh encypted connection</ar>
   <bg>A simple way to use remote resources via an ssh encypted connection</bg>
   <bn>A simple way to use remote resources via an ssh encypted connection</bn>
   <ca>Mètode senzill per usar recursos remots mitjançant connexions encriptades SSH</ca>
   <cs>A simple way to use remote resources via an ssh encypted connection</cs>
   <da>En simpel måde at bruge fjernressourcer via en ssh-krypteret forbindelse</da>
   <de>Auf einfache Art und Weise auf entfernte Rechnern über verschlüsselte SSH-Verbindungen zuzugreifen.</de>
   <el>Απλός τρόπος για τη χρήση απομακρυσμένων πόρων μέσω κρυπτογραφημένης σύνδεσης ssh</el>
   <en>A simple way to use remote resources via an ssh encypted connection</en>
   <es>Una forma sencilla de usar recursos remotos a través de una conexión cifrada ssh</es>
   <et>A simple way to use remote resources via an ssh encypted connection</et>
   <eu>A simple way to use remote resources via an ssh encypted connection</eu>
   <fa>A simple way to use remote resources via an ssh encypted connection</fa>
   <fil_PH>A simple way to use remote resources via an ssh encypted connection</fil_PH>
   <fi>Helppo tapa käyttää etäresursseja ssh-salatun yhteyden kautta</fi>
   <fr>Utilisation simple de ressources à distance au moyen d’une connexion cryptée ssh</fr>
   <he_IL>A simple way to use remote resources via an ssh encypted connection</he_IL>
   <hi>एसएसएच एन्क्रिप्टेड कनेक्शन द्वारा दूरस्थ सामग्री उपयोग करने का सरल मार्ग</hi>
   <hr>A simple way to use remote resources via an ssh encypted connection</hr>
   <hu>A simple way to use remote resources via an ssh encypted connection</hu>
   <id>A simple way to use remote resources via an ssh encypted connection</id>
   <is>A simple way to use remote resources via an ssh encypted connection</is>
   <it>Un modo semplice per usare risorse remote attraverso una connessione ssh criptata</it>
   <ja>ssh で暗号化された接続を介してリモートリソースを使用する簡単な方法</ja>
   <kk>A simple way to use remote resources via an ssh encypted connection</kk>
   <ko>A simple way to use remote resources via an ssh encypted connection</ko>
   <lt>A simple way to use remote resources via an ssh encypted connection</lt>
   <mk>A simple way to use remote resources via an ssh encypted connection</mk>
   <mr>A simple way to use remote resources via an ssh encypted connection</mr>
   <nb>Enkel tilgang til fjernressurser via SSH-kryptert tilkobling</nb>
   <nl>Een simpele manier om bronnen op afstand e gebruiken middels een ssh versleutelde verbinding</nl>
   <pl>prosty sposób korzystania ze zdalnych zasobów za pośrednictwem zaszyfrowanego połączenia ssh</pl>
   <pt_BR>Um jeito simples de acessar recursos remotos através de uma conexão criptografada/encriptada com o protocolo SSH (Secure Shell)</pt_BR>
   <pt>Uma forma simples de usar recursos remotos através duma ligação ssh encriptada</pt>
   <ro>A simple way to use remote resources via an ssh encypted connection</ro>
   <ru>Лёгкий способ использовать удаленные ресурсы через шифрованное соединение ssh</ru>
   <sk>A simple way to use remote resources via an ssh encypted connection</sk>
   <sl>Preprost način za rabo oddaljenih virov preko ssh šifrirane povezave</sl>
   <sq>Një rrugë e thjeshtë për të përdorur burime së largëti përmes një lidhjeje ssh të fshehtëzuar</sq>
   <sr>A simple way to use remote resources via an ssh encypted connection</sr>
   <sv>Ett enkelt sätt att använda fjärresurser via en ssh-krypterad anslutming</sv>
   <tr>SSH şifreli bağlantı üzerinden uzak kaynakları kullanmanın basit bir yolu</tr>
   <uk>A simple way to use remote resources via an ssh encypted connection</uk>
   <vi>A simple way to use remote resources via an ssh encypted connection</vi>
   <zh_CN>A simple way to use remote resources via an ssh encypted connection</zh_CN>
   <zh_TW>A simple way to use remote resources via an ssh encypted connection</zh_TW>
</description>

<installable>
all
</installable>

<screenshot>none</screenshot>

<preinstall>
apt-get install antix-archive-keyring
echo "deb http://la.mxrepo.com/antix/bullseye bullseye main">/etc/apt/sources.list.d/mxpitemp.list
apt-get update
</preinstall>

<install_package_names>
ssh-conduit-antix
</install_package_names>


<postinstall>
rm /etc/apt/sources.list.d/mxpitemp.list
apt-get update
</postinstall>


<uninstall_package_names>
ssh-conduit-antix
</uninstall_package_names>
</app>
