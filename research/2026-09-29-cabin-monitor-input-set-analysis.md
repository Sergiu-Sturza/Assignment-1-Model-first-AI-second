# Cabin monitor — analysis of the input set

**Date:** 2026-09-29 · **Scope:** the five drawings in [`project/diagrams/input/`](../project/diagrams/input/) · **Status:** exploration complete, implementation not started

This document records what the input set says, where it is ambiguous, contradictory or incomplete,
which assumptions an implementation has to make, and how the planned mock-up (a C++ core compiled
to WebAssembly and published on GitHub Pages) will treat the input set as ground truth.

Every parse result quoted below was produced read-only with the repository's own verifier code
(`umlverify.*.drawio_read.read`), so it is exactly what [`tools/verify.py`](../tools/verify.py)
will compare an implementation against.

---

## 1. Summary

The input set describes a **cabin monitoring system**: a cabin has temperature, power and security
sensors; readings are collected periodically, stored, and checked against limits; an out-of-range
reading raises an alert that is pushed to the cabin owner's mobile app; the owner can log in and view
current values and history.

| Verdict | |
|---|---|
| Class diagram | Implementable. **As drawn, the best achievable alignment is about 78 %** (32 of 41 elements), because of drawing details the verifier reads differently than the authors intended (§4). With six small drawing fixes it reaches 100 %. |
| State machine | Not verifiable as drawn: wrong file name, no owning class, composite state, two of four arrows not attached (§5). |
| Sequence diagram | Parsed, but only 3 of 9 lifelines correspond to classes, messages are prose, and `loop`/`alt` are not supported (§6). |
| Activity / use-case diagram | Not verified by the tool (no flow exists for them). Used as requirements only (§7, §8). |

**Decisions already taken** (2026-09-29):

1. The mock-up is the **C++ core in `project/impl/`, compiled to WebAssembly** in GitHub Actions and
   driving an interactive dashboard on GitHub Pages. One model, the one the verifier checks.
2. The input drawings are **implemented as drawn and not edited**. Each unavoidable deviation is
   documented here (§4) with the fix the team can make later.
3. Components that appear only in the sequence/state diagrams (Cabin Gateway, Backend Server,
   Database, Notification Service, Mobile App, the Setup/Running controller) live in a **mock-up
   layer** outside the verified core.

**Most important open questions** (full list in §14): the limit values and who may change them; the
alert kinds (frost / high consumption / security) versus `Severity`; whether the team will apply the
drawing fixes in §4 so the class diagram can reach 100 %.

---

## 2. Inventory

| File | Diagram | Added by (commit) | Handled by `tools/verify.py`? |
|---|---|---|---|
| [`class.drawio`](../project/diagrams/input/class.drawio) | Class | MaggyBoy (`c19b91e`) | **Yes** — class-diagram flow |
| [`StateMachine.drawio`](../project/diagrams/input/StateMachine.drawio) | State machine | Safe-IV (`bdf819f`) | **No** — the name must be `state-<class>.drawio`; skipped as "no verification exists" |
| [`sequence-monitoring_cycle.drawio`](../project/diagrams/input/sequence-monitoring_cycle.drawio) | Sequence | Sergiu Sturza (`132bf91`) | **Yes by name**, but it cannot match (§6) |
| [`activity_diagram.drawio`](../project/diagrams/input/activity_diagram.drawio) | Activity | Sergiu Sturza (`132bf91`) | **No** — diagram type not supported |
| [`usecase.drawio`](../project/diagrams/input/usecase.drawio) | Use case | Masopp (`21cea39`) | **No** — diagram type not supported |

The sequence and activity drawings were generated with an AI tool (`agent="Claude"` in the file
header); the other three were drawn by hand in the VS Code draw.io editor.

---

## 3. Class diagram as the verifier reads it

### 3.1 Classifiers

