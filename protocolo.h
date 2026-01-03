#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

#define DEBUG false // activa modo debug

#define LNG_CODIGO_OPERACION 1
#define LNG_CODIGO_PRODUCTO 5 // longitud efectiva 4, necesario contar cierre cadena
#define LNG_NOMBRE 44 // long efectiva 43
#define LNG_CANTIDAD 4 // long efectiva 3
#define N_SEPARADORES 3   
#define LNG_PET LNG_CODIGO_PRODUCTO + LNG_NOMBRE + LNG_CANTIDAD + N_SEPARADORES

#define OP_LISTAR 1
#define OP_BUSCAR 2
#define OP_INSERTAR 3
#define OP_APAGAR 4
#define OP_SALIR 5

#define ARCHIVO_CLAVE "protocolo.h"
#define CLAVE_PETICIONES ftok(ARCHIVO_CLAVE, 'P');
#define CLAVE_RESPUESTAS ftok(ARCHIVO_CLAVE, 'R');

typedef struct {
    char codigo[LNG_CODIGO_PRODUCTO];
    char nombre[LNG_NOMBRE];
    int cantidad;
} Producto;

typedef struct {
    long tipo;
    char cuerpo[LNG_PET];
} Mensaje;