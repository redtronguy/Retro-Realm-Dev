#include "rr/rr_proto.h"

#include <string.h>

/* ---- primitive writer / reader ----------------------------------------- */

void rr_writer_init(rr_writer *w, uint8_t *buf, size_t cap)
{
    w->buf = buf;
    w->cap = cap;
    w->len = 0;
    w->err = 0;
}

void rr_w_bytes(rr_writer *w, const void *p, size_t n)
{
    if (w->err || w->len + n > w->cap) {
        w->err = 1;
        return;
    }
    memcpy(w->buf + w->len, p, n);
    w->len += n;
}

void rr_w_u8(rr_writer *w, uint8_t v) { rr_w_bytes(w, &v, 1); }

void rr_w_u16(rr_writer *w, uint16_t v)
{
    uint8_t b[2] = {(uint8_t)v, (uint8_t)(v >> 8)};
    rr_w_bytes(w, b, 2);
}

void rr_w_u32(rr_writer *w, uint32_t v)
{
    uint8_t b[4] = {(uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24)};
    rr_w_bytes(w, b, 4);
}

void rr_reader_init(rr_reader *r, const uint8_t *buf, size_t len)
{
    r->buf = buf;
    r->len = len;
    r->pos = 0;
    r->err = 0;
}

void rr_r_bytes(rr_reader *r, void *p, size_t n)
{
    if (r->err || r->pos + n > r->len) {
        r->err = 1;
        memset(p, 0, n);
        return;
    }
    memcpy(p, r->buf + r->pos, n);
    r->pos += n;
}

uint8_t rr_r_u8(rr_reader *r)
{
    uint8_t v;
    rr_r_bytes(r, &v, 1);
    return v;
}

uint16_t rr_r_u16(rr_reader *r)
{
    uint8_t b[2];
    rr_r_bytes(r, b, 2);
    return (uint16_t)(b[0] | b[1] << 8);
}

