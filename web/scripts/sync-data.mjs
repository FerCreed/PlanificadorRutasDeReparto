import { copyFileSync, existsSync, mkdirSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const webDir = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const root = resolve(webDir, "..");
const destDir = resolve(webDir, "public", "data");

mkdirSync(destDir, { recursive: true });
copyFileSync(resolve(root, "data", "map.json"), resolve(destDir, "map.json"));
copyFileSync(resolve(root, "data", "orders.json"), resolve(destDir, "orders.json"));
const sorted = resolve(root, "data", "orders_sorted.json");
if (existsSync(sorted)) {
  copyFileSync(sorted, resolve(destDir, "orders_sorted.json"));
}
console.log("Synced data/ → web/public/data/");
