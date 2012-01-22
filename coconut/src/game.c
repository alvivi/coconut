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
 * game.c
 * Implementa las funciones y procesos que controlan una partida de un escenraio
 * o nivel.
 */

#include <string.h>
#include <core.h>
#include <SDL/SDL_ttf.h>
#include "misc.h"
#include "game.h"

/*
 * Variable globales
 */

/* Indica si se ha empezado una partida */
int HasGameInit = 0;
/* Tamaño mitad de un tile del escenario actual, utilizado frecuentemente */
int AuxMidSize = 0;
/* Estrellas que quedan por comer */
Uint32 StarsCount = 0;
/* Vidas que quedan */
Uint8 * LifesCount;
/* Puntuación total */
Uint32 * Score;
/* Tiempo restante */
Sint32 GameTime = 0;
/* Cadenas que guardan la información anterior en modo texto */
char InfoText[4][32];
/* Gráficos del comecocos */
creMGf * MGfCoco = NULL;
/* Gráficos de las estrellas */
creMGf * MGfStar = NULL;
/* Gráficos de los fantasmas */
creMGf * MGfGhost = NULL;
/* Escenario actual */
creMSc * MScScen = NULL;
/* Fuente de texto  grande*/
TTF_Font * TTFCoco = NULL;
/* Fuente de texto pequeña */
TTF_Font * TTFCocoSmall = NULL;
/* Puntero a la única instancia que puede haber del comecocos */
creProcess * CurrentCoco = NULL;


/*
 * Implementación de los métododos de los procesos.
 */

/*
 * PDownBar
 * Barra de información de la partida que aparece en la parte inferior del
 * juego.
 */

/* Definción de clase */
creProcess PDownBar = {0, TYPE_PDBAR, CRE_PS_CLASS, "Barra de info", NULL,
    PDownBar_New, PDownBar_Loop, PDownBar_Free,   NULL, 400, 300,
    Z_DBAR, 255, 0, 100, 100, NULL};

/* Constructor */
creProcess * PDownBar_New()
{
    creProcess * This;

    /* Creamos la instancia del proceso */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PDownBar;
    This->State = CRE_PS_DEFAULT;
    /* Le asignamos el gráfico y su posición */
    This->Graph = MGfMisc->Gfx[IND_DBAR];
    This->Y = 600 - MGfMisc->Gfx[1]->h / 2;
    /* Añadimos la instancia a la lista de procesos */
    CRE_AddProcess(This);

    /* Creamos los textos de información */
    PText.New("Vidas",     360, 555, TTFCocoSmall, TEXT_CONST);
    PText.New(InfoText[0], 400, 575, TTFCoco,      TEXT_VAR);
    PText.New("Puntos",    750, 545, TTFCocoSmall, TEXT_CONST);
    PText.New(InfoText[1], 720, 575, TTFCoco,      TEXT_VAR);
    PText.New("Estrellas",  50, 540, TTFCocoSmall, TEXT_CONST);
    PText.New(InfoText[2],  50, 570, TTFCoco,      TEXT_VAR);
    PText.New("Tiempo",    160, 545, TTFCocoSmall, TEXT_CONST);
    PText.New(InfoText[3], 160, 575, TTFCoco,      TEXT_VAR);

    return This;
}

/* Bucle */
void PDownBar_Loop(creProcess * This)
{
    /* Actualizamos los textos en pantalla */
    sprintf(InfoText[0], "%d", *LifesCount);
    sprintf(InfoText[1], "%d", *Score);
    sprintf(InfoText[2], "%d", StarsCount);
    sprintf(InfoText[3], "%d", GameTime);
}

/* Destructor */
void PDownBar_Free(creProcess * This)
{
    free(This);
}


/*
 * PCoco
 * Proceso que implementa el comecocos.
 */

/* Definción de clase */
creProcess PCoco = {0, TYPE_PCOCO, CRE_PS_CLASS, "Comecocos", NULL, PCoco_New,
    PCoco_Loop, PCoco_Free,   NULL, 0, 0, Z_COCO, 255, 0, 100, 100, NULL};

