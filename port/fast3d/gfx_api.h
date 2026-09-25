#ifndef GFX_API_H
#define GFX_API_H

#ifndef __cplusplus
#include <stdint.h>
#include <stdbool.h>
#endif

#include "gfx_rendering_api.h"
#include "gfx_window_manager_api.h"

struct XYWidthHeight {
    int16_t x, y;
    uint32_t width, height;
};

struct GfxDimensions {
    float internal_mul;
    uint32_t width, height;
    float aspect_ratio;
};

struct GfxInitSettings {
    struct GfxWindowManagerAPI *wapi;
    struct GfxRenderingAPI *rapi;
    struct GfxWindowInitSettings window_settings;
};

extern struct GfxDimensions gfx_current_window_dimensions; // The dimensions of the window
extern struct GfxDimensions
    gfx_current_dimensions; // The dimensions of the draw area the game draws to, before scaling (if applicable)
extern struct XYWidthHeight
    gfx_current_game_window_viewport; // The area of the window the game is drawn to, (0, 0) is top-left corner
extern uint32_t gfx_msaa_level;
extern struct XYWidthHeight gfx_current_native_viewport; // The internal/native video mode of the game
extern float gfx_current_native_aspect; // The aspect ratio of the above mode
extern bool gfx_framebuffers_enabled;
extern bool gfx_detail_textures_enabled;

void gfx_init(const struct GfxInitSettings *settings);
void gfx_destroy(void);
struct GfxRenderingAPI* gfx_get_current_rendering_api(void);
void gfx_start_frame(void);
void gfx_run(Gfx* commands);
void gfx_end_frame(void);
void gfx_set_target_fps(int);
void gfx_set_texture_filter(enum FilteringMode mode);
void gfx_set_mipmap_filter(enum MipmapFilteringMode mode);
void gfx_set_fix_mip_textures(int on);
void gfx_set_wrap_fix(int on);
void gfx_set_anisotropy_level(int level);  /* 1 = off; clamped to GL max */
void gfx_set_safe_area_crop(int on);       /* crop the N64 TV-overscan safe-area margin instead of showing it as black bars */
/* On-window pixel rect (top-left origin) the full VI logical canvas (0,0)-
 * (SCREEN_WIDTH, SCREEN_HEIGHT) currently maps to, honoring the safe-area
 * crop above -- for inverting a window mouse click into logical 2D UI space
 * (see port/src/optionsoverlay.c, D316). */
void gfx_get_ui_screen_rect(int32_t *outX, int32_t *outY, int32_t *outW, int32_t *outH);
/* D323: Video.AspectMode=1 -- fit the game's draw area to a centred 4:3 rect
 * inside the window (pillarbox on wide windows, letterbox on tall ones)
 * instead of stretching it to the full window. 0 = stretch (original). */
void gfx_set_aspect_fit(int on);
/* D323: on-window pixels per logical X unit divided by on-window pixels per
 * logical Y unit, as gfx_adjust_viewport_or_scissor currently maps them
 * (window/fit size + safe-area crop). 1.0 = square logical pixels, i.e. the
 * N64's own 4:3 display; 2.667 on a 32:9 window with the crop off. The Hor+
 * projection correction (port/src/video.c portScaleAspect) multiplies the
 * game's aspect by this so the world is undistorted however it is mapped. */
float gfx_get_logical_pixel_aspect(void);
void gfx_texture_cache_clear(void);
void gfx_texture_cache_delete(const uint8_t *orig_addr);
void gfx_texture_cache_delete_range(const uint8_t *start, const uint8_t *end);
int gfx_create_framebuffer(uint32_t width, uint32_t height, int upscale, int autoresize);
void gfx_resize_framebuffer(int fb, uint32_t width, uint32_t height, int upscale, int autoresize);
void gfx_set_framebuffer(int fb, float noise_scale) ;
void gfx_reset_framebuffer(void);
void gfx_copy_framebuffer(int fb_dst, int fb_src, int left, int top, int use_back);

#endif
