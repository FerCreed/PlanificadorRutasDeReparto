#include "prim.h"

#include "cola_prioridad.h"
#include "dijkstra.h"

int prim_mst(const Grafo *grafo, int indice_inicio, ResultadoPrim *salida) {
    int i;
    int en_arbol[GRAFO_MAX_NODOS];
    double mejor[GRAFO_MAX_NODOS];
    int padre[GRAFO_MAX_NODOS];
    ColaPrioridad cola;
    ElementoCola extraido;
    int tomados;

    salida->n_aristas = 0;
    salida->peso_total = 0.0;
    if (indice_inicio < 0 || grafo->n_nodos <= 0) {
        return -1;
    }
    for (i = 0; i < grafo->n_nodos; i++) {
        en_arbol[i] = 0;
        mejor[i] = DIJKSTRA_INF;
        padre[i] = -1;
    }
    mejor[indice_inicio] = 0.0;
    cola_iniciar(&cola);
    cola_insertar(&cola, indice_inicio, 0.0);
    tomados = 0;
    while (!cola_vacia(&cola) && tomados < grafo->n_nodos) {
        const NodoGrafo *nodo;
        int u;
        if (cola_extraer_minimo(&cola, &extraido) != 0) {
            break;
        }
        u = extraido.indice_nodo;
        if (en_arbol[u]) {
            continue;
        }
        en_arbol[u] = 1;
        tomados++;
        if (padre[u] >= 0) {
            salida->origen[salida->n_aristas] = padre[u];
            salida->destino[salida->n_aristas] = u;
            salida->peso[salida->n_aristas] = mejor[u];
            salida->peso_total += mejor[u];
            salida->n_aristas++;
        }
        nodo = &grafo->nodos[u];
        for (i = 0; i < nodo->n_ady; i++) {
            int v = nodo->adyacencia[i].indice_destino;
            double w = nodo->adyacencia[i].peso;
            if (!en_arbol[v] && w < mejor[v]) {
                mejor[v] = w;
                padre[v] = u;
                cola_insertar(&cola, v, w);
            }
        }
    }
    cola_liberar(&cola);
    return 0;
}
