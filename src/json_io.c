#include "json_io.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *s;
    const char *end;
    const char *p;
    char err[256];
} AnalizadorJson;

static void analizador_iniciar(AnalizadorJson *ps, const char *text) {
    ps->s = text;
    ps->p = text;
    ps->end = text + strlen(text);
    ps->err[0] = '\0';
}

static void saltar_espacios(AnalizadorJson *ps) {
    while (ps->p < ps->end && isspace((unsigned char)*ps->p)) {
        ps->p++;
    }
}

static int fallar(AnalizadorJson *ps, const char *msg) {
    snprintf(ps->err, sizeof(ps->err), "%s", msg);
    return -1;
}

static int mirar(AnalizadorJson *ps) {
    saltar_espacios(ps);
    if (ps->p >= ps->end) {
        return 0;
    }
    return (unsigned char)*ps->p;
}

static int consumir(AnalizadorJson *ps, char c) {
    saltar_espacios(ps);
    if (ps->p >= ps->end || *ps->p != c) {
        return fallar(ps, "unexpected character");
    }
    ps->p++;
    return 0;
}

static int parsear_cadena(AnalizadorJson *ps, char *out, size_t cap) {
    size_t n = 0;
    saltar_espacios(ps);
    if (ps->p >= ps->end || *ps->p != '"') {
        return fallar(ps, "expected string");
    }
    ps->p++;
    while (ps->p < ps->end && *ps->p != '"') {
        char c = *ps->p++;
        if (c == '\\' && ps->p < ps->end) {
            c = *ps->p++;
            if (c == 'n') {
                c = '\n';
            } else if (c == 't') {
                c = '\t';
            }
        }
        if (n + 1 >= cap) {
            return fallar(ps, "string too long");
        }
        out[n++] = c;
    }
    if (ps->p >= ps->end) {
        return fallar(ps, "unterminated string");
    }
    ps->p++;
    out[n] = '\0';
    return 0;
}

static int parsear_numero(AnalizadorJson *ps, double *out) {
    char *endp = NULL;
    saltar_espacios(ps);
    errno = 0;
    *out = strtod(ps->p, &endp);
    if (endp == ps->p || errno == ERANGE) {
        return fallar(ps, "invalid number");
    }
    ps->p = endp;
    return 0;
}

static int parsear_i64(AnalizadorJson *ps, int64_t *out) {
    char *endp = NULL;
    saltar_espacios(ps);
    errno = 0;
    *out = (int64_t)strtoll(ps->p, &endp, 10);
    if (endp == ps->p || errno == ERANGE) {
        return fallar(ps, "invalid integer");
    }
    ps->p = endp;
    return 0;
}

static int parsear_entero(AnalizadorJson *ps, int *out) {
    int64_t v;
    if (parsear_i64(ps, &v) != 0) {
        return -1;
    }
    *out = (int)v;
    return 0;
}

static int parsear_booleano(AnalizadorJson *ps, int *out) {
    saltar_espacios(ps);
    if (ps->p + 4 <= ps->end && strncmp(ps->p, "true", 4) == 0) {
        ps->p += 4;
        *out = 1;
        return 0;
    }
    if (ps->p + 5 <= ps->end && strncmp(ps->p, "false", 5) == 0) {
        ps->p += 5;
        *out = 0;
        return 0;
    }
    return fallar(ps, "expected boolean");
}

static int saltar_valor(AnalizadorJson *ps);

static int saltar_objeto_o_arreglo(AnalizadorJson *ps, char open_c, char close_c) {
    int depth = 1;
    if (consumir(ps, open_c) != 0) {
        return -1;
    }
    while (ps->p < ps->end && depth > 0) {
        char c = *ps->p;
        if (c == '"') {
            char tmp[256];
            if (parsear_cadena(ps, tmp, sizeof(tmp)) != 0) {
                return -1;
            }
            continue;
        }
        if (c == '{' || c == '[') {
            depth++;
        } else if (c == '}' || c == ']') {
            depth--;
        }
        ps->p++;
    }
    if (depth != 0) {
        return fallar(ps, "unbalanced brackets");
    }
    (void)close_c;
    return 0;
}

