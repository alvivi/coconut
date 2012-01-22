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
 * Implementa las funciones y procesos que controlan los menus del juego y
 * algunos procesos auxiliares.
 */

#include <string.h>
#include <core.h>
#include <SDL/SDL_ttf.h>
#include "misc.h"
#include "game.h"


/*
 * Variable globales
 */

/* Opción que va elegir el usuario */
Uint8 MenuOption = OP_NEWGAME;

/* Graficos asociados a los procesos de este archivo */
creMGf * MGfMisc = NULL;


/*
 * Implementación de los métododos de los procesos
 */

/*
 * PText
 * Proceso que visualiza una cadena de texto en pantalla.
 */

/* Definición de la clase */
creProcess PText = {0, TYPE_PTEXT, CRE_PS_CLASS, "Texto", NULL,
    PText_New, PText_Loop, PText_Free,   NULL, 400, 575, Z_PTEXT, 255, 0, 100,
    100, NULL};

/* Constructor */
creProcess * PText_New(char * Str, Sint32 X, Sint32 Y, TTF_Font * Font,
  Sint32 Type)
{
    creProcess * This;

    /* Creamos el proceso */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PText;
    This->State = CRE_PS_DEFAULT;
    This->X = X; This->Y = Y;

    if(Type) {
        This->Graph = This->Graph = TTF_RenderText_Blended(Font, Str,
          (SDL_Color){255, 255, 255, 0});
        This->Loop = NULL;
    } else {
        This->Data = malloc(sizeof(PTextData));
        *((PTextData *) This->Data) = (PTextData) {Str, Font};
    }
    CRE_AddProcess(This);

    return This;
}

/* Bucle */

void PText_Loop(creProcess * This)
{
    PTextData * Text = (PTextData *) This->Data;

    if(This->Graph != NULL)
        SDL_FreeSurface(This->Graph);

    This->Graph = TTF_RenderText_Solid(Text->Font, Text->String,
      (SDL_Color){255, 255, 255, 0});

    return;
}

/* Destructor */
void PText_Free(creProcess * This)
{
    if(This->Graph == NULL)
        SDL_FreeSurface(This->Graph);
    /* Liberamos memoria */
    if (This->Data != NULL)
      free(This->Data);
    free(This);
}


/*
 * PExitWindows
 * Ventana que pregunta si de verdad de la buena y de todo corazón queremos
 * terminar la maravillosa partida ;)
 */

/* Definción de clase */
creProcess PExitWindows = {0, TYPE_PEXIT, CRE_PS_CLASS, "Ventana de salida",
    NULL, PExitWindows_New, PExitWindows_Loop, PExitWindows_Free,
    NULL, 400, 268, Z_PEXIT, 55, 0, 100, 100, NULL};

/* Constructor */
creProcess * PExitWindows_New()
{
    creProcess * This;

    /* Creamos el proceso */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PExitWindows;
    This->State = CRE_PS_DEFAULT;

    /* Asociamos al proceso su imagen */
    This->Graph = MGfMisc->Gfx[IND_EXITWINDOWS];

    /*
     * Limpiamos la lista de eventos ya que la ventana espera recibir qualquier
     * evento.
     */
    creEList.Size = 0;

    /* Lo ejecutamos y pausamos todos los demás procesos */
    CRE_TSetState(0, CRE_PS_PAUSE);
    CRE_AddProcess(This);

    return This;
}

