#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define LNG_MSG 50

int C_PETICIONES;
int C_RESPUESTAS;

typedef struct {
    long tipo;
    char cuerpo[LNG_MSG];
} Mensaje;

int obtener_cola(int cola) {
    if (cola == C_PETICIONES && C_PETICIONES != -1) {
        return C_PETICIONES;
    } else if (cola == C_RESPUESTAS && C_RESPUESTAS != -1) {
        return C_RESPUESTAS;
    } else {
        return -1;
    }
}