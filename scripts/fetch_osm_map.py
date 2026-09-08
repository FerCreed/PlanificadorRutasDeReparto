#!/usr/bin/env python3
"""Extract a 25-50 node connected street graph around UABC Otay (Tijuana)."""

from __future__ import annotations

import json
import math
import sys
import urllib.error
import urllib.request
from collections import defaultdict, deque
from pathlib import Path

LATITUD_CENTRO = 32.5308
LONGITUD_CENTRO = -116.9665
LATITUD_ALMACEN = 32.5296
LONGITUD_ALMACEN = -116.9678
NODOS_OBJETIVO = 40
NODOS_MIN = 25
NODOS_MAX = 50

# Campus UABC Tijuana (Otay), no el Tecnológico (más al oeste, ~-116.987)
CAJA = (32.5248, -116.9748, 32.5368, -116.9565)  # sur, oeste, norte, este
HIGHWAYS = "residential|tertiary|secondary|primary|service|unclassified|living_street"

OVERPASS_URLS = (
    "https://overpass-api.de/api/interpreter",
    "https://overpass.kumi.systems/api/interpreter",
)

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "data"


def haversine_m(lat1: float, lon1: float, lat2: float, lon2: float) -> float:
    r = 6371000.0
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dphi = math.radians(lat2 - lat1)
    dlmb = math.radians(lon2 - lon1)
    a = math.sin(dphi / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dlmb / 2) ** 2
    return 2 * r * math.asin(min(1.0, math.sqrt(a)))


def overpass_query() -> str:
    s, w, n, e = CAJA
    return f"""
[out:json][timeout:60];
way["highway"~"^({HIGHWAYS})$"]({s},{w},{n},{e});
(._;>;);
out body;
""".strip()


def fetch_overpass() -> dict:
    payload = overpass_query().encode("utf-8")
    last_err: Exception | None = None
    for url in OVERPASS_URLS:
        req = urllib.request.Request(
            url,
            data=payload,
            headers={"Content-Type": "application/x-www-form-urlencoded", "User-Agent": "uabc-route-planner/1.0"},
            method="POST",
        )
        try:
            with urllib.request.urlopen(req, timeout=90) as resp:
                return json.loads(resp.read().decode("utf-8"))
        except (urllib.error.URLError, TimeoutError, json.JSONDecodeError) as err:
            last_err = err
            continue
    raise RuntimeError(f"Overpass failed: {last_err}")


def build_raw_graph(osm: dict) -> tuple[dict[int, tuple[float, float]], dict[int, list[tuple[int, float]]]]:
    coords: dict[int, tuple[float, float]] = {}
    ways: list[list[int]] = []
    for el in osm.get("elements", []):
        if el.get("type") == "node":
            coords[int(el["id"])] = (float(el["lat"]), float(el["lon"]))
        elif el.get("type") == "way":
            ways.append([int(n) for n in el.get("nodes", [])])

    adj: dict[int, list[tuple[int, float]]] = defaultdict(list)
    seen: set[tuple[int, int]] = set()
    for node_ids in ways:
        for a, b in zip(node_ids, node_ids[1:]):
            if a not in coords or b not in coords or a == b:
                continue
            key = (a, b) if a < b else (b, a)
            if key in seen:
                continue
            seen.add(key)
            w = haversine_m(*coords[a], *coords[b])
            if w <= 0.5:
                continue
            adj[a].append((b, w))
            adj[b].append((a, w))
    return coords, adj


def largest_component(adj: dict[int, list[tuple[int, float]]]) -> set[int]:
    seen: set[int] = set()
    best: set[int] = set()
    for start in adj:
        if start in seen:
            continue
        q = deque([start])
        seen.add(start)
        comp: set[int] = {start}
        while q:
            u = q.popleft()
            for v, _ in adj[u]:
                if v not in seen:
                    seen.add(v)
                    comp.add(v)
                    q.append(v)
        if len(comp) > len(best):
            best = comp
    return best


