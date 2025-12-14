#include "protocolo.h"

int main(int argc, char *argv[]) {
    pid_t pid = getpid();
    int cola_peticiones, cola_respuestas;

    Mensaje mensaje;
    mensaje.tipo = (long)pid;
    strcpy(mensaje.cuerpo, argv[1]);

    int tam = sizeof(mensaje) - sizeof(mensaje.tipo);

    key_t clave_peticiones = CLAVE_PETICIONES;
    key_t clave_respuestas = CLAVE_RESPUESTAS;

    cola_peticiones = msgget(clave_peticiones, 0);
    cola_respuestas = msgget(clave_respuestas, 0);  

    if (cola_peticiones == -1 || cola_respuestas == -1) {
        printf("El servidor está apagado\n");
        return 0;
    }

    

    int p = msgsnd(cola_peticiones, &mensaje, tam, 0);

    int r = msgrcv(cola_respuestas, &mensaje, tam, pid, 0);

    printf("%s\n", mensaje.cuerpo);

    return 0;
} 