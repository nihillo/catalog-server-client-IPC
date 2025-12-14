#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LNG_MSG 50

#define OP_APAGAR "EXIT"

#define ARCHIVO_CLAVE "protocolo.h"
#define CLAVE_PETICIONES ftok(ARCHIVO_CLAVE, 'P');
#define CLAVE_RESPUESTAS ftok(ARCHIVO_CLAVE, 'R');


typedef struct {
    long tipo;
    char cuerpo[LNG_MSG];
} Mensaje;