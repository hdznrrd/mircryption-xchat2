#!/bin/bash

OLD_FOLDER=$(pwd)
MY_FOLDER=$(dirname $0)

cd $MY_FOLDER

if [ ! -d MircryptionExtras ]
then
	rm -rf MircryptionExtras*
	wget http://www.donationcoder.com/Software/Mouser/mircryption/downloads/MircryptionExtras.zip
	unzip MircryptionExtras.zip
fi

cd $OLD_FOLDER