/* Constructor */
creProcess * PCoco_New()
{
    creProcess * This;

    /* Creamos el proceso */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = (creProcess) PCoco;
    This->State = CRE_PS_DEFAULT;

    /* Cargamos el gráfico por defecto */
    This->Graph = MGfCoco->Gfx[IND_COCO_FRONT];

    /* Calculamos la posición inicial */
    This->X = MScScen->KeyPoints[IND_KP_START].X * MScScen->Size + AuxMidSize;
    This->Y = MScScen->KeyPoints[IND_KP_START].Y * MScScen->Size + AuxMidSize;

    /* Asignamos memoria para los datos y los inicializamos */
    This->Data = malloc(sizeof(PCocoData));
    *((PCocoData *)This->Data) = (PCocoData){MScScen->KeyPoints[IND_KP_START].X,
        MScScen->KeyPoints[IND_KP_START].Y, 0, IN_SUGAR, IND_COCO_FRONT,
        SDL_GetTicks(), TO_FRONT, 1, 0};

    /* Añadimos el proceso */
    CRE_AddProcess(This);

    return This;
}

/* Bucle */
void PCoco_Loop(creProcess * This)
{
    PCocoData * Info = (PCocoData *) This->Data;

    /* Gestionamos los eventos de teclado */
    for(creEList.CurrentEvent = 0; creEList.CurrentEvent < creEList.Size;
      creEList.CurrentEvent++)
        if(creEList.Events[creEList.CurrentEvent].type == SDL_KEYDOWN) {

            if(creEList.Events[creEList.CurrentEvent].key.keysym.sym ==
              SDLK_LEFT)
                Info->NextDir = TO_LEFT;

            else if(creEList.Events[creEList.CurrentEvent].key.keysym.sym ==
              SDLK_RIGHT)
                Info->NextDir = TO_RIGHT;

            else if(creEList.Events[creEList.CurrentEvent].key.keysym.sym ==
              SDLK_UP)
                Info->NextDir = TO_UP;

            else if(creEList.Events[creEList.CurrentEvent].key.keysym.sym ==
              SDLK_DOWN)
                Info->NextDir = TO_DOWN;
        }

    /*
     * Si no hay ninguna animación en marcha, y el usuario nos ha indicado que
     * que nos deplazemos, lo hacemos, siempre que sea posible
     */
    if(Info->Enable)
        switch(Info->NextDir) {
            /* Desplazamiento a la izquierda */
            case TO_LEFT:
                /* Comprobamos si es posible el desplazamiento */
                if(MScScen->Map[MAX(0,(Info->X - 1)) + Info->Y *
                  MScScen->W] == 0) {
                    /* Si lo es, preparamos la animación */
                    Info->Enable = Info->Loop = 0;
                    Info->Frame = IND_COCO_LEFT;
                    Info->Dir = TO_LEFT;
                /*
                 * Si no lo es, y anteriormente no estaba activo la animación de
                 * 'en frente', es decir, sin movimiento, la activamos.
                 */
                } else if(Info->Dir != TO_FRONT) {
                  Info->Frame = IND_COCO_FRONT;
                  Info->NextDir = Info->Dir = TO_FRONT;
                  Info->Loop = 0;
                }
                break;

            /* Desplazamiento a la derecha */
            case TO_RIGHT:
                if(MScScen->Map[MIN(MScScen->W + 1, Info->X + 1) + Info->Y *
                  MScScen->W] == 0) {
                    Info->Enable = Info->Loop = 0;
                    Info->Frame = IND_COCO_RIGHT;
                    Info->Dir = TO_RIGHT;
                } else if(Info->Dir != TO_FRONT) {
                  Info->Frame = IND_COCO_FRONT;
                  Info->NextDir = Info->Dir = TO_FRONT;
                  Info->Loop = 0;
                }
                break;

            /* Desplazamiento hacia arriba */
            case TO_UP:
                if(MScScen->Map[Info->X + (MAX(0, (Info->Y - 1)) *
                  MScScen->W)] == 0) {
                    Info->Enable = Info->Loop = 0;
                    Info->Frame = IND_COCO_UP;
                    Info->Dir = TO_UP;
                } else if(Info->Dir != TO_FRONT) {
                  Info->Frame = IND_COCO_FRONT;
                  Info->NextDir = Info->Dir = TO_FRONT;
                  Info->Loop = 0;
                }
                break;

            /* Desplazamiento hacia abajo */
            case TO_DOWN:
                if(MScScen->Map[Info->X + (MIN((MScScen->H - 1), (Info->Y + 1))
                  * MScScen->W)] == 0) {
                    Info->Enable = Info->Loop = 0;
                    Info->Frame = IND_COCO_DOWN;
                    Info->Dir = TO_DOWN;
                } else if(Info->Dir != TO_FRONT) {
                  Info->Frame = IND_COCO_FRONT;
                  Info->NextDir = Info->Dir = TO_FRONT;
                  Info->Loop = 0;
                }
                break;
        }

    /* Gestionamos las animaciones */
    switch(Info->Dir) {
        /* Animación de movimiento hacia la izquierda */
        case TO_LEFT:
            if(Info->Loop) {
                if(Info->Frame == IND_COCO_LEFT) {
                    Info->X--;
                    Info->Enable = 1;
                } else {
                    Info->Frame--;
                    This->X -= 3;
                }
            } else {
                if(Info->Frame == IND_COCO_LEFT + IND_COCO_LEFT_SIZE) {
                    Info->Loop = ~Info->Loop;
                    This->X -= 2;
                } else {
                    Info->Frame++;
                    This->X -= 3;
                }
            }
            break;

        /* Animación de movimiento hacia la derecha */
        case TO_RIGHT:
            /*
             * Según el estado en el que este la animación, es decir, incremento
             * de la animación positivo o negativo (ida o vuelta de la ani)
             */
            /* Vuelta (incremento negativo) */
            if(Info->Loop) {
                /* En caso de que la animación llege de nuevo al principio */
                if(Info->Frame == IND_COCO_RIGHT) {
                    /* Avanzamos la posición en el mapa de tiles */
                    Info->X++;
                    /* Indicamos que hemos terminado la animación */
                    Info->Enable = 1;
                /* Si la animación no ha llegado a su principio */
                } else {
                    /* Actualizamos la imagen (Frame) */
                    Info->Frame--;
                    /* Actualizamos la posición en pantalla (no la del mapa) */
                    This->X += 3;
                }
            /* Ida (incremento positivo) */
            } else {
                /* Si la animación ha llegado al final */
                if(Info->Frame == IND_COCO_RIGHT + IND_COCO_RIGHT_SIZE) {
                    /* Indicamos que vamos de vuelta */
                    Info->Loop = ~Info->Loop;
                    /* Avanzamos la posición en pantalla */
                    This->X += 2;
                /* Si aún no a llegado */
                } else {
                    /* Actualizamos el frame y la posición en pantalla */
                    Info->Frame++;
                    This->X += 3;
                }
            }
            break;

        /* Animación de movimiento hacia arriba */
        case TO_UP:
            if(Info->Loop) {
                if(Info->Frame == IND_COCO_UP) {
                    Info->Y--;
                    Info->Enable = 1;
                } else {
                    Info->Frame--;
                    This->Y -= 3;
                }
            } else {
                if(Info->Frame == IND_COCO_UP + IND_COCO_UP_SIZE) {
                    Info->Loop = ~Info->Loop;
                    This->Y -= 2;
                } else {
                    Info->Frame++;
                    This->Y -= 3;
                }
            }
            break;

        /* Animación de movimiento hacia abajo */
        case TO_DOWN:
            if(Info->Loop) {
                if(Info->Frame == IND_COCO_DOWN) {
                    Info->Y++;
                    Info->Enable = 1;
                } else {
                    Info->Frame--;
                    This->Y += 3;
                }
            } else {
                if(Info->Frame == IND_COCO_DOWN + IND_COCO_DOWN_SIZE) {
                    Info->Loop = ~Info->Loop;
                    This->Y += 2;
                } else {
                    Info->Frame++;
                    This->Y += 3;
                }
            }
            break;

        /* Animación de no movimiento, estado 'de frente' */
        default:
            if(Info->Loop) {
                if(Info->Frame == IND_COCO_FRONT)
                    Info->Loop = ~Info->Loop;
                else
                    Info->Frame--;
            } else {
                if(Info->Frame == IND_COCO_FRONT + IND_COCO_FRONT_SIZE)
                    Info->Loop = ~Info->Loop;
                else
                    Info->Frame++;
            }
            break;
    }

    /*
     * En caso de que el comecocos este en el estado invulnerable provocamos
     * un parpadeo temporal.
     */
    if(Info->State == IN_SUGAR) {
        if((SDL_GetTicks() - Info->SugarTime) >= 2000)
            Info->State = IN_NONE;

        if(Info->Frame % 2)
            This->Graph = NULL;
        else
            This->Graph = MGfCoco->Gfx[Info->Frame];
    /* Si no esta en este estado mostramos su gráfico normalmente */
    } else
        This->Graph = MGfCoco->Gfx[Info->Frame];
}

