// A stand-in for the Arduino, for trying the website without hardware: it speaks the same
// protocol as the firmware (a reading every 2 s, SET and GET answered by the same rules).
// Same interface as SerialConnection.

import { parseDeviceLine, thresholdProblem } from "./protocol.js";

const INTERVAL_MS = 2000;

function oneDecimal(value) {
  return (Math.round(value * 10) / 10).toFixed(1);
}

export class SimulatedDevice {
  #onLine;
  #onStatus;
  #timer = null;
  #celsius = 22;
  #drift = 0.4;
  #low = 18;
  #high = 28;
  #random;

  /** @param {object} options  onLine, onStatus as for SerialConnection; random for tests */
  constructor({ onLine, onStatus, random = Math.random }) {
    this.#onLine = onLine;
    this.#onStatus = onStatus;
    this.#random = random;
  }

  get label() {
    return "Simulated device";
  }

  async connect() {
    this.#onStatus("connected");
    this.#timer = setInterval(() => this.#measure(), INTERVAL_MS);
    this.#measure();
  }

  /** A slow random walk that now and then leaves the threshold, and rarely fails. */
  #measure() {
    if (this.#random() < 0.03) {
      this.#emit(`T,,ERROR,${oneDecimal(this.#low)},${oneDecimal(this.#high)}`);
      return;
    }
    if (this.#random() < 0.08) this.#drift = -this.#drift;
    this.#celsius += this.#drift * (0.5 + this.#random());
    this.#celsius = Math.min(40, Math.max(5, this.#celsius));
    if (this.#celsius >= 40 || this.#celsius <= 5) this.#drift = -this.#drift;
    const status = this.#celsius < this.#low ? "LOW" : this.#celsius > this.#high ? "HIGH" : "OK";
    this.#emit(`T,${oneDecimal(this.#celsius)},${status},${oneDecimal(this.#low)},${oneDecimal(this.#high)}`);
  }

  #emit(line) {
    queueMicrotask(() => this.#onLine(line));
  }

  async send(text) {
    for (const line of text.split("\n").filter(Boolean)) {
      if (line === "GET") {
        this.#emit(`TH,${oneDecimal(this.#low)},${oneDecimal(this.#high)}`);
        continue;
      }
      const match = /^SET,(-?\d+(?:\.\d+)?),(-?\d+(?:\.\d+)?)$/.exec(line);
      const low = match ? Number(match[1]) : NaN;
      const high = match ? Number(match[2]) : NaN;
      if (!match || thresholdProblem(low, high)) {
        this.#emit("ERR,INVALID");
        continue;
      }
      this.#low = low;
      this.#high = high;
      this.#emit(`TH,${oneDecimal(low)},${oneDecimal(high)}`);
    }
  }

  async disconnect() {
    clearInterval(this.#timer);
    this.#timer = null;
    this.#onStatus("disconnected");
  }
}

// Keeps the simulator honest: whatever it emits must be valid protocol.
export function isValidDeviceLine(line) {
  return parseDeviceLine(line).type !== "unknown";
}
