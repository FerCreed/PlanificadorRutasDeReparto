import { useCallback, useEffect, useMemo, useState } from "react";
import { VistaMapa } from "./VistaMapa";
import { TablaPedidos } from "./TablaPedidos";
import { TablaBenchmark } from "./TablaBenchmark";
import type {
  AlgoritmoFoco,
  BenchmarkSort,
  CapasVisibles,
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

const titulos: Record<AlgoritmoFoco, string> = {
  grafo: "Mapa de la zona (lista de adyacencia)",
  dijkstra: "Ruta más corta almacén → pedido (Dijkstra)",
  prim: "Red mínima de calles (Prim)",
  gira: "Secuencia de entregas (backtracking)",
};

export default function App() {
  const [vista, setVista] = useState<"mapa" | "benchmark">("mapa");
  const [mapa, setMapa] = useState<MapaZona | null>(null);
  const [pedidos, setPedidos] = useState<Pedido[]>([]);
  const [rutas, setRutas] = useState<RutasDijkstra | null>(null);
  const [mst, setMst] = useState<RedMinima | null>(null);
  const [secuencia, setSecuencia] = useState<Secuencia | null>(null);
  const [bench, setBench] = useState<BenchmarkSort | null>(null);
  const [foco, setFoco] = useState<AlgoritmoFoco>("grafo");
  const [capas, setCapas] = useState<CapasVisibles>({
    grafo: true,
    pedidos: true,
    dijkstra: false,
    prim: false,
    gira: false,
  });
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

  const enfocar = (id: AlgoritmoFoco) => {
    setFoco(id);
  };

  const toggleCapa = (clave: keyof CapasVisibles) => {
    setCapas((prev) => ({ ...prev, [clave]: !prev[clave] }));
  };

  return (
    <div className="app">
      <header className="topbar">
        <div className="marca">UABC Rutas</div>
        <nav>
          <button type="button" className={vista === "mapa" ? "nav-btn activa" : "nav-btn"} onClick={() => setVista("mapa")}>
            Mapa
          </button>
          <button
            type="button"
            className={vista === "benchmark" ? "nav-btn activa" : "nav-btn"}
            onClick={() => setVista("benchmark")}
          >
            Benchmark
          </button>
        </nav>
        <button type="button" className="nav-btn" onClick={() => void cargar()}>
          Recargar
        </button>
      </header>

      {vista === "mapa" ? (
        <>
          <div className="subheader">
            <h1>{titulos[foco]}</h1>
            <span className="zona">UABC Otay · OpenStreetMap</span>
          </div>
          {error ? <p className="error">{error}</p> : null}
          <div className="cuerpo">
            <div className="mapa-wrap">
              {mapa ? (
                <VistaMapa
                  mapa={mapa}
                  pedidos={pedidos}
                  capas={capas}
                  rutaDijkstra={capas.dijkstra ? rutaElegida : null}
                  redMinima={mst}
                  secuencia={secuencia}
                />
              ) : (
                <p className="cargando">Cargando mapa…</p>
              )}
              <aside className="tarjeta">
                <h2>Capas en el mapa</h2>
                <p className="mini">El cuadro muestra u oculta en el mapa. El nombre solo cambia la info (no apaga las otras capas).</p>
                {(
                  [
                    ["grafo", "Grafo de calles", true],
                    ["dijkstra", "Dijkstra (un pedido)", Boolean(rutas)],
                    ["prim", "Red mínima Prim", Boolean(mst)],
                    ["gira", "Gira / backtracking", Boolean(secuencia)],
                  ] as const
                ).map(([id, etiqueta, listo]) => (
                  <div key={id} className={`check-fila ${foco === id ? "foco" : ""}`}>
                    <input
                      type="checkbox"
                      checked={capas[id]}
                      disabled={!listo}
                      onChange={() => {
                        if (!listo) {
                          return;
                        }
                        const encendida = !capas[id];
                        setCapas((prev) => ({ ...prev, [id]: encendida }));
                        if (encendida) {
                          enfocar(id);
                        }
                      }}
                    />
                    <span
                      role="button"
                      tabIndex={0}
                      className="link-algo"
                      onClick={() => listo && enfocar(id)}
                      onKeyDown={(e) => {
                        if (listo && (e.key === "Enter" || e.key === " ")) {
                          enfocar(id);
                        }
                      }}
                    >
                      {etiqueta}
                      {!listo ? " (falta JSON)" : ""}
                    </span>
                  </div>
                ))}
                <label className="check-fila">
                  <input type="checkbox" checked={capas.pedidos} onChange={() => toggleCapa("pedidos")} />
                  Marcadores de pedidos
                </label>

                {foco === "dijkstra" && rutas ? (
                  <div className="bloque">
                    <h3>Pedido destino</h3>
                    <select value={idPedidoRuta ?? ""} onChange={(e) => setIdPedidoRuta(Number(e.target.value))}>
                      {rutas.rutas.map((r) => (
                        <option key={r.id_pedido} value={r.id_pedido}>
                          #{r.id_pedido} {r.alcanzable ? `${r.distancia_m?.toFixed(0)} m` : "inalcanzable"}
                        </option>
                      ))}
                    </select>
                    <p className="info">
                      Exacto. Solo almacén → un punto. No es la gira del día.
                    </p>
                  </div>
                ) : null}
                {foco === "prim" && mst ? (
                  <div className="bloque">
                    <h3>Información</h3>
                    <p className="info">
                      Peso total {mst.peso_total_m.toFixed(0)} m. {mst.nota}. Exacto.
                    </p>
                  </div>
                ) : null}
                {foco === "gira" && secuencia ? (
                  <div className="bloque">
                    <h3>Información</h3>
                    <p className="info">
                      {secuencia.exact ? "Backtracking exacto" : `Heurística ${secuencia.heuristic}`} ·{" "}
                      {secuencia.distancia_total_m.toFixed(0)} m · {secuencia.nodos_explorados} nodos explorados.
                    </p>
                  </div>
                ) : null}
                {foco === "grafo" ? (
                  <div className="bloque">
                    <h3>Información</h3>
                    <p className="info">Calles ponderadas (metros). Base para Dijkstra y Prim.</p>
                  </div>
                ) : null}

                <div className="stats">
                  <div className="stat">
                    <strong>{mapa?.nodes.length ?? 0}</strong>
                    nodos
                  </div>
                  <div className="stat">
                    <strong>{pedidos.length}</strong>
                    pedidos
                  </div>
                  <div className="stat verde">
                    <strong>{rutaElegida?.alcanzable || secuencia?.exact ? "Exacto" : "—"}</strong>
                    etiqueta
                  </div>
                </div>
              </aside>
            </div>
            <section className="panel-der">
              {foco === "gira" && secuencia ? (
                <>
                  <h2>Paradas de la gira</h2>
                  <ol className="paradas">
                    {secuencia.paradas.map((p) => (
                      <li key={p.id_pedido}>
                        Pedido {p.id_pedido}
                        <span>nodo {p.nodo_destino}</span>
                      </li>
                    ))}
                  </ol>
                </>
              ) : (
                <>
                  <h2>Pedidos</h2>
                  <p className="mini">Clic en una fila para ver Dijkstra a ese destino.</p>
                  <TablaPedidos
                    pedidos={pedidos}
                    pedidoActivo={idPedidoRuta}
                    onSeleccionar={(id) => {
                      setIdPedidoRuta(id);
                      enfocar("dijkstra");
                      setCapas((prev) => ({ ...prev, dijkstra: true }));
                    }}
                  />
                </>
              )}
            </section>
          </div>
        </>
      ) : (
        <TablaBenchmark datos={bench} />
      )}
    </div>
  );
}