/* Destructor */
void PCoco_Free(creProcess * This)
{
    /* Liberamos memoria */
    if (This->Data != NULL)
      free(This->Data);

    free(This);
}


/*
 * PStar
 * Proceso que representa una estrella (comida del comecocos)
 */

/* Definción de clase */
creProcess PStar = {0, TYPE_PSTAR, CRE_PS_CLASS, "Estrella", NULL, PStar_New,
    PStar_Loop, PStar_Free, NULL, 0, 0, Z_STAR, 255, 0, 100, 100, NULL};

/* Constructor */
creProcess * PStar_New(Sint32 X, Sint32 Y)
{
    creProcess * This;

    /* Creamos la instancia del proceso */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PStar;
    This->State = CRE_PS_DEFAULT;

    /* Le asignamos su gráficco */
    This->Graph = MGfStar->Gfx[IND_STAR];

    /* Calculamos su posición en pantalla */
    This->X = X * MScScen->Size + AuxMidSize;
    This->Y = Y * MScScen->Size + AuxMidSize;

    /* Creamos e inicializamos los datos de la instancia */
    This->Data = malloc(sizeof(PStarData));
    *((PStarData *)This->Data) = (PStarData) {X, Y,
      IND_STAR + random() % IND_STAR_SIZE};

    /* Añadimos la instancia a la lista */
    CRE_AddProcess(This);

    /* Indicamos que se ha creado un estrella */
    StarsCount++;

    return This;
}

