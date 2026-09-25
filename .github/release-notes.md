## GoldenEye 007 PC Port <version>

<p align="center">
  <img src="https://github.com/jkdansereau/goldeneye-pc-port/raw/v0.3.0/docs/media/goldeneye-gh-preview.gif" width="480"
       alt="~12 s gameplay montage from live v0.3.0 play sessions (no audio track)">
</p>

> **Fork build.** This is [Sleepsong/goldeneye-pc-port](https://github.com/Sleepsong/goldeneye-pc-port),
> a fork of [jkdansereau/goldeneye-pc-port](https://github.com/jkdansereau/goldeneye-pc-port)
> v0.3.0 that adds **opt-in widescreen / ultrawide support**. Everything else
> is v0.3.0: the full single-player campaign runs at a steady 60 fps with
> audio, on Windows and Linux including Steam Deck, and is completable end to
> end. **This is a pre-1.0 build, not a finished product**; the widescreen
> options are new and have been playtested on one setup (Linux, 5120x1440
> 32:9). Feedback is very welcome.

### What's new since v0.3.0: widescreen and ultrawide

All of these are **off by default**; with default settings the game is
unchanged from v0.3.0. Both new options are in the F10 overlay (and under
`[Video]` in `ge007.ini`) and apply instantly.

- **Aspect ratio** (`AspectMode`):
  - **STRETCH** (default): as before, the whole picture is stretched to the
    window.
  - **4:3**: the original picture, centred, with black bars at the sides (or
    top and bottom on tall windows). Works in fullscreen, so there's no need
    to force a 4:3 window size (D323).
  - **HOR+**: the 3D world renders undistorted at your display's shape. The
    vertical view matches the N64 and the horizontal view widens to fill:
    about 128° at 32:9. Culling, aiming and bullet spread follow the wider
    view, so there's no pop-in at the screen edges and aim behaves exactly as
    on the N64 (D323). Menus, logos and mission select are shown 4:3 with
    bars (D325).
- **HUD layout** (`HudLayout`):
  - **STRETCH** (default): as before.
  - **EDGES**: the in-game HUD is drawn unstretched, with each element pinned
    to its screen edge. Ammo is at the bottom right (dual-wield ammo bottom
    left), pickup and dialogue text on the left, and the health/armour bars
    and countdown timer centred.
  - **16:9**: the same, but pinned to a centred 16:9 area, so nothing ends up
    in the far corners of a 21:9 or 32:9 display.
  - In both modes the crosshair is round again, but stays exactly where your
    aim is (D324).
- **Pause watch** undistorted under HOR+ / a HUD layout. On wide displays
  the sides fade to black as the watch zooms in, so the pause screen is a
  clean pillarbox (D324).
- **F10 overlay** panel drawn unstretched under HOR+ / a HUD layout (D325).
- **Fixes along the way:**
  - the FPS counter's top half was cut off by the overscan crop in every
    mode (D325);
  - a startup log line now explains that fullscreen always uses the desktop
    resolution, and the F10 windowed resolutions gain 1920x1440 (D323).

For everything that changed up to v0.3.0, see the
[v0.3.0 release notes](https://github.com/jkdansereau/goldeneye-pc-port/releases/tag/v0.3.0).

### Known issues

- The front-end **Nintendo logo renders as two white blobs**, and the
  Rareware logo is close but its texture filtering looks off (D75).
- **Particle colors: a real fix landed in v0.3.0, but the rainbow effect
  can still occur intermittently.** One genuine, reproducible cause (an
  out-of-bounds vertex-buffer write in explosion rendering) is fixed. A
  second, still-unidentified cause remains, on both Windows and Steam Deck —
  it doesn't reproduce every time or on every level, so treat this as
  improved, not fully resolved (D252).
- **Some muzzle flashes draw an extra, erroneous long flash straight up from
  the gun** (seen on the M16, among others), on top of the normal, correctly
  drawn flash. Cosmetic only (D303).
- **Surface 1: the 2D billboard trees near the start render as a solid wall
  of tree texture** instead of discrete sprites (D236). Under active
  investigation across ten+ passes; no fix yet.
- **The widescreen options are new and lightly playtested** (one setup).
  Known limits:
  - split-screen multiplayer gets no HUD anchoring and hasn't been tested
    with them;
  - HUD text keeps the N64's size relative to the screen height, so at high
    resolutions it can look large and soft. There is no HUD scale option yet
    (D226).
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
  behavior (D281, a regression of D259/D257; not yet root-caused).
- Water on `IsWater` levels shows a moving seam between two patterns (D245).
- **Occasional z-fighting on some levels' geometry** (D308) — a Dam intro/
  truck-wheel instance showing odd transparent-looking areas has also been
  seen and may be related (D306). Minor, cosmetic; not investigated for
  this release.
- **In-level security camera props can occasionally end up facing backwards**
  (the wrong direction) on some levels, seen on Bunker — this is the
  gameplay security-camera object, not the player's own view (D307). Not
  investigated for this release.
- **A rare, self-clearing visual quirk**: on an occasional animation
  transition, Bond's model can briefly render out of place. It has no effect
  on gameplay or your save, and it clears on its own or by re-entering the
  level; a safeguard added in v0.3.0 guarantees it can never affect game
  stability (D311).
- **Two collision oddities found in final playtesting**: throwable items
  (mines, grenades) can occasionally clip through walls in certain spots
  (D312), and bullet impacts don't appear on some surfaces — doors and
  windows are unaffected (D313). Both are still open; if
  you hit either, an issue with the level and spot helps a lot.

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

This fork: <https://github.com/Sleepsong/goldeneye-pc-port> (widescreen work:
findings D323–D325 in `docs/dev/findings.md`). Upstream project:
<https://github.com/jkdansereau/goldeneye-pc-port>, built on the
[GoldenEye 007 decompilation](https://github.com/n64decomp/007), architecture
after the [Perfect Dark PC port](https://github.com/fgsfdsfgs/perfect_dark).
Non-commercial fan preservation/research project; not affiliated with any
rights holder. **AI disclosure:** built through agentic AI coding (Claude
Code + a local open-weight model), directed by one person in their spare
time — as much a study of what agentic development gets wrong on a
game-sized codebase as it is a port. This fork's widescreen additions were
likewise built with Claude Code. See the README's
[Background section](https://github.com/jkdansereau/goldeneye-pc-port#background)
for the full account.
