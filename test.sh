#!/bin/bash

echo "### PRUEBAS DE MENSAJES ###"
echo "M01 - Invocación de cliente sin parámetros" && echo "./gestor_cliente" && ./gestor_cliente
echo "M02 - Listar" && echo "./gestor_cliente listar" && ./gestor_cliente listar
echo "M03 - Listar con parámetros (no necesarios)" && echo "./gestor_cliente listar parametro1 parametro2" && ./gestor_cliente listar parametro1 parametro2
echo "M04 - Buscar sin parámetro" && echo "./gestor_cliente buscar" && ./gestor_cliente buscar
echo "M05 - Buscar" && echo "./gestor_cliente buscar 1234" && ./gestor_cliente buscar 1234
echo "M05 - Insertar sin parámetros" && echo "./gestor_cliente insertar" && ./gestor_cliente insertar
echo "M06 - Insertar con parámetros insuficientes" && echo "./gestor_cliente insertar 1234 \"Nombre Producto\"" && ./gestor_cliente insertar 1234 "Nombre Producto"
echo "M07 - Insertar" && echo "./gestor_cliente insertar 1234 \"Nombre Producto\" 320" && ./gestor_cliente insertar 1234 "Nombre Producto" 320
echo "M08 - Apagar servidor con parámetros (no necesarios)" && echo "./gestor_cliente apagar 1235" && ./gestor_cliente apagar 1235
echo "M09 - Apagar servidor" && echo "./gestor_cliente apagar" && ./gestor_cliente apagar