/* Bucle */
void PExitWindows_Loop(creProcess * This)
{
    if(This->Alpha < 255) {
        This->Alpha += 10;
    }

    for(creEList.CurrentEvent = 0; creEList.CurrentEvent < creEList.Size;
      creEList.CurrentEvent++) {
        if(creEList.Events[creEList.CurrentEvent].type == SDL_QUIT) {
            CRE_LetPrcsAlone(0);
            MenuOption = OP_EXIT;
        }

        if(creEList.Events[creEList.CurrentEvent].type == SDL_KEYDOWN) {
            if(creEList.Events[creEList.CurrentEvent].key.keysym.sym ==
              SDLK_ESCAPE ||
              creEList.Events[creEList.CurrentEvent].key.keysym.sym == SDLK_n) {
                CRE_TSetState(0, CRE_PS_WAKEUP);
                This->State = CRE_PS_DEAD;
                creEList.Size = 0;
                MenuOption = OP_NEWGAME;
            }

            if(creEList.Events[creEList.CurrentEvent].key.keysym.sym ==
              SDLK_s) {
                CRE_LetPrcsAlone(0);
                MenuOption = OP_EXIT;
            }
        }
    }

}

/* Destructor */
void PExitWindows_Free(creProcess * This)
{
    /* Liberamos memoria */
    free(This);
}


/*
 * PMainMenu
 * Proceso que gestiona el menu principal del juego
 */

/* Definción de clase */
creProcess PMainMenu = {0, TYPE_PMENU, CRE_PS_CLASS, "Menu principal",
    NULL, PMainMenu_New, PMainMenu_Loop, PMainMenu_Free,
    NULL, 400, 300, Z_PMENU, 255, 0, 100, 100, NULL};

/* Constructor */
creProcess * PMainMenu_New()
{
    creProcess * This;

    /* Creamos el proceso */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PMainMenu;
    This->State = CRE_PS_DEFAULT;

    This->Graph = MGfMisc->Gfx[IND_MENUBG];

    CRE_AddProcess(This);

    return This;
}

/* Bucle */
void PMainMenu_Loop(creProcess * This)
{
    /* Comprobamos los eventos */
    for(creEList.CurrentEvent = 0; creEList.CurrentEvent < creEList.Size;
      creEList.CurrentEvent++)
        if(creEList.Events[creEList.CurrentEvent].type == SDL_QUIT ||
          (creEList.Events[creEList.CurrentEvent].type == SDL_KEYDOWN &&
          creEList.Events[creEList.CurrentEvent].key.keysym.sym ==
          SDLK_ESCAPE)) {
            /* Mostramos la ventana de confirmación */
            PExitWindows.New();
        }
}

/* Destructor */
void PMainMenu_Free(creProcess * This)
{
    /* Liberamos memoria */
    free(This);
}


/*
 * PMainMenu
 * Proceso que gestiona el menu principal del juego
 */

/* Definción de clase */
creProcess PArrow = {0, TYPE_PARROW, CRE_PS_CLASS, "Flecha",
    NULL, PArrow_New, PArrow_Loop, PArrow_Free, NULL, 400, 300, Z_PARROW, 255,
    0, 100, 100, NULL};

/* Constructor */
creProcess * PArrow_New()
{
    creProcess * This;

    /* Creamos el proceso */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PArrow;
    This->State = CRE_PS_DEFAULT;

    This->Graph = MGfMisc->Gfx[IND_ARROW];

    This->Data = malloc(sizeof(PArrowData));
    *((PArrowData *) This->Data) = (PArrowData) {0, 0};

    CRE_AddProcess(This);

    return This;
}

