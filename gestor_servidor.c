#include "protocolo.h"

void crear_colas() {
    char f_clave[] = "protocolo.h";
    char p_peticiones = 'P';
    char p_respuestas = 'R';

    int clave_peticiones = ftok(f_clave, p_peticiones);
    C_PETICIONES = msgget(clave_peticiones, IPC_CREAT | 0644);

    int clave_respuestas = ftok(f_clave, p_respuestas);
    C_RESPUESTAS = msgget(clave_respuestas, IPC_CREAT | 0644);
}

int main(int argc, char *argv[]) {
    // Temp: silenciar warnings de parámetros no usados
    (void)argc;
    (void)argv;

    printf("Inicializando servidor...\n");

    crear_colas();

    int servidor_activo = 1;

    while (servidor_activo) {
        Mensaje mensaje;

        int tam = sizeof(mensaje) - sizeof(mensaje.tipo);

        int r = msgrcv(C_PETICIONES, &mensaje, tam, 0, 0);

        if (r == -1) {
            perror("msgrcv");
            exit(1);
        }

        printf("Mensaje recibido: %s\n", mensaje.cuerpo);

        if (strcmp(mensaje.cuerpo, "EXIT") == 0) {
            servidor_activo = 0;
        }
    }    

    return 0;
}