#!/usr/bin/env python3
"""Measure the Figure of Merit of the temperature alarm and add it to its history.

    python arduino/tools/fom.py "what changed in this iteration"
    python arduino/tools/fom.py --plot-only          redraw fom/fom.svg and fom/README.md

One run is one iteration. Every number comes from a tool, never from a judgement:

    design gap      100 % - the mean alignment of every report tools/verify.py writes for
                    arduino/ (class, state machine, both scenarios). 0 % = the code is
                    exactly the design. THE Figure of Merit.
    tests           passed / total of the C++ unit tests (doctest) and the website's
                    JavaScript tests (node --test)
    update ns       the benchmark of one TemperatureMonitor::update() measurement cycle on
                    the host, median of the benchmark's repetitions
    flash / RAM     what the firmware uses on an Arduino UNO, from arduino-cli

A step that cannot run (no arduino-cli, no node) is recorded as empty, not as zero.
The row goes to arduino/fom/history.csv; fom.svg and README.md are redrawn from it.
"""
import argparse
import csv
import datetime as dt
import html
import json
import math
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
ARDUINO = REPO / "arduino"
IMPL = ARDUINO / "impl"
FOM = ARDUINO / "fom"
HISTORY = FOM / "history.csv"
sys.path.insert(0, str(REPO / "tools"))

from umlverify.core import toolchain  # noqa: E402

COLUMNS = ["iteration", "date", "commit", "note",
           "class_pct", "state_pct", "seq_measure_pct", "seq_threshold_pct", "design_gap_pct",
           "cpp_tests_passed", "cpp_tests_total", "js_tests_passed", "js_tests_total",
           "update_ns", "parse_ns", "format_ns", "flash_bytes", "ram_bytes"]
REPORTS = {"class_pct": "class-report.md",
           "state_pct": "state-temperature_monitor-report.md",
           "seq_measure_pct": "sequence-measure_temperature-report.md",
           "seq_threshold_pct": "sequence-change_threshold-report.md"}
ARDUINO_CLI = [Path(r"C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"),
               Path("/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli")]
ELECTRON = [Path(r"C:\Program Files\Arduino IDE\Arduino IDE.exe")]


def python():
    venv = REPO / ".venv" / ("Scripts/python.exe" if os.name == "nt" else "bin/python")
    return str(venv if venv.exists() else sys.executable)


def run(cmd, **kw):
    print("  $ " + " ".join(str(c) for c in cmd), flush=True)
    return subprocess.run([str(c) for c in cmd], capture_output=True, text=True, errors="replace", **kw)


# ------------------------------------------------------------------ measurements

def design_alignment():
    """Run the verifier on arduino/ and read the alignment off each report."""
    run([python(), "-X", "utf8", REPO / "tools" / "verify.py", ARDUINO])
    found = {}
    for column, name in REPORTS.items():
        report = ARDUINO / "reports" / name
        text = report.read_text(encoding="utf-8") if report.exists() else ""
        m = re.search(r"\*\*Alignment: ([\d.]+) %\*\*", text)
        found[column] = float(m.group(1)) if m else 0.0     # no report = nothing aligned
    found["design_gap_pct"] = round(100 - sum(found.values()) / len(REPORTS), 2)
    return found


