#!/bin/bash
URL='https://github.com/ChemicalDevelopment/ezc/releases/download/2.5.0/ezcc.deb'
FILE=`mktemp`
echo "URL: $URL"
echo "FILE: $FILE"
wget "$URL" -qO $FILE && sudo dpkg --force-architecture -i $FILE && echo "Installed"
sudo apt-get -f install
rm $FILE && echo "Removed file"

