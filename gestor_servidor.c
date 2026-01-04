#include "protocolo.h"

/* * FUNCIÓN AUXILIAR: comparadorProductos
 * Esta función es necesaria para la función estándar qsort().
 * Compara dos estructuras Producto basándose en su código alfanumérico.
 * Esto asegura que el archivo productos.dat se mantenga siempre ordenado.
 */
int comparadorProductos(const void *a, const void *b) {
    Producto *p1 = (Producto *)a;
    Producto *p2 = (Producto *)b;

    return strcmp(p1->codigo, p2->codigo);
}


/* -- FUNCION PRINCIPAL -- */
int main(int argc, char *argv[]) {

    // Silenciar warnings de parámetros no usados
    // (Necesitamos 'argc' para comprobar que el programa se invoque sin parámetros, pero 'argv' no se usa)
    (void)argv;


    // Control de errores: Verifica que no se pasen argumentos por línea de comandos
    if (argc > 1) {
        fprintf(stderr, "Error: El programa no acepta argumentos de línea de comandos.\n");
        exit(EXIT_FAILURE);
    }

    printf("Inicializando servidor...\n");

    // Obtención de claves únicas definidas en protocolo.h
    key_t clave_peticiones = CLAVE_PETICIONES;
    key_t clave_respuestas   = CLAVE_RESPUESTAS;


    // Creación de colas
    // Control de errores:
    // - Se usa IPC_CREAT | IPC_EXCL para asegurar que el servidor es quien crea las colas.
    // - Si msgget falla con EEXIST, significa que ya hay un servidor en ejecución o que han 
    //   quedado colas abiertas de un cierre anterior no limpio
    int cola_peticiones = msgget(clave_peticiones, IPC_CREAT | IPC_EXCL | 0666);
    int cola_respuestas = msgget(clave_respuestas, IPC_CREAT | IPC_EXCL | 0666);
    
    if (cola_peticiones == -1 || cola_respuestas == -1) {
        if (errno == EEXIST) {
            printf("Colas de mensajes ya inicializadas. Esto pudede deberse a que el servidor ya esté inicializado, o a que hayan quedado abiertas de un apagado anterior no limpio.\n");
        } else {
            perror("Error al crear las colas de mensajes (msgget)");
        }
        exit(EXIT_FAILURE);
    }

    printf("Colas de mensajes inicializadas. Peticiones ID: %d, Respuestas ID: %d\n", cola_peticiones, cola_respuestas);

    // Declaración de variables globales
    Mensaje peticion;   // mensaje petición
    Mensaje respuesta;  // mensaje respuesta
    int p, r;           // resultados de peticiones y respuestas, necesarios para el control de errores
    FILE *f;            // variable sobre la que se abrirá en memoria el archivo productos.dat 

    bool servidor_activo = true;                        // flag que indica que el servidor está activo
    int tam = sizeof(peticion) - sizeof(peticion.tipo); // tamaño del mensaje

    // BUCLE PRINCIPAL DE ESCUCHA A MENSAJES PROVENIENTES DE CLIENTES
    while (servidor_activo) {
        // Limpiamos los buffers para evitar basura de peticiones anterior
        memset(&peticion, 0, tam);
        memset(&respuesta, 0, tam);
        p = r = 0;

        /* RECUPERACIÓN DE PETICIÓN:
         * msgrcv con tipo '0' permite leer el primer mensaje disponible en la cola,
         * sin importar qué cliente lo envió.
         */
        p = msgrcv(cola_peticiones, &peticion, tam, 0, 0);
        // Siempre que recibimos o enviamos mensajes necesitamos controlar los posibles errores
        if (p == -1) {
            perror("Error en la comunicación");
            servidor_activo = false;
        }
        
        // Se asigna al tipo de la respuesta el mismo de la petición, de forma que sea recibida por
        // el mismo cliente
        respuesta.tipo = peticion.tipo;

        if (DEBUG) { 
            // Confirmación de recepción de mensaje, en modo DEBUG
            sprintf(respuesta.cuerpo, "Debug: Mensaje recibido: %s", peticion.cuerpo);
            r = msgsnd(cola_respuestas, &respuesta, tam, 0);
            
            if (r == -1) {
                perror("Error en la comunicación");
                servidor_activo = false;
            }
        }

        if (servidor_activo) {
            // Deserialización. Extraemos las partes del cuerpo del mensaje a las variables correspondientes
            int operacion;
            char codigo[LNG_CODIGO_PRODUCTO];
            char nombre[LNG_NOMBRE];
            char cantidad[LNG_CANTIDAD]; // 

            sscanf(peticion.cuerpo, "%d|%[^|]|%[^|]|%s", &operacion, codigo, nombre, cantidad);

            // Se determina la operación a realizar
            switch (operacion) {

                case OP_LISTAR: // operación listar
                    if (DEBUG) {
                        sprintf(respuesta.cuerpo, "LISTAR");
                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }
                    }

                    // Lógica para determinar si el archivo existe y tiene registros
                    int num_productos = 0;
                    f = fopen("productos.dat", "rb");
                    if (f == NULL) {
                        num_productos = 0;
                    } else {
                        fseek(f, 0, SEEK_END);
                        long tam_archivo = ftell(f);
                        if (tam_archivo == 0) {
                            num_productos = 0;
                        } else {
                            num_productos = tam_archivo / sizeof(Producto);
                        }
                        fclose(f);
                    }
                    
                    if (num_productos == 0) {
                        // Si no hay productos, devolvemos directamente respuesta
                        sprintf(respuesta.cuerpo, "No existen productos en el inventario.");
                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        // Se controlan errores siempre que se recibe o envía un mensaje
                        // En este caso, en caso de error se marca servidor_activo a false para salir del bucle
                        // y cerrar limpiamente
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }
                    } else {

                        // Si hay productos, enviamos en primer lugar las lineas de cabecera, y después el listado
                        sprintf(respuesta.cuerpo, "Listado de productos (Código, Nombre, Cantidad)");
                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }

                        sprintf(respuesta.cuerpo, "-----------------------------------------------");
                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }

                    
                        f = fopen("productos.dat", "rb");
                        if (f == NULL) {
                            perror("Error: No se pudo abrir el archivo productos.dat");
                            exit(EXIT_FAILURE);
                        }
                        

                        Producto producto_lst;
                        rewind(f);

                        // Por decisión de diseño, se envía cada producto en un mensaje, de forma que se mantenga controlado
                        // el tamaño de mensaje definido.
                        // Se itera sobre las lineas del archivo, y por cada una de ellas se envía la linea formateada
                        while (fread(&producto_lst, sizeof(Producto), 1, f) == 1) {
                            
                            sprintf(respuesta.cuerpo, "%s\t%s\t%d ud.", producto_lst.codigo, producto_lst.nombre, producto_lst.cantidad);
                            r = msgsnd(cola_respuestas, &respuesta, tam, 0);

                            // Siempre que enviamos o recibimos mensajes se controlan posibles errores
                            if (r == -1) {
                                perror("Error en la comunicación");
                                servidor_activo = false;
                            }
                        }
                        
                        // Siempre que se deja de trabajar con el archivo se debe cerrar
                        fclose(f);
                    }
                    break;

                case OP_INSERTAR: // operación insertar
                    
                    if (DEBUG) {
                        sprintf(respuesta.cuerpo, "INSERTAR: Cod %s, Nom %s, Cant %s", codigo, nombre, cantidad);

                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }
                    }

                    // Se crea la instancia de Producto
                    Producto producto_ins;
                    strcpy(producto_ins.codigo, codigo);
                    strcpy(producto_ins.nombre, nombre);
                    producto_ins.cantidad = atoi(cantidad); // se convierte cantidad, tratada hasta como cadena, a entero

                    // Comprobación de existencia del archivo productos.dat
                    f = fopen("productos.dat", "rb");
                    if (f == NULL) {
                        // En caso de no existir, se crea
                        printf("El archivo productos.dat no existe. Se creará uno nuevo...\n");
                        f = fopen("productos.dat", "wb");
                        if (f == NULL) {
                            // Control de errores: si el archivo no existe y no se puede crear, se sale del programa
                            perror("Error fatal: No se pudo crear el archivo productos.dat");
                            exit(EXIT_FAILURE);
                        }
                    } 

                    // Verificar que no exista ya un producto con el mismo código
                    Producto temp;
                    bool producto_existe = false;
                    rewind(f);
                    while (fread(&temp, sizeof(Producto), 1, f) == 1) {
                        if (strcmp(temp.codigo, producto_ins.codigo) == 0) {
                            producto_existe = true;
                            break;
                        }
                    }

                    if (producto_existe) {
                        // Si el producto existe, se informa en la respuesta y no se inserta
                        sprintf(respuesta.cuerpo, "El producto con código %s ya existe.", producto_ins.codigo);
                    } else {
                        // Si el producto no existe, lo añadimos
                        fseek(f, 0, SEEK_END); 
                        long tam_archivo = ftell(f);
                        
                        // Se determina el número de productos existentes, como paso previo necesario a la carga en 
                        // memoria del inventario, necesaria para mantenerlo ordenado por código
                        int num_productos;
                        if (tam_archivo == 0) {
                            num_productos = 0;
                        } else {
                            num_productos = tam_archivo / sizeof(Producto);
                        }

                        // Reserva de memoria
                        Producto *inventario = malloc((num_productos + 1) * sizeof(Producto));
                        if (inventario == NULL) {
                            // Control de errores: si falla se sale del programa
                            perror("Error: No se pudo reservar memoria para el inventario");
                            exit(EXIT_FAILURE);
                        }

                        // Se carga en memoria el inventario
                        rewind(f);
                        fread(inventario, sizeof(Producto), num_productos, f);
                        
                        // Se añade el producto al final del array
                        inventario[num_productos] = producto_ins;
                        fseek(f, 0, SEEK_SET);
                        
                        // Se ordena el inventario por código de producto
                        qsort(inventario, num_productos + 1, sizeof(Producto), comparadorProductos);
                       
                        // Se cierra el archivo para volverlo a abrir en modo de escritura
                        fclose(f);
                        f = fopen("productos.dat", "wb");
                        if (f == NULL) {
                            // Control de errores: si falla se libera la memoria, se cierra el archivo, y se sale del programa
                            perror("Error al abrir el archivo productos.dat en modo escritura");
                            free(inventario);
                            fclose(f);
                            exit(EXIT_FAILURE);
                        }

                        // Se escribe el inventario con el producto nuevo añadido, y reordenado, al archivo
                        size_t escrito = fwrite(inventario, sizeof(Producto), num_productos + 1, f);

                        // Control de errores: si el tamaño en bytes escrito al archivo no coincide con el tamaño
                        // en bytes del número de productos incluyendo el nuevo añadido, se considera que ha habido
                        // un error de escritura. En ese caso se libera el espacio en memoria, se cierra el archivo, 
                        // y se sale del programa.
                        if (escrito != (size_t)(num_productos + 1)) {
                            perror("Error al escribir en el archivo productos.dat");
                            free(inventario);
                            fclose(f);
                            exit(EXIT_FAILURE);
                        }

                        // Una vez reescrito el inventario al archivo, se libera la reserva de espacio en memoria
                        free(inventario);

                        // Se carga en el cuerpo de la respuesta mensaje de operación realizada con éxito
                        sprintf(respuesta.cuerpo, "Producto insertado con éxito");
                    }
                    
                    // Se cierra el archivo productos.dat
                    fclose(f);

                    // Enviamos la respuesta al cliente
                    r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                    // Control de errores: si el envío falla, se desmarca servidor_activo de forma que se cierre limpiamente
                    if (r == -1) {
                        perror("Error en la comunicación");
                        servidor_activo = false;
                    }
                    break;

                case OP_BUSCAR: // operación buscar

                    if (DEBUG) {
                        sprintf(respuesta.cuerpo, "BUSCAR: Cod %s", codigo);
                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }
                    }
                    
                    // Se abre el archivo productos.dat en modo lectura
                    f = fopen("productos.dat", "rb");
                    if (f == NULL) {
                        perror("Error: No se pudo abrir el archivo productos.dat");
                        exit(EXIT_FAILURE);
                    }
                    
                    // Se busca el producto en el archivo
                    Producto producto_bus;
                    rewind(f);
                    
                    // Se lee el archivo linea a linea hasta que se encuentre un producto 
                    // cuyo código coincida con el buscado
                    bool encontrado = false;
                    while (!encontrado && fread(&producto_bus, sizeof(Producto), 1, f) == 1) {
                        if (strcmp(producto_bus.codigo, codigo) == 0) {
                            encontrado = true;
                        }
                    }

                    if (encontrado) {
                        // Se envía a la cola un mensaje por cada linea a imprimir, para evitar desbordar el tamaño de mensaje
                        // ej.
                        // Código: [COD]
                        // NOmbre: [NOM]
                        // Cantidad: [CANT]

                        sprintf(respuesta.cuerpo, "Código: %s", producto_bus.codigo);
                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }

                        sprintf(respuesta.cuerpo, "Nombre: %s", producto_bus.nombre);
                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }

                        sprintf(respuesta.cuerpo, "Cantidad: %d", producto_bus.cantidad);
                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }

                    } else {
                        // En caso de que se termine de recorrer el archivo sin encontrar el producto
                        // buscado, se indica en el mensaje devuelto.
                        sprintf(respuesta.cuerpo, "Producto con código %s no encontrado", codigo);

                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }
                    }

                    // Se cierra el archivo
                    fclose(f);

                    break;

                case OP_APAGAR: // operación apagar
                    // En la operación apagar no se envía ningún mensaje, simplemente se desmarca servidor_activo
                    // para salir del bucle y que se cierre el servidor limpiamente.

                    // Mensaje impreso en el lado del servidor, que no necesariamente estará en la misma terminal
                    // que el cliente. En caso de apagado, el cliente tiene la responsabilidad de detectar que el
                    // servidor ya no está activo e imprimir sus propios mensajes indicándolo.
                    printf("Recibida petición de APAGADO\n");
                    servidor_activo = false;
                    break;

                default:
                    break;
            }


            // Enviamos fin de secuencia para indicar al cliente que ya no hay más mensajes por enviar
            strcpy(respuesta.cuerpo, "--END");
            r = msgsnd(cola_respuestas, &respuesta, tam, 0);
        
            if (r == -1) {
                perror("Error en la comunicación");
                servidor_activo = false;
            }

        }
    }    


    // Una vez se sale del bucle de escucha (cuando se desactiva el flag servidor_activo), 
    // se cierra siempre el servidor, eliminando en primer lugar las colas de mensajes.
    printf("Apagando servidor...\n");
    printf("Eliminando colas de mensajes...\n");
    if (msgctl(cola_peticiones, IPC_RMID, NULL) == -1) {
        // Control de errores: en caso del que el resultado de la eliminación sea -1, se informa el error
        perror("Error al terminar la cola de peticiones (msgctl)");
    }
    
    if (msgctl(cola_respuestas, IPC_RMID, NULL) == -1) {
        // Control de errores: en caso del que el resultado de la eliminación sea -1, se informa el error
        perror("Error al terminar la cola de respuestas (msgctl)");
    }

    // Mensaje de apagado impreso en el lado del servidor, que no necesariamente estará en la misma terminal
    // que el cliente. En caso de apagado, el cliente tiene la responsabilidad de detectar que el
    // servidor ya no está activo e imprimir sus propios mensajes indicándolo.
    printf("Servidor apagado.\n");

    return 0;
}