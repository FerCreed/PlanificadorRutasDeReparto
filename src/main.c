#include "dijkstra.h"
#include "geo.h"
#include "graph.h"
#include "json_io.h"
#include "mergesort.h"
#include "order.h"
#include "prim.h"
#include "secuenciacion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

static double reloj_segundos(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, ahora;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&ahora);
    return (double)ahora.QuadPart / (double)freq.QuadPart;
#else
    return (double)clock() / (double)CLOCKS_PER_SEC;
#endif
}

static void imprimir_uso(const char *argv0) {
    fprintf(stderr,
            "uso:\n"
            "  %s dump-graph <map.json>\n"
            "  %s sort-orders <orders.json|-> [--map map.json] --key prioridad|distancia|urgencia\n"
            "  %s benchmark-sort\n"
            "  %s rutas-cortas <map.json> <orders.json>\n"
            "  %s red-minima <map.json>\n"
            "  %s secuenciar <map.json> <orders.json> [--max N] [--todos]\n",
            argv0, argv0, argv0, argv0, argv0, argv0);
}

static int cargar_grafo(const char *ruta, Grafo *grafo) {
    char *texto = json_leer_archivo_o_stdin(ruta);
    int rc;
    if (!texto) {
        return -1;
    }
    rc = json_parsear_grafo(texto, grafo);
    free(texto);
    return rc;
}

static int cargar_pedidos(const char *ruta, ListaPedidos *lista) {
    char *texto = json_leer_archivo_o_stdin(ruta);
    int rc;
    if (!texto) {
        return -1;
    }
    rc = json_parsear_pedidos(texto, lista);
    free(texto);
    return rc;
}

static int comando_volcar_grafo(const char *ruta) {
    Grafo grafo;
    if (cargar_grafo(ruta, &grafo) != 0) {
        return 1;
    }
    grafo_imprimir(&grafo);
    grafo_liberar(&grafo);
    return 0;
}

static void rellenar_distancias_dijkstra(ListaPedidos *lista, const Grafo *grafo) {
    ResultadoDijkstra r;
    int indice_almacen = grafo_buscar_indice(grafo, grafo->id_nodo_almacen);
    int i;
    if (indice_almacen < 0) {
        return;
    }
    dijkstra_desde(grafo, indice_almacen, &r);
    for (i = 0; i < lista->n_pedidos; i++) {
        int di = grafo_buscar_indice(grafo, lista->pedidos[i].nodo_destino);
        if (di < 0 || !dijkstra_alcanzable(&r, di)) {
            lista->pedidos[i].distancia_al_almacen = DIJKSTRA_INF;
        } else {
            lista->pedidos[i].distancia_al_almacen = r.distancia[di];
        }
    }
}

static void rellenar_distancias_haversine(ListaPedidos *lista, const Grafo *grafo) {
    int i;
    int wi = grafo_buscar_indice(grafo, grafo->id_nodo_almacen);
    const NodoGrafo *almacen;
    if (wi < 0) {
        return;
    }
    almacen = &grafo->nodos[wi];
    for (i = 0; i < lista->n_pedidos; i++) {
        int di = grafo_buscar_indice(grafo, lista->pedidos[i].nodo_destino);
        if (di < 0) {
            continue;
        }
        lista->pedidos[i].distancia_al_almacen =
            geo_haversine_metros(almacen->lat, almacen->lon, grafo->nodos[di].lat, grafo->nodos[di].lon);
    }
}

static int validar_nodos_de_pedidos(const ListaPedidos *lista, const Grafo *grafo) {
    int i;
    for (i = 0; i < lista->n_pedidos; i++) {
        if (grafo_buscar_indice(grafo, lista->pedidos[i].nodo_destino) < 0) {
            fprintf(stderr, "error: el pedido %d apunta al nodo %lld, que no esta en el mapa\n",
                    lista->pedidos[i].id_pedido, (long long)lista->pedidos[i].nodo_destino);
            return -1;
        }
    }
    return 0;
}

