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
  pedidoActivo: number | null;
  onSeleccionar: (id: number) => void;
};

export function TablaPedidos({ pedidos, pedidoActivo, onSeleccionar }: Propiedades) {
  return (
    <table className="tabla-panel">
      <thead>
        <tr>
          <th>ID</th>
          <th>Pri.</th>
          <th>Peso</th>
          <th>Ventana</th>
          <th>Dist. (m)</th>
        </tr>
      </thead>
      <tbody>
        {pedidos.map((pedido) => (
          <tr
            key={pedido.id_pedido}
            className={pedido.id_pedido === pedidoActivo ? "fila-activa" : ""}
            onClick={() => onSeleccionar(pedido.id_pedido)}
          >
            <td>{pedido.id_pedido}</td>
            <td>{pedido.prioridad}</td>
            <td>{pedido.peso_paquete.toFixed(1)}</td>
            <td>{formatearVentana(pedido.ventana_inicio, pedido.ventana_fin)}</td>
            <td>{Number.isFinite(pedido.distancia_al_almacen) ? pedido.distancia_al_almacen.toFixed(0) : "—"}</td>
          </tr>
        ))}
      </tbody>
    </table>
  );
}
