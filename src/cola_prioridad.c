#include "cola_prioridad.h"

#include <stdio.h>
#include <stdlib.h>

void cola_iniciar(ColaPrioridad *cola) {
    cola->datos = NULL;
    cola->n = 0;
    cola->capacidad = 0;
}

void cola_liberar(ColaPrioridad *cola) {
    free(cola->datos);
    cola->datos = NULL;
    cola->n = 0;
    cola->capacidad = 0;
}

static void intercambiar(ElementoCola *a, ElementoCola *b) {
    ElementoCola t = *a;
    *a = *b;
    *b = t;
}

static void subir(ColaPrioridad *cola, int i) {
    while (i > 0) {
        int padre = (i - 1) / 2;
        if (cola->datos[i].clave >= cola->datos[padre].clave) {
            break;
        }
        intercambiar(&cola->datos[i], &cola->datos[padre]);
        i = padre;
    }
}

static void bajar(ColaPrioridad *cola, int i) {
    for (;;) {
        int izq = 2 * i + 1;
        int der = 2 * i + 2;
        int menor = i;
        if (izq < cola->n && cola->datos[izq].clave < cola->datos[menor].clave) {
            menor = izq;
        }
        if (der < cola->n && cola->datos[der].clave < cola->datos[menor].clave) {
            menor = der;
        }
        if (menor == i) {
            break;
        }
        intercambiar(&cola->datos[i], &cola->datos[menor]);
        i = menor;
    }
}

int cola_insertar(ColaPrioridad *cola, int indice_nodo, double clave) {
    ElementoCola *nuevo;
    if (cola->n >= cola->capacidad) {
        int cap = cola->capacidad == 0 ? 16 : cola->capacidad * 2;
        nuevo = realloc(cola->datos, (size_t)cap * sizeof(ElementoCola));
        if (!nuevo) {
            fprintf(stderr, "error: sin memoria en cola de prioridad\n");
            return -1;
        }
        cola->datos = nuevo;
        cola->capacidad = cap;
    }
    cola->datos[cola->n].indice_nodo = indice_nodo;
    cola->datos[cola->n].clave = clave;
    subir(cola, cola->n);
    cola->n++;
    return 0;
}

int cola_extraer_minimo(ColaPrioridad *cola, ElementoCola *salida) {
    if (cola->n <= 0) {
        return -1;
    }
    *salida = cola->datos[0];
    cola->n--;
    if (cola->n > 0) {
        cola->datos[0] = cola->datos[cola->n];
        bajar(cola, 0);
    }
    return 0;
}

int cola_vacia(const ColaPrioridad *cola) {
    return cola->n <= 0;
}