uint32_t rr_r_u32(rr_reader *r)
{
    uint8_t b[4];
    rr_r_bytes(r, b, 4);
    return (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
}

/* ---- framing ------------------------------------------------------------ */

size_t rr_frame_begin(rr_writer *w, uint8_t type)
{
    size_t start = w->len;
    rr_w_u16(w, 0);
    rr_w_u8(w, type);
    return start;
}

void rr_frame_end(rr_writer *w, size_t start)
{
    if (w->err) return;
    size_t n = w->len - start - 2;
    if (n > RR_MAX_FRAME) {
        w->err = 1;
        return;
    }
    w->buf[start] = (uint8_t)n;
    w->buf[start + 1] = (uint8_t)(n >> 8);
}

long rr_frame_peek(const uint8_t *buf, size_t len, uint8_t *type, const uint8_t **payload, size_t *plen)
{
    if (len < 2) return 0;
    size_t n = (size_t)buf[0] | (size_t)buf[1] << 8;
    if (n < 1 || n > RR_MAX_FRAME) return -1;
    if (len < n + 2) return 0;
    *type = buf[2];
    *payload = buf + 3;
    *plen = n - 1;
    return (long)(n + 2);
}

/* Writes the whole frame or nothing, so a full buffer never leaves half a frame. */
#define FRAME(w, type, ...)               \
    do {                                   \
        size_t saved_ = (w)->len;          \
        size_t s_ = rr_frame_begin(w, type); \
        __VA_ARGS__;                          \
        rr_frame_end(w, s_);               \
        if ((w)->err) {                    \
            (w)->len = saved_;             \
            (w)->err = 0;                  \
            return 0;                      \
        }                                  \
        return 1;                          \
    } while (0)

static void w_name(rr_writer *w, const char *name)
{
    char buf[RR_NAME_LEN] = {0};
    size_t n = name ? strlen(name) : 0;
    if (n) memcpy(buf, name, n > RR_NAME_LEN ? RR_NAME_LEN : n);
    rr_w_bytes(w, buf, RR_NAME_LEN);
}

static void r_name(rr_reader *r, char out[RR_NAME_LEN + 1])
{
    rr_r_bytes(r, out, RR_NAME_LEN);
    out[RR_NAME_LEN] = 0;
}

/* ---- messages ----------------------------------------------------------- */

int rr_msg_hello(rr_writer *w, const char *name)
{
    FRAME(w, RR_MSG_HELLO, {
        rr_w_u8(w, RR_PROTOCOL_VERSION);
        w_name(w, name);
    });
}

int rr_msg_cmd(rr_writer *w, const rr_command *c)
{
    FRAME(w, RR_MSG_CMD, {
        rr_w_u8(w, c->type);
        rr_w_u8(w, c->arg);
        rr_w_u8(w, c->x);
        rr_w_u8(w, c->y);
    });
}

int rr_msg_welcome(rr_writer *w, uint8_t player_id, uint32_t seed)
{
    FRAME(w, RR_MSG_WELCOME, {
        rr_w_u8(w, player_id);
        rr_w_u32(w, seed);
    });
}

int rr_msg_map(rr_writer *w, const rr_world *world)
{
    FRAME(w, RR_MSG_MAP, {
        rr_w_u8(w, world->spawn_x);
        rr_w_u8(w, world->spawn_y);
        rr_w_bytes(w, world->tiles, sizeof(world->tiles));
    });
}

int rr_msg_snapshot(rr_writer *w, const rr_world *world)
{
    FRAME(w, RR_MSG_SNAPSHOT, {
        const rr_city *c = &world->city;
        rr_w_u32(w, world->tick);
        rr_w_u32(w, (uint32_t)c->funds);
        rr_w_u16(w, c->population);
        rr_w_u16(w, c->jobs);
        rr_w_u16(w, (uint16_t)c->demand_r);
        rr_w_u16(w, (uint16_t)c->demand_c);
        rr_w_u16(w, (uint16_t)c->demand_i);
        rr_w_u8(w, c->tax_rate);
        rr_w_u16(w, c->day);
        rr_w_u16(w, c->time);

        uint8_t mask = 0;
        for (int i = 0; i < RR_MAX_PLAYERS; i++)
            if (world->players[i].active) mask |= (uint8_t)(1u << i);
        rr_w_u8(w, mask);
        for (int i = 0; i < RR_MAX_PLAYERS; i++) {
            const rr_player *p = &world->players[i];
            if (!p->active) continue;
            rr_w_u8(w, p->x);
            rr_w_u8(w, p->y);
            rr_w_u8(w, p->facing);
            rr_w_u8(w, p->hp);
            rr_w_u8(w, p->max_hp);
            rr_w_u8(w, p->level);
            rr_w_u8(w, p->hurt);
            rr_w_u16(w, p->xp);
            rr_w_u32(w, p->gold);
            w_name(w, p->name);
        }

        uint8_t n = 0;
        for (int i = 0; i < RR_MAX_MOBS; i++) n += world->mobs[i].kind != RR_MOB_NONE;
        rr_w_u8(w, n);
        for (int i = 0; i < RR_MAX_MOBS; i++) {
            const rr_mob *m = &world->mobs[i];
            if (!m->kind) continue;
            rr_w_u8(w, (uint8_t)i);
            rr_w_u8(w, m->kind);
            rr_w_u8(w, m->x);
            rr_w_u8(w, m->y);
            rr_w_u8(w, m->hp);
            rr_w_u8(w, m->facing);
            rr_w_u8(w, m->hurt);
        }
    });
}

int rr_msg_tiles(rr_writer *w, const rr_world *world)
{
    FRAME(w, RR_MSG_TILES, {
        rr_w_u16(w, world->change_count);
        for (int i = 0; i < world->change_count; i++) {
            uint8_t x = world->changes[i][0], y = world->changes[i][1];
            rr_w_u8(w, x);
            rr_w_u8(w, y);
            rr_w_u8(w, world->tiles[y][x]);
        }
    });
}

int rr_msg_reject(rr_writer *w, const char *reason)
{
    FRAME(w, RR_MSG_REJECT, { rr_w_bytes(w, reason, strlen(reason)); });
}

int rr_read_hello(rr_reader *r, uint8_t *version, char name[RR_NAME_LEN + 1])
{
    *version = rr_r_u8(r);
    r_name(r, name);
    return !r->err;
}

int rr_read_cmd(rr_reader *r, rr_command *c)
{
    c->type = rr_r_u8(r);
    c->arg = rr_r_u8(r);
    c->x = rr_r_u8(r);
    c->y = rr_r_u8(r);
    return !r->err;
}

int rr_read_map(rr_reader *r, rr_world *world)
{
    uint8_t sx = rr_r_u8(r), sy = rr_r_u8(r);
    if (r->err || r->len - r->pos != sizeof(world->tiles)) return 0;
    world->spawn_x = sx;
    world->spawn_y = sy;
    rr_r_bytes(r, world->tiles, sizeof(world->tiles));
    for (int y = 0; y < RR_MAP_H; y++)
        for (int x = 0; x < RR_MAP_W; x++)
            if (world->tiles[y][x] >= RR_T_COUNT) world->tiles[y][x] = RR_T_GRASS;
    rr_world_rescan(world);
    return !r->err;
}

int rr_read_snapshot(rr_reader *r, rr_world *world)
{
    rr_city c;
    uint32_t tick = rr_r_u32(r);
    c.funds = (int32_t)rr_r_u32(r);
    c.population = rr_r_u16(r);
    c.jobs = rr_r_u16(r);
    c.demand_r = (int16_t)rr_r_u16(r);
    c.demand_c = (int16_t)rr_r_u16(r);
    c.demand_i = (int16_t)rr_r_u16(r);
    c.tax_rate = rr_r_u8(r);
    c.day = rr_r_u16(r);
    c.time = rr_r_u16(r);

    rr_player players[RR_MAX_PLAYERS];
    memset(players, 0, sizeof(players));
    uint8_t mask = rr_r_u8(r);
    for (int i = 0; i < RR_MAX_PLAYERS; i++) {
        if (!(mask & (1u << i))) continue;
        rr_player *p = &players[i];
        p->active = 1;
        p->x = rr_r_u8(r);
        p->y = rr_r_u8(r);
        p->facing = rr_r_u8(r);
        p->hp = rr_r_u8(r);
        p->max_hp = rr_r_u8(r);
        p->level = rr_r_u8(r);
        p->hurt = rr_r_u8(r);
        p->xp = rr_r_u16(r);
        p->gold = rr_r_u32(r);
        r_name(r, p->name);
    }

    rr_mob mobs[RR_MAX_MOBS];
    memset(mobs, 0, sizeof(mobs));
    uint8_t n = rr_r_u8(r);
    for (int k = 0; k < n && !r->err; k++) {
        uint8_t i = rr_r_u8(r);
        rr_mob m;
        m.kind = rr_r_u8(r);
        m.x = rr_r_u8(r);
        m.y = rr_r_u8(r);
        m.hp = rr_r_u8(r);
        m.facing = rr_r_u8(r);
        m.hurt = rr_r_u8(r);
        m.cd = 0;
        if (i >= RR_MAX_MOBS || m.kind > RR_MOB_TROLL) r->err = 1;
        else mobs[i] = m;
    }
    if (r->err) return 0;

    world->tick = tick;
    world->city = c;
    memcpy(world->players, players, sizeof(players));
    memcpy(world->mobs, mobs, sizeof(mobs));
    return 1;
}

int rr_read_tiles(rr_reader *r, rr_world *world)
{
    uint16_t n = rr_r_u16(r);
    for (int i = 0; i < n && !r->err; i++) {
        uint8_t x = rr_r_u8(r), y = rr_r_u8(r), t = rr_r_u8(r);
        if (r->err) break;
        if (x < RR_MAP_W && y < RR_MAP_H && t < RR_T_COUNT) world->tiles[y][x] = t;
    }
    return !r->err;
}
