#
# Makefile for installing the Mingw32 version of the SDL library

CWD := $(shell pwd)
CROSS_PATH := /usr/local/cross-tools/i386-mingw32msvc/

all:
	@echo "Type \"make native\" to install to /usr"
	@echo "Type \"make cross\" to install to $(CROSS_PATH)"

native:
	make install prefix=/usr

cross:
	make install prefix=$(CROSS_PATH)

install:
	cp -rv bin include lib share $(prefix)/
	@ln -sf i386-mingw32msvc-sdl-config $(prefix)bin/sdl-config

dist:
	@test -d bin || mkdir bin
	@(cd $(CROSS_PATH) && \
		cp -v bin/i386-mingw32msvc-sdl-config bin/SDL.dll $(CWD)/bin/)
	@test -d include || mkdir include
	@(cd $(CROSS_PATH) && \
		cp -av include/SDL $(CWD)/include/)
	@test -d lib || mkdir lib
	@(cd $(CROSS_PATH) && \
		cp -v lib/libSDL.a lib/libSDL.dll.a lib/libSDL.la lib/libSDLmain.a  $(CWD)/lib/)
	@test -d share/aclocal || mkdir -p share/aclocal
	@(cd $(CROSS_PATH) && \
		cp -v share/aclocal/sdl.m4 $(CWD)/share/aclocal)
