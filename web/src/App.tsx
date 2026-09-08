import { useCallback, useEffect, useState } from "react";
import { VistaMapa } from "./VistaMapa";
import { TablaPedidos } from "./TablaPedidos";
import type { MapaZona, Pedido, PedidosOrdenados } from "./types";

async function cargarJson<T>(url: string): Promise<T> {
  const respuesta = await fetch(url);
  if (!respuesta.ok) {
    throw new Error(`${url}: ${respuesta.status}`);
  }
  return (await respuesta.json()) as T;
}

export default function App() {
  const [mapa, setMapa] = useState<MapaZona | null>(null);
  const [pedidos, setPedidos] = useState<Pedido[]>([]);
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
      setMapa(zona);
      setPedidos(empaquetado.orders);
    } catch (err) {
      setError(err instanceof Error ? err.message : String(err));
    }
  }, []);

  useEffect(() => {
    void cargar();
  }, [cargar]);

  return (
    <div className="layout">
      <header>
        <h1>Planificador de rutas — Fase 1</h1>
        <p>Grafo OSM (UABC Otay) + MergeSort de pedidos.</p>
      </header>
      {error ? <p className="error">{error}</p> : null}
      {mapa ? <VistaMapa mapa={mapa} pedidos={pedidos} /> : <p>Cargando mapa…</p>}
      <TablaPedidos pedidos={pedidos} recargar={() => void cargar()} />
    </div>
  );
}
