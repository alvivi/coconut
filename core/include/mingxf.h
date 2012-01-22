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
 * @file mingxf.h
 * Define todas las funciones de tratamiento de ficheros mGx y mGf, los cuales
 * se utilizan para guardar imágene y ficheros de imágenes respectivamente.
 **/


#ifndef CORE_MINGXF_H
#define CORE_MINGXF_H

#include <SDL/SDL.h>
#include <zlib.h>


/*
 * Definición de tipos
 */

/* Definición de un fichero de gráficos */
typedef struct creMGf {
    SDL_Surface ** Gfx;
     Uint32 Size;
} creMGf;


/*
 * Definición de funciones
 */

/**
 * @brief Guarda una imagen en un stream en formato mGx
 * @param Src Superficie SDL del gráfico a guardar
 * @param File Stream de datos donde guardar el archivo
 * Esta función guarda una imágen dada en formato SDL en un flujo de datos
 * también dado en el formato de los archivos mGx.
 * @return 0 si el gráfico a sido guardado con éxito, -1 en caso contrario.
 **/
extern int CRE_SaveMGxToStream(SDL_Surface * Src, gzFile * File);

/**
 * @brief Guarda una imagen en un archivo en formato mGx
 * @param Src Superficie SDL del gráfico a guardar
 * @param FileName Ruta del archivo donde guardar la imágen
 * Esta función guarda una imágen dada en formato SDL en un archivo cuya ruta
 * se pasa por parámetro.
 * @return 0 si el gráfico a sido guardado con éxito, -1 en caso contrario.
 **/
extern int CRE_SaveMGx(SDL_Surface * Src, char * FileName);

/**
 * @brief Carga una imagen de un stream en formato mGx
 * @param File Stream de datos de donde cargar el archivo
 * Función que intenta carga una imágen en formato mGx desde un flujo de datos.
 * @return Puntero a la superfice SDL, NULL si algo ha fallado.
 **/
extern SDL_Surface * CRE_LoadMGxFromStream(gzFile * File);

/**
 * @brief Carga una imagen de un archivo en formato mGx
 * @param FileName Ruta del archivo de donde cargar la imágen
 * Función que intenta carga una imágen en formato mGx desde un flujo de datos.
 * @return Puntero a la superfice SDL, NULL si algo ha fallado.
 **/
extern SDL_Surface * CRE_LoadMGx(char * FileName);

/**
 * @brief Guarda un vector de imágenes en un Stream en formato mGf
 * @param Src Fichero de gráficos ha guardar
 * @param File Flujo de datos donde guardar el archivo
 * La finalidad de esta función es la de guardar un vector o agrupación de
 * imágenes (Superficies de SDL) en un mismo stream con el formato mGf
 * @return 0 si no ha ocurrido ningún error, -1 en caso contrario.
 **/
extern int CRE_SaveMGfToStream(creMGf * Src, gzFile * File);

/**
 * @brief Guarda un vector de imágenes en un archivo en formato mGf
 * @param Src Fichero de gráficos.
 * @param FileName Ruta del archivo donde guardar la imágen
 * Función que guarda un vector de imágenes en un archivo en formato mGf.
 * @return 0 si no ha ocurrido ningún error, -1 en caso contrario.
 **/
extern int CRE_SaveMGf(creMGf * Src, char * FileName);

/**
 * @brief Carga un vector de imágenes de un archivo mGf
 * @param FileName Ruta del archivo ha cargar.
 * Esta función carga vector de imágenes desde la ubicación de un archivo mGf
 * @return NULL si ha ocurrido algún error o un ficheros de gráficos .
 **/
extern creMGf * CRE_LoadMGf(char * FileName);

/**
 * @brief Elimina un fichero de gráficos
 * @param Src Fichero de gráficos ha eliminar
 * Función que se encarga de eliminar y liberar la memoria de un fichero de
 * de gráficos (incluidos todos lo graficos de este, que quedan inaccesibles)
 **/
extern void CRE_FreeMGf(creMGf * Src);

#endif