/* Bucle */
void PStar_Loop(creProcess * This)
{
    PStarData * Info = (PStarData *) This->Data;
    PCocoData * CInfo = (PCocoData *) CurrentCoco->Data;

    /* En el caso de que el comecocos este sobre la estrella actual */
    if(Info->X == CInfo->X && Info->Y == CInfo->Y) {
        /* Eliminamos la estrella */
        This->State = CRE_PS_DEAD;
        /* Añadimos puntos a la puntuación del jugador */
        *Score += GameTime;
        /* Indicamos que ha desaparecido una estrella */
        StarsCount--;
    }

    /* Actualizamos el estado de la animación */
    Info->Frame = (++Info->Frame) % IND_STAR_SIZE;
    This->Graph = MGfStar->Gfx[Info->Frame];

}

/* Destructor */
void PStar_Free(creProcess * This)
{
    /* Liberamos memoria */
  if (This->Data != NULL)
    free(This->Data);
  free(This);
}



/*
 * PGhost
 * Proceso de un fantasma.
 */

/* Definción de clase */
creProcess PGhost = {0, TYPE_PGHOST, CRE_PS_CLASS, "Fantasma", NULL, PGhost_New,
    PGhost_Loop, PGhost_Free, NULL, 0, 0, Z_GHOSTN, A_GHOSTN, 0, 100, 100,
    NULL};

/* Constructor */
creProcess * PGhost_New(Sint32 X, Sint32 Y)
{
    creProcess * This;

    /* Creamos la instancia */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PGhost;
    This->State = CRE_PS_DEFAULT;

    /* Le asignamos un gráfico aleatoriamente */
    This->Graph = MGfGhost->Gfx[IND_GHOST + (random() % IND_GHOST_SIZE)];

    /* Calculamos su posición */
    This->X = X * MScScen->Size + AuxMidSize;
    This->Y = Y * MScScen->Size + AuxMidSize;

    /* Le asignamos un nivel de transparencia aleatorio */
    This->Alpha = A_GHOSTN + random() % (A_GHOSTN - A_GHOSTN_MAX);

    /* Incializamos la información de la instancia */
    This->Data = malloc(sizeof(PGhostData));
    *((PGhostData *)This->Data) = (PGhostData) {X, Y, 0, TO_FRONT, 1, 0};

    /* Añadimos el proceso a lista */
    CRE_AddProcess(This);

    return This;
}

