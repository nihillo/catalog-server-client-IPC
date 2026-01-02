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

    char entrada_menu[100];
    int opcion;
        
    fgets(entrada_menu, sizeof(entrada_menu), stdin);
    sscanf(entrada_menu, "%d", &opcion);
    
    printf("\n");

    return opcion;
}

void obtener_codigo(char *codigo) {
    char entrada_codigo[100];
    char codigo_temp[100]; // buffer auxiliar para validar sin desbordar

    while (1) {
        printf("Introduzca código de producto: ");
        fgets(entrada_codigo, sizeof(entrada_codigo), stdin);

        if (sscanf(entrada_codigo, "%s", codigo_temp) == 1) {
            bool valido = true;
            
            if (strlen(codigo_temp) != LNG_CODIGO_PRODUCTO - 1) {
                valido = false;
                printf("Error: El código es demasiado largo (máximo %d caracteres).\n", LNG_CODIGO_PRODUCTO - 1);
            }

            for (int i = 0; i < LNG_CODIGO_PRODUCTO - 1; i++) {
                if (!isdigit(codigo_temp[i])) {
                    valido = false;
                    printf("Formato no válido: %s. Solo se permiten caracteres numéricos.\n", codigo_temp);
                    break;
                }
            }

            if (valido) {
                strcpy(codigo, codigo_temp);
                break;
            }        
        } else {
            printf("\nFormato no válido\n");
        }

        // Limpiamos el buffer de entrada (stdin) consumiendo caracteres en caso de que 
        // hayan quedado caracteres sin leer, de tal modo de que en caso de que se haya 
        // introducido una cadena más larga que lo asignado a entrada_nombre, quede 
        // limpio para la siguiente iteración
        if (strchr(entrada_codigo, '\n') == NULL) {
            char c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }
} 

void obtener_nombre(char *nombre) {
    char entrada_nombre[100];
    char nombre_temp[100]; // buffer auxiliar para validar sin desbordar

    while(1) {
        printf("Introduzca nombre de producto: ");
        fgets(entrada_nombre, sizeof(entrada_nombre), stdin);

        if (sscanf(entrada_nombre, "%[^\n]", nombre_temp) == 1) {
            if (strlen(nombre) < LNG_NOMBRE) {
                strcpy(nombre, nombre_temp);
                break;
            } else {
                printf("Error: El nombre es demasiado largo (máximo %d caracteres).\n", LNG_NOMBRE - 1);
            }
        } else {
            printf("\nFormato no válido\n");
        }

        // Limpiamos el buffer de entrada (stdin) consumiendo caracteres en caso de que 
        // hayan quedado caracteres sin leer, de tal modo de que en caso de que se haya 
        // introducido una cadena más larga que lo asignado a entrada_nombre, quede 
        // limpio para la siguiente iteración
        if (strchr(entrada_nombre, '\n') == NULL) {
            char c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }
} 

void obtener_cantidad(char *cantidad) {
    char entrada_cantidad[100];
    int cantidad_temp;

    while(1) {
        printf("Introduzca cantidad: ");
        fgets(entrada_cantidad, sizeof(entrada_cantidad), stdin);

        if (sscanf(entrada_cantidad, "%d", &cantidad_temp) == 1) {
            if (cantidad_temp >= 0 && cantidad_temp < 1000) {
                snprintf(cantidad, LNG_CANTIDAD, "%d", cantidad_temp);
                break;
            } else {
                printf("Error: cantidad no válida (0-999)\n");
            }
        } else {
            printf("\nFormato no válido\n");
        }

        // Limpiamos el buffer de entrada (stdin) consumiendo caracteres en caso de que 
        // hayan quedado caracteres sin leer, de tal modo de que en caso de que se haya 
        // introducido una cadena más larga que lo asignado a entrada_nombre, quede 
        // limpio para la siguiente iteración
        if (strchr(entrada_cantidad, '\n') == NULL) {
            char c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
    }
}

int main(int argc, char *argv[]) {
    (void)argv; // silenciar advertencias de parámetros no usados

    if (argc > 1) {
        fprintf(stderr, "Error: El programa no acepta argumentos de línea de comandos.\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();
    int cola_peticiones, cola_respuestas;

    key_t clave_peticiones = CLAVE_PETICIONES;
    key_t clave_respuestas = CLAVE_RESPUESTAS; 

    Mensaje peticion;
    Mensaje respuesta;

    peticion.tipo = (long)pid;
    int tam = sizeof(peticion) - sizeof(peticion.tipo);

    bool servidor_activo = true;
    bool cliente_activo = true;
    while (cliente_activo) {
       
        // Obtener colas. En caso de no existir, se cierra con error de servidor apagado 
        cola_peticiones = msgget(clave_peticiones, 0);
        cola_respuestas = msgget(clave_respuestas, 0);

        if (cola_peticiones == -1 || cola_respuestas == -1 || !servidor_activo) {
            printf("El servidor está apagado. Se cerrará el cliente...\n");
            exit(EXIT_SUCCESS);
        }

        // Si el servidor no está activo, forzamos salida del cliente
        int operacion = servidor_activo ? obtener_opcion_menu() : OP_SALIR;
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
                
                // Se asume servidor inactivo para evitar errores por condición de carrera
                servidor_activo = false;
                break;

            case OP_SALIR:
                printf("Cerrando cliente\n");
                cliente_activo = false;
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

        
        // Recibir secuencia de mensajes
        bool recibir_secuencia = true;
        while(recibir_secuencia) {
            int r = msgrcv(cola_respuestas, &respuesta, tam, pid, 0);

            if (r == -1) {
                if (errno == EIDRM || errno == EINVAL) {
                    // Si se elimina cola_respuestas mientras esperamos un mensaje, significa
                    // que se ha apagado el servidor
                    printf("Servidor apagado.\n");
                    // Desactivamos la recepción de secuencia para salir del bucle 
                    recibir_secuencia = false;
                } else {
                    perror("Error en la recepción de la respuesta");
                    exit(EXIT_FAILURE);
                }
            }

            if (recibir_secuencia && strcmp(respuesta.cuerpo, "--END") == 0) {
                if (DEBUG) printf("Debug: Fin de mensaje recibido\n");
                recibir_secuencia = false;
            }

            if (recibir_secuencia) {
                printf("%s\n", respuesta.cuerpo);
            }
        }
    }

    return 0;
} 