static int comando_ordenar_pedidos(int argc, char **argv) {
    const char *ruta_pedidos = NULL;
    const char *ruta_mapa = NULL;
    const char *clave = NULL;
    ListaPedidos lista;
    Grafo grafo;
    int i;
    ComparadorPedido comparar;
    int tiene_mapa = 0;

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
    if (cargar_pedidos(ruta_pedidos, &lista) != 0) {
        return 1;
    }
    if (ruta_mapa) {
        if (cargar_grafo(ruta_mapa, &grafo) != 0) {
            return 1;
        }
        tiene_mapa = 1;
        if (validar_nodos_de_pedidos(&lista, &grafo) != 0) {
            grafo_liberar(&grafo);
            return 1;
        }
        if (strcmp(clave, "distancia") == 0) {
            rellenar_distancias_dijkstra(&lista, &grafo);
        } else {
            rellenar_distancias_haversine(&lista, &grafo);
        }
        grafo_liberar(&grafo);
    }
    (void)tiene_mapa;
    ordenar_pedidos_por_mezcla(lista.pedidos, (size_t)lista.n_pedidos, comparar);
    json_imprimir_pedidos_ordenados(&lista, clave);
    return 0;
}

static void llenar_sinteticos(Pedido *arr, int n, unsigned semilla) {
    int i;
    srand(semilla);
    for (i = 0; i < n; i++) {
        memset(&arr[i], 0, sizeof(Pedido));
        arr[i].id_pedido = i + 1;
        arr[i].nodo_destino = i;
        arr[i].peso_paquete = 1.0;
        arr[i].prioridad = 1 + rand() % 5;
        arr[i].ventana_inicio = -1;
        arr[i].ventana_fin = -1;
        arr[i].distancia_al_almacen = (double)(rand() % 20000);
    }
}

static double medir_ms(void (*fn)(Pedido *, size_t, ComparadorPedido), Pedido *base, Pedido *tmp, int n, int reps) {
    int r;
    double t0, t1;
    t0 = reloj_segundos();
    for (r = 0; r < reps; r++) {
        memcpy(tmp, base, (size_t)n * sizeof(Pedido));
        fn(tmp, (size_t)n, comparar_prioridad);
    }
    t1 = reloj_segundos();
    return 1000.0 * (t1 - t0) / (double)reps;
}

static int comando_benchmark_sort(void) {
    const int tamanos[] = {10, 50, 100, 200, 400};
    const int n_tam = 5;
    int t;
    printf("{\n");
    printf("  \"algorithm\": \"benchmark_sort\",\n");
    printf("  \"exact\": true,\n");
    printf("  \"comparador\": \"prioridad\",\n");
    printf("  \"filas\": [\n");
    for (t = 0; t < n_tam; t++) {
        int n = tamanos[t];
        int reps = n <= 10 ? 80000 : (n <= 50 ? 20000 : (n <= 100 ? 6000 : (n <= 200 ? 1500 : 400)));
        Pedido *base = malloc((size_t)n * sizeof(Pedido));
        Pedido *tmp = malloc((size_t)n * sizeof(Pedido));
        double ms_m, ms_i, ms_b;
        if (!base || !tmp) {
            free(base);
            free(tmp);
            fprintf(stderr, "error: sin memoria\n");
            return 1;
        }
        llenar_sinteticos(base, n, 42u + (unsigned)n);
        ms_m = medir_ms(ordenar_pedidos_por_mezcla, base, tmp, n, reps);
        ms_i = medir_ms(ordenar_pedidos_por_insercion, base, tmp, n, reps);
        ms_b = medir_ms(ordenar_pedidos_por_burbuja, base, tmp, n, reps);
        printf("    { \"n\": %d, \"repeticiones\": %d, \"mergesort_ms\": %.6f, \"insercion_ms\": %.6f, \"burbuja_ms\": %.6f }%s\n",
               n, reps, ms_m, ms_i, ms_b, t + 1 < n_tam ? "," : "");
        free(base);
        free(tmp);
    }
    printf("  ]\n");
    printf("}\n");
    return 0;
}

static void imprimir_ids_camino(const Grafo *grafo, const int *indices, int n) {
    int i;
    printf("[");
    for (i = 0; i < n; i++) {
        printf("%lld%s", (long long)grafo->nodos[indices[i]].id, i + 1 < n ? ", " : "");
    }
    printf("]");
}

