#ifndef GRAPH_H
#define GRAPH_H

#include <stddef.h>
#include <stdint.h>

#define GRAFO_MAX_NODOS 64

typedef struct {
    int64_t destino;
    double peso;
    int indice_destino;
} AristaAdyacente;

typedef struct {
    int64_t id;
    double lat;
    double lon;
    int n_ady;
    int capacidad_ady;
    AristaAdyacente *adyacencia;
} NodoGrafo;

typedef struct {
    int64_t id_nodo_almacen;
    int n_nodos;
    int n_aristas_no_dirigidas;
    NodoGrafo nodos[GRAFO_MAX_NODOS];
} Grafo;

void grafo_iniciar(Grafo *grafo);
void grafo_liberar(Grafo *grafo);
int grafo_buscar_indice(const Grafo *grafo, int64_t id);
int grafo_agregar_nodo(Grafo *grafo, int64_t id, double lat, double lon);
int grafo_agregar_arista_no_dirigida(Grafo *grafo, int64_t origen, int64_t destino, double peso);
void grafo_imprimir(const Grafo *grafo);

#endif
