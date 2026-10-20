// Stores the readings and threshold changes in the browser, with Dexie (IndexedDB).
// Dexie is passed in, not imported, so the tests can run this module with a fake.

export const DATABASE_NAME = "temperature-alarm";

export class ReadingStore {
  /** @param {typeof import("dexie").Dexie} Dexie the Dexie class */
  constructor(Dexie, name = DATABASE_NAME) {
    this.db = new Dexie(name);
    this.db.version(1).stores({
      readings: "++id, time",            // {time, celsius (null on ERROR), status, low, high}
      thresholdChanges: "++id, time",    // {time, low, high}
    });
  }

  /** Stores one reading message from the device; time in ms since 1970. */
  addReading({ celsius, status, low, high }, time = Date.now()) {
    return this.db.readings.add({ time, celsius, status, low, high });
  }

  /** The readings at or after @p fromTime, oldest first. */
  since(fromTime) {
    return this.db.readings.where("time").aboveOrEqual(fromTime).toArray();
  }

  /** The @p limit newest readings, newest first. */
  latest(limit) {
    return this.db.readings.orderBy("time").reverse().limit(limit).toArray();
  }

  count() {
    return this.db.readings.count();
  }

  /** Stores the threshold the device confirmed. */
  addThresholdChange({ low, high }, time = Date.now()) {
    return this.db.thresholdChanges.add({ time, low, high });
  }

  /** The last confirmed threshold, or undefined. */
  latestThreshold() {
    return this.db.thresholdChanges.orderBy("time").last();
  }

  /** Deletes every reading and threshold change. */
  clear() {
    return this.db.transaction("rw", this.db.readings, this.db.thresholdChanges, () =>
      Promise.all([this.db.readings.clear(), this.db.thresholdChanges.clear()]),
    );
  }

  /** Every reading as CSV, oldest first. */
  async exportCsv() {
    return toCsv(await this.db.readings.orderBy("time").toArray());
  }
}

/** Readings -> CSV text with a header row; times as ISO 8601 (UTC). */
export function toCsv(readings) {
  const lines = ["time,celsius,status,low,high"];
  for (const r of readings) {
    lines.push([new Date(r.time).toISOString(), r.celsius ?? "", r.status, r.low, r.high].join(","));
  }
  return lines.join("\n") + "\n";
}
