/*
 * coconut - A nowadays Pacman remake.
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


/*
 * game.h
 * Define las funciones y procesos que controlan una partida de un escenraio
 * o nivel.
 */

#ifndef COCONUT_GAME_H
#define COCONUT_GAME_H


#include <core.h>

/*
 * Definici�n de macros y constantes
 */

/* Rutas de los archivos asociados al juego */
#define MGFMISC_PATH  "gfx/misc.mGf"
#define MGFCOCO_PATH  "gfx/coco.mGf"
#define MGFSTAR_PATH  "gfx/star.mGf"
#define MGFGHOST_PATH "gfx/ghost.mGf"
#define TTFCOCO_PATH  "ttf/0001.ttf"

/* Direcciones de los personajes */
#define TO_FRONT 0x00
#define TO_LEFT  0x01
#define TO_RIGHT 0x02
#define TO_UP    0x04
#define TO_DOWN  0x08

/* Estado de los personajes */
#define IN_NONE  0x00
#define IN_SHOCK 0x80
#define IN_SUGAR 0x08

/* Puntos clave de los escenario */
#define IND_KP_START        0
#define IND_KP_ULBOX        1
#define IND_KP_DRBOX        2
#define IND_KP_BOXGATE      3
#define IND_KP_GHOST_TIMES  4
#define IND_KP_GLOBAL_TIME  5
#define IND_KP_BONUSP_START 6


/*
 * Definici�n de variables globales
 */

/* Ficheros de gr�ficos asociados al juego */
extern creMGf * MGfCoco;
extern creMGf * MGfStar;
extern creMGf * MGfGhost;

/* Fuente de texto asociados al juego */
extern TTF_Font * TTFCoco;
extern TTF_Font * TTFCocoSmall;


/*
 * Definici�n de procesos
 */

/*
 * PDownBar
 * Barra de informaci�n de la partida que aparece en la parte inferior del
 * juego. Se encarga de mostrar puntuaci�n total, las vidas restantes,
 * las estrellas restantes y el tiempo.
 */

/* Definici�n de constates y macros asociados al proceso */
#define TYPE_PDBAR   1 /* Identificador de tipo */
#define Z_DBAR     200 /* Profundidad del proceso */
#define IND_DBAR     1 /* Indices del gr�fico */

/* Definici�n de m�todos */
/* Constructor */
extern creProcess * PDownBar_New();
/* Bucle */
extern void PDownBar_Loop(creProcess * This);
/* Destructor */
extern void PDownBar_Free(creProcess * This);

/* Definci�n de clase */
extern creProcess PDownBar;


/*
 * PCoco
 * Proceso que implementa el personaje del comecocos. La informaci�n de la
 * instancia �nica del proceso es p�blica gracias a una variable global. De
 * este modo, son los dem�s proceso los que interactuan sobre �ste.
 */

/* Definici�n de constates y macros asociados al proceso */
#define TYPE_PCOCO           4 /* Identificador de tipo */
#define Z_COCO             100 /* Profundidad del proceso */
#define IND_COCO_FRONT       1 /* Indice del gr�fico 'de frente' */
#define IND_COCO_FRONT_SIZE  5 /* Tama�o de la animacion 'de frente' */
#define IND_COCO_LEFT       14 /* Indice del gr�fico 'izquierda' */
#define IND_COCO_LEFT_SIZE   5 /* Tama�o de la animacion 'izquierda' */
#define IND_COCO_RIGHT      20 /* Indice del gr�fico 'derecha' */
#define IND_COCO_RIGHT_SIZE  5 /* Tama�o de la animacion 'derecha' */
#define IND_COCO_UP         26 /* Indice del gr�fico 'arriba' */
#define IND_COCO_UP_SIZE     5 /* Tama�o de la animacion 'arriba' */
#define IND_COCO_DOWN        8 /* Indice del gr�fico 'abajo' */
#define IND_COCO_DOWN_SIZE   5 /* Tama�o de la animacion 'abajo' */
#define COCO_SHOCKTIME   15000 /* Duraci�n en ms del estado de shock */

/* Definici�n de m�todos */
/* Constructor */
extern creProcess * PCoco_New();
/* Bucle */
extern void PCoco_Loop(creProcess * This);
/* Destructor */
extern void PCoco_Free(creProcess * This);

/* Definci�n de clase */
extern creProcess PCoco;

/* Definici�n de datos de la clase */
typedef struct PCocoData {
    Uint16 X, Y;                      /* Posici�n del comecocos en tiles */
    Uint8  Dir, State;                      /* Direcci�n y estado de �ste*/
    Sint32 Frame;           /* Frame en el que se encuentra la animaci�n */
    Uint32 SugarTime;                   /* Tiempo en el que invulnerable */
    Uint8  NextDir, Enable, Loop; /* Variables auxilizar de la animaci�n */
} PCocoData;


/*
 * PStar
 * Proceso que representa una estrella (comida del comecocos). Se considera
 * como objetivo de todos los escenario que no quede ninguna estrella en juego.
 */

/* Definici�n de constates y macros asociados al proceso */
#define TYPE_PSTAR     5 /* Identificador de tipo */
#define Z_STAR        50 /* Profundidad del proceso */
#define IND_STAR       0 /* Indice del gr�fico inicial de la animaci�n */
#define IND_STAR_SIZE 40 /* Tama�o de la animaci�n */

