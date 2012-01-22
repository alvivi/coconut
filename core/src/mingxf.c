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
 * @file mingxf.c
 * Implementa las funciones que permiten cargar los archivos de gráficos
 * minimalistas mGx y los ficheros de archivos gráficos mGf. Más información
 * en el archivo de cabecera.
 **/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include <zlib.h>
#include "core.h"
#include "mingxf.h"


/*
 * Definición de macros
 */

/* Define los bits constantes de las cabeceras de los archivos */
#define MGX_MAGIC "MGx\x69\xFF\x0D"
#define MGF_MAGIC "MGf\x69\xFF|x1D"


/*
 * Definición de tipos
 */

/* Define la cabecera de un archivo de un gráfico mGx */
typedef struct creMGxHeader {
    /* Cabecera identificadora */
    char Magic[8];
    /* Ancho de la imagen */
    int W;
    /* Alto de la imagen */
    int H;
} creMGxHeader;

/* Define la cabecera de un archivo de ficheros de gráficos mGf */
typedef struct creMGfHeader {
    /* Cabecera identificadora */
    char Magic[8];
    /* Númeor de elementos */
    int Size;
} creMGfHeader;


/*
 * Implementación de funciones
 */

/*
 * CRE_SaveMGxToStream
 * Dado un gráfico en el formato de SDL lo guarda a un stream de datos siguiendo
 * el formato de los archivos mGx
 */
int CRE_SaveMGxToStream(SDL_Surface * Src, gzFile * File)
{
    SDL_Surface * Model, * Tmp;
    creMGxHeader Head;
    int size;

    if(File == NULL) return -1;

    strcpy(Head.Magic, MGX_MAGIC);
    Head.W = Src->w;
    Head.H = Src->h;

    gzwrite(File, &Head, sizeof(creMGxHeader));

    Model = SDL_CreateRGBSurface(SDL_SWSURFACE, 1, 1, 32, 0xFF000000,
        0xFF0000, 0xFF00, 0xFF);
    Tmp = SDL_ConvertSurface(Src, Model->format, SDL_SWSURFACE);

    if(Tmp == NULL) return -1;

    size = Tmp->w * Tmp->h;
    if(gzwrite(File, Tmp->pixels, 4 * size) < size) return -1;

    SDL_FreeSurface(Tmp);
    SDL_FreeSurface(Model);

    return 0;
}


/*
 * CRE_SaveMGx
 * Dado un gráfico en el formato de SDL lo guarda a archivo mGx en la ruta
 * especificada.
 */
int CRE_SaveMGx(SDL_Surface * Src, char * FileName)
{
    gzFile * File;

    File = gzopen(FileName, "wb");
    if(File == NULL) return -1;
    if(CRE_SaveMGxToStream(Src, File)) return -1;
    gzclose(File);

    return 0;
}


/*
 * CRE_LoadMGxFromStream
 * Dado un stream, intenta leer en éste un archivo de tipo mGx y cargarlo en
 * un gráfico de formato SDL.
 */
SDL_Surface * CRE_LoadMGxFromStream(gzFile * File)
{
    SDL_Surface * Tmp, * Res;
    creMGxHeader Head;
    int Size;
    int * Buffer;

    if(File == NULL) return NULL;

    if(gzread(File, &Head, sizeof(creMGxHeader)) < 1) return NULL;

    if(strcmp(Head.Magic, MGX_MAGIC) != 0) return NULL;

    if((Size = Head.W * Head.H) < 1) return NULL;
    Buffer = (int *) malloc(Size * 4);

    if(gzread(File, Buffer, 4 * Size) < Size){
        free(Buffer);
        return NULL;
    }

    Tmp = SDL_CreateRGBSurfaceFrom((void *)Buffer, Head.W, Head.H, 32,
      4 * Head.W, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);

    Res = SDL_DisplayFormatAlpha(Tmp);
    SDL_FreeSurface(Tmp);
    free(Buffer);

    return Res;
}


/*
 * CRE_LoadMGx
 * Dado la ruta de un archivo, intenta leer en éste un archivo de tipo mGx
 * y cargarlo en un gráfico de formato SDL.
 */
SDL_Surface * CRE_LoadMGx(char * FileName)
{
    gzFile * File;
    SDL_Surface * Tmp;

    File = gzopen(FileName, "rb");
    if(File == NULL) return NULL;
    Tmp = CRE_LoadMGxFromStream(File);
    gzclose(File);

    return Tmp;
}


/*
 * CRE_SaveMGfToStream
 * Gurada en un stream dado todo el vector de imágenes dado en el formato
 * mGf.
 */
int CRE_SaveMGfToStream(creMGf * Src, gzFile * File)
{
    creMGfHeader Head;
    Uint32 i;
    SDL_Surface ** Tmp;

    if(File == NULL) return -1;
    if(Src->Size <= 0) return -1;

    strcpy(Head.Magic, MGF_MAGIC);
    Head.Size = Src->Size;

    gzwrite(File, &Head, sizeof(creMGfHeader));

    for(i = Src->Size, Tmp = Src->Gfx; i > 0; i--, Tmp++)
        if(CRE_SaveMGxToStream(*Tmp, File))
            return -1;

    return 0;
}


/*
 * CRE_SaveMGf
 * Gurada en un fichero con la dirección especificada todo el vector de imágenes
 * dado en el formato mGf.
 */
int CRE_SaveMGf(creMGf * Src, char * FileName)
{
    gzFile * File;

    File = gzopen(FileName, "wb");
    if(File == NULL) return -1;
    if(CRE_SaveMGfToStream(Src, File)) return -1;
    gzclose(File);

    return 0;
}


/*
 * CRE_LoadMGf
 * Carga el vector de imágenes de un fichero mGf
 */
creMGf * CRE_LoadMGf(char * FileName)
{
    creMGf * Trg;
    creMGfHeader Head;
    gzFile * File;
    int i;

    if((File = gzopen(FileName, "rb")) == NULL) return NULL;

    if(gzread(File, &Head, sizeof(creMGfHeader)) < 1) return NULL;

    Trg = (creMGf *) malloc(sizeof(creMGf));
    if(Trg == NULL) return NULL;
    Trg->Size = Head.Size;

    if(Head.Size < 1) return NULL;
    Trg->Gfx = malloc(sizeof(SDL_Surface *) * Head.Size);

    for(i = 0; i < Head.Size; i++)
        Trg->Gfx[i] = CRE_LoadMGxFromStream(File);

    gzclose(File);
    return Trg;
}


/*
 * CRE_FreeMGf
 * Libera la memoria de un fichero gráfico.
 */
void CRE_FreeMGf(creMGf * Src)
{
    int i;
    
    if(Src != NULL) {
        for(i = 0; i < Src->Size; i++)
            SDL_FreeSurface(Src->Gfx[i]);
        free(Src->Gfx);
        free(Src);
    }
}
