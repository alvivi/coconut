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
 * @file process.c
 * Archivo en el que se encuentran implementadas las funciones de gestión de
 * procesos. Más información en archivo de cabecera.
 **/


#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include "process.h"
#include "gfx.h"

/*TODO: Los canvas de los procesos siempre se vuelve a dibujar, aunque no hayan
        cambiado desde la itineración anterior, consumiendo en estos casos,
        gran cantidad del tiempo del procesador al aplicar los blendops.
        Relaizar una propiedad de los procesos denominada Hash que calcule si
        algún parámetro del proceso anterior ha sido modificado. Si no lo a sido
        no volver a dibujar el canvas. Datos a tener en cuenta: Graph, SizeW,
        SizeX, Angle y Alpha. Acosta de ganar tiempo se pierde memoria,
        comprobar si el gasto es rentable */


/*
 * Variables gloables al fichero
 */

/* Indica si un bucle se esta ejecutando */
Uint8 creAnyLoop = 0;
/* Puntero a la superficie de pantalla */
SDL_Surface * creScreen = NULL;
/* Puntero al primer proceso */
creProcess * creFirstProcess = NULL;
/* Velocidad del juego en SPF (El iverso de FPS) */
Uint32 creSPF = 0;
/* Velocidad real del juego en FPS */
float creRealFPS = 0;
/* Estructura que contiene los eventos de cada frame */
creEventsList creEList;
/* Color con el que se limpia la panatalla */
Uint32 creClearColor = 0;


/*
 * Implementación de funciones
 */


/*
 * CRE_GetNewPId
 * Devuelve una nueva id para un proceso válida y único para él.
 */
__inline Uint32 CRE_GetNewPId(void)
{
    static Uint32 IdCount = 1;

    return IdCount++;
}


/*
 * CRE_UpdateEList
 * Rellena la lista de eventos con los eventos ocurridos hasta el momento,
 * eliminando los que ya poseia.
 */
void CRE_UpdateEList(void)
{
    /* Reinicializamos la lista */
    creEList.Size = 0;

    /* Mientra haya eventos que procesar y espacio para almacenarlos */
    while(SDL_PollEvent(creEList.Events + creEList.Size) && (creEList.Size < CRE_MAX_SIM_EVENTS)){
        /* Si esta definido el modo debug */
        #ifdef CRE_PROCESS_DEBUG_MODE
            if(creEList.Events[creEList.Size].type == SDL_KEYDOWN &&
                creEList.Events[creEList.Size].key.keysym.sym == SDLK_F10)
                CRE_GetProcessesInfo(CRE_PROCESS_DEBUG_FILE);
        #endif
        /* Actualizamos la lista de eventos */
        creEList.Size++;
    }

}


/*
 * CRE_MainLoop
 * Fucnión que contiene el bucle principal de gestión de procesos. Es el
 * encargado de cuidar el estado de los procesos, así, si alguno tiene el flag
 * de "muerto" lo elimina, si esta "congelado" no lo ejecuta, etc. Como se ha
 * dicho, también ejecuta el método loop de todos los procesos que permiten
 * a estos actualizar sus valores. Luego, después de ejecutar cada proceso
 * hace un blit de su gráfico asociado a pantalla. Este bucle también es el
 * encargado de actualizar la lista de eventos en cada instante, para que los
 * procesos puedan acceder a ellos sin problemas e interaccionar con el usuario.
 * También es el encargado de gestionar la velocidad del juego. Pa ello se
 * establece una velocidad en FPS, y si el proceso de ejecución a sido más
 * rápido q los SPF (FPS^-1) el bucle espera para funcionar a la velocidad
 * adecuada.
 */
