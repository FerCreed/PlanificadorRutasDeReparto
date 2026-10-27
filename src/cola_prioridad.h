#ifndef COLA_PRIORIDAD_H
#define COLA_PRIORIDAD_H

typedef struct {
    int indice_nodo;
    double clave;
} ElementoCola;

typedef struct {
    ElementoCola *datos;
    int n;
    int capacidad;
} ColaPrioridad;

void cola_iniciar(ColaPrioridad *cola);
void cola_liberar(ColaPrioridad *cola);
int cola_insertar(ColaPrioridad *cola, int indice_nodo, double clave);
int cola_extraer_minimo(ColaPrioridad *cola, ElementoCola *salida);
int cola_vacia(const ColaPrioridad *cola);

#endif
