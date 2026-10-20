// Draws the temperature over time as an SVG line chart, with the threshold as a band,
// readings outside it marked, and a crosshair tooltip. Only drawing; no data access.

const SVG = "http://www.w3.org/2000/svg";
const MARGIN = { top: 16, right: 16, bottom: 30, left: 48 };
const HEIGHT = 300;
const GAP_MS = 10_000;   // readings further apart than this are not joined by the line

function el(name, attrs = {}, parent = null) {
  const node = document.createElementNS(SVG, name);
  for (const [k, v] of Object.entries(attrs)) node.setAttribute(k, v);
  if (parent) parent.appendChild(node);
  return node;
}

/** Up to ~6 round tick values covering [lo, hi]. */
export function niceTicks(lo, hi, count = 5) {
  const span = hi - lo || 1;
  const raw = span / count;
  const magnitude = 10 ** Math.floor(Math.log10(raw));
  const step = [1, 2, 2.5, 5, 10].map((m) => m * magnitude).find((s) => s >= raw);
  const ticks = [];
  for (let v = Math.ceil(lo / step) * step; v <= hi + step / 1e6; v += step) ticks.push(Number(v.toFixed(6)));
  return ticks;
}

function timeLabel(time, spanMs) {
  const options = spanMs > 36 * 3600e3 ? { month: "short", day: "numeric", hour: "2-digit" }
    : spanMs > 600e3 ? { hour: "2-digit", minute: "2-digit" }
      : { hour: "2-digit", minute: "2-digit", second: "2-digit" };
  return new Date(time).toLocaleString([], options);
}

export class TemperatureChart {
  #root;
  #tooltip;
  #data = { readings: [], threshold: null, from: 0, to: 0 };

