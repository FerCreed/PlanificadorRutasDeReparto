import { CircleMarker, MapContainer, Polyline, Popup, TileLayer } from "react-leaflet";
import type { LatLngExpression } from "leaflet";
import type { MapaZona, Pedido } from "./types";

function indiceNodos(mapa: MapaZona): Map<number, { lat: number; lon: number }> {
  return new Map(mapa.nodes.map((nodo) => [nodo.id, { lat: nodo.lat, lon: nodo.lon }]));
}

function formatearVentana(inicio: number | null, fin: number | null): string {
  const aHora = (minutos: number) => {
    const horas = Math.floor(minutos / 60);
    const mins = minutos % 60;
    return `${String(horas).padStart(2, "0")}:${String(mins).padStart(2, "0")}`;
  };
  if ((inicio == null || inicio < 0) && (fin == null || fin < 0)) {
    return "sin ventana";
  }
  return `${inicio == null ? "?" : aHora(inicio)}–${fin == null ? "?" : aHora(fin)}`;
}

type Propiedades = {
  mapa: MapaZona;
  pedidos: Pedido[];
};

export function VistaMapa({ mapa, pedidos }: Propiedades) {
  const porId = indiceNodos(mapa);
  const latitudes = mapa.nodes.map((nodo) => nodo.lat);
  const longitudes = mapa.nodes.map((nodo) => nodo.lon);
  const centro: LatLngExpression = [
    (Math.min(...latitudes) + Math.max(...latitudes)) / 2,
    (Math.min(...longitudes) + Math.max(...longitudes)) / 2,
  ];

  const polilineas = mapa.edges
    .map((arista) => {
      const origen = porId.get(arista.from);
      const destino = porId.get(arista.to);
      if (!origen || !destino) {
        return null;
      }
      return {
        clave: `${arista.from}-${arista.to}`,
        posiciones: [
          [origen.lat, origen.lon],
          [destino.lat, destino.lon],
        ] as LatLngExpression[],
        peso: arista.weight,
      };
    })
    .filter((x): x is NonNullable<typeof x> => x !== null);

  const pedidosPorNodo = new Map<number, Pedido[]>();
  for (const pedido of pedidos) {
    const lista = pedidosPorNodo.get(pedido.nodo_destino) ?? [];
    lista.push(pedido);
    pedidosPorNodo.set(pedido.nodo_destino, lista);
  }

  return (
    <MapContainer center={centro} zoom={16} className="map">
      <TileLayer
        attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a>'
        url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
      />
      {polilineas.map((linea) => (
        <Polyline
          key={linea.clave}
          positions={linea.posiciones}
          pathOptions={{ color: "#2563eb", weight: 4, opacity: 0.75 }}
        >
          <Popup>{linea.peso.toFixed(1)} m</Popup>
        </Polyline>
      ))}
      {mapa.nodes.map((nodo) => {
        const esAlmacen = nodo.id === mapa.warehouse_node_id;
        return (
          <CircleMarker
            key={nodo.id}
            center={[nodo.lat, nodo.lon]}
            radius={esAlmacen ? 11 : 5}
            pathOptions={{
              color: esAlmacen ? "#b45309" : "#0f172a",
              fillColor: esAlmacen ? "#f59e0b" : "#e2e8f0",
              fillOpacity: 1,
              weight: 2,
            }}
          >
            <Popup>
              {esAlmacen ? "Almacén" : "Intersección"} #{nodo.id}
              <br />
              {nodo.lat.toFixed(6)}, {nodo.lon.toFixed(6)}
            </Popup>
          </CircleMarker>
        );
      })}
      {pedidos.map((pedido) => {
        const posicion = porId.get(pedido.nodo_destino);
        if (!posicion) {
          return null;
        }
        const pila = pedidosPorNodo.get(pedido.nodo_destino) ?? [pedido];
        const indice = pila.findIndex((x) => x.id_pedido === pedido.id_pedido);
        const desplazamiento = indice * 0.00004;
        return (
          <CircleMarker
            key={`pedido-${pedido.id_pedido}`}
            center={[posicion.lat + desplazamiento, posicion.lon + desplazamiento]}
            radius={8}
            pathOptions={{ color: "#9f1239", fillColor: "#f43f5e", fillOpacity: 0.95, weight: 2 }}
          >
            <Popup>
              Pedido {pedido.id_pedido}
              <br />
              Prioridad {pedido.prioridad} · {pedido.peso_paquete} kg
              <br />
              {formatearVentana(pedido.ventana_inicio, pedido.ventana_fin)}
              <br />
              Dist. almacén (haversine): {pedido.distancia_al_almacen.toFixed(1)} m
            </Popup>
          </CircleMarker>
        );
      })}
    </MapContainer>
  );
}
