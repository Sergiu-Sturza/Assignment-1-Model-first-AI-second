# Cabin monitor: implementation, verification and code-quality report

**Date:** 2026-10-13 · **Scope:** `project/impl/` against the drawings in [`project/diagrams/input/`](../project/diagrams/input/) as of commit `b48a5ad` · **Spec:** [2026-10-13-cabin-monitor-specification.md](../project/process/specs/2026-10-13-cabin-monitor-specification.md) · **Follows:** [v3 analysis](2026-10-13-cabin-monitor-input-set-analysis-v3.md)

This report covers four things:

- what was verified, and how;
- every failure, deviation and assumption;
- an assessment of the code against common industry practice for C++;
- what is left for the team.

It is written to be read without the code open; file links are given for checking.

---

## 1. Summary

| Check | Result |
|---|---|
| State machine (`state-cabin-report.md`) | ✅ **100 %**: 14 / 14 identical |
| Sequence diagram (`sequence-check_sensors-report.md`) | ✅ **100 %**: 5 lifelines and 7 calls identical, no extra calls |
| Class diagram (`class-report.md`) | ⚠️ **93.5 %**: 72 identical, **5 changed**, 0 missing, 0 extra. All 5 come from a **verifier defect** (§3.1), not from the code |
| Build warnings (`-Wall -Wextra`, normal and traced verifier builds) | ✅ 0 |
| Unit tests (doctest, `-Wall -Wextra -Wpedantic -Werror`) | ✅ 47 / 47 test cases, 147 / 147 assertions |
| Line coverage of `src/` (gcovr, excluding `demo.cpp`) | ✅ 97 % (159 / 163); the missed lines are explained in §2.3 |
| Decision coverage of `src/` (gcovr `--decisions`) | ✅ 97 % (39 / 40); the one miss is the unreachable action check (§2.3) |
| Raw branch coverage of `src/` | ⚠️ 70 % (105 / 148); the misses are compiler-generated exception edges (§2.3) |
| `demo` and `scenario_check_sensors` | ✅ Both run and exit with 0 |
| Activity and use-case diagrams | ➖ No verifier exists for them; they were checked by hand against the requirements in spec §7 |

**Deviations from the model:** none. Every drawn class, attribute, method and relation is implemented
with the drawn name, type, visibility and multiplicity. Nothing undrawn was added to a class:

- no extra method or data member;
- no extra class.

What the C++ adds beyond the drawings is all permitted by AGENTS.md, or invisible to the model:

- constructors and deleted copy operations;
- `friend` declarations;
- the `DateTime` type alias;
- the `cabin::hardware` free functions.

**Each of these is listed in §4.**

---

## 2. Verification

### 2.1 Model alignment

The verifier was run with `python tools/verify.py project`; §6 explains why one environment variable
was also needed.

| Design | In design | Identical | Changed | Missing | Extra | Alignment |
|---|---:|---:|---:|---:|---:|---:|
| `class.drawio` | 77 (9 classes, 21 attributes and literals, 39 methods, 8 relations) | 72 | 5 | 0 | 0 | 93.5 % |
| `state-cabin.drawio` | 14 (3 states, 4 events, initial, 6 transitions) | 14 | 0 | 0 | 0 | 100 % |
| `sequence-check_sensors.drawio` | 12 (5 lifelines, 7 calls) | 12 | 0 | 0 | 0 | 100 % |

**Verifier warnings:**

- **State.** The transition *Deleted cabin* from *Unconfigured* to the end symbol has no C++
  counterpart; it was skipped. This is expected: a terminal state needs no row. Deleting a cabin is
  `User::removeCabin(id)`.
- **Activity, use case.** There is no verification for these diagram types. This is expected; they
  are used as requirements.

**Sequence, "not counted".** Calls `Cabin` makes to itself (`handle`, the guards) are listed as
not counted, as the mapping intends.

### 2.2 Build

| Build | Configuration | Warnings |
|---|---|---|
| Verifier build, `project/build` | GCC 16.2.0 (MSYS2 UCRT64), C++20, `-Wall -Wextra` | 0 (clean rebuild) |
| Verifier traced build, `project/build-trace` | The same, plus `-finstrument-functions` | 0 (clean rebuild) |
| Test build | `-DCABIN_BUILD_TESTS=ON`; tests built with `-Wall -Wextra -Wpedantic -Werror` | 0; the build succeeds under `-Werror` |

### 2.3 Unit tests and coverage

