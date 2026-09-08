#ifndef JSON_IO_H
#define JSON_IO_H

#include "graph.h"
#include "order.h"

char *json_leer_archivo_o_stdin(const char *ruta);
int json_parsear_grafo(const char *texto, Grafo *grafo);
int json_parsear_pedidos(const char *texto, ListaPedidos *lista);
void json_imprimir_pedidos_ordenados(const ListaPedidos *lista, const char *clave);

#endif
