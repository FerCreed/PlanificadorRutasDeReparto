#ifndef MERGESORT_H
#define MERGESORT_H

#include "order.h"

void ordenar_pedidos_por_mezcla(Pedido *arreglo, size_t n, ComparadorPedido comparar);
void ordenar_pedidos_por_insercion(Pedido *arreglo, size_t n, ComparadorPedido comparar);
void ordenar_pedidos_por_burbuja(Pedido *arreglo, size_t n, ComparadorPedido comparar);

#endif
