#!/bin/sh

cd src
SOURCES=`find ./ -type f \( -iname \*.c -o -iname \*.py  -o -iname \*.ezc  -o -iname \*.h \)`
cd ..

INSTALL_DIR=$1
SRC_DIR=$2
I_MPFR=$3
KEEP_MPFR=$4

mkdir -p $SRC_DIR
mkdir -p $INSTALL_DIR

PLATFORM=$(./scripts/platform.sh)
ARCHSTR=$(uname -m)
VERSION=$(cat VERSION)


echo "Found platform: $PLATFORM"

EZC_BIN="#!/bin/sh \npython \$(dirname \$0)/src/ezcc.py \"\${@}\" \n"


if [ "$KEEP_MPFR" != "true" ]; then
	if [ "$I_MPFR" != "true" ]; then
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


for SRC in $SOURCES
do
	parentname="$(basename "$(dirname "$SRC")")"
	mkdir -p $SRC_DIR/$parentname
	cp ./src/$SRC $SRC_DIR/$SRC
	sed -i -e "s/@VERSION@/$VERSION/g" $SRC_DIR/$SRC
	sed -i -e "s/@ARCH@/$ARCHSTR/g" $SRC_DIR/$SRC
	sed -i -e "s/@PLATFORM@/$PLATFORM/g" $SRC_DIR/$SRC
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

FROM=$PWD
cd $INSTALL_DIR
rm -Rf share/
rm a.out
cd $FROM

cd $SRC_DIR
DTFMT=`date +'%Y-%m-%d %H:%M:%S %z'`
printf "\ndate=\"$DTFMT\"" >> ezlogging/log.py
cd $FROM


