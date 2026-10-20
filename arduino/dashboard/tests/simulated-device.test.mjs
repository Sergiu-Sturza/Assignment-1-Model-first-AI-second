import assert from "node:assert/strict";
import { test } from "node:test";

import { isValidDeviceLine, SimulatedDevice } from "../js/simulated-device.js";

const flush = () => new Promise((resolve) => setTimeout(resolve, 0));

function device(random = () => 0.5) {
  const lines = [];
  const states = [];
  const d = new SimulatedDevice({ onLine: (l) => lines.push(l), onStatus: (s) => states.push(s), random });
  return { d, lines, states };
}

test("connects, measures at once, and speaks valid protocol", async () => {
  const { d, lines, states } = device();
  await d.connect();
  await flush();
  await d.disconnect();
  assert.deepEqual(states, ["connected", "disconnected"]);
  assert.equal(lines.length, 1);
  assert.match(lines[0], /^T,\d+\.\d,OK,18\.0,28\.0$/);
  assert.ok(lines.every(isValidDeviceLine));
});

test("answers SET and GET by the device's rules", async () => {
  const { d, lines } = device();
  await d.send("SET,20.0,30.0\nGET\nSET,30,20\nHELLO\n");
  await flush();
  assert.deepEqual(lines, ["TH,20.0,30.0", "TH,20.0,30.0", "ERR,INVALID", "ERR,INVALID"]);
});

test("a sensor failure is reported like the firmware does", async () => {
  const { d, lines } = device(() => 0.01);
  await d.connect();
  await flush();
  await d.disconnect();
  assert.deepEqual(lines, ["T,,ERROR,18.0,28.0"]);
});
