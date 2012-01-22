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
 * menu.c
 * Define las funciones y procesos que controlan los menus del juego y
 * algunos procesos auxiliares.
 */

#ifndef COCONUT_MISC_H
#define COCONUT_MISC_H


#include <core.h>

/*
 * Definición de variables globales
 */

/* Gráficos generales asociados a los procesos de este archivo */
extern creMGf * MGfMisc;


/*
 * Definición de procesos
 */

/*
 * PText
 * Proceso que visualiza un texto por pantalla. El texto puede contener
 * información variable.
 */

/* Definición de constates y macros asociados al proceso */
#define TEXT_CONST   1 /* Indica que el texto nunca variará */
#define TEXT_VAR     0 /* Indica que el texto cambia, y debe actualizarse */
#define TYPE_PTEXT  20 /* Identificador de tipo */
#define Z_PTEXT    240 /* Profundidad de los textos */

/* Definición de métodos */
/* Constructor */
extern creProcess * PText_New(char * Str, Sint32 X, Sint32 Y, TTF_Font * Font,
  Sint32 Type);
/* Bucle */
extern void PText_Loop(creProcess * This);
/* Destructor */
extern void PText_Free(creProcess * This);

/* Definción de clase */
extern creProcess PText;

/* Definición de datos de la clase */
typedef struct PTextData {
    char     * String; /* Cadena de texto que mostrar */
    TTF_Font * Font;   /* Fuente con la que se muestra el texto */
} PTextData;


/*
 * PExitWindows
 * Ventana que pregunta si queremos termina la partida actual.
 */

/* Definición de constates y macros asociados al proceso */
#define TYPE_PEXIT       21 /* Identificador de tipo */
#define Z_PEXIT         250 /* Profundidad del proceso */
#define IND_EXITWINDOWS   0 /* Indices del gráfico */

/* Definición de métodos */
/* Constructor */
extern creProcess * PExitWindows_New();
/* Bucle */
extern void PExitWindows_Loop(creProcess * This);
/* Destructor */
extern void PExitWindows_Free(creProcess * This);

/* Definción de clase */
extern creProcess PExitWindows;


/*
 * PMainMenu
 * Proceso que gestiona y visualiza el menu principal del juego.
 */

/* Definición de constates y macros asociados al proceso */
#define TYPE_PMENU  22 /* Identificador de tipo */
#define Z_PMENU      10 /* Profundidad del proceso */
#define IND_MENUBG   3 /* Indices del gráfico */

/* Definición de métodos */
/* Constructor */
extern creProcess * PMainMenu_New();
/* Bucle */
extern void PMainMenu_Loop(creProcess * This);
/* Destructor */
extern void PMainMenu_Free(creProcess * This);

/* Definción de clase */
extern creProcess PMainMenu;


/*
 * PArrow
 * Proceso que gestiona la flecha de selección del menu.
 */

/* Definición de constates y macros asociados al proceso */
#define TYPE_PARROW    23 /* Identificador de tipo */
#define Z_PARROW      200 /* Profundidad de los textos */
#define IND_ARROW       5 /* Indice del gráfico */
#define MAX_ARROW_DESP 15 /* Deplazamiento máximo en pixel */
#define MAX_ARROW_INC   2 /* Velocidad de desplazamiento */

/* Definición de métodos */
/* Constructor */
extern creProcess * PArrow_New();
/* Bucle */
extern void PArrow_Loop(creProcess * This);
/* Destructor */
extern void PArrow_Free(creProcess * This);

/* Definción de clase */
extern creProcess PArrow;

/* Definición de datos de la clase */
typedef struct PArrowData {
    Sint32 Desp; /* Indicador de desplazamiento horizontal */
    Uint8  Dir;  /* Indicador de dirección */
} PArrowData;



/*
 * PCredits
 * Los mini creditos del juego! :D
 */

/* Definición de constates y macros asociados al proceso */
#define TYPE_PCREDITS  24 /* Identificador de tipo */
#define Z_PCREDITS    240 /* Profundidad de los textos */
#define IND_CREDITS     4 /* Indice del gráfico */
#define GOTO_X        180 /* Posición de desplazamiento */
#define GOTO_SPD       10 /* Velocidad de desplazamiento */
#define ALPHA_SPD       7 /* Velocidad de cambio de transparencia */
#define CSTATE_ON       1 /* Estado en el que aparen los creditos */
#define CSTATE_NONE     0 /* Estado en el que se muestran los creditos */
#define CSTATE_OFF      2 /* Estado en el que desaparecen los creditos*/

/* Definición de métodos */
/* Constructor */
extern creProcess * PCredits_New();
/* Bucle */
extern void PCredits_Loop(creProcess * This);
/* Destructor */
extern void PCredits_Free(creProcess * This);

/* Definción de clase */
extern creProcess PCredits;


/*
 * PEndWindows
 * Ventana que se utiliza al final de una partida para indicar que el jugador
 * ha ganado o perdido la partida
 */

/* Definición de constates y macros asociados al proceso */
#define TYPE_PEND       25 /* Identificador de tipo */
#define Z_PEND         254 /* Profundidad del proceso */
#define IND_GAMEOVER     6 /* Indice del gráfico de 'gameobver' */
#define IND_LEVELUP      7 /* Indice del gráfico de 'levelup' */

/* Definición de métodos */
/* Constructor */
extern creProcess * PEndWindows_New(Sint32 GraphId);
/* Bucle */
extern void PEndWindows_Loop(creProcess * This);
/* Destructor */
extern void PEndWindows_Free(creProcess * This);

/* Definción de clase */
extern creProcess PEndWindows;


/*
 * PWinScreen
 * Pantalla que se muestra cuando el jugador termina todos los niveles del
 * juego.
 */

/* Definición de constates y macros asociados al proceso */
#define TYPE_PWIN       26 /* Identificador de tipo */
#define Z_PWIN          50 /* Profundidad del proceso */
#define IND_WINBG        8 /* Indice del gráfico de 'gameobver' */
/* Texto que se muestra en la pantalla */
#define PWIN_TEXT "Tu puntuacion final fue de %d puntos"

/* Definición de métodos */
/* Constructor */
extern creProcess * PWinScreen_New(Uint32 Score);
/* Bucle */
extern void PWinScreen_Loop(creProcess * This);
/* Destructor */
extern void PWinScreen_Free(creProcess * This);

/* Definción de clase */
extern creProcess PWinScreen;


/*
 * Definición de funciones
 */

/*
 * COCO_MainMenu
 * Muestra el menu principal y devuelve la opción que ha elegido el usuario. En
 * esta versión sólo puede ser elegidas las opciones de 'nueva partida' y
 * 'salir'
 */

/* Opciones del menu */
#define OP_COUNT     3 /* Número de opciones */
#define OP_NEWGAME   0 /* Opción del menu: nueva partida */
#define OP_INFO      1 /* Opción del menu: más información */
#define OP_EXIT      2 /* Opción del menu: salir */

extern Uint8 COCO_MainMenu(void);

#endif
