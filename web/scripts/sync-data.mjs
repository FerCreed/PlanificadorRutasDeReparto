import { copyFileSync, existsSync, mkdirSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const directorioWeb = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const raiz = resolve(directorioWeb, "..");
const destino = resolve(directorioWeb, "public", "data");

mkdirSync(destino, { recursive: true });
const archivos = [
  "map.json",
  "orders.json",
  "orders_sorted.json",
  "benchmark_sort.json",
  "rutas_dijkstra.json",
  "mst_prim.json",
  "secuencia.json",
];
for (const nombre of archivos) {
  const origen = resolve(raiz, "data", nombre);
  if (existsSync(origen)) {
    copyFileSync(origen, resolve(destino, nombre));
  }
}
console.log("Synced data/ → web/public/data/");
