#include "secuenciacion.h"

#include "conectividad.h"
#include "dijkstra.h"

#include <string.h>

#define SEQ_INF (DIJKSTRA_INF / 2.0)

static double matriz[SEQ_MAX_DESTINOS + 1][SEQ_MAX_DESTINOS + 1];
static int k_dest;
static int usado[SEQ_MAX_DESTINOS];
static int camino_actual[SEQ_MAX_DESTINOS];
static int mejor_camino[SEQ_MAX_DESTINOS];
static double mejor_costo;
static long nodos_explorados;

static void rellenar_matriz(const Grafo *grafo, int n_puntos, const int *indices_grafo) {
    int i, j;
    ResultadoDijkstra r;
    for (i = 0; i < n_puntos; i++) {
        dijkstra_desde(grafo, indices_grafo[i], &r);
        for (j = 0; j < n_puntos; j++) {
            matriz[i][j] = (i == j) ? 0.0 : r.distancia[indices_grafo[j]];
        }
    }
}

static void backtrack(int profundidad, int ultimo, double costo) {
    int i;
    nodos_explorados++;
    if (costo >= mejor_costo) {
        return;
    }
    if (profundidad == k_dest) {
        mejor_costo = costo;
        memcpy(mejor_camino, camino_actual, (size_t)k_dest * sizeof(int));
        return;
    }
    for (i = 0; i < k_dest; i++) {
        double extra;
        if (usado[i]) {
            continue;
        }
        extra = matriz[ultimo][i + 1];
        if (extra >= SEQ_INF) {
            continue;
        }
        usado[i] = 1;
        camino_actual[profundidad] = i;
        backtrack(profundidad + 1, i + 1, costo + extra);
        usado[i] = 0;
    }
}

static void vecino_mas_cercano(void) {
    int i, tomados, ultimo;
    double costo = 0.0;
    for (i = 0; i < k_dest; i++) {
        usado[i] = 0;
    }
    ultimo = 0;
    for (tomados = 0; tomados < k_dest; tomados++) {
        int mejor = -1;
        double dmin = SEQ_INF;
        for (i = 0; i < k_dest; i++) {
            if (!usado[i] && matriz[ultimo][i + 1] < dmin) {
                dmin = matriz[ultimo][i + 1];
                mejor = i;
            }
        }
        if (mejor < 0) {
            costo = SEQ_INF;
            break;
        }
        usado[mejor] = 1;
        camino_actual[tomados] = mejor;
        costo += dmin;
        ultimo = mejor + 1;
        nodos_explorados++;
    }
    mejor_costo = costo;
    memcpy(mejor_camino, camino_actual, (size_t)k_dest * sizeof(int));
}

static void ordenar_candidatos_prioridad(int *idx_pedido, int n, const ListaPedidos *lista) {
    int i, j;
    for (i = 1; i < n; i++) {
        int actual = idx_pedido[i];
        j = i;
        while (j > 0 && lista->pedidos[actual].prioridad > lista->pedidos[idx_pedido[j - 1]].prioridad) {
            idx_pedido[j] = idx_pedido[j - 1];
            j--;
        }
        idx_pedido[j] = actual;
    }
}

int secuenciar_reparto(const Grafo *grafo, const ListaPedidos *lista, int max_destinos, int usar_todos,
                       ResultadoSecuencia *salida) {
    int i;
    int indice_almacen;
    int marcado_bfs[GRAFO_MAX_NODOS];
    int marcado_dfs[GRAFO_MAX_NODOS];
    int candidatos[PEDIDOS_MAX];
    int n_cand = 0;
    int indices_puntos[SEQ_MAX_DESTINOS + 1];
    int n_puntos;

    memset(salida, 0, sizeof(*salida));
    salida->heuristica = NULL;
    indice_almacen = grafo_buscar_indice(grafo, grafo->id_nodo_almacen);
    if (indice_almacen < 0) {
        return -1;
    }
    bfs_marcar_alcanzables(grafo, indice_almacen, marcado_bfs);
    dfs_marcar_alcanzables(grafo, indice_almacen, marcado_dfs);
    (void)marcado_dfs;

    for (i = 0; i < lista->n_pedidos; i++) {
        int di = grafo_buscar_indice(grafo, lista->pedidos[i].nodo_destino);
        if (di < 0 || !marcado_bfs[di]) {
            salida->indices_pedido_inalcanzable[salida->n_inalcanzables++] = i;
        } else {
            candidatos[n_cand++] = i;
        }
    }
    ordenar_candidatos_prioridad(candidatos, n_cand, lista);
    if (max_destinos < 1) {
        max_destinos = 10;
    }
    if (max_destinos > SEQ_MAX_DESTINOS) {
        max_destinos = SEQ_MAX_DESTINOS;
    }
    k_dest = n_cand;
    if (!usar_todos && k_dest > max_destinos) {
        k_dest = max_destinos;
    }
    if (k_dest > SEQ_MAX_DESTINOS) {
        k_dest = SEQ_MAX_DESTINOS;
    }
    salida->n_destinos = k_dest;
    for (i = 0; i < k_dest; i++) {
        int pi = candidatos[i];
        salida->indices_pedido[i] = pi;
        salida->indices_nodo[i] = grafo_buscar_indice(grafo, lista->pedidos[pi].nodo_destino);
    }
    indices_puntos[0] = indice_almacen;
    for (i = 0; i < k_dest; i++) {
        indices_puntos[i + 1] = salida->indices_nodo[i];
    }
    n_puntos = k_dest + 1;
    rellenar_matriz(grafo, n_puntos, indices_puntos);

    nodos_explorados = 0;
    mejor_costo = SEQ_INF;
    for (i = 0; i < k_dest; i++) {
        usado[i] = 0;
    }
    if (k_dest == 0) {
        salida->distancia_total = 0.0;
        salida->exacto = 1;
        salida->nodos_explorados = 0;
        return 0;
    }
    if (k_dest > 10) {
        vecino_mas_cercano();
        salida->exacto = 0;
        salida->heuristica = "vecino_mas_cercano";
    } else {
        backtrack(0, 0, 0.0);
        salida->exacto = 1;
        salida->heuristica = NULL;
    }
    salida->distancia_total = mejor_costo;
    salida->nodos_explorados = nodos_explorados;
    for (i = 0; i < k_dest; i++) {
        salida->orden[i] = mejor_camino[i];
    }
    return 0;
}
