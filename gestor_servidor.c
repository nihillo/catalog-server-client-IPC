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
    Mensaje peticion;
    Mensaje respuesta;
    int p, r;

    int servidor_activo = 1;
    int tam = sizeof(peticion) - sizeof(peticion.tipo);

    while (servidor_activo) {
        // Limpiamos variables de mensajes
        memset(&peticion, 0, tam);
        memset(&respuesta, 0, tam);
        p = r = 0;

        p = msgrcv(cola_peticiones, &peticion, tam, 0, 0);
        if (p == -1) {
            perror("Error en la comunicación");
            servidor_activo = 0;
        }
        
        respuesta.tipo = peticion.tipo;
        sprintf(respuesta.cuerpo, "Debug: Mensaje recibido: %s", peticion.cuerpo);
        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
        
        if (r == -1) {
            perror("Error en la comunicación");
            servidor_activo = 0;
        }

        if (servidor_activo) {
            // Descomponemos la petición en variables
            char operacion;
            char codigo[LNG_CODIGO_PRODUCTO];
            char nombre[LNG_NOMBRE];
            char cantidad[LNG_CANTIDAD];

            sscanf(peticion.cuerpo, "%c|%[^|]|%[^|]|%s", &operacion, codigo, nombre, cantidad);

            // Se determina la operación a realizar
            switch (operacion)
            {
                case OP_LISTAR:
                    strcpy(respuesta.cuerpo, "LISTAR");
                    r = msgsnd(cola_respuestas, &respuesta, tam, 0);
        
                    if (r == -1) {
                        perror("Error en la comunicación");
                        servidor_activo = 0;
                    }
                    break;

                case OP_INSERTAR:
                    /* code */
                    break;

                case OP_BUSCAR:
                    /* code */
                    break;

                case OP_APAGAR:
                    printf("Recibida petición de APAGADO\n");
                    servidor_activo = 0;
                    break;

                default:
                    break;
            }

            strcpy(respuesta.cuerpo, "--END");
            r = msgsnd(cola_respuestas, &respuesta, tam, 0);
        
            if (r == -1) {
                perror("Error en la comunicación");
                servidor_activo = 0;
            }
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