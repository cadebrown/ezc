#!/bin/bash
URL='https://github.com/ChemicalDevelopment/ezc/releases/download/2.2.5/ezcc_2.2.5_amd64.deb'
echo "URL: $URL"
FILE=`mktemp` 
wget "$URL" -qO $FILE && sudo dpkg -i $FILE && echo "Installed"
echo "Removing file"
rm $FILE

