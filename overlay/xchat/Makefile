#############################################################################
# SEE BELOW FOR DIFFERENT CHANGES TO MAKE FOR MACOSX OR UNIX/LINUX/PC
# updated 07/11/03 to support macosx options (thanks darkfiber)
# updated 09/17/03 to offer some alternative file link locations
# updated 11/10/03 to better explain link options, and modify freebsd to use altlink option
# updated 1/26/04 with mirc color code source file added
# updated 3/24/04 added include directory ./ (from SUSE linux text)
# updated 8/30/04 added support for -fPIC option on nix64 target
# updated 01/8/05 added new blowfish cbc files
#############################################################################


#############################################################################
CC = gcc
LOAD = gcc
#############################################################################


#############################################################################
# PLATFORM DEPENDENT FLAGS

CFLAGS_MAC = "-c -x c++ -DMACOSX"
LDFLAGS_MAC = " -Wl -dynamiclib"
CFLAGS_GENERIC = "-c -x c++"
CFLAGS_GENERIC64 = "-c -x c++ -fPIC"
LDFLAGS_GENERIC = "-shared -Wl,-soname,mircryption.so -lssl -lcrypto"
LDFLAGS_GENERIC64 = "-shared -fPIC -Wl,-soname,mircryption.so -lssl -lcrypto"
CFLAGS2_BIGENDIAN = "-DBIG_ENDIAN"
CFLAGS2_LITTLENDIAN = "-DLITTLE_ENDIAN"
#############################################################################


#############################################################################
# PLATFORM TARGETS

all: .DEFAULT

.DEFAULT:
	@echo "To Build mircryption, type 'make TARGET' (generic | linux | nix64bit | freebsd | macosx | macosxintel | windows | altlink | altlink2)"
	@echo " or type: 'make clean' to clean .o and .so"
	@echo " or type: 'make install' to try to install the mircryption.so dll into ~/.xchat2"
	@echo "try altlink1 or altlink2 if your linker complains about not being able to find the .o files"
	@echo "    altlink1 may be good for freebsd"	
macosx:
	$(MAKE) _mircryption $(MACFLAGS) CFLAGS=$(CFLAGS_MAC) LDFLAGS=$(LDFLAGS_MAC) CFLAGS2=$(CFLAGS2_BIGENDIAN)
macosxintel:
	$(MAKE) _mircryption $(MACFLAGS) CFLAGS=$(CFLAGS_MAC) LDFLAGS=$(LDFLAGS_MAC)
unix linux windows generic:
	$(MAKE) _mircryption $(GENERICFLAGS) CFLAGS=$(CFLAGS_GENERIC) LDFLAGS=$(LDFLAGS_GENERIC)
nix64bit:
	$(MAKE) _mircryption $(GENERICFLAGS) CFLAGS=$(CFLAGS_GENERIC64) LDFLAGS=$(LDFLAGS_GENERIC64)
altlink freebsd:
	$(MAKE) _mircryption_altlink $(GENERICFLAGS) CFLAGS=$(CFLAGS_GENERIC) LDFLAGS=$(LDFLAGS_GENERIC)
altlink2:
	$(MAKE) _mircryption_altlink2 $(GENERICFLAGS) CFLAGS=$(CFLAGS_GENERIC) LDFLAGS=$(LDFLAGS_GENERIC)
#############################################################################




#############################################################################
SOURCE = ../mircryptionclass.cpp ../md5class.cpp ../md5c.cpp ../mc_blowfish.cpp ../b64stuff.cpp ../oldblowfish.cpp ../newblowfish.cpp ../BlowfishCbc.cpp mircryption.cpp mirc_codes.cpp ./dh1080/b64stuff_static.cpp ./dh1080/dh1080.cpp
OBJECTS = ../mircryptionclass.o ../md5class.o ../md5c.o ../mc_blowfish.o ../b64stuff.o ../oldblowfish.o ../newblowfish.o ../BlowfishCbc.o mircryption.o mirc_codes.o ./dh1080/b64stuff_static.o ./dh1080/dh1080.o
INCDIRS = -I../ -I./ -I./dh1080/

%.o : %.cpp
	@echo Compiling $*
	$(CC) $(CFLAGS) $(CFLAGS2) $(INCDIRS) $*.cpp -o $*.o

_mircryption: $(OBJECTS)
	@echo "Linking mircryption library..."
# old link statement was incompatible with some platforms:  $(LOAD) $(LDFLAGS) -shared -Wl,-soname,mircryption.so -o mircryption.so $(OBJECTS) -lstdc++
	$(LOAD) $(LDFLAGS) -o mircryption.so *.o ../*.o ./dh1080/*.o -lstdc++

_mircryption_altlink: $(OBJECTS)
	@echo "Linking mircryption library..."
# this is for linkers (freebsd?) which put all the .o in the current directory
	$(LOAD) $(LDFLAGS) -o mircryption.so *.o -lstdc++

_mircryption_altlink2: $(OBJECTS)
	@echo "Linking mircryption library..."
	$(LOAD) $(LDFLAGS) -shared -Wl,-soname,mircryption.so -o mircryption.so $(OBJECTS) -lstdc++
	
install:
	@echo "Trying to install mircryption.so into ~/.xchat2 (see readme.txt for more info)"
	cp mircryption.so ~/.xchat2

clean:
	@echo Deleting intermediate files
	rm -f *.so
	rm -f *.o
	rm -f ../*.o
	rm -f ./dh1080/*.o
#############################################################################
