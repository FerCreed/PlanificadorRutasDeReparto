#include "conectividad.h"

static void dfs_rec(const Grafo *grafo, int u, int *marcado) {
    int i;
    marcado[u] = 1;
    for (i = 0; i < grafo->nodos[u].n_ady; i++) {
        int v = grafo->nodos[u].adyacencia[i].indice_destino;
        if (!marcado[v]) {
            dfs_rec(grafo, v, marcado);
        }
    }
}

void bfs_marcar_alcanzables(const Grafo *grafo, int indice_origen, int *marcado) {
    int cola[GRAFO_MAX_NODOS];
    int cabeza = 0;
    int cola_n = 0;
    int i;
    for (i = 0; i < grafo->n_nodos; i++) {
        marcado[i] = 0;
    }
    if (indice_origen < 0) {
        return;
    }
    marcado[indice_origen] = 1;
    cola[cola_n++] = indice_origen;
    while (cabeza < cola_n) {
        int u = cola[cabeza++];
        for (i = 0; i < grafo->nodos[u].n_ady; i++) {
            int v = grafo->nodos[u].adyacencia[i].indice_destino;
            if (!marcado[v]) {
                marcado[v] = 1;
                cola[cola_n++] = v;
            }
        }
    }
}

void dfs_marcar_alcanzables(const Grafo *grafo, int indice_origen, int *marcado) {
    int i;
    for (i = 0; i < grafo->n_nodos; i++) {
        marcado[i] = 0;
    }
    if (indice_origen < 0) {
        return;
    }
    dfs_rec(grafo, indice_origen, marcado);
}
