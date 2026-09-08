export type NodoMapa = {
  id: number;
  lat: number;
  lon: number;
};

export type AristaMapa = {
  from: number;
  to: number;
  weight: number;
};

export type MapaZona = {
  warehouse_node_id: number;
  nodes: NodoMapa[];
  edges: AristaMapa[];
};

export type Pedido = {
  id_pedido: number;
  nodo_destino: number;
  peso_paquete: number;
  prioridad: number;
  ventana_inicio: number | null;
  ventana_fin: number | null;
  distancia_al_almacen: number;
};

export type PedidosOrdenados = {
  key?: string;
  algorithm?: string;
  exact?: boolean;
  orders: Pedido[];
};