/* Bucle */
void PGhost_Loop(creProcess * This)
{
    Uint8 Moves[4] = {0, 0, 0, 0}, MCount = 0, Tmp;
    PGhostData * Info = (PGhostData *) This->Data;
    PCocoData * CInfo = (PCocoData *) CurrentCoco->Data;

    /* Comprobamos los movimientos disponibles */
    /* Izquierda */
    Tmp = MScScen->Map[MAX(0,(Info->X - 1)) + Info->Y * MScScen->W];
    if(Tmp == 0 || Tmp == 11) Moves[MCount++] = TO_LEFT;
    /* Derecha */
    Tmp = MScScen->Map[MIN(MScScen->W + 1, Info->X + 1) + Info->Y * MScScen->W];
    if(Tmp == 0 || Tmp == 11) Moves[MCount++] = TO_RIGHT;
    /* Arriba */
    Tmp = MScScen->Map[Info->X + (MAX(0, (Info->Y - 1)) * MScScen->W)];
    if(Tmp == 0 || Tmp == 11) Moves[MCount++] = TO_UP;
    /* Abajo */
    Tmp = MScScen->Map[Info->X + (MIN((MScScen->H - 1), (Info->Y + 1))
      * MScScen->W)];
    if(Tmp == 0 || Tmp == 11) Moves[MCount++] = TO_DOWN;


    /* Implementación de la IA */
    if(Info->Enabled) {

        if(MCount != 0)
            if(Info->Dir != TO_FRONT)
                if(random() % 100 < 90)
                    if(random() % 100 < 98)
                        goto IA_MOV;
                    else
                        goto IA_RANDOM_MOV;
                else
                    goto IA_NOMOV;
            else
                goto IA_RANDOM_MOV;
        else
            goto IA_NOMOV;

        IA_RANDOM_MOV:
            Info->Dir = Moves[random() % MCount];
            Info->Frame = 0;
            Info->Enabled = 0;
            goto IA_END;

        IA_MOV:
            for(Tmp = 0; Tmp < MCount; Tmp++)
                if(Moves[Tmp] == Info->Dir) {
                    Info->Frame = 0;
                    Info->Enabled = 0;
                    goto IA_END;
                }
            goto IA_RANDOM_MOV;

        IA_NOMOV:
            Info->Dir = TO_FRONT;
    }
    IA_END:

    /* Animación de movimiento */
    if(Info->Dir != TO_FRONT) {
        switch(Info->Dir) {
            case TO_LEFT:
                This->X -= 4;
                Info->Frame += 4;
                if(Info->Frame >= MScScen->Size) {
                    Info->X--;
                    Info->Enabled = 1;
                }
                break;
            case TO_RIGHT:
                This->X += 4;
                Info->Frame += 4;
                if(Info->Frame >= MScScen->Size) {
                    Info->X++;
                    Info->Enabled = 1;
                }
                break;
            case TO_UP:
                This->Y -= 4;
                Info->Frame += 4;
                if(Info->Frame >= MScScen->Size) {
                    Info->Y--;
                    Info->Enabled = 1;
                }
                break;
            case TO_DOWN:
                This->Y += 4;
                Info->Frame += 4;
                if(Info->Frame >= MScScen->Size) {
                    Info->Y++;
                    Info->Enabled = 1;
                }
                break;
        }
    }

    /* Animación de transparencia */
    if(CInfo->State != IN_SHOCK) {
        This->Z = Z_GHOSTN;
        if(Info->Alp) {
            if(This->Alpha < A_GHOSTN)
                Info->Alp = ~Info->Alp;
            else
                This->Alpha -= random() % 5;
        } else {
            if(This->Alpha > A_GHOSTN_MAX)
                Info->Alp = ~Info->Alp;
            else
                This->Alpha += random() % 5;
        }
    } else {
        This->Z = Z_GHOSTF;
        This->Alpha = A_GHOSTF;
    }

    /* Comprobamos si estamos tocando al coco, y lo matamos */
    if(Info->X == CInfo->X && Info->Y == CInfo->Y)
        switch(CInfo->State) {
            case IN_NONE:
                (*LifesCount)--;
                CRE_TSetState(TYPE_PCOCO, CRE_PS_DEAD);
		/* Si quedan vidas, creamos un nuevo comecocos */
		if(*LifesCount > 0)
		  CurrentCoco = PCoco.New();
		/* Sino, mostramos el mensaje de 'game over' */
		else
		  PEndWindows.New(IND_GAMEOVER);

                break;

            /* Si va ciego el coco, nos cargamos al fantasma */
            case IN_SHOCK:
                This->State = CRE_PS_DEAD;
                *Score += 5000;
                break;
        }
}

