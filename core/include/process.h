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
  * @file process.h
  * Archivo de definici�n de funciones relacionadas con el manejo y
  * mantenimiento del sistema de procesos. Pertenece al core del juego.
  **/


#ifndef CORE_PROCESS_H
#define CORE_PROCESS_H

#include <SDL/SDL.h>


/*
 * Definici�n de macros
 */

/* Modo debug de procesos, desactivado por defecto */
/**
 * Define este macro en tu proyecto con el valor 1 para que cuando se
 * pulse la tecla F12 se imprima la informaci�n de debug de procesos en
 * el stdout de tu proyecto.
 **/
#define CRE_PROCESS_DEBUG_MODE

/* Fichero de salida debug */
#ifndef CRE_PROCESS_DEBUG_FILE
    #define CRE_PROCESS_DEBUG_FILE stdout
#endif

/* Definici�n de los estados de un proceso */
/** El proceso se recuper� de cualquier estado temporal anterior */
#define CRE_PS_WAKEUP  0x80
/** El proceso se mostrar� pero no se ejecutar� temporalmente */
#define CRE_PS_PAUSE   0x40
/** El proceso no se mostrar� en pantalla temporalmente */
#define CRE_PS_GHOST   0x20
/** El proceso no se ejecutar� ni se mostrar� temporalmente */
#define CRE_PS_FREEZE  0x10
/** El proceso esta muerto, no se ejecuta y se destruir� su instancia */
#define CRE_PS_DEAD    0x08
/** El proceso se considera una clase de la cual parten las instancias */
#define CRE_PS_CLASS   0x04
/** Indica que el proceso debe de ser dibujado con la m�xima calidad */
#define CRE_PS_HIGHGFX 0x02
/** Indica que el estado del proceso ha cambiando desde el ciclo anterior */
#define CRE_PS_CHANGED 0x01
/** Indica el estado normal o de inicio */
#define CRE_PS_DEFAULT 0x00

/* Definiciones generales */
/**
 * Define el n�mero m�ximo de procesos simult�neos recogidos en cada frame.
 * Los eventos que no quepan ser�n procesados en el frame siguiente, siguiendo
 * una lista FIFO. El m�ximo es 256.
 **/
#define CRE_MAX_SIM_EVENTS 16

/*
 * Definici�n de tipos
 */

/**
 * Estructura que define un proceso.
 **/
typedef struct creProcess {
    /* Informaci�n de gesti�n interna */
    /** Identificador �nico del proceso (Asignado automaticamente) */
    Uint32 Id;
    /** Tipo de proceso (Valor opcional) */
    Uint16 Type;
    /** Flags que indican el estado en el que se encuentra un proceso */
    Uint8 State;
    /** Cadena opcional que identifica al proceso */
    char * Info;
    /** Puntero al siguiente proceso (Lista enlaz.) */
    struct creProcess * Next;
    /** M�todo de inicio de una instancia */
    struct creProcess * (* New)(/* ... */);
    /** M�todo bucle */
    void (* Loop)(struct creProcess * This);
    /** M�todo destructor */
    void (* Free)(struct creProcess * This);
    /* Informaci�n gr�fica */
    /** Puntero al gr�fico del proceso */
    SDL_Surface * Graph;
    /** Coordenadas en pantalla de la esquina s-i */
    Sint32 X, Y;
    /** Coordenada de la capa en la que se situa el proceso */
    Uint8 Z;
    /** Canal Alpha, indicada la cantidad de transparencia */
    Uint8 Alpha;
    /** �ngulo con el que se dibuja el gr�fico (Miligrados) */
    Sint32 Angle;
    /** Tama�o del gr�fico, si no coincide con Graph es redimensionado (%)*/
    Sint16 SizeW, SizeH;
    /* Informaci�n de ca instancia */
    /** Puntero de uso general para la informaci�n propia de cada proceso */
    void * Data;
} creProcess;

/**
 * Estructura que define una lista de eventos
 **/
typedef struct creEventsList {
    /** Inica la cantidad de procesos almacenados (0 < size < CRE_MAX...) */
    Uint8 Size;
    /** Itinerador auxiliar que ayuda a los procesos a buscar los eventos */
    Uint8 CurrentEvent;
    /** Array con los eventos */
    SDL_Event Events[CRE_MAX_SIM_EVENTS];
} creEventsList;


/*
 * Variables globales
 */

/** Puntero a pantalla usado por los procesos */
extern SDL_Surface * creScreen;
/** Estrucutra que contiene en cada momento los eventos procesado en un frame */
extern creEventsList creEList;


/*
 * Declaraci�n de funciones
 */

/**
 * Esta funci�n inicializa, siempre que sea posible, el bucle que gestiona y
 * ejecuta todos los procesos que ya esten a�adidos a las lista de ejecuci�n.
 * No debe de haber activo otro bucle de procesos.
 * @brief Inicializa el bucle de procesos.
 * @return 0 si todo ha sido correcto, -1 en caso contrario.
 **/
