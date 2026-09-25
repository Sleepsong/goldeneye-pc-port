#ifndef _PORTHUD_H_
#define _PORTHUD_H_

/*
 * D324: port-side HUD anchoring hooks, called from #ifdef PORT blocks in the
 * game's HUD code (src/game/bondview2.c, gunfire.c, propobj.c). They emit
 * PD's G_EXTRAGEOMETRYMODE_EXT aspect command (fast3d, port/fast3d/gbiex.h),
 * which draws the following 2D/3D unstretched (square logical pixels) and
 * slides it to a screen edge. Each returns gdl untouched -- nothing emitted --
 * while its option is off, so the display list is byte-identical at defaults.
 * Single-player only: split-screen viewports would need per-quadrant anchors.
 */

#include <PR/ultratypes.h>
#include <PR/gbi.h>

#define PORT_HUD_NONE   0   /* back to the plain full-window stretch */
#define PORT_HUD_LEFT   1   /* canvas left edge on the screen/16:9 left edge */
#define PORT_HUD_RIGHT  2   /* canvas right edge on the screen/16:9 right edge */
#define PORT_HUD_CENTER 3   /* unstretched about the screen centre */

#ifdef __cplusplus
extern "C" {
#endif

/* 2D HUD element anchor; gated by Video.HudLayout. Pair every
 * LEFT/RIGHT/CENTER with a PORT_HUD_NONE after the element. */
Gfx *portHudAnchor(Gfx *gdl, s32 anchor);

/* The pause watch's own fixed-aspect 3D (bondviewRenderWatch): CENTER while
 * on. Gated by Video.AspectMode=2 (Hor+, where it is the 3D-correct
 * projection) or Video.HudLayout (it is the pause UI). */
Gfx *portWatchAspect(Gfx *gdl, s32 on);

/* Multiplier for a centred HUD sprite's horizontal half-size (the crosshair)
 * that makes it unstretched while its position -- which must keep matching
 * the aim direction -- stays in the stretched mapping. 1.0 when off. */
f32 portHudSpriteWidthScale(void);

#ifdef __cplusplus
}
#endif

#endif /* _PORTHUD_H_ */
