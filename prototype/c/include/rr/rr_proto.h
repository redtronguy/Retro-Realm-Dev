/*
 * Wire protocol: length-prefixed binary frames over a TCP stream.
 *
 *   frame := u16 length (of type + payload, little endian) | u8 type | payload
 *
 * Clients send HELLO once, then CMD frames. The server answers HELLO with
 * WELCOME + MAP (or REJECT), then streams TILES deltas and SNAPSHOTs every tick.
 */
#ifndef RR_PROTO_H
#define RR_PROTO_H

#include <stddef.h>
#include <stdint.h>
#include "rr_world.h"

enum {
    RR_MSG_HELLO = 1,    /* c->s: u8 protocol version, name[RR_NAME_LEN] */
    RR_MSG_CMD = 2,      /* c->s: rr_command */
    RR_MSG_WELCOME = 10, /* s->c: u8 player id, u32 seed */
    RR_MSG_MAP = 11,     /* s->c: u8 spawn x, u8 spawn y, tiles[] */
    RR_MSG_SNAPSHOT = 12,/* s->c: tick, city, players, mobs */
    RR_MSG_TILES = 13,   /* s->c: u16 count, (u8 x, u8 y, u8 tile)* */
    RR_MSG_REJECT = 14,  /* s->c: reason string */
};

#define RR_MAX_FRAME 20000

typedef struct {
    uint8_t *buf;
    size_t cap, len;
    int err;
} rr_writer;

typedef struct {
    const uint8_t *buf;
    size_t len, pos;
    int err;
} rr_reader;

void rr_writer_init(rr_writer *w, uint8_t *buf, size_t cap);
void rr_w_u8(rr_writer *w, uint8_t v);
void rr_w_u16(rr_writer *w, uint16_t v);
void rr_w_u32(rr_writer *w, uint32_t v);
void rr_w_bytes(rr_writer *w, const void *p, size_t n);

void rr_reader_init(rr_reader *r, const uint8_t *buf, size_t len);
uint8_t rr_r_u8(rr_reader *r);
uint16_t rr_r_u16(rr_reader *r);
uint32_t rr_r_u32(rr_reader *r);
void rr_r_bytes(rr_reader *r, void *p, size_t n);

/* Frame helpers. begin returns the offset to pass to end. */
size_t rr_frame_begin(rr_writer *w, uint8_t type);
void rr_frame_end(rr_writer *w, size_t start);
/* Returns total frame size if a full frame is buffered, 0 if incomplete, -1 if malformed. */
long rr_frame_peek(const uint8_t *buf, size_t len, uint8_t *type, const uint8_t **payload, size_t *plen);

/* Encoders append one complete frame. They return 0 if the buffer is full. */
int rr_msg_hello(rr_writer *w, const char *name);
int rr_msg_cmd(rr_writer *w, const rr_command *c);
int rr_msg_welcome(rr_writer *w, uint8_t player_id, uint32_t seed);
int rr_msg_map(rr_writer *w, const rr_world *world);
int rr_msg_snapshot(rr_writer *w, const rr_world *world);
int rr_msg_tiles(rr_writer *w, const rr_world *world);
int rr_msg_reject(rr_writer *w, const char *reason);

/* Decoders read a frame payload. They return 0 on malformed input. */
int rr_read_hello(rr_reader *r, uint8_t *version, char name[RR_NAME_LEN + 1]);
int rr_read_cmd(rr_reader *r, rr_command *c);
int rr_read_map(rr_reader *r, rr_world *world);
int rr_read_snapshot(rr_reader *r, rr_world *world);
int rr_read_tiles(rr_reader *r, rr_world *world);

#endif