def subgraph(
    coords: dict[int, tuple[float, float]],
    adj: dict[int, list[tuple[int, float]]],
    keep: set[int],
) -> tuple[dict[int, tuple[float, float]], dict[int, list[tuple[int, float]]]]:
    c = {i: coords[i] for i in keep if i in coords}
    a: dict[int, list[tuple[int, float]]] = {}
    for u in keep:
        a[u] = [(v, w) for v, w in adj.get(u, []) if v in keep]
    return c, a


def contract_degree2(
    coords: dict[int, tuple[float, float]],
    adj: dict[int, list[tuple[int, float]]],
    protected: set[int],
) -> tuple[dict[int, tuple[float, float]], dict[int, list[tuple[int, float]]]]:
    adj = {u: list(vs) for u, vs in adj.items()}

    def degree(u: int) -> int:
        nbrs = {v for v, _ in adj.get(u, [])}
        return len(nbrs)

    changed = True
    while changed:
        changed = False
        for u in list(adj.keys()):
            if u in protected or degree(u) != 2:
                continue
            nbrs = adj[u]
            if len(nbrs) != 2:
                continue
            (a, wa), (b, wb) = nbrs[0], nbrs[1]
            if a == b:
                continue
            # replace u with a--b
            adj[a] = [(v, w) for v, w in adj[a] if v != u]
            adj[b] = [(v, w) for v, w in adj[b] if v != u]
            weight = wa + wb
            adj[a].append((b, weight))
            adj[b].append((a, weight))
            del adj[u]
            coords.pop(u, None)
            changed = True
            break
    return coords, adj


def bfs_limit(
    coords: dict[int, tuple[float, float]],
    adj: dict[int, list[tuple[int, float]]],
    start: int,
    limit: int,
) -> set[int]:
    keep: set[int] = set()
    q = deque([start])
    keep.add(start)
    while q and len(keep) < limit:
        u = q.popleft()
        for v, _ in sorted(adj.get(u, []), key=lambda t: t[1]):
            if v not in keep:
                keep.add(v)
                q.append(v)
                if len(keep) >= limit:
                    break
    return keep


def nearest_node(coords: dict[int, tuple[float, float]], lat: float, lon: float) -> int:
    return min(coords, key=lambda i: haversine_m(lat, lon, *coords[i]))


def dump_map(coords: dict[int, tuple[float, float]], adj: dict[int, list[tuple[int, float]]], warehouse: int, path: Path) -> None:
    nodes = [{"id": i, "lat": coords[i][0], "lon": coords[i][1]} for i in sorted(coords)]
    edges = []
    seen: set[tuple[int, int]] = set()
    for u, nbrs in adj.items():
        for v, w in nbrs:
            key = (u, v) if u < v else (v, u)
            if key in seen:
                continue
            seen.add(key)
            edges.append({"from": key[0], "to": key[1], "weight": round(w, 2)})
    payload = {"warehouse_node_id": warehouse, "nodes": nodes, "edges": edges}
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(f"Wrote {path} ({len(nodes)} nodes, {len(edges)} edges, warehouse={warehouse})")


def synthetic_map() -> tuple[dict[int, tuple[float, float]], dict[int, list[tuple[int, float]]], int]:
    """Connected grid near Otay if Overpass is unavailable."""
    rows, cols = 6, 7
    # ~90 m spacing
    dlat = 90.0 / 111320.0
    dlon = 90.0 / (111320.0 * math.cos(math.radians(LATITUD_CENTRO)))
    coords: dict[int, tuple[float, float]] = {}
    nid = 1
    grid: list[list[int]] = []
    for r in range(rows):
        row: list[int] = []
        for c in range(cols):
            lat = LATITUD_CENTRO + (r - (rows - 1) / 2) * dlat
            lon = LONGITUD_CENTRO + (c - (cols - 1) / 2) * dlon
            coords[nid] = (lat, lon)
            row.append(nid)
            nid += 1
        grid.append(row)
    adj: dict[int, list[tuple[int, float]]] = defaultdict(list)
    for r in range(rows):
        for c in range(cols):
            u = grid[r][c]
            if c + 1 < cols:
                v = grid[r][c + 1]
                w = haversine_m(*coords[u], *coords[v])
                adj[u].append((v, w))
                adj[v].append((u, w))
            if r + 1 < rows:
                v = grid[r + 1][c]
                w = haversine_m(*coords[u], *coords[v])
                adj[u].append((v, w))
                adj[v].append((u, w))
    warehouse = nearest_node(coords, LATITUD_ALMACEN, LONGITUD_ALMACEN)
    return coords, adj, warehouse


