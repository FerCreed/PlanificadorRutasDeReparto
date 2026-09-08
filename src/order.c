#include "order.h"

int comparar_prioridad(const Pedido *a, const Pedido *b) {
    if (a->prioridad > b->prioridad) {
        return -1;
    }
    if (a->prioridad < b->prioridad) {
        return 1;
    }
    return 0;
}

int comparar_distancia(const Pedido *a, const Pedido *b) {
    if (a->distancia_al_almacen < b->distancia_al_almacen) {
        return -1;
    }
    if (a->distancia_al_almacen > b->distancia_al_almacen) {
        return 1;
    }
    return 0;
}

int comparar_urgencia(const Pedido *a, const Pedido *b) {
    int cierre_a = a->ventana_fin < 0 ? 24 * 60 + 1 : a->ventana_fin;
    int cierre_b = b->ventana_fin < 0 ? 24 * 60 + 1 : b->ventana_fin;
    if (cierre_a < cierre_b) {
        return -1;
    }
    if (cierre_a > cierre_b) {
        return 1;
    }
    return comparar_prioridad(a, b);
}
