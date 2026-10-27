#ifndef SECUENCIACION_H
#define SECUENCIACION_H

#include "graph.h"
#include "order.h"

#define SEQ_MAX_DESTINOS 12

typedef struct {
    int n_destinos;
    int indices_pedido[SEQ_MAX_DESTINOS];
    int indices_nodo[SEQ_MAX_DESTINOS];
    int orden[SEQ_MAX_DESTINOS];
    double distancia_total;
    int exacto;
    const char *heuristica;
    long nodos_explorados;
    int n_inalcanzables;
    int indices_pedido_inalcanzable[PEDIDOS_MAX];
} ResultadoSecuencia;

int secuenciar_reparto(const Grafo *grafo, const ListaPedidos *lista, int max_destinos, int usar_todos,
                       ResultadoSecuencia *salida);

#endif
