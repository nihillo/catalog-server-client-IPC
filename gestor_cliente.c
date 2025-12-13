#include "protocolo.h"

int main(int argc, char *argv[]) {

    pid_t pid = getpid();

    Mensaje mensaje;
    mensaje.tipo = (long)pid;
    strcpy(mensaje.cuerpo, argv[1]);

    int tam = sizeof(mensaje) - sizeof(mensaje.tipo);

    obtener_colas();

    int p = msgsnd(C_PETICIONES, &mensaje, tam, 0);

    int r = msgrcv(C_RESPUESTAS, &mensaje, tam, pid, 0);

    printf("%s\n", mensaje.cuerpo);

    return 0;
} 