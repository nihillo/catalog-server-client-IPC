#include "protocolo.h"

int main(int argc, char *argv[]) {
    // Temp: silenciar warnings de parámetros no usados
    (void)argc;
    (void)argv;

    printf("Inicializando servidor...\n");

    obtener_colas();

    int servidor_activo = 1;

    while (servidor_activo) {

        Mensaje mensaje;

        int tam = sizeof(mensaje) - sizeof(mensaje.tipo);

        int p = msgrcv(C_PETICIONES, &mensaje, tam, 0, 0);

        if (p == -1) {
            perror("msgrcv");
            exit(1);
        }

        char msg_peticion[LNG_MSG];
        strcpy(msg_peticion, mensaje.cuerpo);
        sprintf(mensaje.cuerpo, "Mensaje recibido: %s", msg_peticion);

        int r = msgsnd(C_RESPUESTAS, &mensaje, tam, 0);

        if (strcmp(mensaje.cuerpo, "EXIT") == 0) {
            servidor_activo = 0;
        }
    }    

    return 0;
}