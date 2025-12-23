#include "protocolo.h"

int obtener_opcion_menu() {
    printf("\n");
    printf("***********************************************\n");
    printf("Gestor de inventario remoto\n");
    printf("***********************************************\n");
    printf("1) Listar productos (Pulsar 1)\n");
    printf("2) Buscar producto (Pulsar 2)\n");
    printf("3) Insertar producto (Pulsar 3)\n");
    printf("4) Apagar el servidor (Pulsar 4)\n");
    printf("5) Salir del cliente (Pulsar 5)\n");
    printf("***********************************************\n");
    printf("\n");
    printf("Pulse una opción: ");

    char entrada[100];
    int opcion;
        
    fgets(entrada, sizeof(entrada), stdin);
    sscanf(entrada, "%d", &opcion);
    
    printf("\n");

    return opcion;
}

void obtener_codigo(char *codigo) {
    char entrada[100];

    printf("Introduzca código de producto: ");
    fgets(entrada, sizeof(entrada), stdin);
    sscanf(entrada, "%s", codigo);
} 

void obtener_nombre(char *nombre) {
    char entrada[100];

    printf("Introduzca nombre de producto: ");
    fgets(entrada, sizeof(entrada), stdin);
    sscanf(entrada, "%[^\n]", nombre);
} 

void obtener_cantidad(char *cantidad) {
    char entrada[100];

    printf("Introduzca cantidad: ");
    fgets(entrada, sizeof(entrada), stdin);
    sscanf(entrada, "%s", cantidad);
}

int main(int argc, char *argv[]) {
    (void)argv; // silenciar advertencias de parámetros no usados

    if (argc > 1) {
        fprintf(stderr, "Error: El programa no acepta argumentos de línea de comandos.\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();
    int cola_peticiones, cola_respuestas;

    Mensaje peticion;
    Mensaje respuesta;

    peticion.tipo = (long)pid;

    // Obtener colas de mensajes
    int tam = sizeof(peticion) - sizeof(peticion.tipo);

    key_t clave_peticiones = CLAVE_PETICIONES;
    key_t clave_respuestas = CLAVE_RESPUESTAS;

    cola_peticiones = msgget(clave_peticiones, 0);
    cola_respuestas = msgget(clave_respuestas, 0);  

    if (cola_peticiones == -1 || cola_respuestas == -1) {
        printf("El servidor está apagado\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int operacion = obtener_opcion_menu();
        char codigo[LNG_CODIGO_PRODUCTO] = {0};
        char nombre[LNG_NOMBRE] = {0};
        char cantidad[LNG_CANTIDAD] = {0};
        
        switch (operacion) {
            case OP_LISTAR:
                strncpy(codigo, "", LNG_CODIGO_PRODUCTO - 1);
                strncpy(nombre, "", LNG_NOMBRE - 1);
                strncpy(cantidad, "0", LNG_CANTIDAD - 1);
                break;
            case OP_BUSCAR:
                obtener_codigo(codigo);
                strncpy(nombre, "", LNG_NOMBRE - 1);
                strncpy(cantidad, "0", LNG_CANTIDAD - 1);
                break;
            case OP_INSERTAR:
                obtener_codigo(codigo);
                obtener_nombre(nombre);
                obtener_cantidad(cantidad);
                break;
            case OP_APAGAR:
                strncpy(codigo, "", LNG_CODIGO_PRODUCTO - 1);
                strncpy(nombre, "", LNG_NOMBRE - 1);
                strncpy(cantidad, "0", LNG_CANTIDAD - 1);
                break;
            case OP_SALIR:
                printf("Cerrando cliente\n");
                exit(EXIT_SUCCESS);
                break;
            default:
                printf("Operación no válida\n");
                break;
        }

        // Cerramos correctamente las cadenas
        codigo[LNG_CODIGO_PRODUCTO - 1] = '\0';
        nombre[LNG_NOMBRE - 1] = '\0';
        cantidad[LNG_CANTIDAD - 1] = '\0';

        // Concatenamos el cuerpo del mensaje
        snprintf(
            peticion.cuerpo, 
            LNG_PET, 
            "%d|%s|%s|%s", 
            operacion, 
            codigo, 
            nombre, 
            cantidad
        );

        int p = msgsnd(cola_peticiones, &peticion, tam, 0);

        if (p == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }

        int cliente_activo = 1;

        while(cliente_activo) {
            int r = msgrcv(cola_respuestas, &respuesta, tam, pid, 0);

            if (r == -1) {
                perror("Error en la recepción de la respuesta");
                exit(EXIT_FAILURE);
            }

            if (strcmp(respuesta.cuerpo, "--END") == 0) {
                printf("Fin de mensaje recibido\n");
                cliente_activo = 0;
            }

            if (cliente_activo) {
                printf("%s\n", respuesta.cuerpo);
            }
        }
    }
    
    return 0;
} 