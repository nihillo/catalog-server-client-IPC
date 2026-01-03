#include "protocolo.h"

int comparadorProductos(const void *a, const void *b) {
    Producto *p1 = (Producto *)a;
    Producto *p2 = (Producto *)b;

    return strcmp(p1->codigo, p2->codigo);
}

int main(int argc, char *argv[]) {
    // Temp: silenciar warnings de parámetros no usados
    (void)argc;
    (void)argv;

    printf("Inicializando servidor...\n");

    key_t clave_peticiones = CLAVE_PETICIONES;
    key_t clave_respuestas = CLAVE_RESPUESTAS;

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
    Mensaje peticion;
    Mensaje respuesta;
    int p, r;
    FILE *f;

    bool servidor_activo = true;
    int tam = sizeof(peticion) - sizeof(peticion.tipo);

    while (servidor_activo) {
        // Limpiamos variables de mensajes
        memset(&peticion, 0, tam);
        memset(&respuesta, 0, tam);
        p = r = 0;

        p = msgrcv(cola_peticiones, &peticion, tam, 0, 0);
        if (p == -1) {
            perror("Error en la comunicación");
            servidor_activo = false;
        }
        
        respuesta.tipo = peticion.tipo;

        if (DEBUG) sprintf(respuesta.cuerpo, "Debug: Mensaje recibido: %s", peticion.cuerpo);
        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
        
        if (r == -1) {
            perror("Error en la comunicación");
            servidor_activo = false;
        }

        if (servidor_activo) {
            // Descomponemos la petición en variables
            int operacion;
            char codigo[LNG_CODIGO_PRODUCTO];
            char nombre[LNG_NOMBRE];
            char cantidad[LNG_CANTIDAD];

            sscanf(peticion.cuerpo, "%d|%[^|]|%[^|]|%s", &operacion, codigo, nombre, cantidad);

            // Se determina la operación a realizar
            switch (operacion) {

                case OP_LISTAR: // operación listar
                    ;
                    // Determinamos primero si hay productos
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
                        sprintf(respuesta.cuerpo, "No existen productos en el inventario.");
                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }
                    } else {

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

                        while (fread(&producto_lst, sizeof(Producto), 1, f) == 1) {
                            // Se envía a la cola un mensaje por cada producto
                            sprintf(respuesta.cuerpo, "%s\t%s\t%d ud.", producto_lst.codigo, producto_lst.nombre, producto_lst.cantidad);
                            r = msgsnd(cola_respuestas, &respuesta, tam, 0);
            
                            if (r == -1) {
                                perror("Error en la comunicación");
                                servidor_activo = false;
                            }
                        }

                        fclose(f);
                    }
                    break;

                case OP_INSERTAR: // operación insertar
                    sprintf(respuesta.cuerpo, "INSERTAR: Cod %s, Nom %s, Cant %s", codigo, nombre, cantidad);
                    r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                    if (r == -1) {
                        perror("Error en la comunicación");
                        servidor_activo = false;
                    }

                    Producto producto_ins;
                    strcpy(producto_ins.codigo, codigo);
                    strcpy(producto_ins.nombre, nombre);
                    producto_ins.cantidad = atoi(cantidad);

                    f = fopen("productos.dat", "rb");
                    if (f == NULL) {
                        printf("El archivo productos.dat no existe. Se creará uno nuevo...\n");
                        f = fopen("productos.dat", "wb");
                        if (f == NULL) {
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
                        sprintf(respuesta.cuerpo, "El producto con código %s ya existe.", producto_ins.codigo);
                    } else {
                        // Si el producto no existe, lo añadimos
                        fseek(f, 0, SEEK_END); 
                        long tam_archivo = ftell(f);
                        
                        int num_productos;
                        if (tam_archivo == 0) {
                            num_productos = 0;
                        } else {
                            num_productos = tam_archivo / sizeof(Producto);
                        }

                        Producto *inventario = malloc((num_productos + 1) * sizeof(Producto));
                        if (inventario == NULL) {
                            perror("Error: No se pudo reservar memoria para el inventario");
                            exit(EXIT_FAILURE);
                        }

                        rewind(f);
                        fread(inventario, sizeof(Producto), num_productos, f);
                        
                        inventario[num_productos] = producto_ins;
                        fseek(f, 0, SEEK_SET);
                        
                        qsort(inventario, num_productos + 1, sizeof(Producto), comparadorProductos);
                       
                        // Se cierra el archivo para volverlo a abrir en modo de escritura
                        fclose(f);
                        f = fopen("productos.dat", "wb");
                        if (f == NULL) {
                            perror("Error al abrir el archivo productos.dat en modo escritura");
                            free(inventario);
                            fclose(f);
                            exit(EXIT_FAILURE);
                        }

                        size_t escrito = fwrite(inventario, sizeof(Producto), num_productos + 1, f);

                        if (escrito != (size_t)(num_productos + 1)) {
                            perror("Error al escribir en el archivo productos.dat");
                            free(inventario);
                            fclose(f);
                            exit(EXIT_FAILURE);
                        }

                        free(inventario);

                        sprintf(respuesta.cuerpo, "Producto insertado con éxito");
                    }

                    fclose(f);

                    
                    r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                    if (r == -1) {
                        perror("Error en la comunicación");
                        servidor_activo = false;
                    }
                    break;

                case OP_BUSCAR: // operación buscar
                    
                    f = fopen("productos.dat", "rb");
                    if (f == NULL) {
                        perror("Error: No se pudo abrir el archivo productos.dat");
                        exit(EXIT_FAILURE);
                    }

                    Producto producto_bus;
                    rewind(f);
                    
                    bool encontrado = false;
                    while (!encontrado && fread(&producto_bus, sizeof(Producto), 1, f) == 1) {
                        if (strcmp(producto_bus.codigo, codigo) == 0) {
                            encontrado = true;
                        }
                    }

                    if (encontrado) {
                        // Se envía a la cola un mensaje por cada linea a imprimir, para evitar desbordar el tamaño de mensaje

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
                        sprintf(respuesta.cuerpo, "Producto con código %s no encontrado", codigo);

                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }
                    }

                    fclose(f);

                    break;

                case OP_APAGAR: // operación apagar
                    printf("Recibida petición de APAGADO\n");
                    servidor_activo = false;
                    break;

                default:
                    break;
            }


            // Enviamos fin de secuencia
            strcpy(respuesta.cuerpo, "--END");
            r = msgsnd(cola_respuestas, &respuesta, tam, 0);
        
            if (r == -1) {
                perror("Error en la comunicación");
                servidor_activo = false;
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