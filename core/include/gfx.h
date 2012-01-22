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
 * @file gfx.h
 * Definición de las funciones de tratamiento de gráficos. Básicamente la
 * funciones incluidas son para aplicar el efecto de zoom y rotación (Por
 * A. Schiffler) y tratamiento de transparencias.
 **/


#ifndef CORE_GFX_H
#define CORE_GFX_H

#include <SDL/SDL.h>
#include <math.h>

#ifndef M_PI
#define M_PI	3.141592654
#endif

#include <SDL/SDL.h>


/*
 * Definición de macros
 */

/* Indican si se debe activar el filtro de suavizado */
/** Indica que el filtro de suavizado no debe aplicarse */
#define CRE_GFX_SMOOTH_OFF 0
/** Indica que el filtro de suavizado debe aplicarse */
#define CRE_GFX_SMOOTH_ON 1


/*
 * Definición de tipos
 */


/** Estructura que representa un color en formato 32 bits RGBA */
typedef struct creGfxColorRGBA {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} creGfxColorRGBA;


/** Estructura que representa un color en formato de 8 bits */
typedef struct creGfxColorY {
	Uint8 y;
} creGfxColorY;


/**
 * Aplica un blit entre dos superficies con el canal alpha indicado.
 * Si el gráfico es de 32 bits, hará un blit pixel a pixel y calcular las
 * diferencias entre los canales alpha indicados.
 **/
extern int CRE_GfxAlphaBlit(SDL_Surface * Src, SDL_Surface * Trg,
    SDL_Rect * Rect, Uint8 Alpha);

/**
 * Aplica a un gráfico un zoom y una rotación.
 * @author A. Schiffler
 **/
extern SDL_Surface * CRE_GfxRZSurface(SDL_Surface * src, double angle,
    double zoom, int smooth);

/**
 * Aplica a un gráfico un zoom con distintos niveles de X/Y y una rotación.
 * @author A. Schiffler
 **/
extern SDL_Surface * CRE_GfxRZSurfaceXY(SDL_Surface * src, double angle,
    double zoomx, double zoomy, int smooth);

/**
 * Devuelve el tamaño de un gráfico rotado y ampliado.
 * @author A. Schiffler
 **/
extern void CRE_GfxRZSurfaceSize(int width, int height, double angle,
    double zoom, int *dstwidth, int *dstheight);

/**
 * Devuelve el tamaño de un gráfico rotado y ampliado con distintos niveles de
 * escalado X/Y
 * @author A. Schiffler
 **/
extern void CRE_GfxRZSurfaceSizeXY(int width, int height, double angle,
    double zoomx, double zoomy, int *dstwidth, int *dstheight);

/**
 * Aplica un Zoom a una imagen con distintos niveles de X/Y
 * @author A. Schiffler
 **/
extern SDL_Surface *CRE_GfxZSurface(SDL_Surface * src, double zoomx,
    double zoomy, int smooth);

/**
 * Devuelve el tamaño de un gráfico ampliado.
 * @author A. Schiffler
 **/
extern void CRE_GfxZSurfaceSize(int width, int height, double zoomx,
    double zoomy, int *dstwidth, int *dstheight);


#endif
