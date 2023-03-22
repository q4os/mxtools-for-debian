#/bin/bash
#part of mx-packageinstaller-pkglist
#install-master-pdf-editor.sh

TEMPFILE="/tmp/mdpftemp.text"

#wget website to temp file
wget https://code-industry.net/free-pdf-editor/ -O $TEMPFILE

#process file to get url

URL=$(cat $TEMPFILE)
URL=${URL#*Debian}
URL=${URL#*Debian}
URL=${URL#*<a href\=}
URL=$(echo $URL|cut -d">" -f1|xargs) 

rm $TEMPFILE

# get deb
echo $URL
wget "$URL" -O /tmp/master_pdf.deb

apt install /tmp/master_pdf.deb
rm /tmp/master_pdf.deb
