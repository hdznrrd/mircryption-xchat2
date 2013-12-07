#!/bin/bash

OLD_FOLDER=$(pwd)
MY_FOLDER=$(dirname $0)

OPENSSL_VERSION=$1
if [ "."$OPENSSL_VERSION == "." ]; then
	OPENSSL_VERSION="1.0.1e"
	echo "Defaulting to OpenSLL v$OPENSSL_VERSION"
fi

echo "Downloading OpenSSL v$OPENSSL_VERSION"

cd $MY_FOLDER

if [ ! -d openssl-$OPENSSL_VERSION ]
then
	rm -rf openssl-$OPENSSL_VERSION*
	wget http://www.openssl.org/source/openssl-$OPENSSL_VERSION.tar.gz
	tar xzvf openssl-$OPENSSL_VERSION.tar.gz
else
	echo "OpenSSL v$OPENSSL_VERSION already downloaded"
fi

cd $OLD_FOLDER
