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
cd src

# patch build environment
cp -r ../../overlay/* .

# make it so!
cd xchat
make $1 && cp mircryption.so ../../
