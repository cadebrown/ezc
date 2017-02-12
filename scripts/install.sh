#!/bin/sh
#SOURCES=`echo *.py */`
cd src
SOURCES=`find ./ -type f \( -iname \*.c -o -iname \*.py  -o -iname \*.ezc  -o -iname \*.h \)`
cd ..

#echo $SOURCES

for SRC in $SOURCES; do
	echo $SRC
done

INSTALL_DIR=$1
SRC_DIR=$2
I_MPFR=$3
KEEP_MPFR=$4

mkdir -p $SRC_DIR
mkdir -p $INSTALL_DIR

PLATFORM=$(./scripts/platform.sh)


echo Operating System Type: $PLATFORM

EZC_BIN=$"#!/bin/sh \npython \$(dirname \$0)/src/ezcc.py \"\${@}\" \n"

echo $EZC_BIN

if [ "$KEEP_MPFR" != "true" ]; then
	if [ "$I_MPFR" = "true" ]; then
		echo "Going to build MPFR"
	else
		# Install dependencies
		if [ "$PLATFORM" = "linux" ]; then
			if [ $(cat /etc/debian_version) ]; then
				echo "Asking for sudo to install packages . . ."
				sudo apt-get install libmpfr-dev
			elif [ $(cat /etc/fedora-release) ]; then
				echo "Asking for sudo to install packages . . ."
				sudo dnf install mpfr-devel
			fi

		elif [ "$PLATFORM" = "mac" ]; then 
			echo "Asking for sudo to install packages . . ."
			sudo brew install gcc48
			sudo brew install mpfr 
		elif [ "$PLATFORM" = "bsd" ]; then 
			echo "Asking for su to install packages . . ."
			su -c "pkg install mpfr"
		else
			echo "Warning: OS not found."
			echo "Building MPFR from source"
			I_MPFR="true"
		fi
	fi

	if [ "$I_MPFR" = "true" ]; then
		echo "Installing MPFR from source"
		./scripts/req.sh $SRC_DIR
		FROM=$PWD
		cd $SRC_DIR
		rm -Rf share/
		cd $FROM
	fi
fi

echo Installing sources in $SRC_DIR


for SRC in $SOURCES
do
	parentname="$(basename "$(dirname "$SRC")")"
	mkdir -p $SRC_DIR/$parentname
	cp ./src/$SRC $SRC_DIR/$SRC
done

./scripts/copyutils.sh $INSTALL_DIR
cp ./scripts/makeutils.sh $INSTALL_DIR/utils.sh

echo $EZC_BIN > $INSTALL_DIR/ezc
echo $EZC_BIN > $INSTALL_DIR/ezcc

chmod +x $INSTALL_DIR/ezc
chmod +x $INSTALL_DIR/ezcc

if [ "$I_MPFR" = "true" ] || [ "$KEEP_MPFR" = "true" ]; then
	printf "\nEZC_LIB=\"-w -I%%s/include/ %%s/lib/libmpfr.a %%s/lib/libgmp.a\" %% (EZC_DIR, EZC_DIR, EZC_DIR)\n" >> $SRC_DIR/ezdata.py
fi

echo Removing uneeded dirs

FROM=$PWD
cd $INSTALL_DIR
rm -Rf share/
rm a.out
cd $FROM

cd $SRC_DIR
DTFMT=`date +'%Y-%m-%d %H:%M:%S %z'`
printf "\ndate=\"$DTFMT\"" >> ezlogging/log.py
cd $FROM

echo "If you got any permissions errors, please open an issue: https://github.com/ChemicalDevelopment/ezc/issues"

echo "Done"


