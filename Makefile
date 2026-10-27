CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -O2
LDFLAGS ?= -lm
SRC := src/main.c src/graph.c src/order.c src/mergesort.c src/json_io.c src/geo.c \
	src/cola_prioridad.c src/dijkstra.c src/prim.c src/conectividad.c src/secuenciacion.c
BIN := planner.exe

.PHONY: all asan clean map sync-data

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

asan:
	$(CC) -Wall -Wextra -std=c11 -g -O1 -fsanitize=address -o planner-asan.exe $(SRC) $(LDFLAGS)

map:
	py scripts/fetch_osm_map.py

sync-data:
	mkdir -p web/public/data
	cp -f data/map.json web/public/data/map.json
	cp -f data/orders.json web/public/data/orders.json
	if [ -f data/orders_sorted.json ]; then cp -f data/orders_sorted.json web/public/data/orders_sorted.json; fi
	if [ -f data/benchmark_sort.json ]; then cp -f data/benchmark_sort.json web/public/data/benchmark_sort.json; fi
	if [ -f data/rutas_dijkstra.json ]; then cp -f data/rutas_dijkstra.json web/public/data/rutas_dijkstra.json; fi
	if [ -f data/mst_prim.json ]; then cp -f data/mst_prim.json web/public/data/mst_prim.json; fi
	if [ -f data/secuencia.json ]; then cp -f data/secuencia.json web/public/data/secuencia.json; fi

clean:
	rm -f $(BIN) planner-asan.exe