Sint32 CRE_MainLoop(void)
{
    /* Proceso que hemos ejecutado en el ciclo anterior */
    creProcess * LastProcess;
    /* Proceso que vamos a ejecutar en un determinado ciclo */
    creProcess * CurrentProcess;
    /* Punteros a procesos que son utilizados cuando se ordena la lista */
    creProcess * SortPro1, * SortPro2;
    /* Rectángulo usado temporalmente para el blit gráfico */
    SDL_Rect GfxRect;
    /* Indica el tiempo en ms que hemos tardado en ejecutar cada ciclo */
    Uint32 CurrentTime;
    /* Indica el tiempo que debe esperar el bucle */
    Sint32 WaitTime;
    /* Superficie temporal de dibujo, lienzo */
    SDL_Surface * Canvas = NULL;

    /* Limpiamos los todos los eventos pendientes */
    while(SDL_PollEvent(creEList.Events));

    /*
     * Mientras el usuario indique el bucle debe ejecutarse, y haya algún
     * proceso que gestionar.
     */
    while(creAnyLoop && creFirstProcess != NULL && creScreen != NULL) {

        /* Iniciamos el contador del tiempo para controlar los FPS */
        CurrentTime = SDL_GetTicks();

        /*  Recogemos los eventos que han sucedido hasta el momento */
        CRE_UpdateEList();

        /*
         * Buscamos entre la lista procesos que tengan estado de 'muerto' y los
         * eliminamos de la lista, también nos encargamos de liberar la memoria
         * de la instancia de forma manual (llamando al metodo free de la
         * instancia).
         */

        /* Preparamos los punteros para recorrer la lista */
        LastProcess = NULL;
        CurrentProcess = creFirstProcess;

        /* Recorremos la lista */
        while(CurrentProcess != NULL) {
            /* Comprobamos si tiene el estado de muerto */
            if((CurrentProcess->State & CRE_PS_DEAD) >> 3) {
                /* Si es el primero de la lista */
                if(LastProcess == NULL) {
                    /* Actualizamos la lista */
                    creFirstProcess = CurrentProcess->Next;
                    /* Eliminamos el proceso */
                    if(CurrentProcess->Free != NULL)
                        CurrentProcess->Free(CurrentProcess);
                    /* Continuamos recorriendo la lista */
                    CurrentProcess = creFirstProcess;
                /* Si esta en cualquier otra posición */
                } else {
                    /* Actualizamos la lista */
                    LastProcess->Next = CurrentProcess->Next;
                    /* Eliminamos el proceso */
                    if(CurrentProcess->Free != NULL)
                        CurrentProcess->Free(CurrentProcess);
                    /* Continuamos recorriendo la lista */
                    CurrentProcess = LastProcess->Next;
                }
            /* Si no esta muerto continuamos normalmente */
            } else {
                LastProcess = CurrentProcess;
                CurrentProcess = CurrentProcess->Next;
            }
        }

        /* Verificamos que qeda algún proceso vivo */
        if(creFirstProcess == NULL)
            continue;

        /*
         * Actualizamos la lista de procesos y observamos si esta bien ordenada
         * con respecto a Z de cada proceso. Por defecto, los procesos se añaden
         * a la lista de forma ordenada (de menor Z a mayor Z), pero es posible
         * que un proceso cambie su Z, por lo que debemos reorganizar la lista.
         */

        /* Preparamos los punteros para recorrer la lista */
        LastProcess = creFirstProcess;
        CurrentProcess = LastProcess->Next;

        /* Recorremos la lista */
        while(CurrentProcess != NULL) {
            /* Si el elemento esta desordenado, buscamos su posición */
            if(CurrentProcess->Z < LastProcess->Z) {
                /* Caso especial, su posición es la primera de la lista  */
                if(CurrentProcess->Z <= creFirstProcess->Z) {
                    /* Desenlazamos el elemento */
                    LastProcess->Next = CurrentProcess->Next;
                    /* Y lo enlazamos en la primera posición */
                    CurrentProcess->Next = creFirstProcess;
                    creFirstProcess = CurrentProcess;
                /* Caso general, buscamos su posición */
                } else {
                    /* Preparamos punteros para volver a recorrer la lista */
                    SortPro1 = creFirstProcess;
                    SortPro2 = SortPro1->Next;
                    /* Recorremos la lista hasta que econtramos su sitio */
                    while(SortPro2 != NULL) {
                        if(CurrentProcess->Z <= SortPro2->Z)
                            break;
                        SortPro1 = SortPro2;
                        SortPro2 = SortPro2->Next;
                    }
                    /* Desenlazamos el elemento */
                    LastProcess->Next = CurrentProcess->Next;
                    /* Y lo enlazamos en su nueva posición */
                    SortPro1->Next = CurrentProcess;
                    CurrentProcess->Next = SortPro2;
                }
                /* Avanzamos, pero solo CurrentProcess, ya que es diferente */
                CurrentProcess = LastProcess->Next;
            /* Si el elemento esta ordenado continuamos al siguiente */
            } else {
            /* Avanzamos los punteros  de la lista */
            LastProcess = CurrentProcess;
            CurrentProcess = CurrentProcess->Next;
            }
        }


        /* Preparamos las variables para recorrer la lista de procesos */
        LastProcess = NULL;
        CurrentProcess = creFirstProcess;

        /*
         * En este bucle se actuliza el estado de los procesos, se ejecutan
         * sus métodos loop, se actualiza el estado del bucle principal y se
         * hace el blit básico del gráfico del proceso a la pantalla.
         */
        while(CurrentProcess != NULL) {

            /* Para poder ejecutar un proceso hay que tener en cuenta dos
             * flags de estado muy importantes que alteran radicalmente el
             * comportamiento de éste. El primero es "dead" en cuyo caso
             * eliminamos el proceso y actualizamos la lista de proceso. El
             * segundo es "wakeup", en el cual debemos restablecer los flags.
             */

            /* Comprobamos el estado de "wakeup" por si debemos restablecerlo */
            if((CurrentProcess->State & CRE_PS_WAKEUP) >> 7)
                /* Limpiamos los estados temporales */
                CurrentProcess->State &= 0x0F;


            /*
             * Ejecutamos el método loop del proceso actual si esta disponible
             * y si estado no es el de "pausa" o "congelado"
             */
            if((CurrentProcess->Loop != NULL) &&
               !((CurrentProcess->State & CRE_PS_PAUSE) >> 6) &&
               !((CurrentProcess->State & CRE_PS_FREEZE) >> 4))
                CurrentProcess->Loop(CurrentProcess);

            /* Comprobamos si el proceso ha indicado que el bucle no continue */
            if(!creAnyLoop || creFirstProcess == NULL) break;

            /* Limpiamos el lienzo antes de dibujar */
            if(Canvas != NULL) {
                SDL_FreeSurface(Canvas);
                Canvas = NULL;
            }

            /*
             * Hacemos el blit del gráfico del proceso a pantalla si el estado
             * del proceso así lo indica y hay un gráfico disponible.
             */

            if(CurrentProcess->Graph != NULL &&
               !((CurrentProcess->State & CRE_PS_GHOST) >> 5) &&
               !((CurrentProcess->State & CRE_PS_FREEZE) >> 4)) {
                /*
                 * Dependiendo de las caracterias del proceso: tamaño y ángulo
                 * Aplicamos las transformaciones necesarias o no
                 */

                if(CurrentProcess->Angle != 0 || CurrentProcess->SizeW != 100
                    || CurrentProcess->SizeH != 100) {
                    Canvas = CRE_GfxRZSurfaceXY(CurrentProcess->Graph,
                    CurrentProcess->Angle / 1000.0f,
                    CurrentProcess->SizeW / 100.0f,
                    CurrentProcess->SizeH / 100.0f,
                    (CurrentProcess->State & CRE_PS_HIGHGFX) >> 1);
                }

                /* Si no se a aplicado ningún cambio, dibujamos tal cual */
                if(Canvas == NULL) {
                    /* Preparamos las coordenadas para el blit */
                    GfxRect.x = CurrentProcess->X -
                        (CurrentProcess->Graph->w / 2);
                    GfxRect.y = CurrentProcess->Y -
                        (CurrentProcess->Graph->h / 2);
                    /* Dibujamos el gráfico directamente */
                    CRE_GfxAlphaBlit(CurrentProcess->Graph, creScreen, &GfxRect,
                        CurrentProcess->Alpha);
                /* Si hemos aplicado algun cambio, dibujamos el Canvas Temp. */
                } else {
                    /* Preparamos las coordenadas para el blit */
                    GfxRect.x = CurrentProcess->X - (Canvas->w / 2);
                    GfxRect.y = CurrentProcess->Y - (Canvas->h / 2);
                    /* Dibujamos el Canvas */
                    CRE_GfxAlphaBlit(Canvas, creScreen, &GfxRect,
                        CurrentProcess->Alpha);
                }

            }

            /* Continuamos al elemento siguiente */
            LastProcess = CurrentProcess;
            CurrentProcess = CurrentProcess->Next;

        }

        /* Actualizamos la pantalla */
        SDL_Flip(creScreen);

        /*
         * Una vez ejecutado todos los procesos, y realizado todas acciones
         * relacionadas con éstos debemos comprobar si nos ha sobrado tiempo,
         * y en tal caso, esperar si así esta indicado en SPF (SPF > 0)
         */
        if(creSPF != 0) {
            /* Calculamos el tiempo de espera */
            WaitTime = creSPF - (SDL_GetTicks() - CurrentTime);
            /* Si nos sobra tiempo, esperamos */
            if(WaitTime > 0)
                SDL_Delay(WaitTime);
        }

        /* Obtenemos la velocidad de proceso real para informar */
        creRealFPS = 1.0f/((SDL_GetTicks() - CurrentTime) / 1000.0f);
    }

    return 0;
}


