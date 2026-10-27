#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

#define DIJKSTRA_INF 1e100

typedef struct {
    double distancia[GRAFO_MAX_NODOS];
    int anterior[GRAFO_MAX_NODOS];
} ResultadoDijkstra;

void dijkstra_desde(const Grafo *grafo, int indice_origen, ResultadoDijkstra *salida);
int dijkstra_alcanzable(const ResultadoDijkstra *r, int indice_destino);
int dijkstra_reconstruir(const ResultadoDijkstra *r, int indice_destino, int *salida_indices, int max_salida);

#endif
