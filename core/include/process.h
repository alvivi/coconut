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
  * @file process.h
  * Archivo de definición de funciones relacionadas con el manejo y
  * mantenimiento del sistema de procesos. Pertenece al core del juego.
  **/


#ifndef CORE_PROCESS_H
#define CORE_PROCESS_H

#include <SDL/SDL.h>


/*
 * Definición de macros
 */

/* Modo debug de procesos, desactivado por defecto */
/**
 * Define este macro en tu proyecto con el valor 1 para que cuando se
 * pulse la tecla F12 se imprima la información de debug de procesos en
 * el stdout de tu proyecto.
 **/
#define CRE_PROCESS_DEBUG_MODE

/* Fichero de salida debug */
#ifndef CRE_PROCESS_DEBUG_FILE
    #define CRE_PROCESS_DEBUG_FILE stdout
#endif

/* Definición de los estados de un proceso */
/** El proceso se recuperá de cualquier estado temporal anterior */
#define CRE_PS_WAKEUP  0x80
/** El proceso se mostrará pero no se ejecutará temporalmente */
#define CRE_PS_PAUSE   0x40
/** El proceso no se mostrará en pantalla temporalmente */
#define CRE_PS_GHOST   0x20
/** El proceso no se ejecutará ni se mostrará temporalmente */
#define CRE_PS_FREEZE  0x10
/** El proceso esta muerto, no se ejecuta y se destruirá su instancia */
#define CRE_PS_DEAD    0x08
/** El proceso se considera una clase de la cual parten las instancias */
#define CRE_PS_CLASS   0x04
/** Indica que el proceso debe de ser dibujado con la máxima calidad */
#define CRE_PS_HIGHGFX 0x02
/** Indica que el estado del proceso ha cambiando desde el ciclo anterior */
#define CRE_PS_CHANGED 0x01
/** Indica el estado normal o de inicio */
#define CRE_PS_DEFAULT 0x00

/* Definiciones generales */
/**
 * Define el número máximo de procesos simultáneos recogidos en cada frame.
 * Los eventos que no quepan serán procesados en el frame siguiente, siguiendo
 * una lista FIFO. El máximo es 256.
 **/
#define CRE_MAX_SIM_EVENTS 16

/*
 * Definición de tipos
 */

/**
 * Estructura que define un proceso.
 **/