/*
 * CRE_StartLoop
 * Inicializa el bucle de procesos principal.
 */
Sint32 CRE_StartLoop(void)
{
    /* Si ya hay un bucle ejecutandose */
    if(creAnyLoop != 0 || creScreen == NULL || creFirstProcess == NULL)
        return -1;
    /* Si todo es correcto ejecutamos el bucle */
    creAnyLoop = 1;
    CRE_MainLoop();
    return 0;
}


/*
 * CRE_EndLoop
 * Finaliza el bucle de control de procesos actual después de la última
 * ejecucíón de un proceso y elimina todas las instancias.
 */
Sint32 CRE_EndLoop(void)
{
    creProcess * Current = creFirstProcess;
    creProcess * Tmp;

    /* Comprobamos que exite algún bucle en ejecución */
    if(!creAnyLoop)
        return -1;


    /* Eliminamos todas las instancias */
    while(Current != NULL) {
        /* Guardamos la dircción del siguiente */
        Tmp = Current->Next;
        /* Eliminamos la instancia actual */
        if(Current->Free != NULL)
            Current->Free(Current);
        /* Continuamos la lista */
        Current = Tmp;
    }

    /* Reinicializamos la lista */
    creAnyLoop = 0;
    creFirstProcess = NULL;
    creEList.Size = 0;

    return 0;
}


