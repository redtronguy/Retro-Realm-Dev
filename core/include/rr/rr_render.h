/*
 * Software renderer into an 8-bit indexed framebuffer (RR_SCREEN_W x RR_SCREEN_H),
 * like a SNES PPU. Platforms convert indices to real colours with the palette,
 * which also carries the day/night tint.
 */
#ifndef RR_RENDER_H
#define RR_RENDER_H

#include <stdint.h>
#include "rr_client.h"

#define RR_PALETTE_SIZE 32

/* Builds tile and sprite atlases. Returns 0 if built-in art is malformed. */
int rr_render_init(void);
void rr_render_frame(uint8_t *fb, const rr_client *c);
/* Current palette as 0xRRGGBB, with the day/night tint applied. */
void rr_render_palette(const rr_client *c, uint32_t out[RR_PALETTE_SIZE]);

/* Drawing primitives, also used by platform overlays. */
void rr_draw_rect(uint8_t *fb, int x, int y, int w, int h, uint8_t color);
int rr_draw_text(uint8_t *fb, int x, int y, const char *s, uint8_t color, int scale);
int rr_text_width(const char *s, int scale);

#endif
