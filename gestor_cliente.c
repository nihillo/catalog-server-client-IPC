#include "protocolo.h"

/* --- MENU PRINCIPAL --- */
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
    printf("Pulse una opción: ");

    char entrada_menu[100];
    int opcion;
        
    fgets(entrada_menu, sizeof(entrada_menu), stdin);
    sscanf(entrada_menu, "%d", &opcion);
    
    printf("\n");

    return opcion;
}


/* --- SUBDIALOGOS Y VALIDACIÓN*/
void obtener_codigo(char *codigo) {
    // Validación estricta del código: debe ser numérico y de longitud exacta

    char entrada_codigo[100];
    char codigo_temp[100]; // buffer auxiliar para validar sin desbordar

    while (1) {
        printf("Introduzca código de producto: ");
        fgets(entrada_codigo, sizeof(entrada_codigo), stdin);

        if (sscanf(entrada_codigo, "%s", codigo_temp) == 1) {
            bool valido = true;
            
            // Validación de longitud
            if (strlen(codigo_temp) > LNG_CODIGO_PRODUCTO - 1) {
                valido = false;
                printf("Error: El código es demasiado largo (debe contener %d caracteres numéricos).\n", LNG_CODIGO_PRODUCTO - 1);
            } else if (strlen(codigo_temp) < LNG_CODIGO_PRODUCTO - 1) {
                valido = false;
                printf("Error: El código es demasiado corto (debe contener %d caracteres numéricos).\n", LNG_CODIGO_PRODUCTO - 1);
            } else {
                // Validación de caracteres
                for (int i = 0; i < LNG_CODIGO_PRODUCTO - 1; i++) {
                    if (!isdigit(codigo_temp[i])) {
                        valido = false;
                        printf("Formato no válido: %s. Solo se permiten caracteres numéricos.\n", codigo_temp);
                        break;
                    }
                }
            }
            
            // Si pasa las validaciones, se copia a la variable final
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
    // Validación estricta del nombre: no puede estar vacío ni contener sólo espacios, no puede superar longitud máxima

    char entrada_nombre[100];
    char nombre_temp[100]; // buffer auxiliar para validar sin desbordar

    while(1) {
        printf("Introduzca nombre de producto: ");
        fgets(entrada_nombre, sizeof(entrada_nombre), stdin);

        if (sscanf(entrada_nombre, "%[^\n]", nombre_temp) == 1) {
            // Validación de longitud
            if (strlen(nombre) >= LNG_NOMBRE) {
                printf("Error: El nombre es demasiado largo (máximo %d caracteres).\n", LNG_NOMBRE - 1);
            } else if (strchr(nombre_temp, '|') != NULL) {
                // validar que el nombre no contenga el carácter '|', usado en la construcción de mensajes
                printf("Error: El nombre no puede contener el carácter '|'.\n");
            } else {
                // validar que no sea vacío y no contenga sólo espacios
                bool valido = false;
                for (int i = 0; nombre_temp[i] != '\0'; i++) {
                    if (!isspace(nombre_temp[i]) && isprint(nombre_temp[i])) { 
                        // tan pronto como haya un carácter que no sea espacio y sea imprimible, lo damos por válido
                        valido = true;
                        break;
                    }
                }
                
                if (valido) {
                    // Si pasa las validaciones, se copia a la variable final
                    strcpy(nombre, nombre_temp);
                    break;
                } else {
                    // Si no se ha establecido valido a true, entonces la cadena está vacía o contiene sólo espacios
                    printf("Error: El nombre no puede estar vacío o contener sólo espacios.\n");
                    continue;
                }
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
    // Validación estricta de la cantidad: debe ser numérica y de longitud exacta.
    // Aunque se trate de un valor numérico y obliguemos a que así sea,
    // lo tratamos como cadena para incorporarlo en el cuerpo del mensaje.
    // Será el servidor el que lo convierta a número y lo trate como tal.

    char entrada_cantidad[100];
    int cantidad_temp;

    while(1) {
        printf("Introduzca cantidad: ");
        fgets(entrada_cantidad, sizeof(entrada_cantidad), stdin);

        if (sscanf(entrada_cantidad, "%d", &cantidad_temp) == 1) {
            // Validación de longitud
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


/* --- LÓGICA PRINCIPAL --- */

int main(int argc, char *argv[]) {

    // Silenciar warnings de parámetros no usados
    // (Necesitamos 'argc' para comprobar que el programa se invoque sin parámetros, pero 'argv' no se usa)
    (void)argv;

    // Control de errores: Verifica que no se pasen argumentos por línea de comandos
    if (argc > 1) {
        fprintf(stderr, "Error: El programa no acepta argumentos de línea de comandos.\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();
    int cola_peticiones, cola_respuestas;

    // Se obtienen las claves creadas en protocolo.h
    key_t clave_peticiones = CLAVE_PETICIONES;
    key_t clave_respuestas = CLAVE_RESPUESTAS; 

    // Se declaran las variables 'peticion' y 'respuesta', que se usarán globalmente 
    // independientemente de las operaciones que se seleccionen
    Mensaje peticion;
    Mensaje respuesta;

    // Se asigna al tipo de la petición el pid del proceso. Se usará como filtro para
    // que cada cliente reciba únicamente los mensajes dirigidos a él
    peticion.tipo = (long)pid;
    int tam = sizeof(peticion) - sizeof(peticion.tipo);

    bool servidor_activo = true; // flag que indica que el servidor está activo
    bool cliente_activo = true;  // flag de estado del cliente, usado para salir del bucle principal y cerrarlo si está a false
    while (cliente_activo) {
       
        // Obtener colas. En caso de no existir, se cierra con error de servidor apagado 
        // (Si msgget falla, devuelve -1, el servidor no existe)
        cola_peticiones = msgget(clave_peticiones, 0);
        cola_respuestas = msgget(clave_respuestas, 0);

        if (cola_peticiones == -1 || cola_respuestas == -1 || !servidor_activo) {
            printf("El servidor está apagado. Se cerrará el cliente...\n");
            exit(EXIT_SUCCESS);
        }
        
        // Lógica de empaquetado: creación de la cadena para el cuerpo del mensaje.
        // Esta cadena se construye concatenando:
        // - ID de operación
        // - Código de producto
        // - Nombre
        // - Cantidad
        // Separados por el carácter '|', en la forma OP|COD|NOM|CAT
        int operacion = servidor_activo ? obtener_opcion_menu() : OP_SALIR; // Si el servidor no está activo, forzamos salida del cliente
        char codigo[LNG_CODIGO_PRODUCTO] = {0};
        char nombre[LNG_NOMBRE] = {0};
        char cantidad[LNG_CANTIDAD] = {0};

        // Asignación de las partes de la cadena en función de la operación seleccionada
        // Para las operaciones en que alguna de las partes de la cadena no se necesite, se deja vacía o a 0
        // Para los datos en que se requiera entrada del usuario, se llama a las funciones
        // correspondientes que abren el subdiálogo necesario y validan la entrada antes de asignarla a la variable
        // de la que se leerá para la construcción del mensaje (obtener_codigo, obtener_nombre, obtener_cantidad)
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
                
                // Desde el momento en que se solicita la operación apagar se asume 
                // servidor inactivo para evitar errores por condición de carrera en
                // en la comunicación entre procesos 
                servidor_activo = false;

                // Nota: En la operación apagar no se espera ningún mensaje de respuesta.
                // En la siguiente iteración del bucle, durante las comprobaciones iniciales
                // se detectará que el servidor está apagado, se informa de esto al usuario, 
                // y se fuerza el cierre del cliente. Se ha tomado esta decisión de diseño,
                // aunque no fuera parte de los requisitos, para evitar la inconsistencias derivadas
                // de mantener el cliente abierto estando el servidor cerrado.
                break;

            case OP_SALIR:
                // En la operación salir no se envía ningún mensaje, simplemente se cierra el cliente.
                printf("Cerrando cliente\n");
                cliente_activo = false;
                exit(EXIT_SUCCESS);
                break;

            default:
                printf("Operación no válida\n");
                break;
        }

        // Aseguramos que se cierren correctamente las cadenas
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

        // Envío de la petición a la cola de entrada del servidor
        int p = msgsnd(cola_peticiones, &peticion, tam, 0);

        // Control de errores: siempre que enviamos o recibimos un mensaje de las colas
        // debemos comprobar el resultado y tratar el posible error
        if (p == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }

        // Bucle de recepción: el servidor puede enviar varios mensajes (ej: lista de productos).
        // Se tomó la decisión de diseño de que, para operaciones cuya respuesta pueda venir en múltiples lineas
        // (ej. listado de productos, o bien varias unidades de información a mostrar al usuario),
        // se enviará un mensaje por linea, de forma se pueda mantener acotado el tamaño de la estructura Menasje.
        // Por tanto, el cliente siempre que envía una petición se mantiene a la escucha de respuestas, hasta que
        // el servidor envía un mensaje con la señal '--END' en el cuerpo para indicar el fin de la secuencia.
        bool recibir_secuencia = true;
        while(recibir_secuencia) {
            // msgrcv filtra por el PID de este proceso para no leer respuestas correspondientes a otros
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

            // Protocolo de finalización: si llega "--END", no hay más datos que leer
            if (recibir_secuencia && strcmp(respuesta.cuerpo, "--END") == 0) {
                if (DEBUG) printf("Debug: Fin de mensaje recibido\n");
                recibir_secuencia = false;
            }

            if (recibir_secuencia) { // evitamos imprimir el mensaje si es el de fin de secuencia
                printf("%s\n", respuesta.cuerpo);
            }
        }
    }

    return 0;
} 