static int comando_rutas_cortas(const char *ruta_mapa, const char *ruta_pedidos) {
    Grafo grafo;
    ListaPedidos lista;
    ResultadoDijkstra r;
    int origen;
    int i;
    if (cargar_grafo(ruta_mapa, &grafo) != 0 || cargar_pedidos(ruta_pedidos, &lista) != 0) {
        return 1;
    }
    if (validar_nodos_de_pedidos(&lista, &grafo) != 0) {
        grafo_liberar(&grafo);
        return 1;
    }
    origen = grafo_buscar_indice(&grafo, grafo.id_nodo_almacen);
    dijkstra_desde(&grafo, origen, &r);
    printf("{\n");
    printf("  \"algorithm\": \"dijkstra\",\n");
    printf("  \"exact\": true,\n");
    printf("  \"origen\": %lld,\n", (long long)grafo.id_nodo_almacen);
    printf("  \"rutas\": [\n");
    for (i = 0; i < lista.n_pedidos; i++) {
        int di = grafo_buscar_indice(&grafo, lista.pedidos[i].nodo_destino);
        int camino[GRAFO_MAX_NODOS];
        int n_cam = 0;
        int ok = dijkstra_alcanzable(&r, di);
        if (ok) {
            n_cam = dijkstra_reconstruir(&r, di, camino, GRAFO_MAX_NODOS);
        }
        printf("    {\n");
        printf("      \"id_pedido\": %d,\n", lista.pedidos[i].id_pedido);
        printf("      \"nodo_destino\": %lld,\n", (long long)lista.pedidos[i].nodo_destino);
        printf("      \"alcanzable\": %s,\n", ok ? "true" : "false");
        if (ok) {
            printf("      \"distancia_m\": %.2f,\n", r.distancia[di]);
            printf("      \"nodos\": ");
            imprimir_ids_camino(&grafo, camino, n_cam);
            printf("\n");
        } else {
            printf("      \"distancia_m\": null,\n");
            printf("      \"nodos\": []\n");
        }
        printf("    }%s\n", i + 1 < lista.n_pedidos ? "," : "");
    }
    printf("  ]\n");
    printf("}\n");
    grafo_liberar(&grafo);
    return 0;
}

static int comando_red_minima(const char *ruta_mapa) {
    Grafo grafo;
    ResultadoPrim mst;
    int origen;
    int i;
    if (cargar_grafo(ruta_mapa, &grafo) != 0) {
        return 1;
    }
    origen = grafo_buscar_indice(&grafo, grafo.id_nodo_almacen);
    prim_mst(&grafo, origen, &mst);
    printf("{\n");
    printf("  \"algorithm\": \"prim\",\n");
    printf("  \"exact\": true,\n");
    printf("  \"nota\": \"diseno de red, no ruta de reparto\",\n");
    printf("  \"peso_total_m\": %.2f,\n", mst.peso_total);
    printf("  \"aristas\": [\n");
    for (i = 0; i < mst.n_aristas; i++) {
        printf("    { \"from\": %lld, \"to\": %lld, \"weight\": %.2f }%s\n",
               (long long)grafo.nodos[mst.origen[i]].id,
               (long long)grafo.nodos[mst.destino[i]].id,
               mst.peso[i],
               i + 1 < mst.n_aristas ? "," : "");
    }
    printf("  ]\n");
    printf("}\n");
    grafo_liberar(&grafo);
    return 0;
}

static void concatenar_tramo(const Grafo *grafo, const ResultadoDijkstra *r, int destino, int *acum, int *n_acum,
                             int omitir_primero) {
    int camino[GRAFO_MAX_NODOS];
    int n = dijkstra_reconstruir(r, destino, camino, GRAFO_MAX_NODOS);
    int i;
    int inicio = omitir_primero ? 1 : 0;
    for (i = inicio; i < n && *n_acum < GRAFO_MAX_NODOS * SEQ_MAX_DESTINOS; i++) {
        acum[*n_acum] = camino[i];
        (*n_acum)++;
    }
    (void)grafo;
}