static int saltar_valor(AnalizadorJson *ps) {
    int c = mirar(ps);
    char tmp[256];
    double d;
    int b;
    if (c == '"') {
        return parsear_cadena(ps, tmp, sizeof(tmp));
    }
    if (c == '{') {
        return saltar_objeto_o_arreglo(ps, '{', '}');
    }
    if (c == '[') {
        return saltar_objeto_o_arreglo(ps, '[', ']');
    }
    if (c == 't' || c == 'f') {
        return parsear_booleano(ps, &b);
    }
    if (c == 'n') {
        saltar_espacios(ps);
        if (ps->p + 4 <= ps->end && strncmp(ps->p, "null", 4) == 0) {
            ps->p += 4;
            return 0;
        }
        return fallar(ps, "expected null");
    }
    return parsear_numero(ps, &d);
}

static int parsear_objeto_nodo(AnalizadorJson *ps, Grafo *grafo) {
    int64_t id = 0;
    double lat = 0, lon = 0;
    int have_id = 0, have_lat = 0, have_lon = 0;
    if (consumir(ps, '{') != 0) {
        return -1;
    }
    while (mirar(ps) != '}') {
        char key[64];
        if (parsear_cadena(ps, key, sizeof(key)) != 0 || consumir(ps, ':') != 0) {
            return -1;
        }
        if (strcmp(key, "id") == 0) {
            if (parsear_i64(ps, &id) != 0) {
                return -1;
            }
            have_id = 1;
        } else if (strcmp(key, "lat") == 0) {
            if (parsear_numero(ps, &lat) != 0) {
                return -1;
            }
            have_lat = 1;
        } else if (strcmp(key, "lon") == 0) {
            if (parsear_numero(ps, &lon) != 0) {
                return -1;
            }
            have_lon = 1;
        } else if (saltar_valor(ps) != 0) {
            return -1;
        }
        saltar_espacios(ps);
        if (mirar(ps) == ',') {
            ps->p++;
        }
    }
    if (consumir(ps, '}') != 0) {
        return -1;
    }
    if (!have_id || !have_lat || !have_lon) {
        return fallar(ps, "node missing id/lat/lon");
    }
    return grafo_agregar_nodo(grafo, id, lat, lon);
}

static int parsear_objeto_arista(AnalizadorJson *ps, Grafo *grafo) {
    int64_t from = 0, to = 0;
    double weight = 0;
    int have_from = 0, have_to = 0, have_w = 0;
    if (consumir(ps, '{') != 0) {
        return -1;
    }
    while (mirar(ps) != '}') {
        char key[64];
        if (parsear_cadena(ps, key, sizeof(key)) != 0 || consumir(ps, ':') != 0) {
            return -1;
        }
        if (strcmp(key, "from") == 0) {
            if (parsear_i64(ps, &from) != 0) {
                return -1;
            }
            have_from = 1;
        } else if (strcmp(key, "to") == 0) {
            if (parsear_i64(ps, &to) != 0) {
                return -1;
            }
            have_to = 1;
        } else if (strcmp(key, "weight") == 0) {
            if (parsear_numero(ps, &weight) != 0) {
                return -1;
            }
            have_w = 1;
        } else if (saltar_valor(ps) != 0) {
            return -1;
        }
        saltar_espacios(ps);
        if (mirar(ps) == ',') {
            ps->p++;
        }
    }
    if (consumir(ps, '}') != 0) {
        return -1;
    }
    if (!have_from || !have_to || !have_w) {
        return fallar(ps, "edge missing from/to/weight");
    }
    return grafo_agregar_arista_no_dirigida(grafo, from, to, weight);
}

static int parsear_arreglo_nodos(AnalizadorJson *ps, Grafo *grafo) {
    if (consumir(ps, '[') != 0) {
        return -1;
    }
    while (mirar(ps) != ']') {
        if (parsear_objeto_nodo(ps, grafo) != 0) {
            return -1;
        }
        saltar_espacios(ps);
        if (mirar(ps) == ',') {
            ps->p++;
        }
    }
    return consumir(ps, ']');
}

static int parsear_arreglo_aristas(AnalizadorJson *ps, Grafo *grafo) {
    if (consumir(ps, '[') != 0) {
        return -1;
    }
    while (mirar(ps) != ']') {
        if (parsear_objeto_arista(ps, grafo) != 0) {
            return -1;
        }
        saltar_espacios(ps);
        if (mirar(ps) == ',') {
            ps->p++;
        }
    }
    return consumir(ps, ']');
}

