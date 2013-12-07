#!/bin/bash

base=$(pwd)

if [ "."$1 == "." ]; then
	echo "Please enter the platform make target to be passed to the mircryption make target"
	exit 1
fi

# fetch and unpack mircryption source
./3rdparty/fetch-mcps.sh

if [ "."$2 != "." ]; then
	./3rdparty/fetch-openssl.sh $2
fi

# setup base build environment
cd $base
rm -rf build
mkdir build
cd build
cp -r ../3rdparty/MircryptionExtras/FullSources/mircryption/src .
cd src

# patch build environment
cp -r ../../overlay/* .
if [ "."$2 != "." ]; then
	cp -r ../../overlay-patch-customopenssl/* .
fi

if [ "."$2 != "." ]; then
# build openssl
	cd $base/3rdparty/openssl-active
	pwd
	./config --prefix=$base/build/openssl
	make && make install
fi

# make it so!
cd $base/build/src/xchat
make $1 && cp mircryption.so ../../