There are 47 doctest cases in 10 files under `project/impl/tests/`, one file per class plus the
state machine and the hardware seam.

| Area | What is tested |
|---|---|
| `Reading`, `Alert` | Construction and every getter |
| `Sensor` (shared) | Id and name; `read()` samples its own id, stamps the time and stores the reading; oldest-first order; **the history bound evicts the oldest at reading 1 001** |
| `TemperatureSensor` | Each limit at, just inside and just outside, for both the default and the constructor limits; `min == max`; `setThreshold` and the constructor reject `min > max`, keeping the old limits |
| `PowerMeter` | At the threshold and just above; `setThreshold`; upper limit only |
| `SecuritySensor` | `0` is quiet; positive and negative values are an intrusion |
| `Cabin` | Getters; sensor order; null and duplicate rejected; remove known and unknown; `checkSensors` with no sensors, everything in range, all three alerting (the drawn scenario: ids, severities, exact messages, times), low and frost temperatures; ids continue across checks and are per cabin; a new cabin at a reused address starts at 1; an unknown `Sensor` subclass gets the generic alert |
| State machine | Initial state; **each of the 6 arrows**; each guard true and false; events with no arrow ignored in each of the 3 states |
| `User` | Getters; cabin order; null and duplicate rejected; remove known (while *Monitoring*) and unknown; alerts delivered with `addAlert` across two checks |
| `hardware` | Default sampler and clock; custom replacements; an empty function restores the default; `reset` |

**Coverage** (gcovr 8 on gcov 16, `src/` only, `demo.cpp` excluded):

- **Lines: 97 %.** Four lines are missed:
  - three are closing braces that gcov attributes to the exception-cleanup paths of
    `Cabin::addSensor`, `Cabin::handle` and `User::getCabins`;
  - one is real: `(this->*t.action)();` in `Cabin::handle`. It is **unreachable by design**: the
    state diagram draws no actions, so every row has `nullptr` there. It is kept because the
    mapping's table pattern requires the action column.
- **Branches: 70 %.** GCC counts the implicit exception edges of every call that can throw
  (allocation, `std::format`) as branches. These cannot be reached without fault injection.
- **Decisions: 97 % (39 / 40)**, measured with gcovr `--decisions`. Every source-level `if`, `?:`
  and `&&`/`||` is taken both ways, except one: `if (t.action != nullptr)` at `cabin.cpp:133`,
  which is the unreachable action check above.

---

## 3. Failures and problems found

### 3.1 Verifier defect: `T[]` is not normalized to `list<T>` (the cause of the 5 *Changed*)

The class report lists these five:

| Element | Design | Implemented |
|---|---|---|
| `Cabin.checkSensors()` return | `Alert[]` | `std::vector<Alert>` |
| `Cabin.getSensors()` return | `Sensor[]` | `std::vector<Sensor*>` |
| `Sensor.getReadings()` return | `Reading[]` | `const std::vector<Reading>&` |
| `User.getAlerts()` return | `Alert[]` | `const std::vector<Alert>&` |
| `User.getCabins()` return | `Cabin[]` | `std::vector<Cabin*>` |

**What should happen.** [UML-CPP-MAPPING.md, line 130](../tools/umlverify/docs/UML-CPP-MAPPING.md)
states that `std::vector<T>` and `T[]` both normalize to `list<T>`.

**What happens.** The comparator
[`elements.ntype()`](../tools/umlverify/class_diagram/elements.py) implements only the
`vector`/`list`/`set` half of that rule. Verified directly:

```
ntype('Reading[]')            -> 'reading'
ntype('std::vector<Reading>') -> 'listreading'
```

So **no C++ type can ever match a drawn `T[]`** as a collection. The only types that would score
*Identical* are a bare `Reading`, `Reading*` or `Reading&`. Those would change the meaning of the
model (one reading instead of many). Using them would hide the defect rather than implement the
design, so the code keeps `std::vector<T>`, as the mapping prescribes.

None of the six examples draws a `T[]`, which is why their reports never exposed the defect.

**Fixes; either one gives 100 %, and neither is mine to make:**
- **Verifier.** In `ntype`, before the `vector|list|set` rule, add
  `t = re.sub(r"^(.+)\[\]$", r"list<\1>", t)`. `tools/` is read-only for this work under
  AGENTS.md.
- **Drawing.** The team writes the five return types as `vector<T>` instead of `T[]`.
  `project/diagrams/input/` must not be edited by the implementer.

### 3.2 Environment: libclang 18 cannot parse GCC 16's standard library