/*
 * CRE_SetFPS
 * Establece la velocidad del bucle de procesos, y por tanto de juego.
 */
void CRE_SetFPS(Uint32 FPS)
{
    if(FPS == 0)
        creSPF = 0;
    else
        creSPF = 1000/FPS;
}


/*
 * CRE_GetFPS
 * Devuelve la velocidad real del juego en FPS;
 */
float CRE_GetFPS(void)
{
    return creRealFPS;
}


/*
 * CRE_GetProcess
 * Dada un Id de proceso válido y si el proceso está activo devuelve un puntero
 * a su estructura.
 */
creProcess * CRE_GetProcess(Uint32 Id)
{
    creProcess * This;

    /* En caso de que el Id no sea válido */
    if(!Id ) return NULL;

    /* Inicializamos las variables para la búsqueda */
    This = creFirstProcess;

    /* Recorremos la lista de procesos */
    while(This != NULL) {
        /* Si el proceso es el buscado paramos de buscar */
        if(This->Id == Id) break;
        /* Avazamos al siguiente elemento de la lista */
        This = This->Next;
    }

    /* Devolvemos el objeto buscado, o un puntero a NULL implícito */
    return This;
}


/*
 * CRE_SetState
 * Función sencilla que pone el estado indicado al proceso con una id dada
 */
Sint32 CRE_SetState(Uint32 Id, Uint8 State)
{
    creProcess * This = CRE_GetProcess(Id);

    if(This == NULL)
        return -1;
    else {
        This->State |= State;
        return 0;
    }
}


/*
 * CRE_TSetState
 * Establece un estado determinado a todos los procesos del mismo tipo.
 */
Sint32 CRE_TSetState(Uint16 Type, Uint8 State)
{
    Sint32 Tmp = -1;
    creProcess * This = creFirstProcess;

    /* Caso optimizado */
    if(Type == 0 && State == CRE_PS_DEAD) {
        CRE_EndLoop();
        return 0;
    }

    /* Recorremos todos los procesos */
    while(This != NULL) {
        /* Si el tipo es el adecuado modificamos su estado */
        if(Type == 0 || This->Type == Type) {
            This->State |= State;
            Tmp = 0;
        }
        /* Continuamos al siguiente */
        This = This->Next;
    }

    return Tmp;
}


/*
 * CRE_LetPrcsAlone
 * Elimina todo los procesos menos el indicado
 */
Sint32 CRE_LetPrcsAlone(Uint32 Id)
{
    creProcess * This = creFirstProcess;

    /* Caso optimizado */
    if(Id == 0) {
        CRE_EndLoop();
        return 0;
    }

    /* Damos el estado de "muerto" a todos los procesos menos el indicado */
    while(This != NULL) {
        /* Si no es el proceso indicado le damos el estado */
        if(This->Id != Id)
            This->State = CRE_PS_DEAD;
        /* Avanzamos en la lista */
        This = This->Next;
    }

    return 0;
}