| Class | Kind as parsed | Attributes | Methods |
|---|---|---|---|
| `Sensor` | abstract (italic name) | `- id: int`, `- name: String` | `+ read(): Reading` (not abstract) |
| `TermperatureSensor` *(sic)* | class | — | `+ read(): Reading` |
| `PowerMeter` | class | — | `+ read(): Reading` |
| `SecuritySensor` | class | — | `+ read(): Reading` |
| `Reading` | abstract (italic name) | `- time: DateTime`, `- value: double` | — |
| `Alert` | abstract (italic name) | `- id: int`, `- time: DateTime`, `- severity: Severity`, `- message: String` | — |
| `User` | abstract (italic name) | `- id: int`, `- name: String`, `- email: String` | `+ login(): void`, `+ viewData(): void` |
| `Cabin` | abstract (italic name) | `- id: int`, `- name: String`, `- location: String` | — |
| `Severity` | **abstract** (should be enumeration, §4.3) | literals `LOW`, `MEDIUM`, `HIGH` | — |

### 3.2 Relations

| Parsed | Meaning | Planned C++ member |
|---|---|---|
| `Sensor <\|-- TermperatureSensor`, `PowerMeter`, `SecuritySensor` | inheritance | `class PowerMeter : public Sensor` |
| `Cabin o-- "*" Sensor : sensors` | aggregation, many | `std::vector<std::shared_ptr<Sensor>> sensors_;` |
| `Sensor --> "*" Reading : readings` | association, many | `std::vector<Reading*> readings_;` |
| `User --> "*" Cabin` *(no role name, §4.5)* | association, many | `std::vector<Cabin*> cabins_;` |
| `User --> "*" Alert : alerts` | association, many | `std::vector<Alert*> alerts_;` |
| `Reading --> "0..1" Alert : alert` | association, optional | no exact form (§4.4); planned `Alert* alert_ = nullptr;` |

Edges drawn without an arrow style (`edgeStyle=none;html=1;`) default to `endArrow=classic`, which the
verifier reads as an association (`-->`). Only `Cabin → Sensor` carries an explicit hollow diamond.

### 3.3 Type mapping

| Design | C++ | Note |
|---|---|---|
| `String` | `std::string` | types compare case-insensitively, so `String` ≡ `string` |
| `int`, `double` | `int`, `double` | |
| `DateTime` | `using DateTime = std::chrono::sys_seconds;` in a header | an alias, not a class, so it adds no class to the implemented diagram. To confirm on the first verifier run that libclang reports the field type as `DateTime` and not the underlying type. |
| `Severity` | `enum class Severity { LOW, MEDIUM, HIGH };` | enums are always attributes, never relations |

All classes go in one namespace (proposed: `namespace cabin { … }`, include path `include/cabin/`).

---

## 4. Conflicts between the class diagram and the verifier

These are places where the drawing, read mechanically, says something the authors almost certainly
did not mean. Following the decision to implement as drawn, each one costs alignment. Each has a
one-line drawing fix.

### 4.1 Italic class names mean «abstract»