def write_sample_orders(coords: dict[int, tuple[float, float]], warehouse: int, path: Path) -> None:
    dests = [i for i in sorted(coords) if i != warehouse]
    # 10 orders on distinct destinations
    dests = dests[:12] if len(dests) >= 10 else dests
    specs = [
        (1, 2.0, 5, 8 * 60, 11 * 60),
        (2, 1.5, 3, 9 * 60, 12 * 60),
        (3, 3.0, 5, -1, -1),
        (4, 0.8, 2, 14 * 60, 16 * 60),
        (5, 1.2, 4, 10 * 60, 11 * 60),
        (6, 2.5, 1, -1, -1),
        (7, 1.0, 4, 8 * 60, 9 * 60 + 30),
        (8, 4.0, 3, 13 * 60, 15 * 60),
        (9, 0.5, 5, 12 * 60, 13 * 60),
        (10, 2.2, 2, -1, -1),
        (11, 1.8, 4, 16 * 60, 18 * 60),
        (12, 3.5, 1, 7 * 60, 20 * 60),
    ]
    orders = []
    for i, spec in enumerate(specs[: max(8, min(12, len(dests)))]):
        oid, peso, pri, t0, t1 = spec
        dest = dests[i % len(dests)]
        dlat, dlon = coords[dest]
        wlat, wlon = coords[warehouse]
        orders.append(
            {
                "id_pedido": oid,
                "nodo_destino": dest,
                "peso_paquete": peso,
                "prioridad": pri,
                "ventana_inicio": t0,
                "ventana_fin": t1,
                "distancia_al_almacen": round(haversine_m(wlat, wlon, dlat, dlon), 2),
            }
        )
    path.write_text(json.dumps({"orders": orders}, indent=2), encoding="utf-8")
    print(f"Wrote {path} ({len(orders)} orders)")


def from_osm() -> tuple[dict[int, tuple[float, float]], dict[int, list[tuple[int, float]]], int]:
    osm = fetch_overpass()
    coords, adj = build_raw_graph(osm)
    if not adj:
        raise RuntimeError("OSM graph is empty")
    keep = largest_component(adj)
    coords, adj = subgraph(coords, adj, keep)
    warehouse = nearest_node(coords, LATITUD_ALMACEN, LONGITUD_ALMACEN)
    if len(coords) > NODOS_MAX:
        coords, adj = contract_degree2(coords, adj, {warehouse})
        keep = largest_component(adj)
        coords, adj = subgraph(coords, adj, keep)
        warehouse = nearest_node(coords, LATITUD_ALMACEN, LONGITUD_ALMACEN)
    if len(coords) > NODOS_MAX:
        keep = bfs_limit(coords, adj, warehouse, NODOS_OBJETIVO)
        coords, adj = subgraph(coords, adj, keep)
    if len(coords) < NODOS_MIN:
        raise RuntimeError(f"Only {len(coords)} nodes after extraction")
    return coords, adj, warehouse


def main() -> int:
    DATA.mkdir(parents=True, exist_ok=True)
    map_path = DATA / "map.json"
    orders_path = DATA / "orders.json"
    try:
        coords, adj, warehouse = from_osm()
        print("Source: OpenStreetMap Overpass")
    except Exception as err:
        print(f"Overpass unavailable ({err}); using synthetic Otay grid", file=sys.stderr)
        coords, adj, warehouse = synthetic_map()
        print("Source: synthetic grid")
    dump_map(coords, adj, warehouse, map_path)
    write_sample_orders(coords, warehouse, orders_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