**The failure.** On the first run, all three flows stopped with this message:

> `libclang could not read the headers … bits/stl_iterator.h:1337: member access into incomplete type 'const __normal_iterator<const char *, basic_string<char>>'`

**The cause.**

- The verifier installs the newest MSYS2 GCC, which is now **16.2.0**.
- Its `requirements.txt` pins the pip wheel `libclang>=18`, and **18.1.1 is the newest wheel**.
- clang 18 rejects a `friend operator-` with a trailing return type that is new in GCC 16's
  `<bits/stl_iterator.h>`. Every header that includes `<memory>`, `<chrono>` or `<format>`
  therefore fails.
- Example 1 passes only because it never includes `<memory>`.

**What was tried.**

1. **Pinning MSYS2 to GCC 15.2.** I confirmed first that GCC 15.2's headers parse under
   libclang 18. The downgrade was then refused by pacman, because GCC 16 split `gcc-libs` into
   `libgcc`, `libstdc++` and `cc-libs`, on which cmake and the rest depend. The transaction
   aborted and nothing changed.
2. **The workaround used.** I installed MSYS2's `clang-libs` package (libclang **23.1.3**) and ran
   the verifier with `LIBCLANG_LIBRARY_PATH=C:/msys64/ucrt64/bin`.
   - The pip bindings honour that variable, so `tools/` and `.venv` are unchanged.
   - **Cross-check:** with libclang 23, `verify.py --all` regenerated every example report
     **byte-identical** to the committed ones. That includes examples 2, 4 and 6, with their
     deliberate mistakes. So the substitution does not change any verdict.

**Consequence for the team.** The VS Code task (Ctrl+Shift+B) does not set the variable. On a
machine with GCC 16 it fails the same way until someone does one of these:

- sets `LIBCLANG_LIBRARY_PATH=C:/msys64/ucrt64/bin` in their environment;
- updates the verifier to point libclang at the MSYS2 copy.

### 3.3 Toolchain installer could not prompt

`toolchain.ensure()` asks "Install it now? [Y/n]" with `input()`. In this non-interactive shell
that raised `EOFError`. I ran the two commands it would have run myself:

```
winget install --id MSYS2.MSYS2 -e --silent --accept-package-agreements --accept-source-agreements
C:\msys64\usr\bin\bash.exe -lc "pacman -Sy --noconfirm --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja"
```

There was no effect on the result.

### 3.4 Not performed

| Check | Why |
|---|---|
| AddressSanitizer / UBSan | Not supported by MinGW-w64 GCC on Windows |
| Static analysis (clang-tidy, cppcheck) | Not installed. It would need MSYS2 `clang-tools-extra`; not done, to keep the environment changes minimal |
| CI pipeline | AGENTS.md limits writes to `project/impl/` and `project/process/`; `.github/` is outside both |
| Verification of the activity and use-case diagrams | The verifier has no flow for them |

---

## 4. Everything beyond the drawings

### 4.1 C++ that the drawings do not show

None of this is visible to, or counted by, the verifier.

| Addition | Where | Why |
|---|---|---|
| Constructors with parameters, and default thresholds | Every class | Construction is not drawn, by convention (AGENTS.md rule 2) |
| `= delete` copy operations | `Sensor` (stops slicing), `Cabin` (alert ids are per object) | Safety; operators are not counted |
| `virtual ~Sensor()` | `Sensor` | Required for polymorphic deletion through `unique_ptr<Sensor>`; the mapping expects it |
| `friend` declarations | `Reading`, `Sensor`, `TemperatureSensor` | Keep the trace to the 7 drawn calls ([ADR-0011](../project/process/adr/0011-friend-exception-revised.md)) |
| `using DateTime = std::chrono::sys_seconds` | `date_time.hpp` | The design names `DateTime` but never defines it. An alias adds no class. **Confirmed:** the report shows `DateTime` identical |
| `cabin::hardware` free functions | `hardware.hpp`, `hardware.cpp` | `read()` takes no input, and scenarios must be deterministic ([ADR-0006](../project/process/adr/0006-hardware-seam.md)) |
| `detail::takeReading` | `src/reading_history.hpp` (not in `include/`) | The shared body of the three `read()` methods; the history bound |
| A file-local alert-id map | `src/cabin.cpp` | `Alert.id` needs a source, and no counter attribute is drawn |
| A `describe` lambda | `Cabin::checkSensors` | Severity and message text without an undrawn helper method |

