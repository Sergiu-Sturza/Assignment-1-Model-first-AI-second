# Cabin monitor: implementation specification

**Date:** 2026-10-13 · **Scope:** the drawings in [`project/diagrams/input/`](../../diagrams/input/) as of commit `b48a5ad` · **Status:** final, implemented and verified (results in [§9](#9-verification-results)) · **Builds on:** the exploratory [v3 analysis](../../../research/2026-10-13-cabin-monitor-input-set-analysis-v3.md) and the [ADRs](../adr/README.md)

This document says **what was built and why**:

- the design, element by element, as the verifier reads it;
- the C++ construct each element maps to;
- the state machine and the scenario;
- every assumption that the drawings do not state.

It has the same section style as the v3 analysis, so the two can be read side by side. The
[implementation report](../../../research/2026-10-13-cabin-monitor-implementation-report.md)
assesses the result.

---

## 1. Summary

**The system.** A user owns cabins, and each cabin owns temperature, power and security sensors.
One call to `Cabin::checkSensors()` is one measurement interval:

1. the cabin reads each sensor;
2. it asks the sensor whether the reading must raise an alert;
3. it creates one `Alert` per positive answer;
4. it returns the alerts.

The caller delivers them to the `User` with `addAlert`.

**Since the v3 analysis, the team fixed every drawing conflict v3 raised** (commits `fe7004e`,
`6b8ec97`, `b48a5ad`):

| v3 item | Now |
|---|---|
| C1: `outOfRange()`/`inRange()` drawn returning `void` | Drawn returning `bool` ✔ |
| C2: `lastCheck: Flag`, with `Flag` undefined | `lastCheck: bool` ✔. The `Flag` alias planned in v3 is dropped |
| C3: `Sequence_diagram.drawio` not picked up by the verifier | Renamed to `sequence-check_sensors.drawio` ✔ |
| G1: no way to give an alert to a `User` | `+ addAlert(a: Alert): void` drawn ✔ |

As a result, **every drawn element can be implemented literally, with no deviation**. The target
is 100 % on all three reports.

| Design | Elements | Target |
|---|---|---|
| `class.drawio` | 9 classes, 21 attributes and literals, 39 methods, 8 relations (77 in all) | 100 % |
| `state-cabin.drawio` | 3 states, 4 events, 1 initial state, 6 transitions (14 in all) | 100 % |
| `sequence-check_sensors.drawio` | 7 calls | 100 % |
| `activity.drawio`, `usecase.drawio` | No verifier flow; used as requirements (§7) | — |

---

## 2. Project layout

```
project/impl/
  CMakeLists.txt                 C++20, compile_commands, static lib "cabin", -Wall -Wextra,
                                 demo, scenario_check_sensors, option CABIN_BUILD_TESTS (OFF)
  include/cabin/                 compared with the class and state diagrams
    severity.hpp  date_time.hpp  reading.hpp  alert.hpp  sensor.hpp
    temperature_sensor.hpp  power_meter.hpp  security_sensor.hpp  cabin.hpp  user.hpp
    hardware.hpp                 free-function seam for values and time (ADR-0006)
  src/                           one .cpp per class, plus hardware.cpp and demo.cpp
    reading_history.hpp          internal helper (not under include/, so it is not compared)
  scenarios/check_sensors.cpp    the sequence-diagram scenario
  tests/                         doctest unit tests, built only with -DCABIN_BUILD_TESTS=ON
```

Everything is in `namespace cabin`; the seam is in `cabin::hardware`.

---

## 3. Class diagram → C++

### 3.1 Classifiers

| Design | C++ | Mapping rule |
|---|---|---|
| `Sensor` «abstract» | `class Sensor` with `virtual Reading read() = 0`, `virtual bool shouldAlert(const Reading&) const = 0`, data members and a virtual destructor | Abstract means a pure virtual method plus data |
| `TemperatureSensor`, `PowerMeter`, `SecuritySensor` | `class X : public Sensor` | Generalization |
| `Cabin`, `User`, `Alert`, `Reading` | Plain classes | — |
| `Severity` «enumeration» `LOW, MEDIUM, HIGH` | `enum class Severity { LOW, MEDIUM, HIGH }` | — |

### 3.2 Members

Visibility is as drawn: `-` is `private:`, `+` is `public:`. Private data members carry a
trailing `_`.

| Class | Drawn | C++ declaration |
|---|---|---|
| Sensor | `- id: int`, `- name: String` | `int id_; std::string name_;` |
| | `+ getId(): int`, `+ getName(): String` | `int getId() const; const std::string& getName() const;` |
| | `+ read(): Reading` *(abstract)* | `virtual Reading read() = 0;` |
| | `+ shouldAlert(r: Reading): bool` *(abstract)* | `virtual bool shouldAlert(const Reading& r) const = 0;` |
| | `+ getReadings(): Reading[]` | `const std::vector<Reading>& getReadings() const;` |
| TemperatureSensor | `- minThreshold: double`, `- maxThreshold: double` | `double minThreshold_; double maxThreshold_;` |
| | `+ read()`, `+ shouldAlert(r)` | `Reading read() override; bool shouldAlert(const Reading& r) const override;` |
| | `+ setThreshold(min: double, max: double): void` | `void setThreshold(double min, double max);` |
| PowerMeter | `- threshold: double` | `double threshold_;` |
| | `+ read()`, `+ shouldAlert(r)`, `+ setThreshold(t: double): void` | the same shape, `void setThreshold(double t);` |
| SecuritySensor | `+ read()`, `+ shouldAlert(r)` | the same shape |
| Reading | `- time: DateTime`, `- value: double` | `DateTime time_; double value_;` |
| | `+ getTime(): DateTime`, `+ getValue(): double` | `DateTime getTime() const; double getValue() const;` |
| Alert | `- id: int`, `- time: DateTime`, `- severity: Severity`, `- message: String` | `int id_; DateTime time_; Severity severity_; std::string message_;` |
| | Four getters | `int getId() const; DateTime getTime() const; Severity getSeverity() const; const std::string& getMessage() const;` |
| User | `- id: int`, `- name: String`, `+ getId()`, `+ getName()` | As for Sensor |
| | `+ addCabin(c: Cabin): void` | `void addCabin(std::unique_ptr<Cabin> c);` (a top-level `unique_ptr<T>` normalizes to `T`) |
| | `+ removeCabin(id: int): void` | `void removeCabin(int id);` |
| | `+ addAlert(a: Alert): void` | `void addAlert(const Alert& a);` |
| | `+ getCabins(): Cabin[]` | `std::vector<Cabin*> getCabins() const;` |
| | `+ getAlerts(): Alert[]` | `const std::vector<Alert>& getAlerts() const;` |
| Cabin | `- id`, `- name`, `- location: String` | `int id_; std::string name_; std::string location_;` |
| | `- state: State`, `- lastCheck: bool` | `State state_ = State::Unconfigured; bool lastCheck_ = false;` |
| | `+ getId()`, `+ getName()`, `+ getLocation()` | const getters |
| | `+ addSensor(s: Sensor)`, `+ removeSensor(id: int)` | `void addSensor(std::unique_ptr<Sensor> s); void removeSensor(int id);` |
| | `+ getSensors(): Sensor[]` | `std::vector<Sensor*> getSensors() const;` |
| | `+ checkSensors(): Alert[]` | `std::vector<Alert> checkSensors();` |
| | `+ handle(event: Event): void`, `+ state(): State` | `void handle(Event event); State state() const;` |
| | `- hasSensors()`, `- noSensors()`, `- outOfRange()`, `- inRange()`: `bool` | `bool … () const;` (private guards) |

### 3.3 Relations

| Design | C++ member | Note |
|---|---|---|
| `Cabin *-- "*" Sensor : sensors` | `std::vector<std::unique_ptr<Sensor>> sensors_;` | Polymorphic, so `unique_ptr` (ADR-0003) |
| `User *-- "*" Cabin : cabins` | `std::vector<std::unique_ptr<Cabin>> cabins_;` | Keeps the addresses that `getCabins()` hands out stable |
| `User *-- "*" Alert : alerts` | `std::vector<Alert> alerts_;` | By value |
| `Sensor *-- "*" Reading : readings` | `protected: std::vector<Reading> readings_;` | Protected, so the subclasses' `read()` can append; relations carry no visibility in the comparison |
| `Cabin ..> Alert` | No member; `Alert` appears only in the return type of `checkSensors()` | Dependency |
| `Sensor <\|-- TemperatureSensor / PowerMeter / SecuritySensor` | Public inheritance | — |

### 3.4 Types that are not classes

| Name | C++ | Why |
|---|---|---|
| `DateTime` | `using DateTime = std::chrono::sys_seconds;` in `date_time.hpp` | A type alias is not a class, so it is not *Extra*. UTC, whole seconds (A1) |
| `String` | `std::string` | Type names compare case-insensitively |
| `State`, `Event` | Nested in `Cabin` | They belong to the state diagram (§4) |

### 3.5 Constructors (not in the diagram, by convention)

| Class | Constructor |
|---|---|
| `Reading` | `Reading(DateTime time, double value)` |
| `Alert` | `Alert(int id, DateTime time, Severity severity, std::string message)` |
| `TemperatureSensor` | `TemperatureSensor(int id, std::string name, double minThreshold = 5.0, double maxThreshold = 30.0)` |
| `PowerMeter` | `PowerMeter(int id, std::string name, double threshold = 3.0)` |
| `SecuritySensor` | `SecuritySensor(int id, std::string name)` |
| `Cabin` | `Cabin(int id, std::string name, std::string location)`; non-copyable, non-movable |
| `User` | `User(int id, std::string name)` |

`Sensor` is non-copyable, to prevent slicing.

---

## 4. State machine `state-cabin.drawio` → `Cabin`

The pattern is copied from [door.hpp](../../../examples/3-state-simple/impl/include/home/door.hpp)
(ADR-0004).

```cpp
enum class State { Unconfigured, Monitoring, AlertingUser };
enum class Event { addSensor, LastSensorRemoved, MeasurementIntervalElapsed, AlertSent };
State state_ = State::Unconfigured;                         // [*] --> Unconfigured
static constexpr Transition transitions[] = {
  {Unconfigured, addSensor,                  Monitoring,   &Cabin::hasSensors, nullptr},
  {Monitoring,   LastSensorRemoved,          Unconfigured, nullptr,            nullptr},
  {Monitoring,   MeasurementIntervalElapsed, AlertingUser, &Cabin::outOfRange, nullptr},
  {Monitoring,   MeasurementIntervalElapsed, Monitoring,   &Cabin::inRange,    nullptr},
  {AlertingUser, AlertSent,                  Monitoring,   nullptr,            nullptr},
  {AlertingUser, LastSensorRemoved,          Unconfigured, nullptr,            nullptr},
};
```

The arrow `Deleted cabin` → end symbol has no C++ counterpart; the verifier skips it, which is
expected. Destroying a cabin is `User::removeCabin(id)`.

| Guard | Definition |
|---|---|
| `hasSensors()` | `!sensors_.empty()` |
| `noSensors()` | `sensors_.empty()`. No transition uses it; `removeSensor` uses it to decide whether to fire `LastSensorRemoved` |
| `outOfRange()` | `lastCheck_` |
| `inRange()` | `!lastCheck_` |

**Who fires which event** (ADR-0005):

| Event | Fired by |
|---|---|
| `addSensor` | `addSensor()`, after storing the sensor |
| `LastSensorRemoved` | `removeSensor()`, after erasing, if `noSensors()` |
| `MeasurementIntervalElapsed` | `checkSensors()`, after setting `lastCheck_` |
| `AlertSent` | `checkSensors()`, right after that, if any alert was produced |

`handle()` takes the first row that matches the state and event and whose guard holds. An event
with no row is ignored.

---

## 5. Sequence diagram `sequence-check_sensors.drawio` → `scenarios/check_sensors.cpp`

| # | Drawn message | Made by |
|---|---|---|
| 1 | User → Cabin : `checkSensors()` | `scenario()` calls `cabin.checkSensors()` |
| 2 | Cabin → TemperatureSensor : `read()` | `sensor->read()` in `checkSensors()` (virtual, so the receiver is the dynamic class) |
| 3 | Cabin → TemperatureSensor : `shouldAlert(r)` | `sensor->shouldAlert(r)` |
| 4–5 | Cabin → PowerMeter : `read()`, `shouldAlert(r)` | the same loop, second sensor |
| 6–7 | Cabin → SecuritySensor : `read()`, `shouldAlert(r)` | the same loop, third sensor |

The return arrows (`reading`, `true`, `alerts`) are documentation and are not compared.

**Setup in `main()`, which is not in the diagram:**

- creates a `Cabin` with the three sensors, in the drawn order;
- sets the seam sampler to out-of-range values (40 °C, 10 kWh, 1), so that every `shouldAlert` is
  `true`, as drawn.

**How the trace is kept exact.** The traced build records every call between project classes,
including inline getters. The code on the drawn path therefore makes no other inter-class calls
(ADR-0011):

| Code | Why it adds no message |
|---|---|
| `read()` | Reads the inherited `id_` through friendship and appends to `readings_` |
| `shouldAlert()` | Reads `r.value_` through friendship |
| `checkSensors()` | Builds each `Alert` (a constructor) from `name_`, `value_`, `time_` and the thresholds, through friendship |
| `handle()` and the guards | Calls of `Cabin` to itself, which are not counted |
| `hardware::sample` / `hardware::now` | Free functions, which are never messages |

---

## 6. Behaviour the drawings leave open: assumptions

Every item here is an assumption, **not** drawn in the inputs. None changes a drawn element.

| # | Assumption | Source / decision |
|---|---|---|
| A1 | `DateTime` = `std::chrono::sys_seconds` (UTC, whole seconds) | v3 A1 |
| A2 | Units: temperature in °C; power in kWh used in the interval; security `0` = quiet, anything else = movement or door open | v3 A2, activity diagram |
| A3 | Alert rules, with **exclusive** limits (a value exactly at a limit does not alert): temperature `< min` or `> max`; power `> threshold` (upper limit only); security `!= 0` | v3 A3, activity diagram |
| A4 | Default thresholds: 5 °C … 30 °C, 3 kWh | v3 A4 (placeholders) |
| A5 | Severity: intrusion **HIGH**; temperature below 0 °C **HIGH**, other temperature alerts **MEDIUM**; consumption **LOW** | v3 A5 |
| A6 | `Alert.id` is per cabin, 1, 2, 3, … in creation order. The counter is file-local in `cabin.cpp`, keyed by the cabin's address and erased in `~Cabin` (there is no drawn attribute for it) | v3 A6 / G7 |
| A7 | `Alert.message` = `"<sensor name>: <kind> (<value><unit>)"`, where the kind is one of `temperature high`, `temperature low`, `consumption`, `intrusion` (from the activity diagram) | v3 A7 |
| A8 | `removeSensor`/`removeCabin` with an unknown id does nothing; `removeCabin` works in any state | ADR-0008 |
| A9 | `std::invalid_argument` for a null pointer or duplicate id in `addSensor`/`addCabin`, and for `setThreshold(min, max)` with `min > max` | ADR-0008 |
| A10 | Alert delivery: `Cabin` has no relation to `User`, so the **caller** of `checkSensors()` passes each alert to `User::addAlert` | **ADR-0010** (new) |
| A11 | The sequence actor `User` is the person, not the `User` class | v3 A11 |
| A12 | One `checkSensors()` call is one measurement interval. `MeasurementIntervalElapsed` and `AlertSent` are fired inside it | ADR-0005 |
| A13 | Reading history is bounded to **1 000 per sensor**; the oldest reading is dropped first | ADR-0009, accepted with the user on 2026-10-13 |
| A14 | `checkSensors()` on a cabin with no sensors returns no alerts. The event is ignored in `Unconfigured` | As drawn (no row) |
| A15 | Values and time come from the `cabin::hardware` seam. The default sampler returns `0.0` for every sensor ("no hardware attached"). The default clock starts at 2026-01-01T00:00:00Z and advances one minute per call | ADR-0006 |
| A16 | The friend list on the drawn call path is `Reading` → {3 subclasses, `Cabin`}, `Sensor` → {3 subclasses, `Cabin`}, `TemperatureSensor` → {`Cabin`} | **ADR-0011** (new, supersedes 0007) |

---

## 7. Requirements from the activity and use-case diagrams

| Requirement | Where it is met |
|---|---|
| Read the temperature, power and security sensors in that order | `checkSensors()` iterates the sensors in insertion order; the scenario adds them T, P, S |
| High or low temperature warning | `TemperatureSensor::shouldAlert`, and the message kind (A7) |
| Consumption warning | `PowerMeter::shouldAlert` |
| Intrusion warning | `SecuritySensor::shouldAlert` |
| Return the alerts to the user | Return value of `checkSensors()`, then `User::addAlert` (A10) |
| Manage sensors | `Cabin::addSensor/removeSensor/getSensors` |
| View sensor values | `Sensor::getReadings`, `Reading` getters |
| Configure threshold limits | `TemperatureSensor::setThreshold`, `PowerMeter::setThreshold` |
| Monitor system metrics | `checkSensors()` and the state machine |
| View dashboard | `User::getCabins/getAlerts` (no UI) |
| Log in, send notification | **Not implemented.** No operation for them is drawn in the class diagram |

---

## 8. Open items for the team

These are not needed for 100 %, but they would make the drawings complete.

1. Draw `- nextAlertId: int` on `Cabin`, so that the id counter is a real attribute (A6).
2. Draw `Deleted cabin` from every state, or say that a cabin with sensors cannot be deleted.
3. Confirm the threshold defaults (A4) and the severities (A5), or draw an `AlertKind`.
4. Confirm the history bound of 1 000 (A13).
5. Decide whether login and notification are in scope.
6. Use-case drawing: write `«extend»` with guillemets, and reverse the extend arrow (v3 U-1, U-2).

---

## 9. Verification results

Verified on 2026-10-13 with `tools/verify.py project`. Detail is in the
[implementation report](../../../research/2026-10-13-cabin-monitor-implementation-report.md).

| Report | Alignment | Identical | Changed | Missing | Extra |
|---|---:|---:|---:|---:|---:|
| `class-report.md` | **93.5 %** | 72 / 77 | 5 | 0 | 0 |
| `state-cabin-report.md` | **100 %** | 14 / 14 | 0 | 0 | 0 |
| `sequence-check_sensors-report.md` | **100 %** | 12 / 12 (5 lifelines, 7 calls) | 0 | 0 | 0 |

**The 5 *Changed* entries are a verifier defect, not a deviation from the model.** They are the
return types `Alert[]`, `Sensor[]`, `Reading[]`, `Alert[]` and `Cabin[]` of `checkSensors`,
`getSensors`, `getReadings`, `getAlerts` and `getCabins`.

- **What should happen.** The mapping document,
  [UML-CPP-MAPPING.md line 130](../../../tools/umlverify/docs/UML-CPP-MAPPING.md), says
  `std::vector<T>` and `T[]` both normalize to `list<T>`.
- **What happens.** The comparator `elements.ntype()` normalizes `Reading[]` to `reading`, and
  `std::vector<Reading>` to `listreading`.
- **Consequence.** No C++ collection can match a drawn `T[]`. The code uses `std::vector<T>`, as
  the mapping prescribes.
- **What would remove them.** Either a one-line fix in the verifier, or the team drawing `vector<T>`.

**Other results:**
- 0 compiler warnings in the verifier's normal and traced builds (`-Wall -Wextra`).
- 47 unit-test cases and 147 assertions pass, with `-Wall -Wextra -Wpedantic -Werror`.
- Coverage of `src/`: lines 97 % (159 / 163); decisions 97 % (39 / 40, the miss is the action call that cannot be reached because no action is drawn); raw branches 70 %.
- The demo and the scenario exit with code 0.
