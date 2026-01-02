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
                    strcpy(respuesta.cuerpo, "LISTAR");
                    r = msgsnd(cola_respuestas, &respuesta, tam, 0);
        
                    if (r == -1) {
                        perror("Error en la comunicación");
                        servidor_activo = false;
                    }
                    break;

                case OP_INSERTAR:
                    sprintf(respuesta.cuerpo, "INSERTAR: Cod %s, Nom %s, Cant %s", codigo, nombre, cantidad);
                    r = msgsnd(cola_respuestas, &respuesta, tam, 0);
                    if (r == -1) {
                        perror("Error en la comunicación");
                        servidor_activo = false;
                    }

                    Producto producto;
                    strcpy(producto.codigo, codigo);
                    strcpy(producto.nombre, nombre);
                    producto.cantidad = atoi(cantidad);

                    FILE *f = fopen("productos.dat", "r+b");
                    if (f == NULL) {
                        printf("El archivo productos.dat no existe. Creando uno nuevo...\n");
                        f = fopen("productos.dat", "w+b");
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
                        if (strcmp(temp.codigo, producto.codigo) == 0) {
                            producto_existe = true;
                            break;
                        }
                    }

                    if (producto_existe) {
                        sprintf(respuesta.cuerpo, "El producto con código %s ya existe.", producto.codigo);
                    } else {
                        // Si no existe, lo añadimos
                        fseek(f, 0, SEEK_END); 
                        fwrite(&producto, sizeof(Producto), 1, f);
                        sprintf(respuesta.cuerpo, "Producto insertado correctamente.");
                    }

                    fclose(f);

                    
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