### 4.2 Behavioural assumptions

The full table is spec §6; these are the ones that matter most for a reviewer.

| # | Assumption | Basis |
|---|---|---|
| A1 | `DateTime` is UTC, in whole seconds | Not specified anywhere |
| A2 | Units: °C, kWh used per interval; security `0` means quiet | Activity diagram wording |
| A3 | Limits are exclusive; power has an upper limit only; security alerts on any value `!= 0` | Activity diagram; `PowerMeter` has a single `threshold` |
| A4 | Defaults: 5–30 °C and 3 kWh | **Placeholders**; there is no source |
| A5 | Severity: intrusion HIGH, temperature below 0 °C HIGH, other temperature MEDIUM, consumption LOW | **Invented**; there is no source |
| A6 | Alert ids are per cabin, starting at 1 | No source |
| A7 | Message format `"<sensor>: <kind> (<value><unit>)"`, with the kinds taken from the activity diagram | No source for the format |
| A8–A9 | Unknown ids are a no-op; null pointers, duplicate ids and `min > max` throw `std::invalid_argument` | [ADR-0008](../project/process/adr/0008-error-policy.md) |
| A10 | The caller of `checkSensors()` delivers alerts with `User::addAlert` | [ADR-0010](../project/process/adr/0010-alert-delivery-by-caller.md); the drawings have no `Cabin → User` link |
| A12 | One `checkSensors()` is one measurement interval, and it fires `MeasurementIntervalElapsed` and `AlertSent` itself | [ADR-0005](../project/process/adr/0005-checksensors-fires-events.md) |
| A13 | Reading history is capped at 1 000 per sensor | [ADR-0009](../project/process/adr/0009-bounded-reading-history.md), accepted with the user |
| A15 | The default sampler returns 0.0 (no hardware attached); the default clock starts at 2026-01-01 00:00 UTC and advances one minute per call | ADR-0006 |

---

## 5. Code-quality assessment

The assessment is against widely used C++ practice: the C++ Core Guidelines, RAII and
const-correctness, and the one-class-per-file layout that AGENTS.md requires.

| Aspect | Assessment | Evidence |
|---|---|---|
| **Fidelity to the model** | Strong | Names, types, visibilities and multiplicities are exactly as drawn. The state table reads row-for-row like the diagram. The scenario's `scenario()` is one line, the drawn actor call |
| **Structure** | Strong | One class per header and source, `snake_case` files, one namespace (`cabin`). The seam is in `cabin::hardware`; the internal helper is kept out of `include/`. 379 header lines and 479 source lines in total; no function longer than about 45 lines |
| **Readability and naming** | Good | Drawn names are used verbatim. Comments explain *why* (the trace, the friendships, the drawn arrow), not *what*. The style follows the examples: Allman braces, trailing `_` on members. The enumerators are mixed case (`Severity::LOW` vs `State::Monitoring`) because both are drawn that way |
| **Documentation** | Good, with gaps | Every class and every non-trivial public operation has a `///` comment (73 lines) covering ownership, units, order, exceptions and dangling-pointer caveats. **Gap:** trivial getters have no comment, so the plan's "every public member" is not fully met. Design rationale is in the spec and 11 ADRs |
| **Ownership and lifetime (RAII)** | Strong | Every composition is `std::unique_ptr` or a by-value member; there is no `shared_ptr`, no `new`/`delete` and no manual cleanup. Ownership transfer is explicit (`addSensor(std::unique_ptr<Sensor>)`). Polymorphic copying is disabled |
| **Const-correctness** | Strong | All getters and guards are `const`, `shouldAlert` is `const`, and collections are returned by `const&` where the drawn signature allows |
| **Error handling** | Good | Invalid configuration throws `std::invalid_argument`, with the class and operation in the message. Every throw is documented (`@throws`). Benign no-ops (unknown id) are documented. **Gap:** `PowerMeter::setThreshold` accepts negative and NaN values, and NaN readings never alert for temperature or power (NaN compares false). The error policy (ADR-0008) does not cover these |
| **Testability** | Strong | The hardware seam makes all time and values deterministic. A fixture resets the global state per test. Tests drive the state machine only through the public API (`handle`, `addSensor`, …), with no test hooks in the production code |
| **Tests** | Good | 47 cases: limits at, inside and outside each boundary; each transition; each guard both ways; ignored events; error paths; ordering; ids; history eviction. 97 % line coverage |
| **Performance** | Adequate | `checkSensors()` is O(sensors), with no per-sensor allocation apart from the alerts themselves. `handle()` walks a 6-row `constexpr` table. **Costs:** `getSensors()`/`getCabins()` allocate a fresh `vector<T*>` per call (the drawn signature returns a collection); evicting history with `erase(begin())` is O(1 000) per read once full. Both are negligible at the expected scale (a handful of sensors, one check per interval) |
| **Build hygiene** | Strong | No warnings at `-Wall -Wextra`; the tests build under `-Wpedantic -Werror`. CMake options are OFF by default, so the verifier's build is the plain one. Third-party code (doctest 2.4.11) is vendored with its MIT licence and included as `SYSTEM` |
| **Portability** | Good | Standard C++20 only (`<chrono>` calendar, `<format>`, `std::erase_if`, ranges), with an MSVC branch in CMake. It has only been compiled with GCC 16 on Windows |
| **Thread safety** | Not provided (by design) | The `hardware` seam and the alert-id map are process-global and unsynchronized. Documented in `hardware.hpp` and ADR-0006. Acceptable for a single-threaded monitor loop, but it must change before concurrent use |

