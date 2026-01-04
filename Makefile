CC = gcc

# Uso de estandar gnu99, que añade soporte de System V,
# entre otras carcterísticas de GNU/Linux
CFLAGS = -std=gnu99

all: gestor_servidor gestor_cliente

gestor_servidor: gestor_servidor.c protocolo.h
	$(CC) $(CFLAGS) gestor_servidor.c -o gestor_servidor

gestor_cliente: gestor_cliente.c protocolo.h
	$(CC) $(CFLAGS) gestor_cliente.c -o gestor_cliente

clean:
	rm -f gestor_servidor gestor_cliente productos.dat

.PHONY: all clean