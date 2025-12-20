#include "protocolo.h"

int main(int argc, char *argv[]) {
    pid_t pid = getpid();
    int cola_peticiones, cola_respuestas;

    Mensaje peticion;
    Mensaje respuesta;

    peticion.tipo = (long)pid;

    if (argc < 2) {
        printf("No se ha introducido la operación a realizar\n");
        exit(1);
    }

    char *arg_op = argv[1];

    if (
        strcmp(arg_op, ARG_LISTAR) != 0
        && strcmp(arg_op, ARG_BUSCAR) != 0
        && strcmp(arg_op, ARG_INSERTAR) != 0
        && strcmp(arg_op, ARG_APAGAR) != 0
    ) {
        printf("Operación no válida\n");
        exit(1);
    }

    char op;
    char codigo[LNG_CODIGO_PRODUCTO] = {0};
    char nombre[LNG_NOMBRE] = {0};
    char cantidad[LNG_CANTIDAD] = {0};

    if (strcmp(arg_op, ARG_LISTAR) == 0) {
        op = OP_LISTAR;
        strncpy(codigo, "", LNG_CODIGO_PRODUCTO - 1);
        strncpy(nombre, "", LNG_NOMBRE - 1);
        strncpy(cantidad, "0", LNG_CANTIDAD - 1);

    } else if (strcmp(arg_op, ARG_BUSCAR) == 0) {
        if (argc < 3) {
            printf("No se ha introducido el código del producto a buscar\n");
            exit(1);
        }

        op = OP_BUSCAR;
        strncpy(codigo, argv[2], LNG_CODIGO_PRODUCTO - 1);
        strncpy(nombre, "", LNG_NOMBRE - 1);
        strncpy(cantidad, "0", LNG_CANTIDAD - 1);

    } else if (strcmp(arg_op, ARG_INSERTAR) == 0) {
        if (argc < 5) {
            printf("No se han introducido todos los datos necesarios del producto a insertar\n");
            printf("insertar [código] [nombre] [cantidad]\n");
            exit(1);
        }

        op = OP_INSERTAR;
        strncpy(codigo, argv[2], LNG_CODIGO_PRODUCTO - 1);
        strncpy(nombre, argv[3], LNG_NOMBRE - 1);
        strncpy(cantidad, argv[4], LNG_CANTIDAD - 1);
        
    } else if (strcmp(arg_op, ARG_APAGAR) == 0) {
        op = OP_APAGAR;
        strncpy(codigo, "", LNG_CODIGO_PRODUCTO - 1);
        strncpy(nombre, "", LNG_NOMBRE - 1);
        strncpy(cantidad, "0", LNG_CANTIDAD - 1);

    }

    // Cerramos correctamente las cadenas
    codigo[LNG_CODIGO_PRODUCTO - 1] = '\0';
    nombre[LNG_NOMBRE - 1] = '\0';
    cantidad[LNG_CANTIDAD - 1] = '\0';

    // Concatenamos el cuerpo del mensaje
    snprintf(
        peticion.cuerpo, 
        LNG_PET, 
        "%c|%s|%s|%s", 
        op, 
        codigo, 
        nombre, 
        cantidad
    );



    int tam = sizeof(peticion) - sizeof(peticion.tipo);

    key_t clave_peticiones = CLAVE_PETICIONES;
    key_t clave_respuestas = CLAVE_RESPUESTAS;

    cola_peticiones = msgget(clave_peticiones, 0);
    cola_respuestas = msgget(clave_respuestas, 0);  

    if (cola_peticiones == -1 || cola_respuestas == -1) {
        printf("El servidor está apagado\n");
        return 0;
    }

    

    int p = msgsnd(cola_peticiones, &peticion, tam, 0);

    if (p == -1) {
        perror("msgsnd");
        exit(1);
    }

    int cliente_activo = 1;

    while(cliente_activo) {
        int r = msgrcv(cola_respuestas, &respuesta, tam, pid, 0);

        if (r == -1) {
            perror("Error en la recepción de la respuesta");
            exit(1);
        }

        if (strcmp(respuesta.cuerpo, "--END") == 0) {
            printf("Fin de mensaje recibido\n");
            cliente_activo = 0;
        }

        if (cliente_activo) {
            printf("%s\n", respuesta.cuerpo);
        }
    }
    

    return 0;
} 