// Everything the page shows, and nothing else: no data access, no protocol, no serial.
// app.js tells it what to show and listens to the events it reports.

import { STATUS_TEXT } from "./temperature-chart.js";

const STATUS_ICON = { OK: "✓", LOW: "▼", HIGH: "▲", ERROR: "!" };
const STATUS_CLASS = { OK: "good", LOW: "critical", HIGH: "critical", ERROR: "warning" };
const LOG_LINES = 200;

const $ = (id) => document.getElementById(id);

function celsius(value) {
  return value === null || value === undefined ? "—" : `${value.toFixed(1)} °C`;
}

export class DashboardView {
  constructor() {
    this.connectButton = $("connect");
    this.demoButton = $("demo");
    this.form = $("threshold-form");
    this.lowInput = $("low");
    this.highInput = $("high");
    this.rangeButtons = [...document.querySelectorAll("[data-range]")];
  }

  /** Shown once if this browser has no WebSerial. */
  showUnsupported() {
    $("unsupported").hidden = false;
    this.connectButton.disabled = true;
  }

  /** Shown once if readings cannot be stored in this browser. */
  showStorageProblem(reason) {
    const notice = $("storage-problem");
    notice.textContent = `Readings cannot be stored in this browser (${reason}), so only this visit's readings are shown.`;
    notice.hidden = false;
    $("count-meta").textContent = "This visit only";
  }

  /** @param {"connected"|"disconnected"|"error"|"connecting"} state */
  showConnection(state, label = "", detail = "") {
    const pill = $("connection");
    pill.dataset.state = state;
    pill.textContent = {
      connected: `Connected — ${label}`,
      connecting: "Connecting…",
      disconnected: "Not connected",
      error: "Connection lost",
    }[state];
    const connected = state === "connected";
    this.connectButton.textContent = connected ? "Disconnect" : "Connect Arduino";
    this.demoButton.hidden = connected;
    for (const input of this.form.elements) input.disabled = !connected;
    if (detail) this.showFeedback(detail, "error");
  }

  /** The latest reading in the stat tiles. */
  showReading({ celsius: value, status, time }) {
    $("current").textContent = celsius(value);
    const tile = $("status");
    tile.dataset.status = STATUS_CLASS[status];
    $("status-icon").textContent = STATUS_ICON[status];
    $("status-text").textContent = STATUS_TEXT[status];
    $("buzzer").textContent = status === "HIGH" || status === "LOW" ? "Buzzer sounding" : "Buzzer off";
    $("updated").textContent = `Measured ${new Date(time).toLocaleTimeString()}`;
  }

  /** The threshold the device reported; also refills the form unless the user is editing it. */
  showThreshold({ low, high }) {
    $("threshold").textContent = `${low.toFixed(1)} – ${high.toFixed(1)} °C`;
    if (!this.form.contains(document.activeElement)) {
      this.lowInput.value = low.toFixed(1);
      this.highInput.value = high.toFixed(1);
    }
  }

  showCount(count) {
    $("count").textContent = count.toLocaleString();
  }

  /** @param {"ok"|"error"|"pending"} kind */
  showFeedback(message, kind) {
    const feedback = $("feedback");
    feedback.textContent = message;
    feedback.dataset.kind = kind;
  }

  formValues() {
    return { low: this.lowInput.valueAsNumber, high: this.highInput.valueAsNumber };
  }

  showRange(range) {
    for (const button of this.rangeButtons) button.setAttribute("aria-pressed", String(button.dataset.range === range));
  }

  /** The newest readings, newest first, as the table view of the chart. */
  showTable(readings) {
    const body = $("readings");
    body.replaceChildren(...readings.map((r) => {
      const row = document.createElement("tr");
      for (const text of [new Date(r.time).toLocaleString(), celsius(r.celsius), STATUS_TEXT[r.status],
        `${r.low.toFixed(1)} – ${r.high.toFixed(1)}`]) {
        const cell = document.createElement("td");
        cell.textContent = text;
        row.appendChild(cell);
      }
      row.dataset.status = STATUS_CLASS[r.status];
      return row;
    }));
    $("table-empty").hidden = readings.length > 0;
  }

  /** One line of serial traffic; @p direction is "in" or "out". */
  log(direction, line) {
    const list = $("log");
    const item = document.createElement("li");
    item.dataset.direction = direction;
    item.textContent = `${new Date().toLocaleTimeString()}  ${direction === "in" ? "←" : "→"}  ${line.trim()}`;
    list.prepend(item);
    while (list.children.length > LOG_LINES) list.lastChild.remove();
  }

  download(filename, text, type = "text/csv") {
    const url = URL.createObjectURL(new Blob([text], { type }));
    const a = Object.assign(document.createElement("a"), { href: url, download: filename });
    a.click();
    URL.revokeObjectURL(url);
  }
}
