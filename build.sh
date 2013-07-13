#!/bin/bash

OLD_FOLDER=$(pwd)
MY_FOLDER=$(dirname $0)

# fetch and unpack mircryption source
cd $MY_FOLDER
./3rdparty/fetch-mcps.sh

# setup base build environment
rm -rf build
mkdir build
cd build
cp -r ../3rdparty/MircryptionExtras/FullSources/mircryption/src .
cd src/xchat

# patch build environment
cp ../../../mircryption.cpp .
cp ../../../Makefile .
cp -r ../../../dh1080 .

make $1 && cp mircryption.so ../../
