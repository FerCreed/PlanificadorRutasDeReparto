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

export type RutaDijkstra = {
  id_pedido: number;
  nodo_destino: number;
  alcanzable: boolean;
  distancia_m: number | null;
  nodos: number[];
};

export type RutasDijkstra = {
  algorithm: string;
  exact: boolean;
  origen: number;
  rutas: RutaDijkstra[];
};

export type AristaMst = {
  from: number;
  to: number;
  weight: number;
};

export type RedMinima = {
  algorithm: string;
  exact: boolean;
  nota: string;
  peso_total_m: number;
  aristas: AristaMst[];
};

export type ParadaSecuencia = {
  orden: number;
  id_pedido: number;
  nodo_destino: number;
};

export type Secuencia = {
  algorithm: string;
  exact: boolean;
  heuristic?: string;
  nodos_explorados: number;
  distancia_total_m: number;
  paradas: ParadaSecuencia[];
  nodos: number[];
  inalcanzables: number[];
};

export type FilaBenchmark = {
  n: number;
  repeticiones: number;
  mergesort_ms: number;
  insercion_ms: number;
  burbuja_ms: number;
};

export type BenchmarkSort = {
  algorithm: string;
  exact: boolean;
  comparador: string;
  filas: FilaBenchmark[];
};

export type CapaMapa = "grafo" | "dijkstra" | "prim" | "gira";
