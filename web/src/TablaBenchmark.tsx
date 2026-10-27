import type { BenchmarkSort } from "./types";

type Propiedades = {
  datos: BenchmarkSort | null;
};

export function TablaBenchmark({ datos }: Propiedades) {
  if (!datos) {
    return (
      <section className="panel">
        <h2>Benchmark de ordenamiento</h2>
        <p className="hint">
          Ejecuta <code>pnpm run bench</code> y recarga. Compara MergeSort, inserción y burbuja.
        </p>
      </section>
    );
  }
  const max = Math.max(
    ...datos.filas.flatMap((f) => [f.mergesort_ms, f.insercion_ms, f.burbuja_ms]),
    0.001,
  );
  return (
    <section className="panel">
      <h2>Benchmark de ordenamiento (U2)</h2>
      <p className="hint">
        Tiempo medio por corrida, solo la función de sort. Comparador: {datos.comparador}. Exacto:{" "}
        {datos.exact ? "sí" : "no"}.
      </p>
      <table>
        <thead>
          <tr>
            <th>n</th>
            <th>Reps</th>
            <th>MergeSort (ms)</th>
            <th>Inserción (ms)</th>
            <th>Burbuja (ms)</th>
          </tr>
        </thead>
        <tbody>
          {datos.filas.map((f) => (
            <tr key={f.n}>
              <td>{f.n}</td>
              <td>{f.repeticiones}</td>
              <td>{f.mergesort_ms.toFixed(4)}</td>
              <td>{f.insercion_ms.toFixed(4)}</td>
              <td>{f.burbuja_ms.toFixed(4)}</td>
            </tr>
          ))}
        </tbody>
      </table>
      <div className="barras">
        {datos.filas.map((f) => (
          <div key={f.n} className="barra-grupo">
            <span className="barra-n">n={f.n}</span>
            <div className="barra merge" style={{ width: `${(100 * f.mergesort_ms) / max}%` }} title="MergeSort" />
            <div className="barra ins" style={{ width: `${(100 * f.insercion_ms) / max}%` }} title="Inserción" />
            <div className="barra bur" style={{ width: `${(100 * f.burbuja_ms) / max}%` }} title="Burbuja" />
          </div>
        ))}
      </div>
      <p className="legend">Azul MergeSort · ámbar inserción · rojo burbuja. Se espera Θ(n log n) vs Θ(n²).</p>
    </section>
  );
}
