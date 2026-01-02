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

                case OP_LISTAR:

                    sprintf(respuesta.cuerpo, "Listado de productos (Código, Nombre, Cantidad)");
                    r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                    if (r == -1) {
                        perror("Error en la comunicación");
                        servidor_activo = false;
                    }

                    sprintf(respuesta.cuerpo, "***********************************************");
                    r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                    if (r == -1) {
                        perror("Error en la comunicación");
                        servidor_activo = false;
                    }

                
                    FILE *f_lst = fopen("productos.dat", "rb");
                    if (f_lst == NULL) {
                        perror("Error: No se pudo abrir el archivo productos.dat");
                        exit(EXIT_FAILURE);
                    }

                    Producto producto_lst;
                    rewind(f_lst);

                    while (fread(&producto_lst, sizeof(Producto), 1, f_lst) == 1) {
                        // Se envía a la cola un mensaje por cada producto
                        sprintf(respuesta.cuerpo, "%s\t%s\t%d", producto_lst.codigo, producto_lst.nombre, producto_lst.cantidad);
                        r = msgsnd(cola_respuestas, &respuesta, tam, 0);
        
                        if (r == -1) {
                            perror("Error en la comunicación");
                            servidor_activo = false;
                        }
                    }
                    break;

                case OP_INSERTAR:
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

                    FILE *f_ins = fopen("productos.dat", "rb");
                    if (f_ins == NULL) {
                        printf("El archivo productos.dat no existe. Se creará uno nuevo...\n");
                        f_ins = fopen("productos.dat", "wb");
                        if (f_ins == NULL) {
                            perror("Error fatal: No se pudo crear el archivo productos.dat");
                            exit(EXIT_FAILURE);
                        }
                    } 

                    // Verificar que no exista ya un producto con el mismo código
                    Producto temp;
                    bool producto_existe = false;
                    rewind(f_ins);

                    while (fread(&temp, sizeof(Producto), 1, f_ins) == 1) {
                        if (strcmp(temp.codigo, producto_ins.codigo) == 0) {
                            producto_existe = true;
                            break;
                        }
                    }

                    if (producto_existe) {
                        sprintf(respuesta.cuerpo, "El producto con código %s ya existe.", producto_ins.codigo);
                    } else {
                        // Si el producto no existe, lo añadimos
                        fseek(f_ins, 0, SEEK_END); 
                        long tam_archivo = ftell(f_ins);
                        
                        int num_productos;
                        if (tam_archivo == 0) {
                            num_productos = 0;
                        } else {
                            num_productos = tam_archivo / sizeof(Producto);
                        }

                        Producto *catalogo = malloc((num_productos + 1) * sizeof(Producto));
                        rewind(f_ins);
                        fread(catalogo, sizeof(Producto), num_productos, f_ins);
                        
                        catalogo[num_productos] = producto_ins;
                        fseek(f_ins, 0, SEEK_SET);
                        
                        qsort(catalogo, num_productos + 1, sizeof(Producto), comparadorProductos);
                       
                        // Se cierra el archivo para volverlo a abrir en modo de escritura
                        fclose(f_ins);
                        f_ins = fopen("productos.dat", "wb");
                        if (f_ins == NULL) {
                            perror("Error fatal: No se pudo abrir el archivo productos.dat en modo escritura");
                            exit(EXIT_FAILURE);
                        }

                        fwrite(catalogo, sizeof(Producto), num_productos + 1, f_ins);

                        free(catalogo);

                        sprintf(respuesta.cuerpo, "Producto insertado con éxito");
                    }

                    fclose(f_ins);

                    
                    r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                    if (r == -1) {
                        perror("Error en la comunicación");
                        servidor_activo = false;
                    }
                    break;

                case OP_BUSCAR:
                    /* code */
                    break;

                case OP_APAGAR:
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