/* Bucle */
void PArrow_Loop(creProcess * This)
{
    PArrowData * Info = (PArrowData *) This->Data;

    /* Comprobamos los eventos */
    for(creEList.CurrentEvent = 0; creEList.CurrentEvent < creEList.Size;
      creEList.CurrentEvent++)
        if(creEList.Events[creEList.CurrentEvent].type == SDL_KEYDOWN) {
            if(creEList.Events[creEList.CurrentEvent].key.keysym.sym ==
              SDLK_UP)
                MenuOption = (!MenuOption) ? OP_COUNT - 1 : MenuOption - 1;
            if(creEList.Events[creEList.CurrentEvent].key.keysym.sym ==
              SDLK_DOWN)
                MenuOption =  (MenuOption + 1) % OP_COUNT;
            if(creEList.Events[creEList.CurrentEvent].key.keysym.sym ==
              SDLK_SPACE || creEList.Events[creEList.CurrentEvent].key.keysym.
              sym == SDLK_RETURN)
                switch(MenuOption) {
                    case OP_NEWGAME:
                    default:
                        CRE_EndLoop();
                        break;
                    case OP_INFO:
                        PCredits.New();
                        break;
                    case OP_EXIT:
                        PExitWindows.New();
                        break;
                }
        }

    /* Gestión de la posición (Selección del menu) */
    switch(MenuOption) {
        case OP_NEWGAME:
        default:
            This->Y = 485;
            This->X = 560;
            break;

        case OP_INFO:
            This->Y = 520;
            This->X = 680;
            break;

        case OP_EXIT:
            This->Y = 555;
            This->X = 655;
            break;
    }

    /* Gestión de la animación */
    if(Info->Dir)
        if(Info->Desp >= MAX_ARROW_DESP)
            Info->Dir = ~Info->Dir;
        else
            Info->Desp += MAX_ARROW_INC;
    else
        if(Info->Desp <= -MAX_ARROW_DESP)
            Info->Dir = ~Info->Dir;
        else
            Info->Desp -= MAX_ARROW_INC;
    This->X += Info->Desp;

}

/* Destructor */
void PArrow_Free(creProcess * This)
{
    /* Liberamos memoria */
    free(This);
}


/*
 * PMainMenu
 * Proceso que gestiona el menu principal del juego
 */

/* Definción de clase */
creProcess PCredits = {0, TYPE_PCREDITS, CRE_PS_CLASS, "Creditos",
    NULL, PCredits_New, PCredits_Loop, PCredits_Free, NULL, 400, 300,
    Z_PCREDITS, 0, 0, 100, 100, NULL};

/* Constructor */
creProcess * PCredits_New()
{
    creProcess * This;

    /* Creamos el proceso */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PCredits;
    This->State = CRE_PS_DEFAULT;

    /* Inicializamos las variables del proceso */
    This->Graph = MGfMisc->Gfx[IND_CREDITS];
    This->X = -GOTO_X;
    This->Data = malloc(sizeof(Uint8));
    *((Uint8 *) This->Data) = CSTATE_ON;

    /* Lo ejecutamos y pausamos todos los demás procesos */
    CRE_TSetState(0, CRE_PS_PAUSE);
    CRE_AddProcess(This);
    /*
     * Limpiamos la lista de eventos ya que el proceso esperará cualquier
     * evento para continuar
     */
    creEList.Size = 0;

    return This;
}

/* Bucle */
void PCredits_Loop(creProcess * This)
{
    Uint8 * State = ((Uint8 *) This->Data);

    switch(*State) {

        case CSTATE_ON:
            This->Alpha = (This->Alpha + ALPHA_SPD >= 255) ? 255 : This->Alpha
              + ALPHA_SPD;
            if(This->X >= GOTO_X)
                *State = CSTATE_NONE;
            else
                This->X = (This->X + GOTO_SPD > GOTO_X) ? GOTO_X :
                  This->X + GOTO_SPD;
            break;

        case CSTATE_OFF:
            This->Alpha = (This->Alpha - ALPHA_SPD <= 0) ? 0 :
              This->Alpha - ALPHA_SPD;
            if(This->X <= -GOTO_X) {
                This->State = CRE_PS_DEAD;
                CRE_TSetState(0, CRE_PS_WAKEUP);
            } else
                This->X = (This->X - GOTO_SPD < -GOTO_X) ? - GOTO_X :
                  This->X - GOTO_SPD;
            break;

        default:
        case CSTATE_NONE:
            /* Comprobamos los eventos */
            for(creEList.CurrentEvent = 0; creEList.CurrentEvent <
              creEList.Size; creEList.CurrentEvent++)
                if(creEList.Events[creEList.CurrentEvent].type == SDL_QUIT ||
                  (creEList.Events[creEList.CurrentEvent].type == SDL_KEYDOWN)){
                    /* Mostramos la ventana de confirmación */
                    *State = CSTATE_OFF;
                }
            break;
    }
}

