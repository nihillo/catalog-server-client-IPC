#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LNG_CODIGO_OPERACION 1
#define LNG_CODIGO_PRODUCTO 5 // longitud efectiva 4, necesario contar cierre cadena
#define LNG_NOMBRE 44 // long efectiva 43
#define LNG_CANTIDAD 4 // long efectiva 3
#define N_SEPARADORES 3   
#define LNG_MSG LNG_CODIGO_PRODUCTO + LNG_NOMBRE + LNG_CANTIDAD + N_SEPARADORES

#define ARG_LISTAR "listar"
#define ARG_BUSCAR "buscar"
#define ARG_INSERTAR "insertar"
#define ARG_APAGAR "apagar"

#define OP_LISTAR 'L'
#define OP_BUSCAR 'B'
#define OP_INSERTAR 'I'
#define OP_APAGAR 'A'

#define ARCHIVO_CLAVE "protocolo.h"
#define CLAVE_PETICIONES ftok(ARCHIVO_CLAVE, 'P');
#define CLAVE_RESPUESTAS ftok(ARCHIVO_CLAVE, 'R');

typedef struct {
    char codigo[LNG_CODIGO_PRODUCTO];
    char nombre[LNG_MSG];
    int cantidad;
} Producto;

typedef struct {
    long tipo;
    char cuerpo[LNG_MSG];
} Mensaje;