## GoldenEye 007 PC Port <version>

<p align="center">
  <img src="https://github.com/jkdansereau/goldeneye-pc-port/raw/v0.2.2/docs/media/goldeneye-gh-preview.gif" width="480"
       alt="~32 s gameplay montage from live play sessions (no audio track)">
</p>

> The full single-player campaign runs at a steady 60 fps with audio (music +
> SFX) playing throughout, on Windows and Linux including Steam Deck, and it
> is completable end to end: the whole campaign has been playtested through
> all 21 missions (Agent difficulty). It's an early public cut: all 21 solo
> missions load and run crash-free, but the known issues below are real;
> feedback is very welcome. **This is a pre-1.0 release, not a finished
> product** — expect rough edges and missing features until v1.0; see the
> [README's Roadmap section](https://github.com/jkdansereau/goldeneye-pc-port#roadmap)
> for where this is headed.

### What's new since v0.2.1

Hotfix batch from Steam Deck/Linux user playtest feedback on v0.2.1 — two
crash fixes plus two QoL asks:

- **Fixed: intermittent SIGSEGV on Steam Deck/Linux when destroying objects**
  (terminals, exploding grenades) (D255). Root cause: `vtxstore_fix_refs`
  read a live object's model pointer through a mistyped 4-byte-enum field
  instead of the real pointer field, truncating it to garbage on 64-bit
  builds — harmless on the original 32-bit N64, fatal on PC. Verified live
  on real Steam Deck hardware, held through Facility (repeated terminal
  destruction) and Caverns' radio-objective terminals with zero crashes.
- **Fixed: a second, separate Steam Deck/Linux crash during explosion-heavy
  combat** (D285) — the audio thread's pool-exhaustion scan read the active-
  sound list without the lock its writer (`sndSetupSound`) already takes,
  so a burst of near-simultaneous SFX (an explosion, plus debris/metal)
  could race a concurrent list mutation. Locked to match the existing
  write-side guard. Live-tested on real Steam Deck hardware with dense,
  sustained overlapping-explosion combat (dual grenade launchers plus
  rocket-launcher-cheat guards) — no crash.
- **Front-end menu navigation (main menu, file select, mission-select map)
  now uses the LEFT analog stick**, matching the F10 options overlay and
  modern controller conventions, instead of requiring the right stick
  (D282).
- **Added a "Quit to desktop" row to the F10 options overlay** — previously
  the only way to exit was Alt+F4 / closing the window, a real gap on
  Steam Deck / controller-only / fullscreen setups (D293).

Full changelog history (v0.2.0, v0.1.0, ...) is on each version's own
[release page](https://github.com/jkdansereau/goldeneye-pc-port/releases);
this note only covers the delta from the immediately previous release.

### Known issues

- **Cutscenes still glitch, mostly with James Bond**: in scripted sequences
  Bond is the one who gets misplaced, hovers, or spins; the other actors are
  fine for the most part now. The Dam level-end cutscene is racy (D243).
  Still the most visible gap in this release.
- **Particle colours cycle through a rainbow palette**: bullet-impact sparks
  and lingering smoke/explosion residue drift through the hues over time
  instead of holding their intended grey/orange palette (D252).
- Water on `IsWater` levels shows a moving seam between two patterns (D245);
  thin pixel strips at the left/right screen edges at non-integer window
  scales (D246).
- The front-end **Nintendo logo renders as two white blobs**, and the
  Rareware logo is close but its texture filtering looks off (D75).
- The F10 overlay's **bottom row duplicates whatever item is currently
  selected** (e.g. the MSAA value appears both on its own row and again at
  the panel bottom); earlier builds showed it as an intermittent 4K-
  fullscreen ghost of the top row.
- **Surface 1: the 2D billboard trees near the start render as a solid wall
  of tree texture** instead of discrete sprites (D236). Under active
  investigation; no fix in this release.
- **No true widescreen support**: at 16:9 the game stretches the
  4:3-authored view (world geometry and HUD) rather than properly expanding
  the horizontal field of view. The F10 *FOV scale %* slider is a manual,
  imperfect workaround for the resulting distortion, not real widescreen.
- **No controller rebinding UI, and no macOS or ARM builds.**
- **Distant geometry can drop out on the biggest open levels** (Streets,
  Egyptian) at default FOV — a culling/LOD issue that sometimes
  self-corrects as you keep moving (D249).
- **Gunshot SFX can sound off during sustained/rapid fire**: cadence can
  drift from the N64 original's rate, and PP7/AK47 fire can occasionally go
  silent under heavy automatic fire near another looping sound (e.g. an
  alarm klaxon) (D240/D241).
- **`All unlocked` is highly experimental — do not enable it until you have
  at least one save written.** Complete a level normally first (e.g. Dam on
  Agent), then turn the toggle on. Enabling it on a brand-new install with
  no prior save can still cause silent audio and odd right-mouse-aim
  behavior even after the D259 fix above. We recommend leaving it off unless
  you specifically want the unlock goodies and are willing to accept an
  experimental feature (D259/D257).
- ~~Linux / Steam Deck: an intermittent SIGSEGV, reproducible by destroying
  Facility's computer terminals or other exploding objects~~ **FIXED
  (D255).** Root cause: a 32-bit pointer read through a mistyped field
  (`ChrRecord.chrflags`, a 4-byte enum aliased onto a real 8-bit pointer
  field) silently truncated the pointer on 64-bit builds — harmless on the
  original 32-bit N64, broken on PC. Verified live on real Steam Deck
  hardware: held through the entire Facility level (including repeated
  terminal destruction) and through Caverns' radio-objective terminals
  with zero crashes. Please still report any other Deck/Linux-specific
  faults.

### Downloads

| File | Platform |
|---|---|
| `goldeneye-pc-port-<version>-win64.zip` | Windows x86-64 |
| `goldeneye-pc-port-<version>-linux-x86_64.tar.gz` | Linux x86-64 (incl. Steam Deck) |

Each contains the engine executable, a README, license texts, and the
`prepare-assets` tool. **No ROM, no game assets.** The Windows bundle carries
its runtime DLLs; the Linux bundle carries SDL2, so on both platforms nothing
needs to be installed first.

### Running it

You supply your own **GoldenEye 007 N64 ROM** (`.z64`, big-endian) that you
legally own: the **NTSC-U (US)** release is what this build supports (PAL / JP
are on the roadmap). No Python, no toolchain, nothing to install.

1. Unpack the archive.
2. Make a `data/` folder next to the executable and put the ROM in it, named
   `ge007.ntsc-final.z64`.
3. Run the executable **from that folder**. The first run takes a few extra
   seconds: it detects your ROM, generates the two derived asset folders
   (`data/pcmodels-ntsc-final/`, `data/pccg-ntsc-final/`) once, and saves them
   for every future run. (The generator is `prepare-assets/ge007-convert`
   inside the bundle; you can also run it manually; it prints what it's doing.)

**Steam Deck:** sideload the unpacked folder (USB or a file manager), do steps
2–3, then add the executable to Games → *Add Game* as a non-Steam game.

**In-game settings on the Deck:** the options overlay is fully gamepad-driven:
it opens with **Select**, the D-pad or left stick (up/down) moves between
options, **A** steps the selected option forward, **B** steps it back, and
**Start** (or Select again) closes. No keyboard needed.

Full steps are in the bundled `README.md`.

### Verify the download

```
sha256sum -c goldeneye-pc-port-<version>-win64.zip.sha256
sha256sum -c goldeneye-pc-port-<version>-linux-x86_64.tar.gz.sha256
```

### Source & docs

<https://github.com/jkdansereau/goldeneye-pc-port>, built on the
[GoldenEye 007 decompilation](https://github.com/n64decomp/007), architecture
after the [Perfect Dark PC port](https://github.com/fgsfdsfgs/perfect_dark).
Non-commercial fan preservation/research project; not affiliated with any
rights holder. **AI disclosure:** built through agentic AI coding (Claude
Code + a local open-weight model), directed by one person in their spare
time — as much a study of what agentic development gets wrong on a
game-sized codebase as it is a port. See the README's
[Background section](https://github.com/jkdansereau/goldeneye-pc-port#background)
for the full account.
