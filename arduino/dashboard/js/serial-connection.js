// The connection to the Arduino over WebSerial: opens the port, delivers received lines,
// sends text. Knows nothing about what the lines mean (that is protocol.js).
//
// A "device" for app.js is anything with connect(), send(text) and disconnect() that calls
// onLine(line) and onStatus(state, detail); SimulatedDevice is the other one.

import { LineSplitter } from "./line-splitter.js";

export const BAUD_RATE = 115200;   // temperature_alarm/board_config.h

export class SerialConnection {
  #serial;
  #onLine;
  #onStatus;
  #port = null;
  #reader = null;
  #closed = null;
  #splitter;

  /**
   * @param {object} options
   * @param {(line: string) => void} options.onLine
   * @param {(state: "connected"|"disconnected"|"error", detail?: string) => void} options.onStatus
   * @param {Serial} [options.serial] navigator.serial, or a stand-in
   */
  constructor({ onLine, onStatus, serial = globalThis.navigator?.serial }) {
    this.#serial = serial;
    this.#onLine = onLine;
    this.#onStatus = onStatus;
    this.#splitter = new LineSplitter(onLine);
  }

  static isSupported() {
    return Boolean(globalThis.navigator?.serial);
  }

  get label() {
    return "Arduino (WebSerial)";
  }

  /** Asks the user to pick the port, opens it, and starts reading. */
  async connect() {
    this.#port = await this.#serial.requestPort();
    await this.#port.open({ baudRate: BAUD_RATE });
    this.#splitter.reset();
    this.#onStatus("connected");
    this.#closed = this.#readLoop();
  }

  async #readLoop() {
    const decoder = new TextDecoderStream();
    const piped = this.#port.readable.pipeTo(decoder.writable).catch(() => {});
    this.#reader = decoder.readable.getReader();
    try {
      for (;;) {
        const { value, done } = await this.#reader.read();
        if (done) break;
        this.#splitter.push(value);
      }
    } catch (error) {
      this.#onStatus("error", `The connection was lost: ${error.message}`);   // e.g. unplugged
    } finally {
      this.#reader.releaseLock();
      await piped;
    }
  }

  /** Sends @p text as it is (the protocol adds the newline). */
  async send(text) {
    if (!this.#port?.writable) throw new Error("Not connected");
    const writer = this.#port.writable.getWriter();
    try {
      await writer.write(new TextEncoder().encode(text));
    } finally {
      writer.releaseLock();
    }
  }

  async disconnect() {
    if (!this.#port) return;
    await this.#reader?.cancel().catch(() => {});
    await this.#closed;
    await this.#port.close().catch(() => {});
    this.#port = null;
    this.#onStatus("disconnected");
  }
}
