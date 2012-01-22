/*
 * core - Minimalist games engine
 * Copyright (C) 2006 �lvaro Vilanova Vidal
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
 * @file tiler.h
 * Contiene todas las definiciones de tratamiento de ficheros escenario (mapas
 * de tiles). Tanto las de gesti�n de ficheros como las gr�ficas.
 **/


#ifndef CORE_TILER_H
#define CORE_TILER_H

#include <SDL/SDL.h>
#include <core.h>

/*
 * Definici�n de tipos
 */

 /**
  * Estructura que contiene la informaci�n de un escenario de Tiles
  **/
 typedef struct creMSc {
     /** Tama�o de los tiles (en pixels) */
     int Size;
     /** Ancho y alto del escenario (en tiles) */
     int W, H;
     /** Nombre del skin que utiliza */
     char Skin[32];
     /** N�mero de puntos clave */
     int KPCount;
     /** Vector que contiene los identificadores de los tiles */
     char * Map;
     /** Puntos clave del escenario (a tratar por la aplicaci�n) */
     crePoint * KeyPoints;
 } creMSc;


/*
 * Declaraci�n de funciones
 */

/**
 * Funci�n que dado el nombre de un archivo escenario lo carga en un variable
 * creMSc para poder ser utilizada.
 * @brief Carga un escenario, mapa de tiles.
 * @param FileName Ruta y nombre del archivo.
 * @return Puntero al escenario, o NULL si no ha sido posible cargarlo.
 **/
extern creMSc * CRE_LoadMSc(char * FileName);

/**
 * Guarda el escenario
 * @brief Guarda un escenario.
 * @param Src Puntero al escenario a guardar
 * @param FileName Ruta y nombre del archivo donde guardarlo.
 * @return 0 si todo ha sido correcto, -1 en caso contrario.
 **/
extern int CRE_SaveMSc(creMSc * Src, char * FileName);

/**
 * Libera de la memoria un escenario, se pierden todos los datos no guardados.
 * @param Src Escenario ha eliminar.
 * @brief Elimina un escenario.
 **/
extern void CRE_FreeMSc(creMSc * Src);

/**
 * Dibuja el mapa de tiles indicado de un escenraio en el gr�fico que se indique
 * y con los gr�ficos del skin que indiquen.
 * @brief Dibuja un escenraio en la superficie indicada
 * @param Src Escenario a dibujar.
 * @param Trg Superficie del gr�fico donde dibujar
 * @param Gfxs Gr�fico a utilizar para dibujar el escenario (Skin)
 * @return 0 si todo ha sido correcto, -1 en caso contrario.
 **/
extern int CRE_DrawMScToSurface(creMSc * Src, SDL_Surface * Trg, creMGf * Gfxs);

/**
 * Dibuja el mapa de tiles indicado de un escenario en un gr�fico, el cual es
 * devuelto por la funci�n.
 * @brief Dibuja un escenario.
 * @param Src Escenario a dibujar.
 * @return El gr�fico del escenario, o NULL si ha ocurrido alg�n error.
 **/
extern SDL_Surface * CRE_DrawMSc(creMSc * Src);

#endif