def cpp_tests_and_bench():
    """Build impl/ with tests and benchmarks (Release) and run both."""
    out = {}
    build = ARDUINO / "build-fom"
    try:
        toolchain.ensure()
    except RuntimeError as e:
        print(f"  no C++ toolchain: {e}")
        return out
    configure = run(["cmake", "-S", IMPL, "-B", build, *toolchain.cmake_generator(build),
                     "-DCMAKE_BUILD_TYPE=Release", "-DTEMPALARM_BUILD_TESTS=ON", "-DTEMPALARM_BUILD_BENCH=ON"])
    built = configure.returncode == 0 and run(["cmake", "--build", build]).returncode == 0
    if not built:
        print("  the implementation with tests does not build")
        out.update(cpp_tests_passed=0, cpp_tests_total=0)
        return out
    exe = ".exe" if os.name == "nt" else ""
    tests = run([build / "tests" / f"tempalarm_tests{exe}"])
    m = re.search(r"test cases:\s*(\d+)\s*\|\s*(\d+) passed", tests.stdout)
    out.update(cpp_tests_total=int(m.group(1)) if m else 0, cpp_tests_passed=int(m.group(2)) if m else 0)
    bench = run([build / "bench" / f"tempalarm_bench{exe}", "--json"])
    try:
        results = json.loads(bench.stdout)
        out.update(update_ns=results.get("monitor_update"), parse_ns=results.get("parse_command"),
                   format_ns=results.get("format_reading"))
    except json.JSONDecodeError:
        print("  the benchmark printed no JSON")
    return out


def node():
    """A Node.js to run the website's tests: node itself, or the Arduino IDE's Electron."""
    found = shutil.which("node")
    if found:
        return [found], {}
    for electron in ELECTRON:
        if electron.exists():
            return [electron], {"ELECTRON_RUN_AS_NODE": "1"}
    return None, None


def js_tests():
    cmd, env = node()
    if cmd is None:
        print("  no node: website tests skipped")
        return {}
    tests = sorted((ARDUINO / "dashboard" / "tests").glob("*.test.mjs"))
    if not tests:
        return {"js_tests_passed": 0, "js_tests_total": 0}
    r = run([*cmd, "--test", *tests], env={**os.environ, **env}, cwd=ARDUINO / "dashboard")
    text = r.stdout + r.stderr
    total = re.search(r"^# tests (\d+)", text, re.M)
    passed = re.search(r"^# pass (\d+)", text, re.M)
    return {"js_tests_total": int(total.group(1)) if total else 0,
            "js_tests_passed": int(passed.group(1)) if passed else 0}


def firmware_size():
    cli = next((c for c in ARDUINO_CLI if c.exists()), None) or shutil.which("arduino-cli")
    if not cli:
        print("  no arduino-cli: firmware size skipped")
        return {}
    sync = run([python(), ARDUINO / "tools" / "sync_firmware.py"])
    if sync.returncode != 0:
        print(sync.stdout + sync.stderr)
        return {}
    r = run([cli, "compile", "--fqbn", "arduino:avr:uno", "--warnings", "all",
             ARDUINO / "temperature_alarm"])
    text = r.stdout + r.stderr
    flash = re.search(r"Sketch uses (\d+) bytes", text)
    ram = re.search(r"Global variables use (\d+) bytes", text)
    if r.returncode != 0:
        print(text[-3000:])
    return {"flash_bytes": int(flash.group(1)) if flash else None,
            "ram_bytes": int(ram.group(1)) if ram else None}


# ------------------------------------------------------------------ history