extern Sint32 CRE_StartLoop(void);

/**
 * Finaliza la ejecuci�n del bucle de control de procesos actual si es posible,
 * y siempre despu�s de la �ltima ejecuci�n de un proceso.
 * @brief Finaliza el bucle de procesos actual
 * @return 0 si se ha podido finalizar, -1 en si no ha sido posible.
 **/
extern Sint32 CRE_EndLoop(void);

/**
 * @brief Establece los FPS del bucle principal
 * @param FPS N�mero de frames por segundo
 * Establece la velocidad del bucle principal en FPS (Frames per second). El
 * valor m�ximo notable depende del hardware de cada equipo, aunque te�ricamente
 * nunca se podr� superar los 1000 fps. Si se le pasa como par�metro 0, el bucle
 * ir� lo m�s r�pido posible que le permita el hardware.
 * @see CRE_GetFPS
 **/
extern void CRE_SetFPS(Uint32 FPS);

/**
 * @brief Devuelve la velocidad en FPS del bucle de procesos
 * @return FPS del bucle
 * Calcula y devuelve la velocidad real del bucle de procesos, suele diferir
 * un poco de la establecida teoricamente con SetFps.
 * @see CRE_SetFPS
 **/
extern float CRE_GetFPS(void);

/**
 * @brief Muestra informaci�n del estado actual de los procesos en lista.
 * @param out Fichero donde se van a escribir los resultados.
 * Funci�n que imprime en el archivo indicado (normalmente stdout) informaci�n
 * del estado actual de la lista de procesos, asi como informaci�n detalla de
 * todos y cada uno de los procesos.
 * @return 0 si todo ha sido correcto, -1 en caso contrario.
 **/
extern void CRE_GetProcessesInfo(FILE * out);

/**
 * @brief A�ade una instancia de proceso ya creada a la lista de ejecuci�n
 * @param Process Puntero a la instancia del proceso a a�adir
 * Esta funci�n a�ade un proceso ya inicializado, es decir, que se considera
 * una instancia y no una clase, a la lista de ejecuci�n de procesos.
 * Normalmente esta funci�n es llamanda dentro de los metodos New de los
 * procesos, as�, una vez creada la instancia por este m�todo lo a�ade a la
 * lista.
 * @return 0 si ha podido se a�adido a la lista, -1 si no ha sido posible.
 **/
extern Sint32 CRE_AddProcess(creProcess * Process);

/**
 * @brief Obtiene la estructura del proceso indicado
 * @param Id Identificador del proceso
 * Funci�n que es utilizada para, una vez obtenido un id v�lido de alg�n proceso
 * en activo, acceder a su estructura de proceso.
 * @return Puntero a la estructura de proceso indicada por el id, o NULL si no
 * existe tal proceso.
 **/
extern creProcess * CRE_GetProcess(Uint32 Id);

/**
 * @brief A�ade un estado a un determinado proceso
 * @param Id Identificador del proceso
 * @param State Estado para a�adir
 * Funci�n que dada la id de un proceso asocia un estado determinado a �ste.
 * Esta funci�n es equivalente ha utilizar CRE_GetProcess y editar manualmente
 * la propiedad State.
 * @return 0 si todo ha funcionado bien, -1 en caso contrario
 **/
extern Sint32 CRE_SetState(Uint32 Id, Uint8 State);

/**
 * @brief A�ade un estado a todos los procesos de un determinado tipo
 * @param Type Tipo de los procesos que deben modificar su estado
 * @param State Estado a a�adir a los procesos
 * Esta funci�n busca todos los procesos activos que sean de un determinado
 * tipo y modifica su estado al indicado en el par�metro. Si se usa el tipo 0
 * todos los procesos se le aplican el estado indicado.
 * @return 0 si ha encontrado alg�n proceso del tipo indicado, -1 si no.
 **/
extern Sint32 CRE_TSetState(Uint16 Type, Uint8 State);

/**
 * @brief Elimina todo los procesos menos el indicado
 * @param Id Identificador del proceso que hay que dejar solo
 * Funci�n que dado un id v�lido de un proceso, elimina todos los dem�s procesos
 * dejando a este como �nico de la lista. Si el Id no es v�lido, no se eliminar�
 * mning�n proceso.
 * @return 0 si el id es v�lido y ahora esta solo, -1 en caso contrario
 **/
extern Sint32 CRE_LetPrcsAlone(Uint32 Id);

/**
 * @brief Estable el color de inicio del frame (ClearColor)
 * @param R Componente roja del color
 * @param G Componente verde del color
 * @param B Componente azul del color
 * Estable el color con el que se limpiar la pantalla al inicio del frame.
 **/
extern void CRE_SetClearColor(Uint8 R, Uint8 G, Uint8 B);

#endif
