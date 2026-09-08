#ifndef ORDER_H
#define ORDER_H

#include <stddef.h>
#include <stdint.h>

#define PEDIDOS_MAX 32

typedef struct {
    int id_pedido;
    int64_t nodo_destino;
    double peso_paquete;
    int prioridad;
    int ventana_inicio;
    int ventana_fin;
    double distancia_al_almacen;
} Pedido;

typedef struct {
    int n_pedidos;
    Pedido pedidos[PEDIDOS_MAX];
} ListaPedidos;

typedef int (*ComparadorPedido)(const Pedido *a, const Pedido *b);

int comparar_prioridad(const Pedido *a, const Pedido *b);
int comparar_distancia(const Pedido *a, const Pedido *b);
int comparar_urgencia(const Pedido *a, const Pedido *b);

#endif
