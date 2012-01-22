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
 * @file tiler.c
 * Implementación de las funciones de escenarios (mapas de tiles).
 **/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include <zlib.h>
#include <core.h>


/*
 * Definición de macros
 */

/* Define los bits constantes de las cabeceras de los archivos */
#define MSC_MAGIC "MSc\x69\xFF\x2D"


/*
 * Definición de tipos
 */

 /*
  * Definición de la cabecera de datos de un archivo MSc (Minimalist Scenario)
  */
 typedef struct creMScHeader {
     /* Cabecera identificadora */
     char Magic[8];
     /* Tamaño de los tiles (en pixels) */
     int Size;
     /* Ancho y alto del escenario (en tiles) */
     int W, H;
     /* Nombre del skin que utiliza */
     char Skin[32];
     /* Número de puntos clave */
     int KPCount;
 } creMScHeader;


/*
 * Implementación de funciones
 */

/*
 * CRE_LoadMSc
 * Carga un archivo de escenario de tiles. Recibe como parámetro la ruta del
 * archivo.
 */
creMSc * CRE_LoadMSc(char * FileName) {
    gzFile * File;
    creMSc * MSc;
    creMScHeader Header;
    int m;

    /* Abrimos el fichero */
    File = gzopen(FileName, "rb");
    if(File == NULL) return NULL;

    /* Leemos la cabecera y comprobamos que es correcto */
    gzread(File, &Header, sizeof(creMScHeader));
    if(strcmp(Header.Magic, MSC_MAGIC) != 0) return NULL;

    /* Creamos la estructura del escenario*/
    MSc = (creMSc *) malloc(sizeof(creMSc));
    MSc->Size = Header.Size;
    MSc->W = Header.W; MSc->H = Header.H;
    strcpy(MSc->Skin, Header.Skin);
    MSc->KPCount = Header.KPCount;

    /* Leemos el mapa de tiles */
    m = Header.W * Header.H;
    MSc->Map = (char *) malloc(sizeof(char) * m);
    gzread(File, MSc->Map, sizeof(char) * m);

    /* Leemos los puntos clave */
    MSc->KeyPoints = (crePoint *) malloc(sizeof(crePoint) * MSc->KPCount);
    gzread(File, MSc->KeyPoints, sizeof(crePoint) * MSc->KPCount);

    /* Cerramos el archivo y devolvemos */
    gzclose(File);
    return MSc;
}


/*
 * CRE_SaveMSc
 * Guarda un estructura del tipo escenario en archivo MSc
 */
int CRE_SaveMSc(creMSc * Src, char * FileName)
{
    gzFile * File;
    creMScHeader Header;

    File = gzopen(FileName, "wb");
    if(File == NULL) return -1;

    strcpy(Header.Magic, MSC_MAGIC);
    strcpy(Header.Skin, Src->Skin);
    Header.W = Src->W; Header.H = Src->H;
    Header.Size = Src->Size;
    Header.KPCount = Src->KPCount;
    gzwrite(File, &Header, sizeof(creMScHeader));

    gzwrite(File, Src->Map, Header.W * Header.H);
    gzwrite(File, Src->KeyPoints, Header.KPCount * sizeof(crePoint));

    gzclose(File);
    return 0;
}


/*
 * CRE_FreeMSc
 * Borra un escenario de la memoria.
 */
void CRE_FreeMSc(creMSc * Src)
{
    free(Src->Map);
    free(Src->KeyPoints);
    free(Src);
}


/*
 * CRE_DrawMScToSurface
 * Dibuja el mapa de tiles sobre la superficie indicada.
 */
int CRE_DrawMScToSurface(creMSc * Src, SDL_Surface * Trg, creMGf * Gfxs)
{
    int i, j, k;
    SDL_Rect Dst = {0, 0, 0, 0};


    /* Comprobamos los parametros */
    if(Src == NULL || Trg == NULL || Gfxs == NULL) return -1;

    /* Dibujamos el mapa de tiles */
    SDL_FillRect(Trg, NULL, 0xFF);
    for(j = k = 0; j < Src->H; j++)
        for(i = 0; i < Src->W; i++, k++) {
            Dst.x = i * Src->Size;
            Dst.y = j * Src->Size;
            SDL_BlitSurface(Gfxs->Gfx[(int) Src->Map[k]], NULL, Trg, &Dst);
        }

    return 0;
}



/*
 * CRE_DrawMSc
 * Dinbuja el mapa de tiles y da el resultado en un gráfico nuevo.
 */
 SDL_Surface * CRE_DrawMSc(creMSc * Src)
 {
    creMGf * Gfxs;
    SDL_Surface * Trg, * Tmp;

    /* Comprobamos los parametros */
    if(Src == NULL) return NULL;

    /* Cremos la superficie de trabajo y cargamos los gráficos */
    Trg = SDL_CreateRGBSurface(creScreen->flags, Src->W * Src->Size,
        Src->H * Src->Size, 32, creScreen->format->Rmask,
        creScreen->format->Gmask, creScreen->format->Bmask,
        creScreen->format->Amask);
    Gfxs = CRE_LoadMGf(Src->Skin);

    /* Dibujamos el mapa de tiles */
    if(Gfxs == NULL || Trg == NULL) return NULL;
    CRE_DrawMScToSurface(Src, Trg, Gfxs);

    /* Liberamos memoria y devolvemos */
    Tmp = SDL_DisplayFormat(Trg);
    CRE_FreeMGf(Gfxs);
    SDL_FreeSurface(Trg);
    return Tmp;
 }


