#include "protocolo.h"

int main(int argc, char *argv[]) {

    printf("Inicializando cliente...\n");

    pid_t pid = getpid();


    Mensaje mensaje;
    mensaje.tipo = (long)pid;
    strcpy(mensaje.cuerpo, argv[1]);

    int tam = sizeof(mensaje) - sizeof(mensaje.tipo);


    int r = msgsnd(C_PETICIONES, &mensaje, tam, 0);

    return 0;
} 