  /** @param {HTMLElement} root an empty element the chart fills; it sets the width */
  constructor(root) {
    this.#root = root;
    this.#tooltip = document.createElement("div");
    this.#tooltip.className = "chart-tooltip";
    this.#tooltip.hidden = true;
    new ResizeObserver(() => this.#draw()).observe(root);
  }

  /**
   * @param {Array<{time, celsius, status}>} readings oldest first
   * @param {{low, high}|null} threshold the current one, drawn as a band
   * @param {number} from start of the time range shown (ms)
   * @param {number} to end of the time range shown (ms)
   */
  show(readings, threshold, from, to) {
    this.#data = { readings, threshold, from, to };
    this.#draw();
  }

  #draw() {
    const { readings, threshold, from, to } = this.#data;
    const width = Math.max(280, this.#root.clientWidth);
    this.#root.replaceChildren();
    const svg = el("svg", { viewBox: `0 0 ${width} ${HEIGHT}`, width, height: HEIGHT, role: "img",
      "aria-label": "Temperature over time, with the threshold band" }, this.#root);
    this.#root.appendChild(this.#tooltip);

    const values = readings.filter((r) => r.celsius !== null).map((r) => r.celsius);
    if (values.length === 0) {
      el("text", { x: width / 2, y: HEIGHT / 2, "text-anchor": "middle", class: "chart-empty" }, svg)
        .textContent = "No readings in this time range yet";
      return;
    }
    const limits = threshold ? [threshold.low, threshold.high] : [];
    let lo = Math.min(...values, ...limits);
    let hi = Math.max(...values, ...limits);
    const pad = Math.max(1, (hi - lo) * 0.1);
    lo -= pad;
    hi += pad;

    const x0 = MARGIN.left, x1 = width - MARGIN.right, y0 = HEIGHT - MARGIN.bottom, y1 = MARGIN.top;
    const start = Math.min(from, readings[0].time);
    const end = Math.max(to, start + 1);
    const px = (t) => x0 + ((t - start) / (end - start)) * (x1 - x0);
    const py = (v) => y0 - ((v - lo) / (hi - lo)) * (y0 - y1);

    // Threshold band first, so everything else sits on top of it.
    if (threshold) {
      el("rect", { x: x0, width: x1 - x0, y: py(threshold.high), height: py(threshold.low) - py(threshold.high),
        class: "chart-band" }, svg);
      for (const [value, name] of [[threshold.high, "high"], [threshold.low, "low"]]) {
        el("line", { x1: x0, x2: x1, y1: py(value), y2: py(value), class: "chart-limit" }, svg);
        el("text", { x: x1 - 4, y: py(value) - 5, "text-anchor": "end", class: "chart-limit-label" }, svg)
          .textContent = `${name} ${value.toFixed(1)} °C`;
      }
    }

    for (const v of niceTicks(lo, hi)) {
      el("line", { x1: x0, x2: x1, y1: py(v), y2: py(v), class: "chart-grid" }, svg);
      el("text", { x: x0 - 8, y: py(v) + 4, "text-anchor": "end", class: "chart-axis" }, svg).textContent = `${v}°`;
    }
    const span = end - start;
    const tickCount = Math.max(2, Math.floor((x1 - x0) / 110));
    for (let i = 0; i <= tickCount; i++) {
      const t = start + (span * i) / tickCount;
      el("text", { x: px(t), y: y0 + 20, "text-anchor": i === 0 ? "start" : i === tickCount ? "end" : "middle",
        class: "chart-axis" }, svg).textContent = timeLabel(t, span);
    }
    el("line", { x1: x0, x2: x1, y1: y0, y2: y0, class: "chart-baseline" }, svg);

    // The line, broken at failed readings and at gaps in the recording.
    let path = "";
    let previous = null;
    for (const r of readings) {
      if (r.celsius === null) { previous = null; continue; }
      const joined = previous && r.time - previous.time <= GAP_MS;
      path += `${joined ? "L" : "M"}${px(r.time).toFixed(1)},${py(r.celsius).toFixed(1)}`;
      previous = r;
    }
    el("path", { d: path, class: "chart-line" }, svg);

    for (const r of readings) {
      if (r.celsius !== null && (r.status === "HIGH" || r.status === "LOW")) {
        el("circle", { cx: px(r.time), cy: py(r.celsius), r: 4, class: "chart-alarm" }, svg);
      }
    }

    this.#hover(svg, readings, px, py, x0, x1, y0, y1);
  }

  #hover(svg, readings, px, py, x0, x1, y0, y1) {
    const cross = el("line", { y1, y2: y0, class: "chart-crosshair", visibility: "hidden" }, svg);
    const dot = el("circle", { r: 5, class: "chart-focus", visibility: "hidden" }, svg);
    const area = el("rect", { x: x0, y: y1, width: x1 - x0, height: y0 - y1, fill: "transparent" }, svg);
    const hide = () => {
      cross.setAttribute("visibility", "hidden");
      dot.setAttribute("visibility", "hidden");
      this.#tooltip.hidden = true;
    };
    area.addEventListener("pointerleave", hide);
    area.addEventListener("pointermove", (event) => {
      const box = svg.getBoundingClientRect();
      const x = ((event.clientX - box.left) / box.width) * svg.viewBox.baseVal.width;
      let nearest = readings[0];
      for (const r of readings) if (Math.abs(px(r.time) - x) < Math.abs(px(nearest.time) - x)) nearest = r;
      const cx = px(nearest.time);
      cross.setAttribute("x1", cx);
      cross.setAttribute("x2", cx);
      cross.setAttribute("visibility", "visible");
      if (nearest.celsius !== null) {
        dot.setAttribute("cx", cx);
        dot.setAttribute("cy", py(nearest.celsius));
        dot.setAttribute("visibility", "visible");
      } else {
        dot.setAttribute("visibility", "hidden");
      }
      const value = nearest.celsius === null ? "sensor error" : `${nearest.celsius.toFixed(1)} °C`;
      this.#tooltip.innerHTML = `<strong>${value}</strong><span>${new Date(nearest.time).toLocaleString()}</span>`
        + `<span>${STATUS_TEXT[nearest.status] ?? nearest.status}</span>`;
      this.#tooltip.hidden = false;
      const left = (cx / svg.viewBox.baseVal.width) * box.width;
      this.#tooltip.style.left = `${Math.min(Math.max(left, 70), box.width - 70)}px`;
    });
  }
}

export const STATUS_TEXT = Object.freeze({
  OK: "In range",
  LOW: "Too low — buzzer on",
  HIGH: "Too high — buzzer on",
  ERROR: "Sensor error",
});
