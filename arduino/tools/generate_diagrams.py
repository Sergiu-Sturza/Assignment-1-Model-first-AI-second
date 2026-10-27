#!/usr/bin/env python3
"""Model first: turn the design in arduino/model/*.mmd into arduino/diagrams/input/*.drawio.

    python arduino/tools/generate_diagrams.py

The mermaid files are the design, written by hand and reviewed like code. This script draws
them with the verifier's own draw.io writers (tools/umlverify), so the drawings are exactly
what the verifier reads back: the round trip drawio -> design.mmd is checked at the end and
the script fails if any diagram does not survive it. Deterministic: the same model always
gives byte-identical drawings.
"""
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
ARDUINO = REPO / "arduino"
sys.path.insert(0, str(REPO / "tools"))

from umlverify.class_diagram import drawio_read as class_read  # noqa: E402
from umlverify.class_diagram import drawio_write as class_write  # noqa: E402
from umlverify.class_diagram import mermaid as class_mmd  # noqa: E402
from umlverify.sequence_diagram import drawio_read as seq_read  # noqa: E402
from umlverify.sequence_diagram import drawio_write as seq_write  # noqa: E402
from umlverify.sequence_diagram import mermaid as seq_mmd  # noqa: E402
from umlverify.state_diagram import drawio_read as state_read  # noqa: E402
from umlverify.state_diagram import drawio_write as state_write  # noqa: E402
from umlverify.state_diagram import mermaid as state_mmd  # noqa: E402

# Where each class box goes: (x, y, width). Coordinators on top, the parts below them,
# values and enumerations at the bottom.
CLASS_LAYOUT = {
    "TemperatureMonitor": (360, 40, 260),
    "CommandHandler": (1240, 40, 200),
    "ThresholdEditor": (800, 40, 260),
    "TemperatureSensor": (40, 330, 180),
    "Buzzer": (250, 330, 160),
    "LcdDisplay": (440, 330, 470),
    "SerialLink": (960, 330, 220),
    "Threshold": (1230, 330, 320),
    "Button": (1600, 330, 240),
    "IntervalTimer": (40, 600, 240),
    "MessageFormatter": (320, 600, 690),
    "CommandParser": (1240, 600, 230),
    "TemperatureStatus": (40, 820, 180),
    "CommandKind": (260, 820, 180),
    "ThresholdLimit": (480, 820, 180),
    "Command": (1240, 820, 230),
}

def _edge(to, exit_, entry, points=(), label=None):
    """One drawn transition: where it leaves and enters its boxes, and its corners."""
    return {"to": to, "points": list(points), "label": label,
            "style": f"exitX={exit_[0]};exitY={exit_[1]};entryX={entry[0]};entryY={entry[1]};"}


# Normal and Alarming side by side, Sensor fault below between them; every line routed by
# hand so that no two lines share a path and no label sits on another line.
STATE_LAYOUT = {
    "states": {
        "Normal": (120, 120, 160, 60),
        "Alarming": (620, 120, 160, 60),
        "Sensor fault": (370, 400, 160, 60),
    },
    "edges": {
        ("normal", "temperatureoutofrange"): _edge("Alarming", (1, 0.3), (0, 0.3), label=(0, 0, 0, -12)),
        ("alarming", "temperatureok"): _edge("Normal", (0, 0.7), (1, 0.7), label=(0, 0, 0, 14)),
        ("normal", "sensorfailed"): _edge("Sensor fault", (0.6, 1), (0, 0.25), [(216, 415)]),
        ("sensorfault", "temperatureok"): _edge("Normal", (0, 0.75), (0.3, 1), [(168, 445)]),
        ("alarming", "sensorfailed"): _edge("Sensor fault", (0.4, 1), (1, 0.25), [(684, 415)], label=(0, 0, -45, 0)),
        ("sensorfault", "temperatureoutofrange"): _edge("Alarming", (1, 0.75), (0.7, 1), [(732, 445)]),
    },
}


def write(path, text):
    path.write_text(text, encoding="utf-8", newline="\n")
    print(f"  wrote {path.relative_to(REPO).as_posix()}")


def class_diagram(src, dst):
    types, relations = class_mmd.parse(src.read_text(encoding="utf-8"))
    missing = set(types) - set(CLASS_LAYOUT)
    if missing:
        sys.exit(f"no position for {sorted(missing)} in CLASS_LAYOUT")
    write(dst, class_write.document(types, relations, {"classes": CLASS_LAYOUT}, name="Temperature alarm"))
    back_types, back_relations, *_ = class_read.read(dst)
    return _same(class_mmd.write(back_types, back_relations), class_mmd.write(types, relations), dst)


def state_diagram(src, dst):
    machine = state_mmd.parse(src.read_text(encoding="utf-8"))
    write(dst, state_write.document(machine, STATE_LAYOUT, name="TemperatureMonitor"))
    back, *_ = state_read.read(dst)
    return _same(state_mmd.write(back), state_mmd.write(machine), dst)


def sequence_diagram(src, dst):
    scenario = seq_mmd.parse(src.read_text(encoding="utf-8"))
    write(dst, seq_write.document(scenario, name=src.stem.removeprefix("sequence-")))
    back, *_ = seq_read.read(dst)
    return _same(_calls(back), _calls(scenario), dst)


def _calls(scenario):
    return ([l["name"] for l in scenario["lifelines"]],
            [(m["from"], m["to"], m["name"], m["kind"]) for m in scenario["messages"]])


def _same(got, want, dst):
    if repr(got) != repr(want):
        print(f"  ✗ {dst.name} does not read back as its model:\n    got  {got}\n    want {want}")
        return False
    return True


def main():
    out = ARDUINO / "diagrams" / "input"
    out.mkdir(parents=True, exist_ok=True)
    ok = True
    for src in sorted((ARDUINO / "model").glob("*.mmd")):
        dst = out / f"{src.stem}.drawio"
        if src.stem == "class":
            ok &= class_diagram(src, dst)
        elif src.stem.startswith("state-"):
            ok &= state_diagram(src, dst)
        elif src.stem.startswith("sequence-"):
            ok &= sequence_diagram(src, dst)
    if not ok:
        sys.exit(1)
    print("  every drawing reads back as its model")


if __name__ == "__main__":
    main()
