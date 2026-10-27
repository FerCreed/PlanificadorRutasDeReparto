# Reporte técnico U2–U4

Mapa de prueba: campus UABC Otay, 40 nodos, 49 aristas, 12 pedidos. Núcleo en C sin librerías de grafos ni de ordenamiento.

## U2 · Ordenamiento

**Algoritmos:** MergeSort estable `ordenar_pedidos_por_mezcla`, inserción y burbuja, mismo `ComparadorPedido`.

**Por qué MergeSort:** estable (cascada de criterios) y \(\Theta(n \log n)\) en el peor caso. Inserción y burbuja son \(\Theta(n^2)\) y sirven de contraste empírico.

**Medición:** `planner.exe benchmark-sort`. Reloj de alta resolución; se cronometra **solo** la función de sort (copia + ordenar), no el I/O. Comparador: prioridad.

| n | Reps | MergeSort (ms) | Inserción (ms) | Burbuja (ms) |
| ---: | ---: | ---: | ---: | ---: |
| 10 | 80000 | 0.000184 | 0.000052 | 0.000086 |
| 50 | 20000 | 0.001063 | 0.000645 | 0.002061 |
| 100 | 6000 | 0.002363 | 0.003113 | 0.009219 |
| 200 | 1500 | 0.005091 | 0.010226 | 0.034975 |
| 400 | 400 | 0.011686 | 0.044714 | 0.146547 |

**Lectura:** con \(n\) chico la inserción gana por constantes (menos copias, mejor caché). Desde \(n=100\) MergeSort supera a inserción. Burbuja crece ~4× al duplicar \(n\) (200→400: 0.035→0.147), alineado con \(\Theta(n^2)\). MergeSort 200→400 ≈ 2.3×, coherente con \(\Theta(n\log n)\).

`sort-orders --key distancia` usa **Dijkstra** (metros de camino) cuando hay `--map`. Las otras claves no necesitan esa distancia; haversine queda como dato auxiliar si no se ordena por distancia.

Etiqueta: el orden es **exacto**.

## U3 · Dijkstra y Prim

**Cola:** heap binario mínimo propio (`cola_prioridad`), no `qsort` ni contenedores.

**Dijkstra** desde el almacén: distancias y `anterior[]` para reconstruir el camino. Complejidad \(O((V+E)\log V)\) con \(V\le 64\). Resultado **exacto** para pesos no negativos (metros haversine en aristas). Destinos inalcanzables se marcan; en esta instancia los 12 son alcanzables.

Ejemplo: pedido 1, 233.47 m, camino de 6 nodos OSM.

**Prim:** árbol de expansión mínimo sobre **todas** las intersecciones. Peso total en la corrida: 2876.10 m. **No es una ruta de reparto**; es diseño de cobertura de red. Exacto.

## U4 · Conectividad y secuenciación

**BFS** (y DFS de verificación) desde el almacén: conjuntos de alcanzables coinciden. `inalcanzables: []` en la instancia UABC.

**Backtracking con poda** (branch and bound): almacén → permutación de destinos; costo = suma de Dijkstra entre paradas. Si el costo parcial \(\ge\) mejor, se corta. Con `--max 8`: **exacto**, 34967 nodos explorados, gira 2360.33 m.

Si \(k>10\) (`--todos` con 12 pedidos): **vecino más cercano**, `exact: false`, `heuristic: vecino_mas_cercano`.

Fuera de este reporte: mochila 0/1 y varios repartidores (U5).
