# Temperature alarm — specification

What to build, from the design in [`model/`](../../model/) (drawn into
[`diagrams/input/`](../../diagrams/input/)), and the C++ construct each element maps to
([UML-CPP-MAPPING.md](../../../tools/umlverify/docs/UML-CPP-MAPPING.md)).

## Requirements

1. Read a temperature sensor (DHT11) every 2 s.
2. Sound a buzzer while the temperature is below the low or above the high limit.
3. Show the temperature and its status on a 16×2 I2C LCD.
4. Send every reading to a website over WebSerial; the website stores it with Dexie (IndexedDB).
5. The website can change the threshold; the device confirms or rejects.
6. One responsibility per class (single responsibility principle).
7. Three buttons on the board change the threshold (added 2026-10-27): D5 selects the low or
   the high limit, D4 raises it by 0.1 °C, D3 lowers it by 0.1 °C. Each press shows the
   threshold on the LCD; each change is sent to the website as `TH,<low>,<high>`.
8. The board is an Arduino Mega (I2C on D20/D21).
9. The same C++ runs on the Arduino (avr-gcc, C++11, 2 KB RAM, no heap) and on the host
   (verifier, tests, benchmarks).

## Classes

| Class | Responsibility | Members | Relations → C++ |
|---|---|---|---|
| `TemperatureMonitor` | one measurement cycle; the alarm state machine | `- state`, `+ update(nowMs)`, `+ handle(event)`, `+ state()`, `- startAlarm()`, `- stopAlarm()` | `-->` sensor, threshold, buzzer, display, link: **references**; `*--` timer, formatter: **by-value members** |
| `CommandHandler` | carry out website commands | `+ poll()` | `-->` link, threshold: references; `*--` parser, formatter: by value |
| `ThresholdEditor` | carry out the board's buttons | `- selected: ThresholdLimit`, `+ poll(nowMs)` | `*--` selectButton, upButton, downButton: by-value `Button`s; `-->` threshold, display, link: references; `*--` formatter: by value |
| `Button` | one debounced push button (50 ms) | `- pin`, `- down`, `- changedMs`, `+ wasPressed(nowMs): bool` | — |
| `TemperatureSensor` | read °C (NaN on failure) | `- pin`, `+ read()` | — |
| `Threshold` | hold [low, high]; classify a value; validate changes | `- low`, `- high`, getters, `+ set(low, high): bool`, `+ classify(celsius)` | — |
| `Buzzer` | drive the buzzer pin | `- pin`, `- on`, `turnOn/turnOff/isOn` | — |
| `LcdDisplay` | the two LCD rows | `- columns`, `showTemperature`, `showSensorError`, `showThreshold(threshold, selected)` | — |
| `SerialLink` | assemble received lines, send lines | `- line: LineText`, `- length`, `- discarding`, `readLine(): Text`, `writeLine(Text)` | — |
| `IntervalTimer` | say when a measurement is due (wrap-safe) | `- intervalMs`, `- lastMs`, `- started`, `isDue(nowMs)` | — |
| `MessageFormatter` | write outgoing lines | `- text: MessageText`, `formatReading/formatThreshold/formatRejected` | `..>` Threshold (signature only) |
| `CommandParser` | read incoming lines | `parse(Text): Command` | `..> Command` |
| `Command` | one parsed command | `+ kind`, `+ low`, `+ high` | — |
| `TemperatureStatus`, `CommandKind`, `ThresholdLimit` | `<<enumeration>>` | → `enum class` | — |

`Text`, `LineText`, `MessageText` are typedefs (`const char*`, `char[32]`, `char[40]`), not
classes. The hardware is reached through free functions in `hal.hpp` (`isButtonDown` added
for the buttons) (not classes, so not in
the diagrams): `hal_arduino.cpp` on the board, `hal_host.cpp` (a simulation) on the host.

## State machine — `TemperatureMonitor`

States `Normal` (initial), `Alarming`, `SensorFault`; events from the measurement's status
(`Ok` → `TemperatureOk`, `TooLow`/`TooHigh` → `TemperatureOutOfRange`, `SensorError` →
`SensorFailed`). Six transitions, actions `startAlarm` / `stopAlarm`, no guards → one row each
in `static constexpr Transition transitions[]`.

## Scenarios

- `measure_temperature` — `update` → `isDue`, `read`, `classify`, self `handle`, `turnOn`,
  `showTemperature`, `formatReading` (→ `getLow`, `getHigh`), `writeLine`.
- `change_threshold` — `poll` → `readLine`, `parse`, `set`, `formatThreshold`
  (→ `getLow`, `getHigh`), `writeLine`.
- `adjust_threshold` — `poll(nowMs)` → `wasPressed` ×3 (select, up, down), `getLow`, `getHigh`,
  `set`, `showThreshold` (→ `getLow`, `getHigh`), `formatThreshold` (→ `getLow`, `getHigh`),
  `writeLine`.

## Protocol (ASCII lines, `\n`-terminated, 115200 baud)

| Direction | Line |
|---|---|
| device → web | `T,<celsius>,<OK\|LOW\|HIGH\|ERROR>,<low>,<high>` (celsius empty on ERROR) |
| device → web | `TH,<low>,<high>` · `ERR,INVALID` |
| web → device | `SET,<low>,<high>` · `GET` |

Numbers: optional `-`, digits, optional `.` and digits. Valid threshold: −40 ≤ low < high ≤ 125.