int json_parsear_grafo(const char *text, Grafo *grafo) {
    AnalizadorJson ps;
    int have_wh = 0;
    analizador_iniciar(&ps, text);
    grafo_iniciar(grafo);
    if (consumir(&ps, '{') != 0) {
        fprintf(stderr, "error: %s\n", ps.err);
        return -1;
    }
    while (mirar(&ps) != '}') {
        char key[64];
        if (parsear_cadena(&ps, key, sizeof(key)) != 0 || consumir(&ps, ':') != 0) {
            fprintf(stderr, "error: %s\n", ps.err);
            grafo_liberar(grafo);
            return -1;
        }
        if (strcmp(key, "warehouse_node_id") == 0) {
            if (parsear_i64(&ps, &grafo->id_nodo_almacen) != 0) {
                fprintf(stderr, "error: %s\n", ps.err);
                grafo_liberar(grafo);
                return -1;
            }
            have_wh = 1;
        } else if (strcmp(key, "nodes") == 0) {
            if (parsear_arreglo_nodos(&ps, grafo) != 0) {
                fprintf(stderr, "error: %s\n", ps.err);
                grafo_liberar(grafo);
                return -1;
            }
        } else if (strcmp(key, "edges") == 0) {
            if (parsear_arreglo_aristas(&ps, grafo) != 0) {
                fprintf(stderr, "error: %s\n", ps.err);
                grafo_liberar(grafo);
                return -1;
            }
        } else if (saltar_valor(&ps) != 0) {
            fprintf(stderr, "error: %s\n", ps.err);
            grafo_liberar(grafo);
            return -1;
        }
        saltar_espacios(&ps);
        if (mirar(&ps) == ',') {
            ps.p++;
        }
    }
    if (consumir(&ps, '}') != 0) {
        fprintf(stderr, "error: %s\n", ps.err);
        grafo_liberar(grafo);
        return -1;
    }
    if (!have_wh) {
        fprintf(stderr, "error: missing warehouse_node_id\n");
        grafo_liberar(grafo);
        return -1;
    }
    if (grafo_buscar_indice(grafo, grafo->id_nodo_almacen) < 0) {
        fprintf(stderr, "error: warehouse_node_id %lld not in nodes\n",
                (long long)grafo->id_nodo_almacen);
        grafo_liberar(grafo);
        return -1;
    }
    return 0;
}

static int parsear_objeto_pedido(AnalizadorJson *ps, Pedido *pedido) {
    int have_id = 0, have_dest = 0, have_peso = 0, have_pri = 0;
    memset(pedido, 0, sizeof(*pedido));
    pedido->ventana_inicio = -1;
    pedido->ventana_fin = -1;
    pedido->distancia_al_almacen = 0.0;
    if (consumir(ps, '{') != 0) {
        return -1;
    }
    while (mirar(ps) != '}') {
        char key[64];
        if (parsear_cadena(ps, key, sizeof(key)) != 0 || consumir(ps, ':') != 0) {
            return -1;
        }
        if (strcmp(key, "id_pedido") == 0) {
            if (parsear_entero(ps, &pedido->id_pedido) != 0) {
                return -1;
            }
            have_id = 1;
        } else if (strcmp(key, "nodo_destino") == 0) {
            if (parsear_i64(ps, &pedido->nodo_destino) != 0) {
                return -1;
            }
            have_dest = 1;
        } else if (strcmp(key, "peso_paquete") == 0) {
            if (parsear_numero(ps, &pedido->peso_paquete) != 0) {
                return -1;
            }
            have_peso = 1;
        } else if (strcmp(key, "prioridad") == 0) {
            if (parsear_entero(ps, &pedido->prioridad) != 0) {
                return -1;
            }
            have_pri = 1;
        } else if (strcmp(key, "ventana_inicio") == 0) {
            if (parsear_entero(ps, &pedido->ventana_inicio) != 0) {
                return -1;
            }
        } else if (strcmp(key, "ventana_fin") == 0) {
            if (parsear_entero(ps, &pedido->ventana_fin) != 0) {
                return -1;
            }
        } else if (strcmp(key, "distancia_al_almacen") == 0) {
            if (parsear_numero(ps, &pedido->distancia_al_almacen) != 0) {
                return -1;
            }
        } else if (saltar_valor(ps) != 0) {
            return -1;
        }
        saltar_espacios(ps);
        if (mirar(ps) == ',') {
            ps->p++;
        }
    }
    if (consumir(ps, '}') != 0) {
        return -1;
    }
    if (!have_id || !have_dest || !have_peso || !have_pri) {
        return fallar(ps, "order missing required fields");
    }
    if (pedido->prioridad < 1 || pedido->prioridad > 5) {
        return fallar(ps, "prioridad must be 1..5");
    }
    return 0;
}

