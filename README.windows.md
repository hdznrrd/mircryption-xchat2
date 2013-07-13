# Windows Build Instructions
Basic changes in 0.4.1 to build against [Win32OpenSSL](http://www.slproweb.com/products/Win32OpenSSL.html) were contributed by [dev00](http://dev00.pytalhost.com/).

You will need to download Win32OpenSSL yourself and place the include and library files into the .\extras\openssl subfolder of the xchat folder.

The automatic build.sh script does not support Windows, so you will also have to fetch the Mircryption sources (see: README.md) yourself and overwrite the official files with those in the overlay folder.
