# Planificador de rutas de reparto — Fase 1

Sistema de apoyo a la asignación de pedidos sobre un mapa de calles. Este repositorio cubre el **primer avance**: representar la zona como grafo, ordenar pedidos con MergeSort en C y visualizar el mapa sobre teselas de OpenStreetMap.

## Qué lleva este avance

| Pieza | Estado | Notas |
| --- | --- | --- |
| Mapa OSM (campus UABC Tijuana, Otay) | Listo | 25–50 nodos; no incluye el Tecnológico |
| Lista de adyacencia en C | Listo | `dump-graph` valida e imprime vecinos |
| Pedidos (8–12) | Listo | Ubicación, peso, prioridad 1–5, ventana opcional |
| MergeSort estable | Listo | Claves: prioridad, distancia, urgencia |
| Distancia al almacén | Parcial | Geodésica (haversine), **no** camino más corto |
| Visor Leaflet | Listo | Nodos, aristas, almacén y pedidos sobre OSM |
| Electron / IPC | Fuera de alcance | El `.exe` se invoca por CLI o `pnpm run sort` |

## Requisitos

- Windows con **gcc** (MinGW / MSYS2)
- **Python 3** (`py` en Windows) para regenerar el mapa
- **Node.js** + **pnpm** para el visor

## Compilar el núcleo en C

Desde la raíz del repositorio:

```bash
gcc -Wall -Wextra -std=c11 -O2 -o planner.exe src/main.c src/graph.c src/order.c src/mergesort.c src/json_io.c src/geo.c -lm
```

AddressSanitizer (`make asan`) requiere `libasan`. En muchos MinGW no está instalado.

## Regenerar el mapa (opcional)

Los JSON de `data/` ya vienen generados. Para volver a extraer calles de OSM:

```bash
py scripts/fetch_osm_map.py
```

Si Overpass no responde, el script escribe un grid sintético con coordenadas en el campus UABC (mismo formato JSON).

## Usar el ejecutable

Validar el grafo en consola:

```bash
./planner.exe dump-graph data/map.json
```

Ordenar pedidos (stdout JSON). `--map` recalcula `distancia_al_almacen` con haversine:

```bash
./planner.exe sort-orders data/orders.json --map data/map.json --key prioridad
./planner.exe sort-orders data/orders.json --map data/map.json --key distancia
./planner.exe sort-orders data/orders.json --map data/map.json --key urgencia
```

También se acepta `-` como archivo de pedidos para leer JSON por stdin.

Claves:

- `prioridad`: mayor primero (5 → 1)
- `distancia`: menor `distancia_al_almacen` primero
- `urgencia`: ventana que cierra antes; sin ventana al final; empate por prioridad

MergeSort es **estable**: dos pedidos con la misma clave conservan su orden relativo.

## Visor web

```bash
cd web
pnpm install
pnpm run sync-data
pnpm dev
```

Abre [http://127.0.0.1:5173/](http://127.0.0.1:5173/).

El navegador **no** ejecuta C. Para reordenar:

```bash
cd web
pnpm run sort -- --key prioridad
```

Eso llama a `planner.exe`, escribe `data/orders_sorted.json` y copia a `web/public/data/`. En la UI pulsa **Recargar datos**.

## Estructura

```
src/          Núcleo C (grafo, pedidos, MergeSort, JSON, haversine)
scripts/      Extracción OSM → data/map.json y pedidos de prueba
data/         map.json, orders.json, orders_sorted.json
web/          Vite + React + TypeScript + Leaflet
docs/         Reporte de avance de esta fase
```