/* Destructor */
void PGhost_Free(creProcess * This)
{
    /* Liberamos memoria */
    if (This->Data != NULL)
      free(This->Data);

    free(This);
}


/*
 * PGlint
 * Es el brillo caracteristico de un comecocos ciego de azucar
 */

/* Definción de clase */
creProcess PGlint = {0, TYPE_GLINT, CRE_PS_CLASS, "Brillo", NULL, PGlint_New,
    PGlint_Loop, PGlint_Free,   NULL, 0, 0, Z_GLINT, 255, 0, 100, 100, NULL};

/* Constructor */
creProcess * PGlint_New()
{
    creProcess * This;

    /* Creamos la instancia */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PGlint;
    This->State = CRE_PS_DEFAULT;

    /* Asignamos el gráfico indicado */
    This->Graph = MGfCoco->Gfx[IND_GLINT];

    /* Damos a la instancia la misma posición que la del comecocos */
    This->X = CurrentCoco->X;
    This->Y = CurrentCoco->Y;

    /* Creamos e incializamos los datos de la instancia */
    This->Data = malloc(sizeof(Uint32));
    *((Uint32 *) This->Data) = SDL_GetTicks();

    /* Añadimos la instancia a la lista de procesos */
    CRE_AddProcess(This);

    return This;
}

/* Bucle */
void PGlint_Loop(creProcess * This)
{
    PCocoData * CInfo = (PCocoData *) CurrentCoco->Data;

    /* Actualizamos la posición del proceso */
    This->X = CurrentCoco->X;
    This->Y = CurrentCoco->Y;

    /* Giramos el gráfico 5º */
    This->Angle += 5000;

    /*
     * Si quedan menos de 3 segundo para que acabe el estado de shock, hacemos
     * que el gráfico se ponga intermintente.
     */
    if(SDL_GetTicks() - *((Uint32 *) This->Data) >= COCO_SHOCKTIME - 3000) {
        if(This->Graph != NULL)
            This->Graph = NULL;
        else
            This->Graph = MGfCoco->Gfx[IND_GLINT];
    }

    /* Si el tiempo se ha agotado, terminamos el estado de shock */
    if(SDL_GetTicks() - *((Uint32 *) This->Data) >= COCO_SHOCKTIME) {
        This->State = CRE_PS_DEAD;
        CInfo->State = IN_NONE;
    }

}

/* Destructor */
void PGlint_Free(creProcess * This)
{
    /* Liberamos memoria */
    if (This->Data != NULL)
      free(This->Data); 
    free(This);
}


/*
 * PCandy
 * Son los caramelos. Tienen azucar y al coco le altera.
 */

/* Definción de clase */
creProcess PCandy = {0, TYPE_CANDY, CRE_PS_CLASS, "Caramelo", NULL, PCandy_New,
    PCandy_Loop, PCandy_Free,   NULL, 0, 0, Z_CANDY, 255, 0, 100, 100, NULL};

