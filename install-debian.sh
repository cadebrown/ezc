#!/bin/bash
URL='https://github.com/ChemicalDevelopment/ezc/releases/download/2.3.0/ezcc_amd64.deb'
FILE=`mktemp`
echo "URL: $URL"
echo "FILE: $FILE"
wget "$URL" -qO $FILE && sudo dpkg -i $FILE && echo "Installed"
sudo apt-get -f install
rm $FILE && echo "Removed file"

