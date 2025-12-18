#include "protocolo.h"

int main(int argc, char *argv[]) {
    // Temp: silenciar warnings de parámetros no usados
    (void)argc;
    (void)argv;

    printf("Inicializando servidor...\n");

    key_t clave_peticiones = CLAVE_PETICIONES;
    key_t clave_respuestas = CLAVE_RESPUESTAS;

    int cola_peticiones = msgget(clave_peticiones, IPC_CREAT | 0666);
    int cola_respuestas = msgget(clave_respuestas, IPC_CREAT | 0666);
    
    if (cola_peticiones == -1 || cola_respuestas == -1) {
        perror("Error al crear las colas de mensajes (msgget)");
        exit(1);
    }

    printf("Colas de mensajes inicializadas. Peticiones ID: %d, Respuestas ID: %d\n", cola_peticiones, cola_respuestas);
    Mensaje mensaje;

    int servidor_activo = 1;
    int tam = sizeof(mensaje) - sizeof(mensaje.tipo);

    while (servidor_activo) {

        int p = msgrcv(cola_peticiones, &mensaje, tam, 0, 0);

        if (p == -1) {
            perror("msgrcv");
            servidor_activo = 0;
        }

        // if (mensaje.cuerpo == OP_APAGAR) {
        //     printf("Recibida petición de APAGADO\n");
        //     servidor_activo = 0;
        // }

        if (servidor_activo) {
            char msg_peticion[LNG_MSG];
            strcpy(msg_peticion, mensaje.cuerpo);
            sprintf(mensaje.cuerpo, "Mensaje recibido: %s", msg_peticion);

            int r = msgsnd(cola_respuestas, &mensaje, tam, 0);
        }
    }    

    printf("Apagando servidor...\n");
    printf("Eliminando colas de mensajes...\n");
    if (msgctl(cola_peticiones, IPC_RMID, NULL) == -1) {
        perror("Error al terminar la cola de peticiones (msgctl)");
    }
    
    if (msgctl(cola_respuestas, IPC_RMID, NULL) == -1) {
        perror("Error al terminar la cola de respuestas (msgctl)");
    }

    printf("Servidor apagado.\n");

    return 0;
}