# D318 Session Handoff — PRNG investigation (Facility / level_34, Ourumov-Trevelyan sequence)

**Status: mid-investigation. No verdict change yet. All work uncommitted.**
Previous context: `docs/HANDOFF.md`, README "Status", findings D318 in `docs/dev/findings.md`
(look up via `docs/dev/findings-index.csv`). This file supersedes the old "next steps" list.

## 1. The problem (user-reported symptoms)

N64 ground truth (GEPD bundle emulator, user has it at
`C:\Users\james\Games\Emulators\Nintendo 64\1964_GEPD_Edition\1964`): in the Facility
tank-room sequence, guards line up and HOLD FIRE during Ourumov's (c78) monologue/countdown
while Trevelyan (c67) kneels; no shooting until tanks detonate; ~1s later Ourumov executes
Trevelyan; then guards open fire on Bond.

Port symptoms:
- **Intermittent**: sometimes guards start shooting immediately (before/instead of the hold).
- **Ourumov "bends down a bit too far"** vs N64 even in "normal" runs — animation suspect:
  possible misroute into the kneel-attack path `sub_GAME_7F0256F0` (selected by
  `act_attack.unk54`: 1=standing `sub_GAME_7F025560`, 0=kneeling). The standing path is what
  ai_22's AI_TRYFireOrAimAtTarget uses, so `unk54==0` during the countdown would be a bug.
- Sequence triggers spottily vs original; user suspects a **foundational** issue (PRNG stream
  parity / tick pacing / objective register state) affecting other areas too.

## 2. What this session established (findings F1–F5)

- **F1 — Boot seed is wall-clock BY DESIGN, faithful to N64.** `src/boss.c:404`:
  `randomSetSeed(osGetCount())`. N64 RSP timer varies per power-on; PC shim (`port/src/libultra.c`
  `osGetCount`) feeds wall-clock scaled to 46.5 MHz. Per-run PRNG variation is original behavior,
  NOT a port bug. D284 (M-140) transform fix stands.
- **F2 — Guard AI consumes the global PRNG heavily per tick.** Measured (15s headless
  `-level_34`, `GE_D318R=1`): top callers `chrCheckTargetInSight` (~7.7 draws/tick),
  `chrlvTickStand`, `ai`, `shuffle_player_ids` (3 draws EVERY main-loop iteration, even solo —
  `src/boss.c:597`). Guard behavior is strongly seed-dependent by design ⇒ N64 holding fire
  reliably under random per-boot seeds implies the **hold-fire gate is deterministic
  (objective-register-driven), not RNG-driven**. This reframes the "guards fire immediately"
  symptom toward objective-register/gate state rather than raw PRNG parity.
- **F3 — Coarse sim state was run-to-run identical** in no-input runs (chr hash `h` matched at
  all samples) — weak check in an idle scene, not a determinism proof.
- **F4 — Residual run-to-run divergence remains even with pinned seed + GE_DETERM=1.** Draw
  caller sequence diverges by draw ~346 (t≈15); 3548/4000 tick stamps differ. Suspect:
  **tick pacing, not the PRNG** — `waitForNextFrame()` (`src/game/frametiming.c`) derives
  `deltaFrames` from wall-clock elapsed time; `g_GlobalTimer += g_ClockTimer` (lv.c:1051) thus
  advances a jittery 0/1/2 per rendered frame. Under GE_DETERM the virtual clock advances on the
  scheduler thread and drifts against the main loop. N64 was single-threaded + VI-locked ⇒
  exactly one tick/frame. **This is the foundational issue: it exists in every level.**
  NOTE: not yet a clean verdict — GE_DETERM itself has pacing non-determinism (harness flaw).
- **F5 — MinGW `strtoull` mangles hex values > LLONG_MAX** (returns sign-extended low 32 bits;
  `"deadbeefcafe0123"` → `ffffffffcafe0123`). Worked around with manual hex parsers in both new
  files. Remember for any future 64-bit hex env parsing.

Also: N64 `g_randomSeed` RAM address = **0x80024460** (u64, big-endian; found by
instruction-pattern search). Initial constant 0xAB8D9F7781280783, but boss.c reseeds at boot.

## 3. Instrumentation added this session (all test-only, env-gated, port-layer)

| Env var | Behavior | File |
|---|---|---|
| `GE_D318T=1` | Timeline probe: obj-bit transitions, c78 phase/action/attack/entity changes, NEAR_MISS, c67 damage/death, **c78 anim internals on change** (`type_of_motion`, `unk54`, `endframe`, `frame1` — answers "bending down": `unk54==0` ⇒ kneel-route misfire), DET fingerprint line every 60 ticks (FNV-1a over all active chrs + full u64 seed + obj bits) | `port/src/d318watchdog.c` `d318TimelineTick()` (hooked in `src/game/lv.c` after `g_GlobalTimer += g_ClockTimer`) |
| `GE_D318R=1` | Per-draw PRNG logger: first 4000 `randomGetNext()` calls as `D318R: n=<i> t=<tick> ra=<retaddr>`; also logs every `randomSetSeed` (`SETSEED arg=... -> ...`) and GE_RSEED overrides | `port/src/random.c` `randomDrawLog()` |
| `GE_RSEED=<hex64>` | Pins the boot seed (overrides wall-clock) for determinism tests | `port/src/random.c` `randomSetSeed()` |
| `GE_RSEED_LV=<hex64>[@tick]` | Injects a seed at a chosen tick (default 0) — A/B parity vs N64 RAM read; DET lines then verify stream parity | `port/src/d318watchdog.c` top of `d318TimelineTick()` |