def read_history():
    if not HISTORY.exists():
        return []
    with HISTORY.open(newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def write_history(rows):
    FOM.mkdir(parents=True, exist_ok=True)
    with HISTORY.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=COLUMNS, lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


def commit():
    r = run(["git", "-C", REPO, "rev-parse", "--short", "HEAD"])
    dirty = run(["git", "-C", REPO, "status", "--porcelain", "--", "arduino"]).stdout.strip()
    return (r.stdout.strip() or "-") + ("+" if dirty else "")


def num(row, key):
    try:
        return float(row[key])
    except (KeyError, TypeError, ValueError):
        return None


# ------------------------------------------------------------------ chart

# Small multiples, one measure per panel and one y-scale each: never two scales on one axis.
PANELS = [("design_gap_pct", "Design gap — the Figure of Merit (lower is better)", "%"),
          ("tests_pct", "Tests passing", "%"),
          ("update_ns", "TemperatureMonitor::update(), host", "ns"),
          ("parse_ns", "CommandParser::parse(), host", "ns"),
          ("flash_bytes", "Firmware flash, Arduino UNO", "B"),
          ("ram_bytes", "Firmware RAM, Arduino UNO", "B")]


def series(rows, key):
    if key == "tests_pct":
        out = []
        for r in rows:
            total = (num(r, "cpp_tests_total") or 0) + (num(r, "js_tests_total") or 0)
            passed = (num(r, "cpp_tests_passed") or 0) + (num(r, "js_tests_passed") or 0)
            out.append(100 * passed / total if total else None)
        return out
    return [num(r, key) for r in rows]


def fmt(v, unit):
    if v is None:
        return "–"
    if unit == "%":
        return f"{v:.1f} %"
    return f"{v:,.0f} {unit}".replace(",", " ")


def svg(rows):
    W, PW, PH, GAP, TOP = 980, 470, 230, 40, 16
    pad_l, pad_r, pad_t, pad_b = 56, 16, 34, 34
    rows_of_panels = (len(PANELS) + 1) // 2
    height = rows_of_panels * PH + (rows_of_panels - 1) * GAP + TOP
    out = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{height}" '
           f'viewBox="0 0 {W} {height}" font-family="system-ui, -apple-system, Segoe UI, sans-serif">',
           '<style>.t{fill:#0b0b0b;font-size:13px;font-weight:600}.a{fill:#52514e;font-size:11px}'
           '.g{stroke:#e4e3df;stroke-width:1}.l{fill:none;stroke:#2a78d6;stroke-width:2;stroke-linejoin:round}'
           '.p{fill:#2a78d6;stroke:#fcfcfb;stroke-width:2}.v{fill:#0b0b0b;font-size:11px;font-weight:600}'
           '@media (prefers-color-scheme: dark){.bg{fill:#1a1a19}.t,.v{fill:#f0efec}.a{fill:#c3c2b7}'
           '.g{stroke:#3a3a37}.l{stroke:#3987e5}.p{fill:#3987e5;stroke:#1a1a19}}</style>',
           f'<rect class="bg" width="100%" height="100%" fill="#fcfcfb"/>']
    n = len(rows)
    for k, (key, title, unit) in enumerate(PANELS):
        ox, oy = (k % 2) * (PW + GAP), TOP + (k // 2) * (PH + GAP)
        vals = series(rows, key)
        known = [v for v in vals if v is not None]
        out.append(f'<text class="t" x="{ox + pad_l}" y="{oy + 14}">{html.escape(title)}</text>')
        x0, x1, y0, y1 = ox + pad_l, ox + PW - pad_r, oy + PH - pad_b, oy + pad_t
        if unit == "%":
            lo, hi = 0.0, 100.0
        else:                                            # 4 round steps above the largest value
            raw = (max(known) * 1.1 if known else 1.0) / 4 or 0.25
            magnitude = 10 ** math.floor(math.log10(raw))
            step = next(m * magnitude for m in (1, 2, 2.5, 5, 10) if m * magnitude >= raw)
            lo, hi = 0.0, step * 4
        for i in range(5):                               # recessive grid, 5 ticks
            v = lo + (hi - lo) * i / 4
            y = y0 - (y0 - y1) * i / 4
            out.append(f'<line class="g" x1="{x0}" x2="{x1}" y1="{y:.1f}" y2="{y:.1f}"/>')
            out.append(f'<text class="a" x="{x0 - 6}" y="{y + 4:.1f}" text-anchor="end">{fmt(v, unit)}</text>')

        def px(i):
            return x0 + (x1 - x0) * (i / (n - 1) if n > 1 else 0.5)

        def py(v):
            return y0 - (y0 - y1) * (v - lo) / (hi - lo)

        step = max(1, (n + 9) // 10)
        for i, r in enumerate(rows):
            if i % step == 0 or i == n - 1:
                out.append(f'<text class="a" x="{px(i):.1f}" y="{y0 + 18}" text-anchor="middle">{r["iteration"]}</text>')
        pts = [(px(i), py(v), i, v) for i, v in enumerate(vals) if v is not None]
        if len(pts) > 1:
            out.append('<polyline class="l" points="' + " ".join(f"{x:.1f},{y:.1f}" for x, y, *_ in pts) + '"/>')
        for x, y, i, v in pts:                            # >= 8 px markers, tooltip per point
            note = html.escape(rows[i]["note"])
            out.append(f'<circle class="p" cx="{x:.1f}" cy="{y:.1f}" r="5"><title>iteration {rows[i]["iteration"]}: '
                       f'{fmt(v, unit)} – {note}</title></circle>')
        if pts:                                           # direct label on the latest value only
            x, y, _, v = pts[-1]
            out.append(f'<text class="v" x="{min(x, x1 - 4):.1f}" y="{y - 10:.1f}" text-anchor="end">{fmt(v, unit)}</text>')
        out.append(f'<text class="a" x="{(x0 + x1) / 2:.1f}" y="{oy + PH - 2}" text-anchor="middle">iteration</text>')
    out.append("</svg>")
    return "\n".join(out) + "\n"


def readme(rows):
    lines = ["# Figure of Merit history", "",
             "Generated by `python arduino/tools/fom.py` — do not edit. One row per iteration.", "",
             "**Figure of Merit: the design gap** = 100 % − the mean alignment of the four verification "
             "reports (class diagram, state machine, two sequence diagrams). 0 % means the C++ is exactly the "
             "UML design. Tests, the `update()` benchmark and the firmware size are tracked beside it.", "",
             "![Figure of Merit over the iterations](fom.svg)", "",
             "| # | Date | Commit | Change | Class | State | measure | threshold | **Gap** | C++ tests | JS tests | update() | parse() | Flash | RAM |",
             "|--:|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|"]
    for r in rows:
        def cell(key, unit="%"):
            return fmt(num(r, key), unit)

        def ratio(p, t):
            return f"{r[p]}/{r[t]}" if r.get(t) not in (None, "") else "–"
        lines.append(f"| {r['iteration']} | {r['date']} | `{r['commit']}` | {r['note']} | {cell('class_pct')} | "
                     f"{cell('state_pct')} | {cell('seq_measure_pct')} | {cell('seq_threshold_pct')} | "
                     f"**{cell('design_gap_pct')}** | {ratio('cpp_tests_passed', 'cpp_tests_total')} | "
                     f"{ratio('js_tests_passed', 'js_tests_total')} | {cell('update_ns', 'ns')} | {cell('parse_ns', 'ns')} | "
                     f"{cell('flash_bytes', 'B')} | {cell('ram_bytes', 'B')} |")
    return "\n".join(lines) + "\n"


def redraw(rows):
    FOM.mkdir(parents=True, exist_ok=True)
    (FOM / "fom.svg").write_text(svg(rows), encoding="utf-8", newline="\n")
    (FOM / "README.md").write_text(readme(rows), encoding="utf-8", newline="\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("note", nargs="?", default="", help="what changed in this iteration")
    parser.add_argument("--plot-only", action="store_true", help="only redraw the chart and table")
    args = parser.parse_args()
    rows = read_history()
    if not args.plot_only:
        if not args.note:
            parser.error("say what changed in this iteration")
        row = {"iteration": len(rows), "date": dt.date.today().isoformat(), "commit": commit(),
               "note": args.note.replace("|", "/")}
        print("\n[1/4] design alignment (tools/verify.py)")
        row.update(design_alignment())
        print("\n[2/4] C++ unit tests and benchmark")
        row.update(cpp_tests_and_bench())
        print("\n[3/4] website tests")
        row.update(js_tests())
        print("\n[4/4] firmware size")
        row.update(firmware_size())
        rows.append({c: ("" if row.get(c) is None else row.get(c)) for c in COLUMNS})
        write_history(rows)
        print("\n" + "  ".join(f"{c}={rows[-1][c]}" for c in COLUMNS[4:]))
    redraw(rows)
    print(f"\n  {HISTORY.relative_to(REPO).as_posix()}: {len(rows)} iterations; fom/fom.svg and fom/README.md redrawn")


if __name__ == "__main__":
    main()
