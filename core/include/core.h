/*
 * core - Minimalist games engine
 * Copyright (C) 2006 Álvaro Vilanova Vidal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/**
 * @file core.h
 * Cabecera que une todos los includes de los archivos que forman parate del
 * core.
 **/


#ifndef CORE_H
#define CORE_H


/*
 * Inclusión de las cabeceras estándars
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <SDL/SDL.h>


/*
 * Definición de macros generales
 */

/* Modificador de llamada inline */
#ifndef __inline
    #define __inline
#endif

/* Macros de máximos y mínimos */
#ifndef MAX
    #define MAX(A, B) (((A) > (B)) ? (A) : (B))
#endif

#ifndef MIN
    #define MIN(A, B) (((A) < (B)) ? (A) : (B))
#endif

/* Unificación de random y macros de número aleatorios */
#ifndef random
    #define random rand
#endif

#ifndef srandom
    #define srandom srand
#endif

#ifndef RANDOM
    #define RANDOM(A, B)  (MIN(A, B) + (random() % (MAX(A, B) - MIN(A, B))))
#endif

/* Definición de mascaras de 32 bits */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define RMASK 0xff000000
    #define GMASK 0x00ff0000
    #define BMASK 0x0000ff00
    #define AMASK 0x000000ff
#else
    #define RMASK 0x000000ff
    #define GMASK 0x0000ff00
    #define BMASK 0x00ff0000
    #define AMASK 0xff000000
#endif



/*
 * Definición de tipos generales
 */

/** Dato que representa un punto (coordenadas enteras) */
typedef struct crePoint {
    /** Coordenadas X e Y */
    int X, Y;
} crePoint;


/*
 * Inclusión de archivos
 */

/*
 * Funciones gráficas.
 */
#include "gfx.h"

/*
 * Sistema central de gestión de procesos
 */
#include "process.h"

/*
 * Funciones de tratamiento de ficheros minGxf (mGx/mGf)
 */
#include "mingxf.h"

/*
 * Motor de tiles
 */
#include "tiler.h"

#endif
