import { spawnSync } from "node:child_process";
import { writeFileSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";

const directorioWeb = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const raiz = resolve(directorioWeb, "..");
const ejecutable = resolve(raiz, "planner.exe");

const modo = process.argv[2];
const extra = process.argv.slice(3);

const comandos = {
  sort: {
    args: ["sort-orders", resolve(raiz, "data", "orders.json"), "--map", resolve(raiz, "data", "map.json"), "--key", extra[0] ?? "prioridad"],
    out: "orders_sorted.json",
  },
  bench: {
    args: ["benchmark-sort"],
    out: "benchmark_sort.json",
  },
  rutas: {
    args: ["rutas-cortas", resolve(raiz, "data", "map.json"), resolve(raiz, "data", "orders.json")],
    out: "rutas_dijkstra.json",
  },
  prim: {
    args: ["red-minima", resolve(raiz, "data", "map.json")],
    out: "mst_prim.json",
  },
  secuencia: {
    args: ["secuenciar", resolve(raiz, "data", "map.json"), resolve(raiz, "data", "orders.json"), ...(extra.length ? extra : ["--max", "8"])],
    out: "secuencia.json",
  },
};

const spec = comandos[modo];
if (!spec) {
  console.error("uso: node scripts/run-c.mjs sort|bench|rutas|prim|secuencia [args]");
  process.exit(1);
}

const resultado = spawnSync(ejecutable, spec.args, { encoding: "utf8" });
if (resultado.status !== 0) {
  process.stderr.write(resultado.stderr || "planner.exe falló\n");
  process.exit(resultado.status ?? 1);
}
writeFileSync(resolve(raiz, "data", spec.out), resultado.stdout, "utf8");
console.log(`Wrote data/${spec.out}`);
await import(pathToFileURL(resolve(directorioWeb, "scripts", "sync-data.mjs")).href);