/*
 * CRE_AddProcess
 * Añade una instancia de un proceso a la lista de procesos en ejecución.
 * Los procesos se ordenan en función del valor de Z.
 */
Sint32 CRE_AddProcess(creProcess * Process)
{
    creProcess * This, * Last;

    /* Comprobamos que el proceso no es una clase */
    if((Process->State & CRE_PS_CLASS) >> 2)
        return -1;
    /* Comprobamos que es un proceso valido */
    if(Process == NULL)
        return -1;

    /* Damos un ID válido al proces */
    Process->Id = CRE_GetNewPId();

    /* En caso de que este sea el primer porceso de la lista */
    if(creFirstProcess == NULL) {
        Process->Next = NULL;
        creFirstProcess = Process;
        return 0;
    }

    /* Preparamos variables para la búsqueda */
    Last = NULL;
    This = creFirstProcess;

    /* Recorremos la lista de procesos en busca de la posición correcta */
    while(This != NULL) {
        /* Observamos el valor de z, para introducir el proceso */
        if(This->Z >= Process->Z)
            break;
        /* Incrementamos la posición de la lista */
        Last = This;
        This = This->Next;
    }

    /* En caso de que el proceso deba estar el primero de la lista */
    if(Last == NULL) {
        Process->Next = This;
        creFirstProcess = Process;
    /* Si esta en cualquier otra posición, actuamos de diferente forma */
    } else {
        Last->Next = Process;
        Process->Next = This;
    }

    return 0;
}


/*
 * CRE_SetClearColor
 * Establece el color con el que se limpia la pantalla en cada frame.
 */
void CRE_SetClearColor(Uint8 R, Uint8 G, Uint8 B)
{
    creClearColor = SDL_MapRGB(creScreen->format, R, G, B);
}


/*
 * CRE_CountProcesses
 * Devuelve el número de procesos activos
 */
Uint32 CRE_CountProcesses(void)
{
    Uint32 Count = 0;
    creProcess * This = creFirstProcess;

    while(This != 0) {
        Count++;
        This = This->Next;
    }

    return Count;
}


/*
 * CRE_GetProcessesInfo
 * Devuelve en el fichero indicado información sobre el estado actual de los
 * procesos en lista de ejecución.
 */
void CRE_GetProcessesInfo(FILE * out)
{
    creProcess * This;

    /* Escribimos una cabecera */
    fprintf(out, ".----------------------------------.\n"
                 "| CORE DEBUG INFO                  |\n"
                 "| > Active processes : %8d    |\n"
                 "| > Succes Time:       %8g    |\n"
                 "| > FPS :              %8g    |\n"
                 "·----------------------------------·\n\n",
        CRE_CountProcesses(), (SDL_GetTicks()/1000.0), CRE_GetFPS());

    /* Inicializamos valores */
    This = creFirstProcess;

    /* En el caso de que no haya procesos en lista */
    if(creFirstProcess == NULL) {
        fprintf(out, "+ NO PROCESSES RUNNING NOW +\n\n\n");
        return;
    }

    /* Recorremos la lista de procesos y mostramos su info */
    while(This != NULL) {

        /* Imprimimos la información del proceso */
        fprintf(out, "+ PROCESS '%s'\n"
                     "|-> Id: %d\n"
                     "|-> Type: %d\n"
                     "|-> State: [Pause = %d] [Ghost = %d] [Freeze = %d] "
                        "[Dead = %d]\n"
                     "|-> X: %d\n"
                     "|-> Y: %d\n"
                     "|-> Z: %d\n"
                     "|-> SizeW: %g\n"
                     "|-> SizeH: %g\n"
                     "|-> Alpha: %d\n"
                     "|-> Angle: %g\n\n",
                This->Info, This->Id, This->Type,
                (This->State & CRE_PS_PAUSE) >> 6,
                (This->State & CRE_PS_GHOST) >> 5,
                (This->State & CRE_PS_FREEZE) >> 4,
                (This->State & CRE_PS_DEAD) >> 3,
                This->X, This->Y, This->Z,
                This->SizeW / 100.0, This->SizeH / 100.0, This->Alpha,
                This->Angle / 1000.0);

        /* Avanzamos en la lista */
        This = This->Next;
    }
}