- **Evidence:** the swimlanes for `Sensor`, `Alert`, `User`, `Cabin`, `Reading` and `Severity` all have
  `fontStyle=2` (italic). The reader turns an italic name into `abstract`
  ([drawio_read.py:77-78](../tools/umlverify/class_diagram/drawio_read.py#L77-L78)). This looks like a
  copied class template rather than intent — only `Sensor` is plausibly abstract.
- **Why it cannot be matched:** an abstract class needs at least one pure-virtual method
  ([UML-CPP-MAPPING.md](../tools/umlverify/docs/UML-CPP-MAPPING.md#classifiers)). `Alert`, `Cabin`
  and `Reading` have no methods at all; adding one would be an *Extra*. `User` could make `login()`
  pure virtual, but then the method's `abstract` detail differs and `User` cannot be instantiated.
- **Plan:** implement `Alert`, `Cabin`, `Reading`, `User` as concrete classes. **Cost: 4 Changed.**
- **Fix:** set `fontStyle=0` on those four class names.

### 4.2 `Sensor` is abstract but `read()` is not

- **Evidence:** `Sensor` italic; its `+ read(): Reading` row is not italic.
- **Plan:** `virtual Reading read() = 0;` in `Sensor`, so the class kind matches and the subclasses
  must implement it. The method's `abstract` detail then differs. **Cost: 1 Changed.**
- **Fix:** make the `read()` row in `Sensor` italic.

### 4.3 `Severity` is not read as an enumeration

- **Evidence:** the label is `<<enumeration>>` + newline + `Severity`. The label cleaner strips
  anything shaped like an HTML tag ([core/drawio.py:70](../tools/umlverify/core/drawio.py#L70)), so
  `<<enumeration>` disappears, leaving `>` — the stereotype is lost and the italic font makes the
  class `abstract`. Checked: `lines('<<enumeration>>\nSeverity') == ['>', 'Severity']`.
- **Plan:** `enum class Severity` (the only sensible C++). **Cost: 1 Changed** (kind).
- **Fix:** write the stereotype as `«enumeration»` (guillemets) and drop the italic.

### 4.4 `Reading --> "0..1" Alert` has no C++ form

- **Evidence:** the mapping gives `0..1` only as `std::optional<T>`, which is a *composition*
  ([cpp_extract.py:18-21](../tools/umlverify/class_diagram/cpp_extract.py#L18-L21)). A non-owning
  optional reference (`Alert*` that may be null, or `std::optional<Alert*>`) is read as association
  `1`, or as a plain attribute.
- **Plan:** `Alert* alert_ = nullptr;` — semantically right (a reading refers to the alert it caused,
  if any). **Cost: 1 Changed** (cardinality `1` vs `0..1`).
- **Fix:** either accept the report entry, or change the relation to composition
  (`Reading *-- "0..1" Alert`) if a reading should *own* its alert. Owning is questionable because
  `User` also points at alerts. Question Q6.

### 4.5 The `cabins` role name is at the wrong end

- **Evidence:** on edge `DXa-JFj0sgl4FdKhsy5u-12` (User → Cabin) the label `cabins` sits at
  `x=-0.27`, the User end, so it is read as a near-end label and the relation has no name. A member
  always has a name, and the relation's identity includes it
  ([elements.py:25-27](../tools/umlverify/class_diagram/elements.py#L25-L27)).
- **Plan:** `std::vector<Cabin*> cabins_;`. **Cost: 1 Missing + 1 Extra.**
- **Fix:** drag the `cabins` label to the Cabin end of the edge.

### 4.6 `TermperatureSensor` is misspelled

- **Evidence:** class name `TermperatureSensor`; the sequence diagram says `Temperature Sensor`.
- **Plan:** keep the typo in the code (`class TermperatureSensor`, file `termperature_sensor.hpp`)
  so the class matches; the dashboard shows "Temperature sensor". **Cost: none now**, but every later
  diagram must repeat the typo, or everything is renamed together.
- **Fix:** rename the class in the drawing; the code follows in one commit.

### 4.7 Nobody owns `Cabin`, `Alert` or `Reading`

Not a verifier conflict but a design gap: every relation to these three classes is a plain
association, so in C++ they are referenced by raw pointer and **someone outside the model must own
them**. Only `Sensor` is (co-)owned, by `Cabin`. The plan puts the owning store in the mock-up layer
(the "Database" of the sequence diagram, §10), which keeps the verified headers exactly as drawn. Its
correctness (no dangling `Reading*` after a history trim) is covered by tests. Question Q6.

### 4.8 Expected alignment

The design has 40 elements: 9 classes, 17 attributes and literals, 6 methods, 8 relations.

| | As drawn (decision taken) | After the fixes in 4.1–4.5 |
|---|---:|---:|
| Identical | 32 | 40 |
| Changed | 7 (4.1 ×4, 4.2, 4.3, 4.4) | 0 |
| Missing / Extra | 1 / 1 (4.5) | 0 / 0 |
| **Alignment** | **32 / 41 ≈ 78 %** | **100 %** |

(4.4 stays at 1 Changed unless the team picks composition or accepts the entry.)

---

## 5. State machine — `StateMachine.drawio`

**Drawn:** initial → **Setup**; Setup —`Setup OK`→ **Running**; Setup —`Setup failed`→ *(nothing)*.
Running is a composite state containing initial → **Monitor**; Monitor —`Updated reading [Out of range]`→
**Warn user**; Warn user —`Updated reading [In range]`→ Monitor.

**Parsed:** states `Setup, Running, Monitor, Warn user`, initial `Setup`, **only the two inner
transitions**, plus three warnings: `Setup OK`, `Setup failed` and the inner initial arrow are "not
attached to a state at both ends; skipped".

| # | Issue | Consequence | Proposed interpretation |
|---|---|---|---|
| S1 | File name is not `state-<class>.drawio` and no class in the class diagram owns this machine | Not verified at all | Owned by a mock-up-layer `MonitoringController` (decision 3) |
| S2 | Composite state `Running` | Not part of the mapping ("the verifier warns") | Flatten: `Setup OK` goes to `Monitor` |
| S3 | `Setup OK` ends on the inner initial pseudo-state, not on `Running` | Arrow dropped | Same as S2 |
| S4 | `Setup failed` has no target | Arrow dropped; behaviour unknown | Q3: retry (self-transition on `Setup`) or a terminal `Failed` state. Assumed: terminal `SetupFailed` state, user-visible |
| S5 | No way out of `Running` | Cannot shut down, although the activity diagram has "shutdown" | Q3: add `shutdown` → final from both inner states |
| S6 | `Updated reading [Out of range]` while already in `Warn user` has no row | A second, different out-of-range reading is ignored: no repeated warning | Q4: assume no repeat (the event is ignored, as the mapping says) |
| S7 | Guards `In range` / `Out of range` need limits | Limits are not in the class diagram | Guard methods `inRange()` / `outOfRange()` compare the latest reading with the configured limits (§13, A4) |
| S8 | "Warn user" versus alert creation in the sequence diagram | Unclear whether entering `Warn user` *is* creating the alert | Action `/ raiseAlert` on the Monitor → Warn user row |
| S9 | Event names are phrases (`Setup OK`, `Updated reading`) | Fine: names compare without spaces | Events `SetupOk`, `SetupFailed`, `UpdatedReading` |

The machine is implemented with the table pattern from
[examples/3-state-simple](../examples/3-state-simple/impl/include/home/door.hpp) even though it is
outside the verified core, so it can move into the core unchanged if the team later draws its class
and renames the file.

---

## 6. Sequence diagram — `sequence-monitoring_cycle.drawio`

**Lifelines (9):** Cabin Owner (actor), Mobile App, Backend Server, Database, Notification Service,
Cabin Gateway, Temperature Sensor, Power Meter, Security Sensor.

**Messages (18)**, in order; `loop [every measurement interval]` around 1–12, `alt [reading outside
safe threshold]` around 10–12:

| # | From → To | Message | Kind |
|---|---|---|---|
| 1 | Cabin Gateway → Temperature Sensor | Read temperature | call |
| 2 | Temperature Sensor → Cabin Gateway | Temperature (°C) | return |
| 3 | Cabin Gateway → Power Meter | Read energy consumption | call |
| 4 | Power Meter → Cabin Gateway | Consumption (kWh) | return |
| 5 | Cabin Gateway → Security Sensor | Read security status | call |
| 6 | Security Sensor → Cabin Gateway | Door / motion status | return |
| 7 | Cabin Gateway → Backend Server | Send sensor data | call |
| 8 | Backend Server → Database | Store readings | call |
| 9 | Database → Backend Server | Ack | return |
| 10 | Backend Server → Notification Service | Create alert | call |
| 11 | Notification Service → Mobile App | Push notification | call |
| 12 | Mobile App → Cabin Owner | Show alert | *return (dashed)* |
| 13 | Cabin Owner → Mobile App | Open app / request status | call |
| 14 | Mobile App → Backend Server | Get latest status | call |
| 15 | Backend Server → Database | Query latest readings | call |
| 16 | Database → Backend Server | Readings | return |
| 17 | Backend Server → Mobile App | Status data | return |
| 18 | Mobile App → Cabin Owner | Display temperature, energy, security | return |

| # | Issue | Consequence |
|---|---|---|
| Q-1 | Six of nine lifelines have no class in the class diagram | Those messages can never be recorded as calls between the project's classes |
| Q-2 | `Temperature Sensor`, `Power Meter`, `Security Sensor` ≠ `TermperatureSensor`, `PowerMeter`, `SecuritySensor` | Names compare without spaces, so the last two match; the first does not (typo, §4.6) |
| Q-3 | Messages are prose ("Read temperature"); the classes have one method, `read()` | Message names never equal method names |
| Q-4 | `loop` and `alt` fragments | Not supported: "one run takes one path, so draw one scenario per path" |
| Q-5 | `alt` has one operand and no `else` | The in-range path (no alert) is implicit |
| Q-6 | Messages 13–18 are a second, independent interaction (the owner asks for status) | Two scenarios in one drawing |
| Q-7 | "Show alert" (12) is drawn as a return but nothing called the app from the owner | A notification to the user, not a return |
| Q-8 | The actor is "Cabin Owner"; the class and use-case actor is `User` | Assumed the same person |
| Q-9 | The Backend both stores readings and decides about alerts, but the activity diagram does both checks after storage and the state machine does them in `Monitor` | Three diagrams place the threshold check in three different components |

**Plan (decision 3):** the drawing is the reference for the mock-up layer's behaviour, not a
verified scenario. The dashboard replays it step by step (§10). If the team later wants it verified,
the drawing should be split into `sequence-monitoring_cycle_alert.drawio` (messages 1–12, the alert
path, no fragments) and `sequence-request_status.drawio` (13–18), with lifelines named after classes
and messages named after methods, e.g.:

| Drawn message | Method name it would need |
|---|---|
| Read temperature / energy / security status | `read()` on each sensor |
| Send sensor data | `CabinGateway → BackendServer : receiveReadings(readings)` |
| Store readings | `Database : storeReadings(readings)` |
| Create alert | `NotificationService : createAlert(alert)` |
| Push notification | `MobileApp : pushNotification(alert)` |
| Get latest status / Query latest readings | `BackendServer : getLatestStatus(cabinId)`, `Database : queryLatestReadings(cabinId)` |

---

## 7. Activity diagram — `activity_diagram.drawio`

**Flow:** start → *Read all sensors: temperature, energy, security* → *Store readings in database* →
fork → three decisions in parallel (*Temperature below frost threshold?*, *Energy use above normal
limit?*, *Security breach detected?*), each *Yes* → generate *frost warning* / *high-consumption
alert* / *security alarm* → join → *Any alert generated?* → Yes: *Send push notification to owner* →
*Wait for next interval* → *System running?* — `yes` loops to the start, `shutdown` ends.

| # | Issue | Assumption / question |
|---|---|---|
| A-1 | The three thresholds have no values and no home in the class diagram | Q1; defaults in §13 A4 |
| A-2 | Alert kinds (frost, high consumption, security) are not in the model; `Alert` has only `severity` and `message` | Kind goes into `message`; `Severity` per kind in §13 A5; Q2 |
| A-3 | The frost check tests *below*, the energy check *above*, security is a boolean — the state machine's single `[Out of range]` hides this | Per-sensor-type limits (min for temperature, max for power, "breach = value ≥ 1" for security) |
| A-4 | Decisions and their *No* branches go straight to the join; a "No" edge into a join is not valid UML (it should pass a merge) | Read as intended: each branch finishes, then join |
| A-5 | Alerts are generated after storage and are not stored themselves | Assumed stored too (`User` has `alerts`, `Reading` has `alert`); Q7 |
| A-6 | The measurement interval is undefined | Q8; the mock-up uses a simulated clock with an adjustable interval |
| A-7 | The final decision's branches are labelled `yes` / `shutdown` for the question "System running?" | Read as running / not running |
| A-8 | One notification per cycle or one per alert? | Assumed one push per alert |

---

## 8. Use-case diagram — `usecase.drawio`

System boundary *Cabin monitor system*; actor **User**; use cases: *log in*, *view sensor values*,
*view data history*, *change limit values*, *administrate sensor*, *monitor metrics*, *send
notification*.

| # | Issue | Consequence |
|---|---|---|
| U-1 | *log in* has no association with the actor | Assumed the User performs it |
| U-2 | Three actor edges end at free points (not attached): they point at *view sensor values*, *view data history*, *change limit values* | Read by position; only *administrate sensor* is really connected |
| U-3 | The extend relation is written `"extended"` (text on the line, straight quotes) instead of `«extend»`, and the arrow runs *monitor metrics* → *send notification* | UML meaning would be "monitor metrics extends send notification"; the intent is surely the reverse (sending a notification extends monitoring when a limit is crossed) |
| U-4 | *send notification* → User is drawn as an association into the actor | Fine: the user receives notifications |
| U-5 | *change limit values* and *administrate sensor* have no operations in the class diagram (`Cabin` has no methods; `User` has only `login()` and `viewData()`) | Cannot be implemented in the verified core without extra methods; implemented in the mock-up layer (Q1, Q9) |
| U-6 | *log in* needs credentials; `User` has no password or token and `login()` takes no arguments and returns nothing | The mock-up simulates login (no real authentication on a static page); Q5 |
| U-7 | *monitor metrics* has no actor | System-initiated (timer), consistent with the activity diagram |
| U-8 | *view data history* versus `viewData()` | Assumed `viewData()` covers both current values and history |

---

## 9. Consistency across the diagrams

✔ defined · ◐ implied / named differently · ✘ absent

| Concept | Class | State | Sequence | Activity | Use case |
|---|:-:|:-:|:-:|:-:|:-:|
| User / Cabin Owner | ✔ `User` | ✘ | ◐ Cabin Owner | ◐ owner | ✔ User |
| Cabin | ✔ | ✘ | ◐ Cabin Gateway | ✘ | ◐ boundary |
| Temperature / power / security sensors | ✔ (typo) | ✘ | ✔ | ✔ | ◐ "sensor" |
| Reading | ✔ | ◐ "Updated reading" | ◐ | ✔ | ◐ "sensor values" |
| Limits / thresholds | ✘ | ◐ guards | ◐ alt guard | ✔ (3 kinds) | ✔ change limit values |
| Alert and its kind | ◐ no kind | ◐ Warn user | ✔ Create alert | ✔ 3 kinds | ◐ |
| Severity | ✔ | ✘ | ✘ | ✘ | ✘ |
| Notification | ✘ | ◐ | ✔ service | ✔ | ✔ |
| Storage / history | ✘ | ✘ | ✔ Database | ✔ | ✔ view history |
| Gateway / backend / app | ✘ | ✘ | ✔ | ✘ | ✘ |
| Login | ✔ `login()` | ✘ | ✘ | ✘ | ✔ (unconnected) |
| Setup / start-up | ✘ | ✔ | ✘ | ✘ | ✘ |
| Sensor administration | ✘ | ✘ | ✘ | ✘ | ✔ |
| Measurement interval | ✘ | ✘ | ✔ loop | ✔ | ✘ |

The class diagram is the smallest of the five: it describes the domain objects but none of the
behaviour the other four diagrams draw. That is why decision 3 (a mock-up layer) is needed.

---

## 10. Proposed architecture

```
project/impl/                         verified core: headers exactly as drawn (§3)
  CMakeLists.txt                      C++20, -Wall -Wextra, static lib "cabin", demo; tests optional
  include/cabin/*.hpp                 sensor, termperature_sensor, power_meter, security_sensor,
                                      reading, alert, user, cabin, severity (+ date_time alias)
  src/*.cpp, src/demo.cpp
  tests/                              GoogleTest (FetchContent); not read by the verifier
mockup/                               mock-up layer (not verified)
  core/                               MonitoringController (state machine §5, table pattern),
                                      CabinGateway, BackendServer, Database (owns Cabins, Readings,
                                      Alerts), NotificationService, Limits, SimulatedClock
  wasm/bindings.cpp                   embind API for the page: step(), setLimits(), injectReading(),
                                      login(), status(), history(), alerts()
  web/                                static dashboard: index.html, app.js, styles.css
  tests/                              GoogleTest for the layer; Playwright smoke test for the page
.github/workflows/
  ci.yml                              build + tests (gcc and clang, warnings as errors),
                                      coverage (gcovr), tools/verify.py project on Linux
  pages.yml                           emsdk build → WASM + web/ + rendered diagrams → GitHub Pages
```

**The page.** Three panels:

1. **Live system** — cabins, sensors and their latest readings; buttons to advance the simulated
   clock one interval, inject an out-of-range reading, change limits, add or remove a sensor; the
   alert feed with severities; the controller's current state (`Setup`, `Monitor`, `Warn user`).
2. **Ground truth** — each input diagram rendered to an image with
   [`tools/render-drawio`](../tools/render-drawio/render.js) and linked to its source, with the
   state being highlighted and the sequence message being replayed as the simulation runs.
3. **Verification** — the class-diagram report and alignment from the CI run, plus the deviations
   in §4 so a reader sees why it is not 100 %.

**Determinism.** Everything runs on a `SimulatedClock` and seeded sensor values, so the page, the
tests and any future scenario program give the same result every time (a requirement of the
sequence verifier).

---

## 11. Quality plan

| Concern | Approach |
|---|---|
| Structure | AGENTS.md layout for `project/impl/`; one class per header; `snake_case` files; one namespace per layer (`cabin`, `cabin::mockup`) |
| Documentation | Doxygen comments on every public member in the headers; a README in `project/impl/` and `mockup/`; specs in `project/process/specs/`, plans in `project/process/plans/` as AGENTS.md requires |
| Unit tests | Every class: construction, getters, `read()` per sensor type, relation wiring (a cabin's sensors, a reading's alert) |
| State machine tests | One test per table row, plus "event with no row is ignored", guard true/false, initial state |
| Scenario tests | The monitoring cycle end to end on the simulated clock: in range → no alert; each of the three out-of-range kinds → one alert, one notification; owner status request returns the latest values |
| Web tests | Playwright smoke test against the built page: loads, WASM initialises, one step changes the readings |
| Coverage | gcovr in CI; target 100 % line coverage of `project/impl/src` and `mockup/core`, reported on the page |
| Warnings | `-Wall -Wextra -Wpedantic -Werror` in CI; the build must be warning-free (AGENTS.md) |
| Performance | Readings kept in contiguous `std::vector`s with a bounded history window per sensor (Q8); O(sensors) per cycle; no allocation in the state-machine step; WASM built with `-O2` |
| Maintainability | Limits and intervals in one `Limits` struct; no magic numbers; the state table is data, so adding a transition is one row |

---

## 12. Environment findings

- **No C++ toolchain, CMake or Node.js on the development PC** (Windows 11). Only Python 3.13 and
  `gh`. Builds, tests, the verifier's C++ extraction (it needs libclang and a compiler) and the
  Emscripten build are therefore planned for **GitHub Actions** (Ubuntu). A devcontainer would give
  the same locally; the README lists it as not yet available.
- **The VS Code task does not work on Windows:** [`.vscode/tasks.json`](../.vscode/tasks.json) calls
  `${workspaceFolder}/.venv/bin/python` and a POSIX shell line; on Windows the interpreter is
  `.venv\Scripts\python.exe`. The workaround is to run the verifier in CI or WSL.
- The repository is **public** and **GitHub Pages is not enabled** yet; `pages.yml` would deploy
  through the "GitHub Actions" Pages source.
- AGENTS.md says to write "in `project/impl/`, and only here". The mock-up layer, workflows and this
  document are outside it, as requested; none of them is compared by the verifier.

---

## 13. Assumptions

| # | Assumption | Why |
|---|---|---|
| A1 | `DateTime` = `std::chrono::sys_seconds` (UTC, whole seconds), declared as an alias | Standard, comparable, no extra class |
| A2 | Reading units: temperature °C, power kWh consumed in the interval, security `0` = secure / `1` = door open or motion | Sequence diagram labels; `value: double` must hold all three |
| A3 | The four italic concrete classes, `Severity`, `0..1` and `cabins` are implemented as in §4 | Decision 2 |
| A4 | Default limits: frost below **5 °C**, power above **3 kWh per interval**, security breach at **value ≥ 1** | Placeholders until Q1 is answered; editable on the page |
| A5 | Severity per kind: security alarm **HIGH**, frost warning **HIGH** below 0 °C and **MEDIUM** below the limit, high consumption **LOW** | Frost and break-ins cause damage; consumption is a cost |
| A6 | The alert kind is written into `Alert::message` | No `kind` attribute in the design |
| A7 | One `User` may have several cabins; each cabin belongs to one user in the mock-up | `User --> "*" Cabin`; near end not drawn |
| A8 | `Cabin Owner` (sequence) = `User` (class, use case) | Same role |
| A9 | Login is simulated (choose a demo user); nothing secret is stored on the static page | GitHub Pages is static and public |
| A10 | Measurement interval defaults to 15 simulated minutes; history keeps the last 7 simulated days per sensor | Needed for determinism and bounded memory |
| A11 | Keep the class name `TermperatureSensor` | Alignment with the drawing (§4.6) |

---

## 14. Open questions for the team

Each has a recommended answer; the implementation uses it until the team decides otherwise.

1. **Limit values** — what are the frost, energy and security limits, per cabin or global, and who
   may change them (*change limit values*)? *Recommended:* per cabin, owner-editable; defaults A4.
2. **Alert kinds** — should `Alert` get a `kind` (frost / consumption / security), or is `message`
   enough? How does `Severity` relate to the kind? *Recommended:* add `- kind: AlertKind` enum; A5.
3. **Setup failure and shutdown** — where does `Setup failed` go, and how does `Running` end?
   *Recommended:* terminal `SetupFailed`; `shutdown` from `Running` to final.
4. **Repeated warnings** — while in `Warn user`, should a new out-of-range reading warn again?
   *Recommended:* no, until the reading is back in range (as drawn).
5. **Authentication** — what does `login()` check? *Recommended:* simulated on the static page.
6. **Ownership** — who owns cabins, readings and alerts? Should `Reading → Alert` be composition?
   *Recommended:* a `Database` in the mock-up layer owns all three; keep associations.
7. **Are alerts stored and shown in history?** *Recommended:* yes.
8. **Interval and history length?** *Recommended:* A10.
9. ***administrate sensor*** — add/remove/rename sensors on a cabin? Which operations should the
   class diagram show? *Recommended:* add and remove, in the mock-up layer.
10. **Will the team apply the drawing fixes in §4** (four italics, `read()` italic, `«enumeration»`,
    `cabins` label, optionally the typo)? That is the only route to 100 %.
11. **Should the state machine and the sequence diagram be verified?** If so: draw the controller
    and the services in the class diagram, rename the state file to `state-<class>.drawio`, and
    split the sequence diagram into fragment-free scenarios (§5, §6).
12. **Test framework** — GoogleTest (recommended) or Catch2?
13. **Pages address** — `https://sergiu-sturza.github.io/My_system_architecure_project/`, deployed
    from `main` by Actions. Acceptable?
