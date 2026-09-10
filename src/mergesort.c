#include "mergesort.h"
#include <stdlib.h>
#include <string.h>

static void fusionar(Pedido **arreglo, Pedido **temporal, size_t izq,
                     size_t medio, size_t der, ComparadorPedido comparar) {

    size_t i = izq;       // c_1: O(1)
    size_t j = medio;     // c_2: O(1)
    size_t k = izq;       // c_3: O(1)

    while (i < medio && j < der) {   // c_4: O(n)
        if (comparar(&arreglo[i], &arreglo[j]) <= 0) {  // c_5: O(1)
            temporal[k++] = arreglo[i++];                // c_6: O(1)
        } else {
            temporal[k++] = arreglo[j++];                // c_7: O(1)
        }
    }

    while (i < medio) {              // c_8: O(n)
        temporal[k++] = arreglo[i++]; // c_9: O(1)
    }

    while (j < der) {                // c_10: O(n)
        temporal[k++] = arreglo[j++]; // c_11: O(1)
    }

    memcpy(arreglo + izq, temporal + izq,
           (der - izq) * sizeof(Pedido*)); // c_12: O(n)
}


static void ordenar_mezcla_rec(Pedido **arreglo, Pedido **temporal,
                               size_t izq, size_t der,
                               ComparadorPedido comparar) {

    size_t medio;                    // c_13: O(1)

    if (der - izq <= 1) {             // c_14: O(1)
        return;                       // c_15: O(1)
    }

    medio = izq + (der - izq) / 2;   // c_16: O(1)

    ordenar_mezcla_rec(arreglo, temporal, izq, medio, comparar);
                                      // c_17: T(n/2)

    ordenar_mezcla_rec(arreglo, temporal, medio, der, comparar);
                                      // c_18: T(n/2)

    fusionar(arreglo, temporal, izq, medio, der, comparar);
                                      // c_19: O(n)
}


void ordenar_pedidos_por_mezcla(Pedido **arreglo, size_t n,
                                ComparadorPedido comparar) {

    Pedido **temporal;                // c_20: O(1)

    if (n < 2) {                       // c_21: O(1)
        return;                        // c_22: O(1)
    }

    temporal = malloc(n * sizeof(Pedido*)); // c_23: O(1)

    if (!temporal) {                   // c_24: O(1)
        return;                        // c_25: O(1)
    }

    ordenar_mezcla_rec(arreglo, temporal, 0, n, comparar);
                                      // c_26: T(n)

    free(temporal);                    // c_27: O(1)
}

