import type { Pedido } from "./types";

function formatearVentana(inicio: number | null, fin: number | null): string {
  const aHora = (minutos: number) => {
    const horas = Math.floor(minutos / 60);
    const mins = minutos % 60;
    return `${String(horas).padStart(2, "0")}:${String(mins).padStart(2, "0")}`;
  };
  if ((inicio == null || inicio < 0) && (fin == null || fin < 0)) {
    return "—";
  }
  return `${inicio == null ? "?" : aHora(inicio)}–${fin == null ? "?" : aHora(fin)}`;
}

type Propiedades = {
  pedidos: Pedido[];
  recargar: () => void;
};

export function TablaPedidos({ pedidos, recargar }: Propiedades) {
  return (
    <section className="panel">
      <div className="panel-head">
        <h2>Pedidos</h2>
        <button type="button" onClick={recargar}>
          Recargar datos
        </button>
      </div>
      <table>
        <thead>
          <tr>
            <th>ID</th>
            <th>Nodo</th>
            <th>Peso</th>
            <th>Prioridad</th>
            <th>Ventana</th>
            <th>Dist. almacén (m)</th>
          </tr>
        </thead>
        <tbody>
          {pedidos.map((pedido) => (
            <tr key={pedido.id_pedido}>
              <td>{pedido.id_pedido}</td>
              <td>{pedido.nodo_destino}</td>
              <td>{pedido.peso_paquete.toFixed(2)}</td>
              <td>{pedido.prioridad}</td>
              <td>{formatearVentana(pedido.ventana_inicio, pedido.ventana_fin)}</td>
              <td>{pedido.distancia_al_almacen.toFixed(1)}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </section>
  );
}
