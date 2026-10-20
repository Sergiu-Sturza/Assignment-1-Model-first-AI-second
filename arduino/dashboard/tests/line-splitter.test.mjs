import assert from "node:assert/strict";
import { test } from "node:test";

import { LineSplitter } from "../js/line-splitter.js";

function collect() {
  const lines = [];
  return { lines, splitter: new LineSplitter((line) => lines.push(line)) };
}

test("lines split across chunks are joined", () => {
  const { lines, splitter } = collect();
  splitter.push("T,23.");
  assert.deepEqual(lines, []);
  splitter.push("4,OK,18.0,28.0\r\nTH,1");
  splitter.push("8.0,28.0\r\n");
  assert.deepEqual(lines, ["T,23.4,OK,18.0,28.0", "TH,18.0,28.0"]);
});

test("empty lines are skipped", () => {
  const { lines, splitter } = collect();
  splitter.push("\r\n\nGET\n\n");
  assert.deepEqual(lines, ["GET"]);
});

test("an endless partial line is dropped instead of growing without limit", () => {
  const { lines, splitter } = collect();
  splitter.push("x".repeat(5000));
  splitter.push("\nOK\n");
  assert.deepEqual(lines, ["OK"]);
});

test("reset forgets a partial line", () => {
  const { lines, splitter } = collect();
  splitter.push("garbage from before");
  splitter.reset();
  splitter.push("TH,1.0,2.0\n");
  assert.deepEqual(lines, ["TH,1.0,2.0"]);
});