typedef struct creProcess {
    /* Información de gestión interna */
    /** Identificador único del proceso (Asignado automaticamente) */
    Uint32 Id;
    /** Tipo de proceso (Valor opcional) */
    Uint16 Type;
    /** Flags que indican el estado en el que se encuentra un proceso */
    Uint8 State;
    /** Cadena opcional que identifica al proceso */
    char * Info;
    /** Puntero al siguiente proceso (Lista enlaz.) */
    struct creProcess * Next;
    /** Método de inicio de una instancia */
    struct creProcess * (* New)(/* ... */);
    /** Método bucle */
    void (* Loop)(struct creProcess * This);
    /** Método destructor */
    void (* Free)(struct creProcess * This);
    /* Información gráfica */
    /** Puntero al gráfico del proceso */
    SDL_Surface * Graph;
    /** Coordenadas en pantalla de la esquina s-i */
    Sint32 X, Y;
    /** Coordenada de la capa en la que se situa el proceso */
    Uint8 Z;
    /** Canal Alpha, indicada la cantidad de transparencia */
    Uint8 Alpha;
    /** Ángulo con el que se dibuja el gráfico (Miligrados) */
    Sint32 Angle;
    /** Tamaño del gráfico, si no coincide con Graph es redimensionado (%)*/
    Sint16 SizeW, SizeH;
    /* Información de ca instancia */
    /** Puntero de uso general para la información propia de cada proceso */
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
 * Declaración de funciones
 */

/**
 * Esta función inicializa, siempre que sea posible, el bucle que gestiona y
 * ejecuta todos los procesos que ya esten añadidos a las lista de ejecución.
 * No debe de haber activo otro bucle de procesos.
 * @brief Inicializa el bucle de procesos.
 * @return 0 si todo ha sido correcto, -1 en caso contrario.
 **/
extern Sint32 CRE_StartLoop(void);

/**
 * Finaliza la ejecución del bucle de control de procesos actual si es posible,
 * y siempre después de la última ejecución de un proceso.
 * @brief Finaliza el bucle de procesos actual
 * @return 0 si se ha podido finalizar, -1 en si no ha sido posible.
 **/
extern Sint32 CRE_EndLoop(void);

/**
 * @brief Establece los FPS del bucle principal
 * @param FPS Número de frames por segundo
 * Establece la velocidad del bucle principal en FPS (Frames per second). El
 * valor máximo notable depende del hardware de cada equipo, aunque teóricamente
 * nunca se podrá superar los 1000 fps. Si se le pasa como parámetro 0, el bucle
 * irá lo más rápido posible que le permita el hardware.
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
 * @brief Muestra información del estado actual de los procesos en lista.
 * @param out Fichero donde se van a escribir los resultados.
 * Función que imprime en el archivo indicado (normalmente stdout) información
 * del estado actual de la lista de procesos, asi como información detalla de
 * todos y cada uno de los procesos.
 * @return 0 si todo ha sido correcto, -1 en caso contrario.
 **/
extern void CRE_GetProcessesInfo(FILE * out);

/**
 * @brief Añade una instancia de proceso ya creada a la lista de ejecución
 * @param Process Puntero a la instancia del proceso a añadir
 * Esta función añade un proceso ya inicializado, es decir, que se considera
 * una instancia y no una clase, a la lista de ejecución de procesos.
 * Normalmente esta función es llamanda dentro de los metodos New de los
 * procesos, así, una vez creada la instancia por este método lo añade a la
 * lista.
 * @return 0 si ha podido se añadido a la lista, -1 si no ha sido posible.
 **/
extern Sint32 CRE_AddProcess(creProcess * Process);

/**
 * @brief Obtiene la estructura del proceso indicado
 * @param Id Identificador del proceso
 * Función que es utilizada para, una vez obtenido un id válido de algún proceso
 * en activo, acceder a su estructura de proceso.
 * @return Puntero a la estructura de proceso indicada por el id, o NULL si no
 * existe tal proceso.
 **/
extern creProcess * CRE_GetProcess(Uint32 Id);

/**
 * @brief Añade un estado a un determinado proceso
 * @param Id Identificador del proceso
 * @param State Estado para añadir
 * Función que dada la id de un proceso asocia un estado determinado a éste.
 * Esta función es equivalente ha utilizar CRE_GetProcess y editar manualmente
 * la propiedad State.
 * @return 0 si todo ha funcionado bien, -1 en caso contrario
 **/
extern Sint32 CRE_SetState(Uint32 Id, Uint8 State);

/**
 * @brief Añade un estado a todos los procesos de un determinado tipo
 * @param Type Tipo de los procesos que deben modificar su estado
 * @param State Estado a añadir a los procesos
 * Esta función busca todos los procesos activos que sean de un determinado
 * tipo y modifica su estado al indicado en el parámetro. Si se usa el tipo 0
 * todos los procesos se le aplican el estado indicado.
 * @return 0 si ha encontrado algún proceso del tipo indicado, -1 si no.
 **/
extern Sint32 CRE_TSetState(Uint16 Type, Uint8 State);

/**
 * @brief Elimina todo los procesos menos el indicado
 * @param Id Identificador del proceso que hay que dejar solo
 * Función que dado un id válido de un proceso, elimina todos los demás procesos
 * dejando a este como único de la lista. Si el Id no es válido, no se eliminará
 * mningún proceso.
 * @return 0 si el id es válido y ahora esta solo, -1 en caso contrario
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