Resolving `ra=` addresses: `nm build-pc/ge007.x86_64.exe` (has symbols, no line info) → nearest
symbol below. Helper scripts in scratch: `d318r_cmp.py` (per-caller counts),
`d318r_seqcmp2.py` / `d318r_seqcmp3.py` (sequence divergence; seqcmp3 compares caller-only vs
tick stamps). Symbol dump: `scratch/syms.txt` (regenerate: `nm build-pc/ge007.x86_64.exe | awk '$2 ~ /[tTwW]/ {print $1, $3}'`).

Build is clean (`cmake --build build-pc`). Probe is silent without env vars (smoke-tested).

## 4. Experiments run (logs in scratch/)

- `d318t_det1/2.log`: two no-input runs, GE_D318T=1 → identical chr hash, divergent seeds
  (explained by F1: different wall-clock boot seeds).
- `d318r_draws*.log` + `detC..F.log`: draw-caller analysis; `detE/F` = GE_DETERM=1 +
  GE_RSEED=0123456789abcdef → caller sequence diverges by draw 346 (F4).

## 5. Next steps (prioritized)

1. **E2 — USER playtest (needs user):** one BAD + one GOOD tank-room run with `GE_D318T=1`,
   capturing console: `ge007.x86_64.exe > scratch/d318t_playtest.log 2>&1` (MSYS2 MINGW64 shell,
   build via `./build-pc.sh ntsc-final`). The obj-bit timeline discriminates "gate opened early"
   (bit 0x04 set before it should) from "guards bypassing gate"; c78 anim-internals lines answer
   the bending-down question (`unk54`/`type_of_motion`/`endframe`).
2. **E3 — USER reads N64 seed (needs user):** in 1964 debugger, read u64 at RAM **0x80024460**
   when Facility gameplay starts; report value. Then run port with `GE_RSEED_LV=<value>` and
   compare guard behavior + verify stream parity: DET lines log the running seed every 60 ticks;
   a second emulator read a few seconds later should match the port's DET seed if streams are
   in lockstep from the injection point. (Caveat: `-level_34` skips menu draws — inject at
   gameplay start, not boot.)
3. **E1b — ME:** fix GE_DETERM main-loop/tick lock (or build single-threaded fixed-tick headless
   harness) for a clean determinism verdict on F4. Lower priority until E2/E3 data lands.
4. If PRNG pacing (F4) confirmed as the behavioral culprit: it's a `port/`-layer fix candidate
   (frametiming/pacing), NOT a game-code edit — but diagnose fully first; document in findings.md.

## 6. Open questions / holds

- **D318 verdict is in question.** The committed-in-working-tree framing ("as-authored latent
  race, N64 has identical softlock") may be wrong: N64 reliably executes ~1s after detonation.
  The derail path (section 0x2a → off=307) IS the normal execution flow on N64. If N64 never
  freezes, the freeze branch is likely PC-specific — re-open D318 findings if E2/E3 confirm.
- **HOLD the D318 commit** (working tree only, uncommitted): if the verdict flips, the watchdog
  (`port/src/d318watchdog.c` `d318WatchdogTick`, opt-out `GE_D318W=0`) should go opt-in or be
  replaced by the real fix before the README auto-recovery claim ships.
- Regression-window suspect for guard behavior: D312/D313 commit (bullet-wall collision,
  last gameplay-affecting change before mostly-working Sep 18 playtest3).

## 7. Git status (uncommitted)

Modified: `.github/release-notes.md`, `.gitignore`, `README.md`, `docs/dev/agentic-development.md`,
`docs/dev/findings-index.csv`, `docs/dev/findings.md`, `docs/index.md`,
`docs/media/goldeneye-gh-preview.gif`, `src/game/lv.c` (D318 hooks), **`port/src/random.c`**
(PRNG logging + GE_RSEED, this session).
Untracked: **`port/src/d318watchdog.c`** (watchdog + timeline probe + seed injection),
various `scratch/*` logs/scripts.

## 8. Key code map (verified this session)

- `src/boss.c:404-405` — boot PRNG reseed from osGetCount; `:596-597` — per-main-iteration
  `lvlManageMpGame()` + `shuffle_player_ids()`.
- `src/game/lv.c:1023+` `lvlManageMpGame()` — g_ClockTimer (pause→0, else speedgraphframes),
  `g_GlobalTimer += g_ClockTimer` at :1051, D318 hooks right after.
- `src/game/frametiming.c` — `waitForNextFrame()` wall-clock deltaFrames + D155 clamp
  (max 6) + D193 telemetry (`GE_D193=1`).
- `src/game/model.c:4800+` `sub_GAME_7F072C10` — render-path function doing 3 PRNG draws per
  vertex (dorottex/rotating-texture effect); also model.c:5251/5310. Render-path PRNG
  consumption is a secondary parity concern (per presented frame, not per tick).
- `src/game/player.c:637` `shuffle_player_ids()` — 3 draws/call.
- `port/src/libultra.c:90+` — osGetCount wall-clock shim + GE_DETERM virtual clock (D117/M-52;
  known pre-task-window non-determinism).
- `port/src/random.c` — PRNG transform (D284-verified) + this session's logging/overrides.
- ai_22 offsets (from prior sessions): off=38 pre-aim, 44 wait loop, 69+ monologue cascade,
  272 execution label, 295 sets bit 0x04, 302 combat section 0x2a, 307 update, 321 sleep/stop.
- CHRFLAG_NEAR_MISS = 0x4 (bondconstants.h), set chr.c:3806 (Bond bullet bounds), cleared
  per-tick (one-frame window).
- N64 emulator: `C:\Users\james\Games\Emulators\Nintendo 64\1964_GEPD_Edition\1964` (user-provided).
