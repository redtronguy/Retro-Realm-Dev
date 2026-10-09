/*
 * Transport-agnostic server session logic. The platform layer (server/main.c)
 * owns the sockets and just shuttles bytes in and out of each peer, which
 * also lets tests run a full client <-> server session in memory.
 */
#ifndef RR_SERVER_H
#define RR_SERVER_H

#include <stddef.h>
#include <stdint.h>
#include "rr_world.h"

#define RR_PEER_IN_CAP 4096
#define RR_PEER_OUT_CAP 65536

typedef struct {
    uint8_t active, welcomed, closing;
    int player;
    uint8_t in[RR_PEER_IN_CAP];
    size_t in_len;
    uint8_t out[RR_PEER_OUT_CAP];
    size_t out_len;
} rr_peer;

typedef struct {
    rr_world world;
    rr_peer peers[RR_MAX_PLAYERS];
} rr_server;

void rr_server_init(rr_server *s, uint32_t seed);

/* Returns a peer index, or -1 if every slot is taken. */
int rr_server_peer_open(rr_server *s);
void rr_server_peer_close(rr_server *s, int peer);
void rr_server_peer_recv(rr_server *s, int peer, const uint8_t *data, size_t len);
/* Bytes waiting to be written to the peer's socket. */
const uint8_t *rr_server_peer_out(rr_server *s, int peer, size_t *len);
void rr_server_peer_sent(rr_server *s, int peer, size_t n);
/* True once the peer should be disconnected (after its output drains). */
int rr_server_peer_closing(const rr_server *s, int peer);

/* Advance the world one tick and queue replication for every peer. */
void rr_server_tick(rr_server *s);
int rr_server_player_count(const rr_server *s);

#endif