/* Constructor */
creProcess * PCandy_New(Sint32 X, Sint32 Y)
{
    creProcess * This;

    /* Creamos la instancia */
    This = (creProcess *) malloc(sizeof(creProcess));
    *This = PCandy;
    This->State = CRE_PS_DEFAULT;

    /* Asignamos el gráfico indicado */
    This->Graph = MGfMisc->Gfx[IND_CANDY];

    /* Asignamos la posición indicada */
    This->X = X * MScScen->Size + AuxMidSize;
    This->Y = Y * MScScen->Size + AuxMidSize;

    /* Creamos e inicializamos los datos del proceso */
    This->Data = malloc(sizeof(PCandyData));
    *((PCandyData *)This->Data) = (PCandyData) {X, Y};

    /* Lo añadimos a la lista de procesos */
    CRE_AddProcess(This);

    return This;
}

/* Bucle */
void PCandy_Loop(creProcess * This)
{
    PStarData * Info = (PStarData *) This->Data;
    PCocoData * CInfo = (PCocoData *) CurrentCoco->Data;

    /* En caso de que el comecocos este encima  */
    if(Info->X == CInfo->X && Info->Y == CInfo->Y) {
        /* Eliminamos el caramelo */
        This->State = CRE_PS_DEAD;
        /* SI el comecocos tenia un destello (Shock) lo eliminamos */
        CRE_TSetState(TYPE_GLINT, CRE_PS_DEAD);
        /* Damos puntos, inciamos el destello y el estado de shock */
        *Score += 2500;
        PGlint.New();
        CInfo->State = IN_SHOCK;
    }
}

/* Destructor */
void PCandy_Free(creProcess * This)
{
    /* Liberamos memoria */
    if (This->Data != NULL)
      free(This->Data);
    free(This);
}


/*
 * PGame
 * Proceso que gestiona una partida
 */

/* Definción de clase */
creProcess PGame = {0, TYPE_PGAME, CRE_PS_CLASS, "CTRL (Escenario)", NULL,
    PGame_New, PGame_Loop, PGame_Free, NULL, 400, 268, Z_GAME, 255, 0, 100,
    100, NULL};

/* Constructor */
creProcess * PGame_New()
{
    creProcess * This;

    /* Si no hay otro en lista */
    if(!HasGameInit) {
        /* Creamos ela instancia */
        This = (creProcess *) malloc(sizeof(creProcess));
        *This = PGame;
        This->State = CRE_PS_DEFAULT;

        /* Construimos el mapa de tiles */
        This->Graph = CRE_DrawMSc(MScScen);

        /* Creamo e inicializamos la información del proceso */
        This->Data = malloc(sizeof(PGameData));
        *((PGameData *)This->Data) = (PGameData)
          {MScScen->KeyPoints[IND_KP_GHOST_TIMES].X, SDL_GetTicks(),
          SDL_GetTicks()};

        /* Lo añadimos a la lista de procesos */
        CRE_AddProcess(This);

        /* Indicamos que se ha iniciado una partida */
        HasGameInit = 1;

        return This;

    /* Si hay otro, no hacemos nada */
    } else
        return NULL;
}

/* Bucle */
void PGame_Loop(creProcess * This)
{
    PGameData * Info = (PGameData *) This->Data;

    /* Comprobamos los eventos */
    for(creEList.CurrentEvent = 0; creEList.CurrentEvent < creEList.Size;
      creEList.CurrentEvent++) {

        /* Si el usuario quiere terminar la partida */
        if(creEList.Events[creEList.CurrentEvent].type == SDL_QUIT ||
          (creEList.Events[creEList.CurrentEvent].type == SDL_KEYDOWN &&
          creEList.Events[creEList.CurrentEvent].key.keysym.sym ==
          SDLK_ESCAPE))
            /* Mostramos la ventana de confirmación */
            PExitWindows.New();

        /* Truco que salta al siguiente nivel */
        if(creEList.Events[creEList.CurrentEvent].type == SDL_KEYDOWN &&
          creEList.Events[creEList.CurrentEvent].key.keysym.sym == SDLK_F9)
            StarsCount = 0;
      }

    /* Creamos nuevos fantasmas cuando pase el tiempo establecido */
    if((SDL_GetTicks() - Info->Time) >= Info->WaitTime) {

        /* Actualizamos el tiempo de espera de creación de fantasmas */
        if(Info->WaitTime > GHOST_MIN_WAITTIME)
            Info->WaitTime -= MScScen->KeyPoints[IND_KP_GHOST_TIMES].Y;
        else
           Info->WaitTime = GHOST_MIN_WAITTIME;

        /* Creamos el moustro :D */
        PGhost.New(RANDOM(MScScen->KeyPoints[IND_KP_ULBOX].X,
          MScScen->KeyPoints[IND_KP_DRBOX].X + 1), RANDOM(MScScen->KeyPoints
          [IND_KP_ULBOX].Y, MScScen->KeyPoints[IND_KP_DRBOX].Y + 1));

        /* Actualizamos el reloj y volvemos a esperar */
        Info->Time = SDL_GetTicks();
    }

    /* Controlamos el tiempo global */
    if((SDL_GetTicks() - Info->GTime) >= 1000) {
        GameTime--;
        Info->GTime = SDL_GetTicks();
    }

    /* Observamos si el tiempo se ha acabado */
    if(GameTime <= 0)
        PEndWindows.New(IND_GAMEOVER);

    /* Si el comecocos ha conseguido la victoria */
    if(StarsCount == 0)
        PEndWindows.New(IND_LEVELUP);
}

