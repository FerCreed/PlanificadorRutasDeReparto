# Planificador de rutas de reparto (U1–U4)

Mapa OSM del campus **UABC Otay**, lógica en **C**, visor **React + Leaflet**. Aún no entra la mochila 0/1 ni varios repartidores (U5).

Detalle técnico: [docs/reporte-u2-u4.md](docs/reporte-u2-u4.md).

## Módulos

| Unidad | Qué hace | Comando C | ¿Exacto? |
| --- | --- | --- | --- |
| U1 | Grafo en lista de adyacencia | `dump-graph` | — |
| U2 | MergeSort (+ inserción/burbuja, benchmark) | `sort-orders`, `benchmark-sort` | Sí |
| U3 | Dijkstra almacén→destino; Prim (red, no gira) | `rutas-cortas`, `red-minima` | Sí |
| U4 | BFS/DFS; backtracking 6–10 destinos | `secuenciar [--max 8]` | Sí si \(k\le 10\) |
| U4 extra | Vecino más cercano si hay más de 10 | `secuenciar --todos` | No (heurístico) |

## Compilar

```bash
gcc -Wall -Wextra -std=c11 -O2 -o planner.exe src/main.c src/graph.c src/order.c src/mergesort.c src/json_io.c src/geo.c src/cola_prioridad.c src/dijkstra.c src/prim.c src/conectividad.c src/secuenciacion.c -lm
```

O `make` si tienes GNU Make.

## Regenerar mapa (opcional)

```bash
py scripts/fetch_osm_map.py
```

## Ejecutable

```bash
./planner.exe dump-graph data/map.json
./planner.exe sort-orders data/orders.json --map data/map.json --key prioridad
./planner.exe sort-orders data/orders.json --map data/map.json --key distancia
./planner.exe benchmark-sort
./planner.exe rutas-cortas data/map.json data/orders.json
./planner.exe red-minima data/map.json
./planner.exe secuenciar data/map.json data/orders.json --max 8
./planner.exe secuenciar data/map.json data/orders.json --todos
```

`--key distancia` usa **camino Dijkstra**. Prioridad/urgencia no requieren esa distancia.

## Visor

```bash
cd web
pnpm install
pnpm run sync-data
pnpm run bench
pnpm run rutas
pnpm run prim
pnpm run secuencia
pnpm dev
```

Pestañas: Grafo, Rutas Dijkstra, Red Prim, Gira. La tabla de benchmark está abajo. **Recargar datos** vuelve a leer JSON; el navegador no ejecuta C.

```bash
pnpm run sort -- prioridad
```

## Estructura

```
src/       C (grafo, sort, cola, Dijkstra, Prim, BFS/DFS, backtracking)
scripts/   OSM → data/map.json
data/      JSON de entrada y salidas de C
web/       Leaflet
docs/      Reportes de unidad
```
