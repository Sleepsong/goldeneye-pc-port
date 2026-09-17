# GEPD Edition — Mouse Injector (vendored reference source)

Vendored **read-only reference** for the PC port's input layer. NOT compiled
by our build; never modify it — treat it like `rsp/graphics/gmain.s`
(ground truth we consult, not code we ship).

## Provenance

Extracted from `source.tar.xz` inside the GEPD Edition 1964 bundle
(`C:\Users\james\Games\Emulators\Nintendo 64\1964_GEPD_Edition\1964\`).
The Mouse Injector is a Project64/1964 input plugin that emulates the N64
controller from keyboard/mouse while injecting mouse-aim state directly into
GoldenEye 007 / Perfect Dark game memory. License: **GPLv2** (see
`LICENSE.TXT`; `manymouse/` carries its own license).

**Version skew:** the tarball is a mix of eras — `device.c`, `ui/` are from
the 2014 v1.5d line; `games/goldeneye.c`, `maindll.h`, `maindll.c` are dated
2021–22; the bundle's binary is a 2023 build with a few later additions. The
aim *model* in `games/goldeneye.c` is stable across those versions and is the
one we already mirrored (finding D194). Consult it for the model and
constants, not as an exact match to any single release.

## Key files

| File | What to look at |
|---|---|
| `games/goldeneye.c` | **The whole GE input/aim model.** `GE_Inject()` = hipfire direct camera write + tank + menu cursor; `GE_AimMode()` = crosshair-position aim (±5.159° limit, 72% edge threshold, 475/s scroll — the D194 source); `GE_Crouch()` = crouch-toggle state machine; `GE_Controller()` = button mapping (wheel→weapon macro, aim=B in menus); `GE_InjectHacks()` = ROM patches we do NOT port (reload key, FOV override) but which document intended behavior. |
| `games/perfectdark.c` | PD analogue of the same model (radial menu, hoverbike, camspy). |
| `device.c` / `manymouse/` | Raw-driver mouse polling + cursor lock (`SetCursorPos` cadence), wheel cooldown handling, multi-device mapping. |
| `maindll.h` | `TICKRATE`/`TIMESTEP` (2 ms overclocked / 4 ms stock) — the units every per-tick constant in `goldeneye.c` assumes. |
| `global.h` | Settings enum (sensitivity range 1..80, acceleration, crosshair-movement %, invert pitch, crouch toggle, aim mode). |
| `vkey.h` | Default key layout (WASD=C-buttons, Q=A, E=B, R=reload, wheel=weapon cycle). |

## Cross-references in our tree

- `docs/dev/GEPD-INPUT-PLAN.md` — plan for the remaining GEPD inheritances
  (issues #89/#90).
- `port/src/input.c` — D194 (`aimGepdCompute`) is our mirror of
  `GE_AimMode()`; D223 wheel cycling mirrors `GE_Controller()`'s weapon macro.
- `docs/dev/findings.md` §D194/D238 — aim-mode history and tuning.
