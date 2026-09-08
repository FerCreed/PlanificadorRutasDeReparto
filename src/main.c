#include "geo.h"
#include "graph.h"
#include "json_io.h"
#include "mergesort.h"
#include "order.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void imprimir_uso(const char *argv0) {
    fprintf(stderr,
            "uso:\n"
            "  %s dump-graph <map.json>\n"
            "  %s sort-orders <orders.json|-> [--map map.json] --key prioridad|distancia|urgencia\n",
            argv0, argv0);
}

static int comando_volcar_grafo(const char *ruta) {
    char *texto;
    Grafo grafo;
    int codigo;
    texto = json_leer_archivo_o_stdin(ruta);
    if (!texto) {
        return 1;
    }
    codigo = json_parsear_grafo(texto, &grafo);
    free(texto);
    if (codigo != 0) {
        return 1;
    }
    grafo_imprimir(&grafo);
    grafo_liberar(&grafo);
    return 0;
}

static void rellenar_distancias_haversine(ListaPedidos *lista, const Grafo *grafo) {
    int i;
    int indice_almacen = grafo_buscar_indice(grafo, grafo->id_nodo_almacen);
    const NodoGrafo *almacen;
    if (indice_almacen < 0) {
        return;
    }
    almacen = &grafo->nodos[indice_almacen];
    for (i = 0; i < lista->n_pedidos; i++) {
        int indice_destino = grafo_buscar_indice(grafo, lista->pedidos[i].nodo_destino);
        if (indice_destino < 0) {
            continue;
        }
        lista->pedidos[i].distancia_al_almacen = geo_haversine_metros(
            almacen->lat, almacen->lon, grafo->nodos[indice_destino].lat, grafo->nodos[indice_destino].lon);
    }
}

static int validar_nodos_de_pedidos(const ListaPedidos *lista, const Grafo *grafo) {
    int i;
    for (i = 0; i < lista->n_pedidos; i++) {
        if (grafo_buscar_indice(grafo, lista->pedidos[i].nodo_destino) < 0) {
            fprintf(stderr, "error: el pedido %d apunta al nodo %lld, que no esta en el mapa\n",
                    lista->pedidos[i].id_pedido,
                    (long long)lista->pedidos[i].nodo_destino);
            return -1;
        }
    }
    return 0;
}

static int comando_ordenar_pedidos(int argc, char **argv) {
    const char *ruta_pedidos = NULL;
    const char *ruta_mapa = NULL;
    const char *clave = NULL;
    char *texto;
    ListaPedidos lista;
    Grafo grafo;
    int i;
    ComparadorPedido comparar;

    for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--map") == 0 && i + 1 < argc) {
            ruta_mapa = argv[++i];
        } else if (strcmp(argv[i], "--key") == 0 && i + 1 < argc) {
            clave = argv[++i];
        } else if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-") == 0 && !ruta_pedidos) {
                ruta_pedidos = argv[i];
            } else {
                fprintf(stderr, "error: opcion desconocida %s\n", argv[i]);
                return 1;
            }
        } else if (!ruta_pedidos) {
            ruta_pedidos = argv[i];
        } else {
            fprintf(stderr, "error: argumento inesperado %s\n", argv[i]);
            return 1;
        }
    }
    if (!ruta_pedidos || !clave) {
        imprimir_uso(argv[0]);
        return 1;
    }
    if (strcmp(clave, "prioridad") == 0) {
        comparar = comparar_prioridad;
    } else if (strcmp(clave, "distancia") == 0) {
        comparar = comparar_distancia;
    } else if (strcmp(clave, "urgencia") == 0) {
        comparar = comparar_urgencia;
    } else {
        fprintf(stderr, "error: clave desconocida %s\n", clave);
        return 1;
    }

    texto = json_leer_archivo_o_stdin(ruta_pedidos);
    if (!texto) {
        return 1;
    }
    if (json_parsear_pedidos(texto, &lista) != 0) {
        free(texto);
        return 1;
    }
    free(texto);

    if (ruta_mapa) {
        char *texto_mapa = json_leer_archivo_o_stdin(ruta_mapa);
        if (!texto_mapa) {
            return 1;
        }
        if (json_parsear_grafo(texto_mapa, &grafo) != 0) {
            free(texto_mapa);
            return 1;
        }
        free(texto_mapa);
        if (validar_nodos_de_pedidos(&lista, &grafo) != 0) {
            grafo_liberar(&grafo);
            return 1;
        }
        rellenar_distancias_haversine(&lista, &grafo);
        grafo_liberar(&grafo);
    }

    ordenar_pedidos_por_mezcla(lista.pedidos, (size_t)lista.n_pedidos, comparar);
    json_imprimir_pedidos_ordenados(&lista, clave);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        imprimir_uso(argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "dump-graph") == 0) {
        if (argc != 3) {
            imprimir_uso(argv[0]);
            return 1;
        }
        return comando_volcar_grafo(argv[2]);
    }
    if (strcmp(argv[1], "sort-orders") == 0) {
        return comando_ordenar_pedidos(argc, argv);
    }
    imprimir_uso(argv[0]);
    return 1;
}
