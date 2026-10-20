import assert from "node:assert/strict";
import { test } from "node:test";

import { formatGetThreshold, formatSetThreshold, parseDeviceLine, thresholdProblem } from "../js/protocol.js";

// Lines exactly as the device writes them (asserted in impl/tests/test_message_formatter.cpp).
test("readings, as the firmware formats them", () => {
  assert.deepEqual(parseDeviceLine("T,23.4,OK,18.0,28.0"),
    { type: "reading", celsius: 23.4, status: "OK", low: 18, high: 28 });
  assert.deepEqual(parseDeviceLine("T,-3.3,LOW,18.0,28.0"),
    { type: "reading", celsius: -3.3, status: "LOW", low: 18, high: 28 });
  assert.deepEqual(parseDeviceLine("T,31.6,HIGH,18.0,28.0\r"),
    { type: "reading", celsius: 31.6, status: "HIGH", low: 18, high: 28 });
});

test("a failed reading has no temperature", () => {
  assert.deepEqual(parseDeviceLine("T,,ERROR,5.0,9.5"),
    { type: "reading", celsius: null, status: "ERROR", low: 5, high: 9.5 });
});

test("threshold and rejection", () => {
  assert.deepEqual(parseDeviceLine("TH,-12.5,40.0"), { type: "threshold", low: -12.5, high: 40 });
  assert.deepEqual(parseDeviceLine("ERR,INVALID"), { type: "rejected", reason: "INVALID" });
});

test("anything malformed is unknown, never a wrong reading", () => {
  for (const line of ["", "Starting...", "T,23.4,OK,18.0", "T,abc,OK,18,28", "T,23.4,WARM,18,28",
    "T,23.4,ERROR,18,28", "T,,OK,18,28", "T,1e3,OK,18,28", "TH,18", "TH,a,b", "ERR,", "ERR"]) {
    assert.equal(parseDeviceLine(line).type, "unknown", line);
  }
});

test("thresholdProblem follows Threshold::set on the device", () => {
  assert.equal(thresholdProblem(18, 28), null);
  assert.equal(thresholdProblem(-40, 125), null);
  assert.match(thresholdProblem(28, 18), /below/);
  assert.match(thresholdProblem(20, 20), /below/);
  assert.match(thresholdProblem(-41, 20), /between/);
  assert.match(thresholdProblem(20, 126), /between/);
  assert.match(thresholdProblem(NaN, 20), /number/);
});

test("commands for the device", () => {
  assert.equal(formatSetThreshold(20, 30.25), "SET,20.0,30.3\n");
  assert.equal(formatSetThreshold(-5.5, 0), "SET,-5.5,0.0\n");
  assert.equal(formatGetThreshold(), "GET\n");
  assert.throws(() => formatSetThreshold(30, 20), RangeError);
});

test("the longest SET fits the device's 31-character line", () => {
  assert.ok(formatSetThreshold(-39.9, 124.9).trimEnd().length <= 31);
});
