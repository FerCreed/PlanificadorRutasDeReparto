import { spawnSync } from "node:child_process";
import { writeFileSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";

const directorioWeb = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const raiz = resolve(directorioWeb, "..");

function valorArgumento(bandera) {
  const indice = process.argv.indexOf(bandera);
  if (indice >= 0 && indice + 1 < process.argv.length) {
    return process.argv[indice + 1];
  }
  return null;
}

const clave = valorArgumento("--key") ?? "prioridad";
if (!["prioridad", "distancia", "urgencia"].includes(clave)) {
  console.error("uso: pnpm run sort -- --key prioridad|distancia|urgencia");
  process.exit(1);
}

const ejecutable = resolve(raiz, "planner.exe");
const resultado = spawnSync(
  ejecutable,
  ["sort-orders", resolve(raiz, "data", "orders.json"), "--map", resolve(raiz, "data", "map.json"), "--key", clave],
  { encoding: "utf8" },
);

if (resultado.status !== 0) {
  process.stderr.write(resultado.stderr || "planner.exe falló\n");
  process.exit(resultado.status ?? 1);
}

const rutaSalida = resolve(raiz, "data", "orders_sorted.json");
writeFileSync(rutaSalida, resultado.stdout, "utf8");
console.log(`Wrote ${rutaSalida} (key=${clave})`);

await import(pathToFileURL(resolve(directorioWeb, "scripts", "sync-data.mjs")).href);
