#include "graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void grafo_iniciar(Grafo *grafo) {
    memset(grafo, 0, sizeof(*grafo));
}

void grafo_liberar(Grafo *grafo) {
    int i;
    if (!grafo) {
        return;
    }
    for (i = 0; i < grafo->n_nodos; i++) {
        free(grafo->nodos[i].adyacencia);
        grafo->nodos[i].adyacencia = NULL;
        grafo->nodos[i].n_ady = 0;
        grafo->nodos[i].capacidad_ady = 0;
    }
    grafo->n_nodos = 0;
}

int grafo_buscar_indice(const Grafo *grafo, int64_t id) {
    int i;
    for (i = 0; i < grafo->n_nodos; i++) {
        if (grafo->nodos[i].id == id) {
            return i;
        }
    }
    return -1;
}

int grafo_agregar_nodo(Grafo *grafo, int64_t id, double lat, double lon) {
    NodoGrafo *nodo;
    if (grafo_buscar_indice(grafo, id) >= 0) {
        return 0;
    }
    if (grafo->n_nodos >= GRAFO_MAX_NODOS) {
        fprintf(stderr, "error: demasiados nodos (max %d)\n", GRAFO_MAX_NODOS);
        return -1;
    }
    nodo = &grafo->nodos[grafo->n_nodos++];
    nodo->id = id;
    nodo->lat = lat;
    nodo->lon = lon;
    nodo->n_ady = 0;
    nodo->capacidad_ady = 0;
    nodo->adyacencia = NULL;
    return 0;
}

static int agregar_dirigida(Grafo *grafo, int indice_origen, int64_t destino, int indice_destino, double peso) {
    NodoGrafo *nodo = &grafo->nodos[indice_origen];
    AristaAdyacente *crecido;
    int i;
    for (i = 0; i < nodo->n_ady; i++) {
        if (nodo->adyacencia[i].destino == destino) {
            return 0;
        }
    }
    if (nodo->n_ady >= nodo->capacidad_ady) {
        int capacidad = nodo->capacidad_ady == 0 ? 4 : nodo->capacidad_ady * 2;
        crecido = realloc(nodo->adyacencia, (size_t)capacidad * sizeof(AristaAdyacente));
        if (!crecido) {
            fprintf(stderr, "error: sin memoria\n");
            return -1;
        }
        nodo->adyacencia = crecido;
        nodo->capacidad_ady = capacidad;
    }
    nodo->adyacencia[nodo->n_ady].destino = destino;
    nodo->adyacencia[nodo->n_ady].indice_destino = indice_destino;
    nodo->adyacencia[nodo->n_ady].peso = peso;
    nodo->n_ady++;
    return 0;
}

int grafo_agregar_arista_no_dirigida(Grafo *grafo, int64_t origen, int64_t destino, double peso) {
    int indice_origen = grafo_buscar_indice(grafo, origen);
    int indice_destino = grafo_buscar_indice(grafo, destino);
    if (indice_origen < 0 || indice_destino < 0) {
        fprintf(stderr, "error: arista hacia nodo desconocido %lld -> %lld\n",
                (long long)origen, (long long)destino);
        return -1;
    }
    if (agregar_dirigida(grafo, indice_origen, destino, indice_destino, peso) != 0) {
        return -1;
    }
    if (agregar_dirigida(grafo, indice_destino, origen, indice_origen, peso) != 0) {
        return -1;
    }
    grafo->n_aristas_no_dirigidas++;
    return 0;
}

void grafo_imprimir(const Grafo *grafo) {
    int i, j;
    int indice_almacen = grafo_buscar_indice(grafo, grafo->id_nodo_almacen);
    printf("id_nodo_almacen=%lld indice=%d\n", (long long)grafo->id_nodo_almacen, indice_almacen);
    printf("n_nodos=%d n_aristas_no_dirigidas=%d\n", grafo->n_nodos, grafo->n_aristas_no_dirigidas);
    for (i = 0; i < grafo->n_nodos; i++) {
        const NodoGrafo *nodo = &grafo->nodos[i];
        printf("nodo id=%lld lat=%.6f lon=%.6f vecinos=%d%s\n",
               (long long)nodo->id, nodo->lat, nodo->lon, nodo->n_ady,
               nodo->id == grafo->id_nodo_almacen ? " [almacen]" : "");
        for (j = 0; j < nodo->n_ady; j++) {
            printf("  -> %lld peso=%.2f\n", (long long)nodo->adyacencia[j].destino, nodo->adyacencia[j].peso);
        }
    }
}