### 5.1 Known weaknesses (technical debt)

1. **Friend declarations (ADR-0011).** These are an encapsulation trade-off made only to satisfy
   the trace rules. Three classes expose their privates to named collaborators. They can be removed
   if the sequence diagram draws the getter calls.
2. **The alert-id counter is hidden state**, kept in a file-local map keyed by object address.
   - It works, and it is tested for address reuse.
   - But it is non-obvious, and it forces `Cabin` to be non-copyable and non-movable.
   - Drawing `- nextAlertId: int` on `Cabin` would turn it into an ordinary member.
3. **Global seam state.** See *Thread safety* above. Real hardware support would want a
   per-cabin or injected sampler, which needs a drawn type.
4. **`getSensors()`/`getCabins()` hand out raw pointers** that dangle after `remove*`. This is
   documented, and dictated by `Sensor[]`/`Cabin[]` plus unique ownership. A non-owning view type
   (`std::span`) would not match the mapping.
5. **Alert text and severity are hard-coded** (A5, A7). An `AlertKind` enumeration in the model
   would make them data.
6. **Input validation gaps.** NaN or negative thresholds are accepted; see *Error handling*.

---

## 6. How to reproduce

From the repository root, on Windows. MSYS2 is installed by the verifier, or manually as in §3.3.

```sh
# libclang 23, needed with GCC 16 (§3.2)
C:\msys64\usr\bin\bash.exe -lc "pacman -S --needed mingw-w64-ucrt-x86_64-clang-libs"

# model alignment -> project/reports/*.md
LIBCLANG_LIBRARY_PATH=C:/msys64/ucrt64/bin python tools/verify.py project

# unit tests (MSYS2 ucrt64/bin on PATH; keep the build path short, CMake limits object paths)
cmake -S project/impl -B %TEMP%/cabin-tests -G Ninja -DCABIN_BUILD_TESTS=ON
cmake --build %TEMP%/cabin-tests && ctest --test-dir %TEMP%/cabin-tests --output-on-failure

# coverage
cmake -S project/impl -B %TEMP%/cabin-cov -G Ninja -DCABIN_BUILD_TESTS=ON -DCABIN_COVERAGE=ON
cmake --build %TEMP%/cabin-cov && %TEMP%/cabin-cov/tests/cabin_tests
.venv/Scripts/python -m pip install gcovr
.venv/Scripts/python -m gcovr -r project/impl --filter project/impl/src/ --exclude ".*demo\.cpp" %TEMP%/cabin-cov
```

---

## 7. Recommendations for the team

In priority order:

1. **Get the class report to 100 %.** Either fix `ntype` for `T[]` (§3.1), or draw the five return
   types as `vector<T>`.
2. **Make the VS Code task work with GCC 16.** Set `LIBCLANG_LIBRARY_PATH`, or point the
   verifier's libclang at MSYS2 (§3.2).
3. **Confirm or replace the invented values:** thresholds (A4), severities (A5) and message
   format (A7). Consider drawing an `AlertKind`.
4. **Draw `- nextAlertId: int` on `Cabin`** (§5.1, item 2).
5. **Draw *Deleted cabin* from every state**, or forbid deleting a cabin that has sensors.
6. **Decide the policy for NaN and negative thresholds and readings**, and extend ADR-0008.
7. **Add CI**: build, tests, coverage and `verify.py`, with the reports kept as artifacts.