/* Destructor */
void PGame_Free(creProcess * This)
{
    /* Liberamos memoria */
    SDL_FreeSurface(This->Graph);
    if (This->Data != NULL)
      free(This->Data);
    free(This);

    /* Indicamos que se ha terminado la partida */
    HasGameInit = 0;
}


/*
 * Implementación de funciones
 */

/*
 * COCO_StartLevel
 * Inicia una partida del archivo escenario indicado. Si por algún motivo no a
 * podido ser iniciada la partida (errores en el archivo del escenario u otros)
 * devuelve -1, sino todo ha ido bien devuelve 0;
 */
int COCO_StartLevel(char * FileName, Uint8 * Lifes, Uint32 * Score_ )
{
    Sint32 i, j, k, p;

    /* Cargamos el escenario */
    MScScen = CRE_LoadMSc(FileName);

    /* Sino existe devolvemos un error */
    if(MScScen == NULL)
        return -1;

    /* Inicializamos el tamaño de ajuste de los gráficos */
    AuxMidSize = MScScen->Size / 2;

    /* Eliminamos todos los procesos anteriores */
    CRE_LetPrcsAlone(0);

    /* Creamos el juego y empezamos el bucle (si no lo estaba antes) */
    PGame.New();

    /* Creamos procesos asociado al juego */
    PDownBar.New();
    CurrentCoco = PCoco.New();

    /* Creamos las estrellas en los huecos del mapa */
    StarsCount = 0;
    for(j = 0; j < MScScen->H; j++)
        for(i = 0; i < MScScen->W; i++) {
            p = 1;
            for(k = 0; k < MScScen->KPCount; k++)
                if(MScScen->KeyPoints[k].X == i && MScScen->KeyPoints[k].Y == j)
                    p = 0;

            if(p && MScScen->Map[i + j *MScScen->W] == 0)
                PStar.New(i, j);
        }

    /* Creamos los fantasmas en el mapa */
    for(i = 0; i < INIT_GHOSTS_NUMBER; i++) {
        PGhost.New(RANDOM(MScScen->KeyPoints[IND_KP_ULBOX].X,
          MScScen->KeyPoints[IND_KP_DRBOX].X + 1), RANDOM(MScScen->KeyPoints
          [IND_KP_ULBOX].Y, MScScen->KeyPoints[IND_KP_DRBOX].Y + 1));
    }

    /* Creamos los caramelos */
    for(i = IND_KP_BONUSP_START; i < MScScen->KPCount; i++)
        PCandy.New(MScScen->KeyPoints[i].X, MScScen->KeyPoints[i].Y);

    /* Actualizamos variables */
    Score = Score_;
    LifesCount = Lifes;
    GameTime = MScScen->KeyPoints[IND_KP_GLOBAL_TIME].X;

    /* Comenzamos el bucle principal y la partida */
    CRE_StartLoop();

    /* Liberamos memoria del mapa */
    CRE_FreeMSc(MScScen);

    /* Devolvemos si el jugador ha tenido éxito */
    return (StarsCount == 0);
}
