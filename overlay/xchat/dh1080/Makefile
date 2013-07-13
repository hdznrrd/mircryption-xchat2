LDFLAGS=-g -fPIC -lssl -lcrypto
CFLAGS=-g -fPIC -x c++
CXXFLAGS=${CFLAGS}
CC=g++


all: dhtest dh1080.o b64stuff_static.o

clean:
	rm -f *.o dhtest


dhtest: dh1080.o b64stuff_static.o
	${CC} ${LDFLAGS} dhtest.cpp dh1080.o b64stuff_static.o -o dhtest

dh1080.o: dh1080.cpp
b64stuff_static.o: b64stuff_static.cpp b64stuff_static.h
