CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -O2
LDFLAGS ?= -lm
SRC := src/main.c src/graph.c src/order.c src/mergesort.c src/json_io.c src/geo.c
BIN := planner.exe

.PHONY: all asan clean map sync-data

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

# Requires libasan (often missing on MinGW). On MSYS2: pacman -S mingw-w64-x86_64-gcc
asan:
	$(CC) -Wall -Wextra -std=c11 -g -O1 -fsanitize=address -o planner-asan.exe $(SRC) $(LDFLAGS)

map:
	py scripts/fetch_osm_map.py

sync-data:
	mkdir -p web/public/data
	cp -f data/map.json web/public/data/map.json
	cp -f data/orders.json web/public/data/orders.json
	if [ -f data/orders_sorted.json ]; then cp -f data/orders_sorted.json web/public/data/orders_sorted.json; fi

clean:
	rm -f $(BIN) planner-asan.exe
