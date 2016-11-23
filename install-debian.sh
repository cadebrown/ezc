#!/bin/bash
URL='https://github.com/ChemicalDevelopment/ezc/releases/download/2.2.5/ezcc_amd64.deb'
FILE=`mktemp` 
wget "$URL" -qO $FILE && sudo dpkg -i $FILE && echo "Installed"
rm $FILE

