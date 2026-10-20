// Turns the chunks a serial port delivers into whole lines. Pure, no I/O.

/** Characters kept while waiting for a newline; beyond this the partial line is dropped. */
const MAX_PENDING = 1024;

export class LineSplitter {
  #pending = "";
  #onLine;

  /** @param {(line: string) => void} onLine called once per complete, non-empty line */
  constructor(onLine) {
    this.#onLine = onLine;
  }

  /** Adds a chunk of received text; a chunk may hold part of a line or several lines. */
  push(chunk) {
    const parts = (this.#pending + chunk).split("\n");
    this.#pending = parts.pop();
    if (this.#pending.length > MAX_PENDING) this.#pending = "";
    for (const part of parts) {
      const line = part.replace(/\r$/, "");
      if (line !== "") this.#onLine(line);
    }
  }

  /** Forgets a partial line, e.g. after reconnecting. */
  reset() {
    this.#pending = "";
  }
}
