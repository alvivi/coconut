# coconut - A nowadays Pacman remake.
# Copyright (C) 2006 Álvaro Vilanova Vidal
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

ifndef target
error:
	@echo ERROR: Debe compilar con make target=X
	@echo        (X puede ser linux o windows)
endif

ifeq ($(target), linux)
LIBS = -lSDL -lz -lSDL_ttf
BORRAR = rm
endif

ifeq ($(target), windows)
LIBS = -mwindows -lmingw32 -lzdll -lSDLmain -lSDL -lSDL_ttf
BORRAR = del
endif

#
# DIRECTORIOS DE LAS LIBRERIAS
#
ZLIB_LIB = -L./libs/zlib123/lib
TTF_LIB  = -Lc:/gnu/SDL-1.2.9/lib
SDL_LIB  = -L./libs/SDL-1.2.9/lib

#
# DIRECTORIOS DE LOS INCLUDES (NO TOCAR)
#
ZLIB_HEADERS = -I./libs/zlib123/include
SDL_HEADERS  = -I./libs/SDL-1.2.9/include
CORE_HEADERS = -I./core/include


#
# COMPILACIÓN DEL JUEGO
#
coconut : game.o misc.o libcore.a
	gcc -Wall ./coconut/src/main.c -o bin/coconut game.o misc.o libcore.a $(LIBS) $(CORE_HEADERS) $(CORE_HEADERS) $(SDL_HEADERS) $(ZLIB_HEADERS) $(SDL_LIB) $(TTF_LIB) $(ZLIB_LIB)
	$(BORRAR) *.o libcore.a
	@echo Ejecutable creado en el subdirectorio ./bin

misc.o : ./coconut/src/misc.c
	gcc -Wall -c ./coconut/src/misc.c -o misc.o $(CORE_HEADERS) $(SDL_HEADERS) $(ZLIB_HEADERS)

game.o : ./coconut/src/game.c
	gcc -Wall -c ./coconut/src/game.c -o game.o $(CORE_HEADERS) $(SDL_HEADERS) $(ZLIB_HEADERS)

#
# COMPILACIÓN DEL CORE
#
libcore.a : gfx.o tiler.o mingxf.o proccess.o
	ar rcs ./libcore.a gfx.o tiler.o mingxf.o proccess.o

proccess.o : ./core/src/process.c
	gcc -Wall -c ./core/src/process.c -o proccess.o $(CORE_HEADERS) $(SDL_HEADERS)

mingxf.o : ./core/src/mingxf.c
	gcc -Wall -c ./core/src/mingxf.c -o mingxf.o $(CORE_HEADERS) $(SDL_HEADERS) $(ZLIB_HEADERS)

tiler.o : ./core/src/tiler.c
	gcc -Wall -c ./core/src/tiler.c -o tiler.o $(CORE_HEADERS) $(SDL_HEADERS) $(ZLIB_HEADERS)

gfx.o : ./core/src/gfx.c
	gcc -Wall -c ./core/src/gfx.c -o gfx.o $(CORE_HEADERS) $(SDL_HEADERS) $(ZLIB_HEADERS)
