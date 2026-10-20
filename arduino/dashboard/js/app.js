// The composition root: creates the parts and connects them. The behaviour lives in the
// parts — protocol.js (what lines mean), serial-connection.js / simulated-device.js (moving
// lines), reading-store.js (Dexie), temperature-chart.js and dashboard-view.js (showing).

import Dexie from "https://cdn.jsdelivr.net/npm/dexie@4.0.8/dist/dexie.mjs";

import { formatGetThreshold, formatSetThreshold, parseDeviceLine, thresholdProblem } from "./protocol.js";
import { ReadingStore, toCsv } from "./reading-store.js";
import { SerialConnection } from "./serial-connection.js";
import { SimulatedDevice } from "./simulated-device.js";
import { TemperatureChart } from "./temperature-chart.js";
import { DashboardView } from "./dashboard-view.js";

const RANGES = { "5m": 5 * 60e3, "1h": 3600e3, "24h": 24 * 3600e3, all: Infinity };
const TABLE_ROWS = 20;

const store = new ReadingStore(Dexie);
const view = new DashboardView();
const chart = new TemperatureChart(document.getElementById("chart"));

let device = null;
let threshold = null;
let range = "5m";
let redrawPending = false;

// When IndexedDB is unavailable (blocked storage, some private windows) the page keeps
// working on the readings of this visit only, and says so.
const SESSION_LIMIT = 5000;
let storageWorks = true;
const session = [];

async function stored(operation, fallback) {
  if (storageWorks) {
    try {
      return await operation();
    } catch (error) {
      storageWorks = false;
      view.showStorageProblem(error.message);
    }
  }
  return typeof fallback === "function" ? fallback() : fallback;
}

/** Opens the database, giving up after a few seconds (another tab may be blocking it). */
async function openStorage(timeoutMs = 4000) {
  try {
    await Promise.race([
      store.db.open(),
      new Promise((_, reject) => setTimeout(() => reject(new Error("the database did not open")), timeoutMs)),
    ]);
  } catch (error) {
    storageWorks = false;
    view.showStorageProblem(error.message);
  }
}

async function refresh() {
  redrawPending = false;
  const now = Date.now();
  const from = Number.isFinite(RANGES[range]) ? now - RANGES[range] : 0;
  const [readings, newest, count] = await Promise.all([
    stored(() => store.since(from), () => session.filter((r) => r.time >= from)),
    stored(() => store.latest(TABLE_ROWS), () => session.slice(-TABLE_ROWS).reverse()),
    stored(() => store.count(), () => session.length),
  ]);
  chart.show(readings, threshold, readings.length && !Number.isFinite(RANGES[range]) ? readings[0].time : from, now);
  view.showTable(newest);
  view.showCount(count);
}

/** Redraws at most once per animation frame, however many lines arrive. */
function scheduleRefresh() {
  if (!redrawPending) {
    redrawPending = true;
    requestAnimationFrame(refresh);
  }
}

async function onLine(line) {
  view.log("in", line);
  const message = parseDeviceLine(line);
  const time = Date.now();
  if (message.type === "reading") {
    session.push({ ...message, time });
    if (session.length > SESSION_LIMIT) session.shift();
    await stored(() => store.addReading(message, time));
    view.showReading({ ...message, time });
    if (!threshold || threshold.low !== message.low || threshold.high !== message.high) {
      threshold = { low: message.low, high: message.high };
      view.showThreshold(threshold);
    }
    scheduleRefresh();
  } else if (message.type === "threshold") {
    threshold = { low: message.low, high: message.high };
    await stored(() => store.addThresholdChange(threshold, time));
    view.showThreshold(threshold);
    view.showFeedback(`The device now alarms below ${message.low.toFixed(1)} °C and above ${message.high.toFixed(1)} °C.`, "ok");
    scheduleRefresh();
  } else if (message.type === "rejected") {
    view.showFeedback("The device did not accept that threshold.", "error");
  }
}

function onStatus(state, detail) {
  view.showConnection(state, device?.label, detail);
  if (state !== "connected") device = null;
}

async function send(text) {
  view.log("out", text);
  await device.send(text);
}

async function start(makeDevice) {
  view.showConnection("connecting");
  device = makeDevice();
  try {
    await device.connect();
    await send(formatGetThreshold());
  } catch (error) {
    device = null;
    // Closing the port picker is not an error worth shouting about.
    view.showConnection("disconnected", "", error.name === "NotFoundError" ? "" : `Could not connect: ${error.message}`);
  }
}

view.connectButton.addEventListener("click", () => {
  if (device) device.disconnect();
  else start(() => new SerialConnection({ onLine, onStatus }));
});

view.demoButton.addEventListener("click", () => {
  if (!device) start(() => new SimulatedDevice({ onLine, onStatus }));
});

view.form.addEventListener("submit", async (event) => {
  event.preventDefault();
  const { low, high } = view.formValues();
  const problem = thresholdProblem(low, high);
  if (problem) {
    view.showFeedback(problem, "error");
    return;
  }
  view.showFeedback("Sending…", "pending");
  await send(formatSetThreshold(low, high));
});

for (const button of view.rangeButtons) {
  button.addEventListener("click", () => {
    range = button.dataset.range;
    view.showRange(range);
    refresh();
  });
}

document.getElementById("export").addEventListener("click", async () => {
  const csv = await stored(() => store.exportCsv(), () => toCsv(session));
  view.download(`temperature-readings-${new Date().toISOString().slice(0, 10)}.csv`, csv);
});

document.getElementById("clear").addEventListener("click", async () => {
  if (confirm("Delete every stored reading and threshold change from this browser?")) {
    session.length = 0;
    await stored(() => store.clear());
    refresh();
  }
});

// Keep the time axis moving while connected, even between readings.
setInterval(() => { if (device) scheduleRefresh(); }, 5000);

if (!SerialConnection.isSupported()) view.showUnsupported();
view.showConnection("disconnected");
view.showRange(range);
await openStorage();
const last = await stored(() => store.latestThreshold());
if (last) {
  threshold = { low: last.low, high: last.high };
  view.showThreshold(threshold);
}
const [newest] = await stored(() => store.latest(1), []);
if (newest) view.showReading(newest);
refresh();

// index.html?demo starts the simulated device straight away.
if (new URLSearchParams(location.search).has("demo")) start(() => new SimulatedDevice({ onLine, onStatus }));