int json_parsear_pedidos(const char *text, ListaPedidos *lista) {
    AnalizadorJson ps;
    memset(lista, 0, sizeof(*lista));
    analizador_iniciar(&ps, text);
    if (consumir(&ps, '{') != 0) {
        fprintf(stderr, "error: %s\n", ps.err);
        return -1;
    }
    while (mirar(&ps) != '}') {
        char key[64];
        if (parsear_cadena(&ps, key, sizeof(key)) != 0 || consumir(&ps, ':') != 0) {
            fprintf(stderr, "error: %s\n", ps.err);
            return -1;
        }
        if (strcmp(key, "orders") == 0) {
            if (consumir(&ps, '[') != 0) {
                fprintf(stderr, "error: %s\n", ps.err);
                return -1;
            }
            while (mirar(&ps) != ']') {
                if (lista->n_pedidos >= PEDIDOS_MAX) {
                    fprintf(stderr, "error: too many orders (max %d)\n", PEDIDOS_MAX);
                    return -1;
                }
                if (parsear_objeto_pedido(&ps, &lista->pedidos[lista->n_pedidos]) != 0) {
                    fprintf(stderr, "error: %s\n", ps.err);
                    return -1;
                }
                lista->n_pedidos++;
                saltar_espacios(&ps);
                if (mirar(&ps) == ',') {
                    ps.p++;
                }
            }
            if (consumir(&ps, ']') != 0) {
                fprintf(stderr, "error: %s\n", ps.err);
                return -1;
            }
        } else if (saltar_valor(&ps) != 0) {
            fprintf(stderr, "error: %s\n", ps.err);
            return -1;
        }
        saltar_espacios(&ps);
        if (mirar(&ps) == ',') {
            ps.p++;
        }
    }
    if (consumir(&ps, '}') != 0) {
        fprintf(stderr, "error: %s\n", ps.err);
        return -1;
    }
    if (lista->n_pedidos == 0) {
        fprintf(stderr, "error: no orders in JSON\n");
        return -1;
    }
    return 0;
}

static void imprimir_ventana(int minutes) {
    if (minutes < 0) {
        printf("null");
    } else {
        printf("%d", minutes);
    }
}

void json_imprimir_pedidos_ordenados(const ListaPedidos *lista, const char *key) {
    int i;
    printf("{\n");
    printf("  \"key\": \"%s\",\n", key);
    printf("  \"algorithm\": \"mergesort\",\n");
    printf("  \"exact\": true,\n");
    printf("  \"orders\": [\n");
    for (i = 0; i < lista->n_pedidos; i++) {
        const Pedido *pedido = &lista->pedidos[i];
        printf("    {\n");
        printf("      \"id_pedido\": %d,\n", pedido->id_pedido);
        printf("      \"nodo_destino\": %lld,\n", (long long)pedido->nodo_destino);
        printf("      \"peso_paquete\": %.2f,\n", pedido->peso_paquete);
        printf("      \"prioridad\": %d,\n", pedido->prioridad);
        printf("      \"ventana_inicio\": ");
        imprimir_ventana(pedido->ventana_inicio);
        printf(",\n");
        printf("      \"ventana_fin\": ");
        imprimir_ventana(pedido->ventana_fin);
        printf(",\n");
        printf("      \"distancia_al_almacen\": %.2f\n", pedido->distancia_al_almacen);
        printf("    }%s\n", i + 1 < lista->n_pedidos ? "," : "");
    }
    printf("  ]\n");
    printf("}\n");
}

char *json_leer_archivo_o_stdin(const char *path) {
    FILE *fp;
    char *buf;
    size_t cap = 4096;
    size_t n = 0;
    int use_stdin = (path && strcmp(path, "-") == 0);
    if (use_stdin) {
        fp = stdin;
    } else {
        fp = fopen(path, "rb");
        if (!fp) {
            fprintf(stderr, "error: cannot open %s\n", path);
            return NULL;
        }
    }
    buf = malloc(cap);
    if (!buf) {
        if (!use_stdin) {
            fclose(fp);
        }
        return NULL;
    }
    for (;;) {
        size_t got;
        if (n + 1024 >= cap) {
            char *nb;
            cap *= 2;
            nb = realloc(buf, cap);
            if (!nb) {
                free(buf);
                if (!use_stdin) {
                    fclose(fp);
                }
                return NULL;
            }
            buf = nb;
        }
        got = fread(buf + n, 1, cap - n - 1, fp);
        n += got;
        if (got == 0) {
            break;
        }
    }
    buf[n] = '\0';
    if (!use_stdin) {
        fclose(fp);
    }
    return buf;
}