static int comando_secuenciar(int argc, char **argv) {
    const char *ruta_mapa = NULL;
    const char *ruta_pedidos = NULL;
    int max_destinos = 10;
    int usar_todos = 0;
    int i;
    Grafo grafo;
    ListaPedidos lista;
    ResultadoSecuencia seq;
    int acum[GRAFO_MAX_NODOS * SEQ_MAX_DESTINOS];
    int n_acum = 0;
    int indice_almacen;
    int ultimo;

    for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--max") == 0 && i + 1 < argc) {
            max_destinos = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--todos") == 0) {
            usar_todos = 1;
        } else if (!ruta_mapa) {
            ruta_mapa = argv[i];
        } else if (!ruta_pedidos) {
            ruta_pedidos = argv[i];
        } else {
            fprintf(stderr, "error: argumento inesperado %s\n", argv[i]);
            return 1;
        }
    }
    if (!ruta_mapa || !ruta_pedidos) {
        imprimir_uso(argv[0]);
        return 1;
    }
    if (cargar_grafo(ruta_mapa, &grafo) != 0 || cargar_pedidos(ruta_pedidos, &lista) != 0) {
        return 1;
    }
    if (secuenciar_reparto(&grafo, &lista, max_destinos, usar_todos, &seq) != 0) {
        grafo_liberar(&grafo);
        return 1;
    }
    indice_almacen = grafo_buscar_indice(&grafo, grafo.id_nodo_almacen);
    ultimo = indice_almacen;
    for (i = 0; i < seq.n_destinos; i++) {
        ResultadoDijkstra r;
        int dest = seq.indices_nodo[seq.orden[i]];
        dijkstra_desde(&grafo, ultimo, &r);
        concatenar_tramo(&grafo, &r, dest, acum, &n_acum, n_acum > 0);
        ultimo = dest;
    }

    printf("{\n");
    printf("  \"algorithm\": \"backtracking\",\n");
    printf("  \"exact\": %s,\n", seq.exacto ? "true" : "false");
    if (seq.heuristica) {
        printf("  \"heuristic\": \"%s\",\n", seq.heuristica);
    }
    printf("  \"nodos_explorados\": %ld,\n", seq.nodos_explorados);
    printf("  \"distancia_total_m\": %.2f,\n", seq.distancia_total);
    printf("  \"paradas\": [\n");
    for (i = 0; i < seq.n_destinos; i++) {
        int pi = seq.indices_pedido[seq.orden[i]];
        printf("    { \"orden\": %d, \"id_pedido\": %d, \"nodo_destino\": %lld }%s\n",
               i + 1, lista.pedidos[pi].id_pedido, (long long)lista.pedidos[pi].nodo_destino,
               i + 1 < seq.n_destinos ? "," : "");
    }
    printf("  ],\n");
    printf("  \"nodos\": ");
    imprimir_ids_camino(&grafo, acum, n_acum);
    printf(",\n");
    printf("  \"inalcanzables\": [");
    for (i = 0; i < seq.n_inalcanzables; i++) {
        int pi = seq.indices_pedido_inalcanzable[i];
        printf("%d%s", lista.pedidos[pi].id_pedido, i + 1 < seq.n_inalcanzables ? ", " : "");
    }
    printf("]\n");
    printf("}\n");
    grafo_liberar(&grafo);
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
    if (strcmp(argv[1], "benchmark-sort") == 0) {
        return comando_benchmark_sort();
    }
    if (strcmp(argv[1], "rutas-cortas") == 0) {
        if (argc != 4) {
            imprimir_uso(argv[0]);
            return 1;
        }
        return comando_rutas_cortas(argv[2], argv[3]);
    }
    if (strcmp(argv[1], "red-minima") == 0) {
        if (argc != 3) {
            imprimir_uso(argv[0]);
            return 1;
        }
        return comando_red_minima(argv[2]);
    }
    if (strcmp(argv[1], "secuenciar") == 0) {
        return comando_secuenciar(argc, argv);
    }
    imprimir_uso(argv[0]);
    return 1;
}
