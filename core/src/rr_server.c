#include "rr/rr_server.h"

#include <string.h>
#include "rr/rr_proto.h"

void rr_server_init(rr_server *s, uint32_t seed)
{
    memset(s->peers, 0, sizeof(s->peers));
    rr_world_init(&s->world, seed);
}

int rr_server_peer_open(rr_server *s)
{
    for (int i = 0; i < RR_MAX_PLAYERS; i++) {
        rr_peer *p = &s->peers[i];
        if (p->active) continue;
        p->active = 1;
        p->welcomed = 0;
        p->closing = 0;
        p->player = -1;
        p->in_len = 0;
        p->out_len = 0;
        return i;
    }
    return -1;
}

void rr_server_peer_close(rr_server *s, int peer)
{
    rr_peer *p = &s->peers[peer];
    if (!p->active) return;
    if (p->player >= 0) rr_world_remove_player(&s->world, p->player);
    p->active = 0;
    p->player = -1;
}

/* Runs `fn` against the free space of the peer's output buffer. A peer that
 * can't keep up (buffer full) is disconnected rather than stalling the world. */
#define PEER_SEND(p, call)                                                \
    do {                                                                  \
        rr_writer w_;                                                     \
        rr_writer_init(&w_, (p)->out + (p)->out_len, RR_PEER_OUT_CAP - (p)->out_len); \
        rr_writer *w = &w_;                                               \
        if (call) (p)->out_len += w_.len;                                 \
        else (p)->closing = 1;                                            \
    } while (0)

static void reject(rr_peer *p, const char *reason)
{
    PEER_SEND(p, rr_msg_reject(w, reason));
    p->closing = 1;
}

static void handle_frame(rr_server *s, rr_peer *p, uint8_t type, const uint8_t *payload, size_t plen)
{
    rr_reader r;
    rr_reader_init(&r, payload, plen);
    switch (type) {
    case RR_MSG_HELLO: {
        uint8_t version;
        char name[RR_NAME_LEN + 1];
        if (p->welcomed) return;
        if (!rr_read_hello(&r, &version, name)) {
            reject(p, "BAD HELLO");
            return;
        }
        if (version != RR_PROTOCOL_VERSION) {
            reject(p, "VERSION MISMATCH");
            return;
        }
        int id = rr_world_add_player(&s->world, name);
        if (id < 0) {
            reject(p, "SERVER FULL");
            return;
        }
        p->player = id;
        p->welcomed = 1;
        PEER_SEND(p, rr_msg_welcome(w, (uint8_t)id, s->world.seed));
        PEER_SEND(p, rr_msg_map(w, &s->world));
        PEER_SEND(p, rr_msg_snapshot(w, &s->world));
        break;
    }
    case RR_MSG_CMD: {
        rr_command c;
        if (p->welcomed && rr_read_cmd(&r, &c)) rr_world_apply(&s->world, p->player, &c);
        break;
    }
    default:
        break; /* unknown messages are ignored for forward compatibility */
    }
}

void rr_server_peer_recv(rr_server *s, int peer, const uint8_t *data, size_t len)
{
    rr_peer *p = &s->peers[peer];
    if (!p->active || p->closing) return;
    while (len > 0) {
        size_t n = RR_PEER_IN_CAP - p->in_len;
        if (n > len) n = len;
        memcpy(p->in + p->in_len, data, n);
        p->in_len += n;
        data += n;
        len -= n;

        for (;;) {
            uint8_t type;
            const uint8_t *payload;
            size_t plen;
            long f = rr_frame_peek(p->in, p->in_len, &type, &payload, &plen);
            if (f < 0 || (f == 0 && p->in_len == RR_PEER_IN_CAP)) {
                p->closing = 1;
                return;
            }
            if (f == 0) break;
            handle_frame(s, p, type, payload, plen);
            memmove(p->in, p->in + f, p->in_len - (size_t)f);
            p->in_len -= (size_t)f;
            if (p->closing) return;
        }
    }
}

const uint8_t *rr_server_peer_out(rr_server *s, int peer, size_t *len)
{
    *len = s->peers[peer].out_len;
    return s->peers[peer].out;
}

void rr_server_peer_sent(rr_server *s, int peer, size_t n)
{
    rr_peer *p = &s->peers[peer];
    if (n > p->out_len) n = p->out_len;
    memmove(p->out, p->out + n, p->out_len - n);
    p->out_len -= n;
}

int rr_server_peer_closing(const rr_server *s, int peer) { return s->peers[peer].closing; }

void rr_server_tick(rr_server *s)
{
    rr_world_tick(&s->world);
    for (int i = 0; i < RR_MAX_PLAYERS; i++) {
        rr_peer *p = &s->peers[i];
        if (!p->active || !p->welcomed || p->closing) continue;
        if (s->world.changes_overflow) PEER_SEND(p, rr_msg_map(w, &s->world));
        else if (s->world.change_count) PEER_SEND(p, rr_msg_tiles(w, &s->world));
        PEER_SEND(p, rr_msg_snapshot(w, &s->world));
    }
    rr_world_clear_changes(&s->world);
}

int rr_server_player_count(const rr_server *s)
{
    int n = 0;
    for (int i = 0; i < RR_MAX_PLAYERS; i++) n += s->peers[i].active && s->peers[i].welcomed;
    return n;
}
