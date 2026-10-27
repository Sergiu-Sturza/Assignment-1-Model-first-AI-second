# Plan — threshold buttons, Arduino Mega

Three push buttons on the Mega change the threshold without the website:
D5 selects the low or the high limit, D4 raises it by 0.1 °C, D3 lowers it by 0.1 °C.
Each button connects its pin to GND (internal pull-up, pressed = LOW).

- [x] Model: `Button`, `ThresholdEditor`, `ThresholdLimit` and `LcdDisplay::showThreshold` in
      `model/class.mmd`; scenario `model/sequence-adjust_threshold.mmd`; layout in
      `tools/generate_diagrams.py`; generate the drawings (they read back as the model).
- [ ] Render `class.drawio` and `sequence-adjust_threshold.drawio` and inspect them
      (needs Node.js: `node tools/render-drawio/render.js …`).
- [x] Spec: the new classes, the scenario and the wiring in `specs/`.
- [x] HAL: `hal::isButtonDown(pin)`; host simulation `sim::setButton`; Arduino: `INPUT_PULLUP`.
- [x] Headers and sources exactly as designed; scenario program `scenarios/adjust_threshold.cpp`.
- [x] Verifier: 100 % on all five reports.
- [x] Unit tests for `Button`, `ThresholdEditor` and `showThreshold`. — 56/56
- [x] Sketch: pins in `board_config.h`, `ThresholdEditor` in the `.ino`; sync the firmware copy.
- [x] Mega instead of UNO: README wiring, `fom.py` board.
- [ ] Compile and upload for the Mega; try the buttons on the board.
- [ ] `python arduino/tools/fom.py "threshold buttons, Mega"` (needs Node.js and arduino-cli).