/* Destructor */
void PCredits_Free(creProcess * This)
{
    /* Liberamos memoria */
    free(This);
}


/*
 * PEndWindows
 * Ventana que se muestra al final de una partida para indicar si el usuario
 * ha ganado o perdido la partida.
 */

/* Definción de clase */
creProcess PEndWindows = {0, TYPE_PEND, CRE_PS_CLASS, "Ventana de info",
    NULL, PEndWindows_New, PEndWindows_Loop, PEndWindows_Free,
    NULL, 400, 268, Z_PEND, 55, 0, 100, 100, NULL};

/* Constructor */
creProcess * PEndWindows_New(Sint32 GraphId)
{
    creProcess * This;

    /* Creamos el proceso */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PEndWindows;
    This->State = CRE_PS_DEFAULT;

    This->Graph = MGfMisc->Gfx[GraphId];

    /* Lo ejecutamos y pausamos todos los demás procesos */
    CRE_TSetState(0, CRE_PS_PAUSE);
    CRE_AddProcess(This);

    return This;
}

/* Bucle */
void PEndWindows_Loop(creProcess * This)
{
    if(This->Alpha < 255) {
        This->Alpha += 10;
    }

    for(creEList.CurrentEvent = 0; creEList.CurrentEvent < creEList.Size;
      creEList.CurrentEvent++)
        if(creEList.Events[creEList.CurrentEvent].type == SDL_KEYDOWN)
            CRE_EndLoop();
}

/* Destructor */
void PEndWindows_Free(creProcess * This)
{
    /* Liberamos memoria */
    free(This);
}


/*
 * PEndWindows
 * Ventana que se muestra al final de una partida para indicar si el usuario
 * ha ganado o perdido la partida.
 */

/* Definción de clase */
creProcess PWinScreen = {0, TYPE_PWIN, CRE_PS_CLASS, "Pantalla de victoria",
    NULL, PWinScreen_New, PWinScreen_Loop, PWinScreen_Free,
    NULL, 400, 300, Z_PWIN, 255, 0, 100, 100, NULL};

/* Constructor */
creProcess * PWinScreen_New(Uint32 Score)
{
    creProcess * This;
    char Tmp[64];

    /* Creamos el proceso */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PWinScreen;
    This->State = CRE_PS_DEFAULT;
    /* Asignamos su gráfico */
    This->Graph = MGfMisc->Gfx[IND_WINBG];
    /* Lo ejecutamos y pausamos todos los demás procesos */
    CRE_LetPrcsAlone(0);
    /* Mostramos la puntuación */
    sprintf(Tmp, PWIN_TEXT, Score);
    PText.New(Tmp, 400, 500, TTFCoco, TEXT_CONST);
    /* Añadimos el proceso */
    CRE_AddProcess(This);

    return This;
}

/* Bucle */
void PWinScreen_Loop(creProcess * This)
{
    for(creEList.CurrentEvent = 0; creEList.CurrentEvent < creEList.Size;
      creEList.CurrentEvent++)
        if(creEList.Events[creEList.CurrentEvent].type == SDL_KEYDOWN)
            CRE_EndLoop();
}

/* Destructor */
void PWinScreen_Free(creProcess * This)
{
    /* Liberamos memoria */
    free(This);
}


/*
 * Implementación de funciones
 */

/*
 * COCO_MainMenu
 * Muestra el menu principal y devuelve la opción que ha elegido el usuario.
 */
Uint8 COCO_MainMenu(void) {

    /* Eliminamos todos los procesos anteriores */
    CRE_LetPrcsAlone(0);

    /* Creamos el menu principal y la flecha de selcección */
    MenuOption = OP_NEWGAME;
    PMainMenu.New();
    PArrow.New();

    /* Y empezamos el bucle pricipal */
    CRE_StartLoop();

    /* Devolvemos la opción escogida por el usuario */
    return MenuOption;
}

