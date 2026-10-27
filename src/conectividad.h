#ifndef CONECTIVIDAD_H
#define CONECTIVIDAD_H

#include "graph.h"

void bfs_marcar_alcanzables(const Grafo *grafo, int indice_origen, int *marcado);
void dfs_marcar_alcanzables(const Grafo *grafo, int indice_origen, int *marcado);

#endif
