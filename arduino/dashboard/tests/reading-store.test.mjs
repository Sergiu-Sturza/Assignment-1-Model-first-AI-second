import assert from "node:assert/strict";
import { test } from "node:test";

import { ReadingStore, toCsv } from "../js/reading-store.js";

// The part of Dexie's API the store uses, in memory, so the store's logic can be tested
// in Node without IndexedDB.
class FakeTable {
  rows = [];
  #id = 0;
  add(row) { this.rows.push({ ...row, id: ++this.#id }); return Promise.resolve(this.#id); }
  count() { return Promise.resolve(this.rows.length); }
  clear() { this.rows = []; return Promise.resolve(); }
  #sorted(key) { return [...this.rows].sort((a, b) => a[key] - b[key]); }
  where(key) {
    return { aboveOrEqual: (v) => ({ toArray: async () => this.#sorted(key).filter((r) => r[key] >= v) }) };
  }
  orderBy(key) {
    const sorted = () => this.#sorted(key);
    const query = (rows, limit = Infinity) => ({
      reverse: () => query(rows().reverse ? () => [...rows()].reverse() : rows, limit),
      limit: (n) => query(rows, n),
      toArray: async () => rows().slice(0, limit),
      last: async () => rows().at(-1),
    });
    return query(sorted);
  }
}

class FakeDexie {
  constructor(name) { this.name = name; }
  version() {
    return { stores: (schema) => { for (const table of Object.keys(schema)) this[table] = new FakeTable(); } };
  }
  transaction(_mode, ...args) { return args.at(-1)(); }
}

test("readings come back oldest first from a time on, newest first for latest()", async () => {
  const store = new ReadingStore(FakeDexie);
  await store.addReading({ celsius: 21, status: "OK", low: 18, high: 28 }, 3000);
  await store.addReading({ celsius: 20, status: "OK", low: 18, high: 28 }, 1000);
  await store.addReading({ celsius: 30, status: "HIGH", low: 18, high: 28 }, 2000);
  assert.deepEqual((await store.since(2000)).map((r) => r.time), [2000, 3000]);
  assert.deepEqual((await store.latest(2)).map((r) => r.time), [3000, 2000]);
  assert.equal(await store.count(), 3);
});

test("the last confirmed threshold is remembered", async () => {
  const store = new ReadingStore(FakeDexie);
  assert.equal(await store.latestThreshold(), undefined);
  await store.addThresholdChange({ low: 18, high: 28 }, 1);
  await store.addThresholdChange({ low: 15, high: 22 }, 2);
  const last = await store.latestThreshold();
  assert.deepEqual([last.low, last.high], [15, 22]);
});

test("clear deletes readings and threshold changes", async () => {
  const store = new ReadingStore(FakeDexie);
  await store.addReading({ celsius: 21, status: "OK", low: 18, high: 28 }, 1);
  await store.addThresholdChange({ low: 18, high: 28 }, 1);
  await store.clear();
  assert.equal(await store.count(), 0);
  assert.equal(await store.latestThreshold(), undefined);
});

test("CSV export: header, ISO times, empty temperature for a sensor error", async () => {
  assert.equal(toCsv([
    { time: Date.UTC(2026, 9, 20, 12, 0, 0), celsius: 23.4, status: "OK", low: 18, high: 28 },
    { time: Date.UTC(2026, 9, 20, 12, 0, 2), celsius: null, status: "ERROR", low: 18, high: 28 },
  ]), "time,celsius,status,low,high\n"
    + "2026-10-20T12:00:00.000Z,23.4,OK,18,28\n"
    + "2026-10-20T12:00:02.000Z,,ERROR,18,28\n");
  const store = new ReadingStore(FakeDexie);
  assert.equal(await store.exportCsv(), "time,celsius,status,low,high\n");
});
