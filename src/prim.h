#ifndef PRIM_H
#define PRIM_H

#include "graph.h"

typedef struct {
    int n_aristas;
    int origen[GRAFO_MAX_NODOS];
    int destino[GRAFO_MAX_NODOS];
    double peso[GRAFO_MAX_NODOS];
    double peso_total;
} ResultadoPrim;

int prim_mst(const Grafo *grafo, int indice_inicio, ResultadoPrim *salida);

#endif
