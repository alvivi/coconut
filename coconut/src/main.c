/*
 * coconut - A nowadays Pacman remake.
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


/*
 * main.c
 * Código fuente que contiene la función principal del programa. Se encarga
 * de algunas tareas principales.
 */

#include <core.h>
#include <SDL/SDL_ttf.h>
#include "misc.h"
#include "game.h"

/* Indica la ruta de los escenarios */
#define LEVELS_PATH "sce/level%d.mSc"

/*
 * FUNCIÓN main
 * Función que representa el punto de entrada del programa. Se encarga
 * básicamente de inicializar las librerías.
 */
int main(int argc, char * argv[])
{
    /* Datos del juego */
    Uint8  Lifes = 5; /* Vidas del comecocos */
    Uint32 Score = 0; /* Puntuación del comecocos */
    Uint8 Option = 0; /* Opciones elejidas en el menu */
    Sint32   End = 0; /* Indica si el jugador ha terminado de jugar */
    Uint8  Level = 1; /* Nivel actual */
    Uint32 Mode;
    /* Cadena temporal donde se escribe la ruta del escenario */
    char LevelPath[32];

    /* Nos aseguramos de que siempre se descargen las librerias */
    atexit(SDL_Quit);
    atexit(TTF_Quit);

    /* Inicializamos la SDL */
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "Couldn't init SDL: %s \n", SDL_GetError());
        exit(1);
    }

    /* Inicializamos la SDL_TTF */
    if(TTF_Init()) {
        fprintf(stderr, "Couldn't init TTF: %s \n", TTF_GetError());
        exit(2);
    }

    /* Activamos el modo de vídeo */

    if (argc>1) 
      Mode = SDL_SWSURFACE | SDL_FULLSCREEN;
    else
      Mode = SDL_HWSURFACE;
      
    creScreen = SDL_SetVideoMode(800, 600, 32, Mode);
    if(creScreen == NULL) {
        fprintf(stderr, "Couldn't init video mode: %s\n", SDL_GetError());
        exit(3);
    }

    /* Inicialización de variables y opciones generales */
    SDL_WM_SetCaption("COCONUT", "coconut");
    SDL_ShowCursor(SDL_DISABLE);
    srandom(time(NULL));
    CRE_SetFPS(42);

    /* Cargamos todos los ficheros gráficos y fuentes */
    MGfMisc      = CRE_LoadMGf(MGFMISC_PATH);
    MGfCoco      = CRE_LoadMGf(MGFCOCO_PATH);
    MGfGhost     = CRE_LoadMGf(MGFGHOST_PATH);
    MGfStar      = CRE_LoadMGf(MGFSTAR_PATH);
    TTFCoco      = TTF_OpenFont(TTFCOCO_PATH, 40);
    TTFCocoSmall = TTF_OpenFont(TTFCOCO_PATH, 20);

    /* Comprobamos que todos han sido cargados correctamente */
    if(MGfMisc == NULL || MGfStar == NULL || MGfCoco == NULL || MGfGhost == NULL
      || TTFCoco == NULL) {
        fprintf(stderr, "Couldn't load some resource file.");
        exit(4);
    }

    /* Empezamos el juego */
    while(1) {
        /* Mostramos el menu */
        Option = COCO_MainMenu();

        /* Si el usuario a elejido comenzar una nueva partida */
        if(Option == OP_NEWGAME) {
            /* Iniciamos las variables para una partida nueva */
            Lifes = 5;
            Score = 0;
            Level = 1;
            End = 0;
            /* Comenzamos el juego. Este bucle recorre todos los niveles */
            while(!End) {
                /* Obtenemos la ruta del nivel actual */
                sprintf(LevelPath, LEVELS_PATH, Level);
                switch(COCO_StartLevel(LevelPath, &Lifes, &Score)) {
                    /* Indica que no quedan más niveles */
                    case LVL_MAPERROR:
                        End = 1;
                        PWinScreen.New(Score);
                        CRE_StartLoop();
                        break;
                    /* El jugador ha ganado */
                    case LVL_PLAYERWINS:
                        Level++;
                        break;
                    /* El jugador ha abandonado */
                    default :
                        End = 1;
                        break;
                }
            }
        /* Sino significa que quiere salir */
        } else
            break;
    }

    /* Descargamos todos los gráficos y funetes */
    CRE_FreeMGf(MGfMisc);
    CRE_FreeMGf(MGfCoco);
    CRE_FreeMGf(MGfStar);
    CRE_FreeMGf(MGfGhost);
    TTF_CloseFont(TTFCoco);
    TTF_CloseFont(TTFCocoSmall);

    exit(0);
}
