import { useCallback, useEffect, useMemo, useState } from "react";
import { VistaMapa } from "./VistaMapa";
import { TablaPedidos } from "./TablaPedidos";
import { TablaBenchmark } from "./TablaBenchmark";
import type {
  BenchmarkSort,
  CapaMapa,
  MapaZona,
  Pedido,
  PedidosOrdenados,
  RedMinima,
  RutasDijkstra,
  Secuencia,
} from "./types";

async function cargarJson<T>(url: string): Promise<T> {
  const respuesta = await fetch(url);
  if (!respuesta.ok) {
    throw new Error(`${url}: ${respuesta.status}`);
  }
  return (await respuesta.json()) as T;
}

async function intentarJson<T>(url: string): Promise<T | null> {
  try {
    return await cargarJson<T>(url);
  } catch {
    return null;
  }
}

export default function App() {
  const [mapa, setMapa] = useState<MapaZona | null>(null);
  const [pedidos, setPedidos] = useState<Pedido[]>([]);
  const [rutas, setRutas] = useState<RutasDijkstra | null>(null);
  const [mst, setMst] = useState<RedMinima | null>(null);
  const [secuencia, setSecuencia] = useState<Secuencia | null>(null);
  const [bench, setBench] = useState<BenchmarkSort | null>(null);
  const [capa, setCapa] = useState<CapaMapa>("grafo");
  const [idPedidoRuta, setIdPedidoRuta] = useState<number | null>(null);
  const [error, setError] = useState<string | null>(null);

  const cargar = useCallback(async () => {
    setError(null);
    try {
      const zona = await cargarJson<MapaZona>("/data/map.json");
      let empaquetado: PedidosOrdenados;
      try {
        empaquetado = await cargarJson<PedidosOrdenados>("/data/orders_sorted.json");
      } catch {
        empaquetado = await cargarJson<PedidosOrdenados>("/data/orders.json");
      }
      const [dj, prim, gira, bm] = await Promise.all([
        intentarJson<RutasDijkstra>("/data/rutas_dijkstra.json"),
        intentarJson<RedMinima>("/data/mst_prim.json"),
        intentarJson<Secuencia>("/data/secuencia.json"),
        intentarJson<BenchmarkSort>("/data/benchmark_sort.json"),
      ]);
      setMapa(zona);
      setPedidos(empaquetado.orders);
      setRutas(dj);
      setMst(prim);
      setSecuencia(gira);
      setBench(bm);
      if (dj && dj.rutas.length > 0) {
        setIdPedidoRuta(dj.rutas[0].id_pedido);
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : String(err));
    }
  }, []);

  useEffect(() => {
    void cargar();
  }, [cargar]);

  const rutaElegida = useMemo(() => {
    if (!rutas || idPedidoRuta == null) {
      return null;
    }
    return rutas.rutas.find((r) => r.id_pedido === idPedidoRuta) ?? null;
  }, [rutas, idPedidoRuta]);

  return (
    <div className="layout">
      <header>
        <h1>Planificador de rutas — U1 a U4</h1>
        <p>Grafo OSM (UABC Otay) · MergeSort · Dijkstra · Prim · backtracking</p>
      </header>
      <nav className="tabs">
        {(
          [
            ["grafo", "Grafo"],
            ["dijkstra", "Rutas Dijkstra"],
            ["prim", "Red Prim"],
            ["gira", "Gira"],
          ] as const
        ).map(([id, etiqueta]) => (
          <button
            key={id}
            type="button"
            className={capa === id ? "tab activa" : "tab"}
            onClick={() => setCapa(id)}
          >
            {etiqueta}
          </button>
        ))}
      </nav>
      {capa === "dijkstra" && rutas ? (
        <div className="toolbar">
          <label>
            Pedido{" "}
            <select
              value={idPedidoRuta ?? ""}
              onChange={(e) => setIdPedidoRuta(Number(e.target.value))}
            >
              {rutas.rutas.map((r) => (
                <option key={r.id_pedido} value={r.id_pedido}>
                  #{r.id_pedido} {r.alcanzable ? `${r.distancia_m?.toFixed(0)} m` : "inalcanzable"}
                </option>
              ))}
            </select>
          </label>
          <span className="hint">Exacto (Dijkstra). Solo trayecto almacén → destino.</span>
        </div>
      ) : null}
      {capa === "prim" && mst ? (
        <div className="toolbar">
          <span className="hint">
            Prim, {mst.peso_total_m.toFixed(1)} m. {mst.nota}. Exacto: {mst.exact ? "sí" : "no"}.
          </span>
        </div>
      ) : null}
      {capa === "gira" && secuencia ? (
        <div className="toolbar">
          <span className="hint">
            {secuencia.exact ? "Backtracking exacto" : `Heurística ${secuencia.heuristic ?? ""}`} ·{" "}
            {secuencia.distancia_total_m.toFixed(1)} m · nodos explorados {secuencia.nodos_explorados}
            {secuencia.inalcanzables.length > 0
              ? ` · inalcanzables: ${secuencia.inalcanzables.join(", ")}`
              : ""}
          </span>
        </div>
      ) : null}
      {error ? <p className="error">{error}</p> : null}
      {mapa ? (
        <VistaMapa
          mapa={mapa}
          pedidos={pedidos}
          capa={capa}
          rutaDijkstra={capa === "dijkstra" ? rutaElegida : null}
          redMinima={mst}
          secuencia={secuencia}
        />
      ) : (
        <p>Cargando mapa…</p>
      )}
      <TablaPedidos pedidos={pedidos} recargar={() => void cargar()} />
      {capa === "gira" && secuencia ? (
        <section className="panel">
          <h2>Paradas de la gira</h2>
          <ol>
            {secuencia.paradas.map((p) => (
              <li key={p.id_pedido}>
                Pedido {p.id_pedido} → nodo {p.nodo_destino}
              </li>
            ))}
          </ol>
        </section>
      ) : null}
      <TablaBenchmark datos={bench} />
    </div>
  );
}
