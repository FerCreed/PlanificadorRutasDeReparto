#include "dijkstra.h"

#include "cola_prioridad.h"

void dijkstra_desde(const Grafo *grafo, int indice_origen, ResultadoDijkstra *salida) {
    int i;
    ColaPrioridad cola;
    ElementoCola extraido;

    for (i = 0; i < grafo->n_nodos; i++) {
        salida->distancia[i] = DIJKSTRA_INF;
        salida->anterior[i] = -1;
    }
    if (indice_origen < 0 || indice_origen >= grafo->n_nodos) {
        return;
    }
    salida->distancia[indice_origen] = 0.0;
    cola_iniciar(&cola);
    cola_insertar(&cola, indice_origen, 0.0);
    while (!cola_vacia(&cola)) {
        const NodoGrafo *nodo;
        if (cola_extraer_minimo(&cola, &extraido) != 0) {
            break;
        }
        if (extraido.clave > salida->distancia[extraido.indice_nodo]) {
            continue;
        }
        nodo = &grafo->nodos[extraido.indice_nodo];
        for (i = 0; i < nodo->n_ady; i++) {
            int v = nodo->adyacencia[i].indice_destino;
            double nd = salida->distancia[extraido.indice_nodo] + nodo->adyacencia[i].peso;
            if (nd < salida->distancia[v]) {
                salida->distancia[v] = nd;
                salida->anterior[v] = extraido.indice_nodo;
                cola_insertar(&cola, v, nd);
            }
        }
    }
    cola_liberar(&cola);
}

int dijkstra_alcanzable(const ResultadoDijkstra *r, int indice_destino) {
    return indice_destino >= 0 && r->distancia[indice_destino] < DIJKSTRA_INF / 2.0;
}

int dijkstra_reconstruir(const ResultadoDijkstra *r, int indice_destino, int *salida_indices, int max_salida) {
    int pila[GRAFO_MAX_NODOS];
    int n_pila = 0;
    int actual = indice_destino;
    int i;
    if (!dijkstra_alcanzable(r, indice_destino) || max_salida <= 0) {
        return 0;
    }
    while (actual >= 0 && n_pila < GRAFO_MAX_NODOS) {
        pila[n_pila++] = actual;
        if (r->anterior[actual] == actual) {
            break;
        }
        actual = r->anterior[actual];
    }
    if (n_pila > max_salida) {
        n_pila = max_salida;
    }
    for (i = 0; i < n_pila; i++) {
        salida_indices[i] = pila[n_pila - 1 - i];
    }
    return n_pila;
}
