/* Retro Realm - global constants shared by every platform and the server. */
#ifndef RR_CONFIG_H
#define RR_CONFIG_H

#define RR_VERSION_STRING "0.1.0"
#define RR_PROTOCOL_VERSION 1

/* SNES-style internal resolution. Every platform scales this framebuffer. */
#define RR_SCREEN_W 256
#define RR_SCREEN_H 224
#define RR_HUD_H 16
#define RR_TILE 16

#define RR_MAP_W 128
#define RR_MAP_H 128

#define RR_MAX_PLAYERS 8
#define RR_MAX_MOBS 32
#define RR_MAX_DUNGEONS 8
#define RR_NAME_LEN 12

/* Simulation runs at a fixed tick; clients render at 60 fps. */
#define RR_TICK_HZ 10
#define RR_FRAME_HZ 60
#define RR_DAY_TICKS 2400 /* one in-game day = 4 real minutes */
#define RR_HOUR_TICKS (RR_DAY_TICKS / 24)

#define RR_DEFAULT_PORT 7777

#endif
