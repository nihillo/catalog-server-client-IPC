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

void obtener_colas() {
    char f_clave[] = "protocolo.h";
    char p_peticiones = 'P';
    char p_respuestas = 'R';

    int clave_peticiones = ftok(f_clave, p_peticiones);
    C_PETICIONES = msgget(clave_peticiones, IPC_CREAT | 0644);

    int clave_respuestas = ftok(f_clave, p_respuestas);
    C_RESPUESTAS = msgget(clave_respuestas, IPC_CREAT | 0644);
}

// int obtener_cola(int cola) {
//     if (cola == C_PETICIONES && C_PETICIONES != -1) {
//         return C_PETICIONES;
//     } else if (cola == C_RESPUESTAS && C_RESPUESTAS != -1) {
//         return C_RESPUESTAS;
//     } else {
//         return -1;
//     }
// }