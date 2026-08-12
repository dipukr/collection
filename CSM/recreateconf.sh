#!/bin/sh

unalias rm

if [ -d ./autom4te.cache ]; then
rm -rf ./autom4te.cache
fi

if [ -f ./Makefile ]; then
rm ./Makefile
fi

if [ -f ./aclocal.m4 ]; then
rm ./aclocal.m4
fi

if [ -f ./config.h.in ]; then
rm ./config.h.in
fi

if [ -f ./config.h ]; then
rm ./config.h
fi

if [ -f ./config.status ]; then
rm ./config.status
fi

if [ -f ./config.cache ]; then
rm ./config.cache
fi

if [ -f ./configure ]; then
rm ./configure
fi

if [ -f ./Makefile.in ]; then
rm ./Makefile.in
fi

aclocal
autoconf
autoheader
automake --foreign -a -c
./configure
