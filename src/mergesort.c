#include "mergesort.h"

#include <stdlib.h>
#include <string.h>

static void fusionar(Pedido *arreglo, Pedido *temporal, size_t izq, size_t medio, size_t der, ComparadorPedido comparar) {
    size_t i = izq;
    size_t j = medio;
    size_t k = izq;
    while (i < medio && j < der) {
        if (comparar(&arreglo[i], &arreglo[j]) <= 0) {
            temporal[k++] = arreglo[i++];
        } else {
            temporal[k++] = arreglo[j++];
        }
    }
    while (i < medio) {
        temporal[k++] = arreglo[i++];
    }
    while (j < der) {
        temporal[k++] = arreglo[j++];
    }
    memcpy(arreglo + izq, temporal + izq, (der - izq) * sizeof(Pedido));
}

static void ordenar_mezcla_rec(Pedido *arreglo, Pedido *temporal, size_t izq, size_t der, ComparadorPedido comparar) {
    size_t medio;
    if (der - izq <= 1) {
        return;
    }
    medio = izq + (der - izq) / 2;
    ordenar_mezcla_rec(arreglo, temporal, izq, medio, comparar);
    ordenar_mezcla_rec(arreglo, temporal, medio, der, comparar);
    fusionar(arreglo, temporal, izq, medio, der, comparar);
}

void ordenar_pedidos_por_mezcla(Pedido *arreglo, size_t n, ComparadorPedido comparar) {
    Pedido *temporal;
    if (n < 2) {
        return;
    }
    temporal = malloc(n * sizeof(Pedido));
    if (!temporal) {
        return;
    }
    ordenar_mezcla_rec(arreglo, temporal, 0, n, comparar);
    free(temporal);
}
