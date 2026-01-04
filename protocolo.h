#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

/* --- CONFIGURACIÓN Y LÍMITES --- */
#define DEBUG false // activa modo debug

// Definición de longitudes para evitar "números mágicos" y facilitar el mantenimiento
#define LNG_CODIGO_OPERACION 1
#define LNG_CODIGO_PRODUCTO 5   // 4 caracteres + '\0'
#define LNG_NOMBRE 51           // 50 caracteres + '\0'
#define LNG_CANTIDAD 4          // 3 dígitos (0-999) + '\0'
#define N_SEPARADORES 3         // Nº separadores (caracteres '|') en el cuerpo del mensaje
#define LNG_PET LNG_CODIGO_PRODUCTO + LNG_NOMBRE + LNG_CANTIDAD + N_SEPARADORES

/* --- CÓDIGOS DE OPERACIÓN --- */
#define OP_LISTAR 1
#define OP_BUSCAR 2
#define OP_INSERTAR 3
#define OP_APAGAR 4
#define OP_SALIR 5

/* --- COMUNICACIÓN IPC --- */
#define ARCHIVO_CLAVE "protocolo.h"
// Generación de claves únicas para las dos colas requeridas
// Uso de ftok para generar claves únicas basadas en un archivo real
#define CLAVE_PETICIONES ftok(ARCHIVO_CLAVE, 'P');
#define CLAVE_RESPUESTAS ftok(ARCHIVO_CLAVE, 'R');

/* --- ESTRUCTURAS DE DATOS --- */
// Estructura interna para el manejo de registros en el archivo productos.dat
typedef struct {
    char codigo[LNG_CODIGO_PRODUCTO];
    char nombre[LNG_NOMBRE];
    int cantidad;
} Producto;

// Estructura del mensaje para System V IPC
typedef struct {
    long tipo;              // Almacenará el PID del cliente
    char cuerpo[LNG_PET];   // Cuerpo del mensaje. En peticiones: Datos serializados ("OP|COD|NOM|CANT") Respuestas: texto a imprimir
} Mensaje;