/* Definici�n de m�todos */
/* Constructor */
extern creProcess * PStar_New();
/* Bucle */
extern void PStar_Loop(creProcess * This);
/*  Destructor*/
extern void PStar_Free(creProcess * This);

/* Definci�n de clase */
extern creProcess PStar;

/* Definici�n de datos de la clase */
typedef struct PStarData {
    Uint16 X, Y;  /* Posici�n en tiles de la estrella */
    Sint32 Frame; /* Frame en que se encuentra la animaci�n */
} PStarData;



/*
 * PGhost
 * Proceso que representa un fantasma, enemigo del juegador. Tiene un movimiento
 * aleatorio.
 */

/* Definici�n de constates y macros asociados al proceso */
#define TYPE_PGHOST        6 /* Identificador de tipo */
#define Z_GHOSTN         101 /* Profundidad del fantasma en estado normal */
#define Z_GHOSTF          55 /* Profundidad del fantasma en estado de shock */
#define A_GHOSTN          50 /* Transparencia del fantasma en estado normal */
#define A_GHOSTN_MAX     150 /* M�ximo indice de trans. en estado normal */
#define A_GHOSTF         255 /* Transparencia del fantasma en estado de shock */
#define IND_GHOST          0 /* Indice del gr�fico inicial de la animaci�n */
#define IND_GHOST_SIZE     5 /* Tama�o de la animaci�n (n�mero de fantasmas) */
#define INIT_GHOSTS_NUMBER 4 /* Fantasmas inciales en el escenario */
#define GHOST_MIN_WAITTIME 5000 /* Tiempo min entre nuevos fantasmas */

/* Definici�n de m�todos */
/* Constructor */
extern creProcess * PGhost_New();
/* Bucle */
extern void PGhost_Loop(creProcess * This);
/* Destructor */
extern void PGhost_Free(creProcess * This);

/* Definci�n de clase */
extern creProcess PGhost;

/* Definici�n de datos de la clase */
typedef struct PGhostData {
    Uint16 X, Y;                    /* Posici�n del fantasma */
    Sint8 Alp, Dir, Enabled, Frame; /* Informaci�n de la animaci�n */
} PGhostData;


/*
 * PGlint
 * Es el brillo caracteristico de un comecocos cuando va de subidon.
 */

/* Definici�n de constates y macros asociados al proceso */
#define TYPE_GLINT  8 /* Identificador de tipo */
#define Z_GLINT    98 /* Profundidad del proceso */
#define IND_GLINT   0 /* Indice del gr�fico 'destello' */

/* Definici�n de m�todos */
/* Constuctor */
extern creProcess * PGlint_New();
/* Bucle */
extern void PGlint_Loop(creProcess * This);
/* Destructor */
extern void PGlint_Free(creProcess * This);

/* Definci�n de clase */
extern creProcess PGlint;


/*
 * PCandy
 * Son los caramelos. Tienen azucar y al coco le altera.
 */

/* Definici�n de constates y macros asociados al proceso */
#define TYPE_CANDY  7 /* Identificador de tipo */
#define Z_CANDY    50 /* Profundidad del proceso */
#define IND_CANDY   2 /* Indice del gr�fico 'caramelo' */

/* Definici�n de m�todos */
/* Constructor */
extern creProcess * PCandy_New();
/* Bucle */
extern void PCandy_Loop(creProcess * This);
/* Destructor */
extern void PCandy_Free(creProcess * This);

/* Definci�n de clase */
extern creProcess PCandy;

/* Definici�n de datos de la clase */
typedef struct PCandyData{
    Uint16 X, Y; /* Posici�n actual en tiles */
} PCandyData;


/*
 * PGame
 * Proceso que gestiona una partida. Se encarga de aspectos generales, como
 * si el jugador quiere acabar la partida, o terminado los objetivos de ella,
 * o simplemente ha sido derrotado.
 */

/* Definici�n de constates y macros asociados al proceso */
#define TYPE_PGAME  3 /* Identificador de tipo */
#define Z_GAME      5 /* Profundidad del proceso */

/* Definici�n de m�todos */
/* Constructor */
extern creProcess * PGame_New();
/* Bucle */
extern void PGame_Loop(creProcess * This);
/* Destructor */
extern void PGame_Free(creProcess * This);

/* Definci�n de clase */
extern creProcess PGame;

/* Definici�n de datos de la clase */
typedef struct PGameData {
    Sint32 WaitTime, Time, GTime; /* Tiempos de la partida */
} PGameData;


/*
 * Definici�n de funciones
 */

/*
 * COCO_StartLevel
 * Inicia una partida del archivo escenario indicado. Si por alg�n motivo no a
 * podido ser iniciada la partida devuelve LVL_MAPEEROR. Si el jugador gana
 * devuelve LVL_PLAYWINS y si pierde o abandona devuelve LVL_PLAYERLOSE.
 */

/* Posibles finales de la partida */
#define LVL_MAPERROR -1
#define LVL_PLAYERWINS 1
#define LVL_PLAYERLOSE 0

extern int COCO_StartLevel(char * FileName, Uint8 * Lifes, Uint32 * Score_ );

#endif
