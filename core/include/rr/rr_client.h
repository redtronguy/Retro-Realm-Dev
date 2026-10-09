/*
 * Client game state: menus, walk/build modes, and either a local simulation
 * (solo) or a mirror of the server's world (online).
 *
 * The client never touches sockets. The platform layer:
 *   - calls rr_client_frame() 60 times a second with the pad state,
 *   - when want_connect is set, opens a connection and reports the result
 *     with rr_client_net_connected() / rr_client_net_failed(),
 *   - pumps received bytes into rr_client_feed() and writes outbox bytes out,
 *   - closes the socket when want_disconnect is set.
 */
#ifndef RR_CLIENT_H
#define RR_CLIENT_H

#include <stddef.h>
#include <stdint.h>
#include "rr_world.h"

typedef enum { RR_SCREEN_TITLE, RR_SCREEN_CONNECTING, RR_SCREEN_PLAY, RR_SCREEN_ERROR } rr_screen;
typedef enum { RR_MODE_WALK, RR_MODE_BUILD } rr_mode;

#define RR_INBOX_CAP 24576
#define RR_OUTBOX_CAP 2048

typedef struct {
    rr_world world;
    uint8_t screen, mode, online;
    int my_id;
    uint8_t have_map;
    char name[RR_NAME_LEN + 1];
    uint32_t seed;

    int cursor_x, cursor_y, tool;
    int repeat;
    uint8_t sent_intent;
    uint16_t buttons, prev_buttons;
    uint32_t frame;

    char message[40];
    int message_timer;

    /* Requests to the platform layer. */
    uint8_t want_connect, want_disconnect;

    uint8_t inbox[RR_INBOX_CAP];
    size_t inbox_len;
    uint8_t outbox[RR_OUTBOX_CAP];
    size_t outbox_len;
} rr_client;

void rr_client_init(rr_client *c, const char *name, uint32_t seed);
void rr_client_frame(rr_client *c, uint16_t buttons);

void rr_client_start_solo(rr_client *c);
void rr_client_request_online(rr_client *c);
void rr_client_net_connected(rr_client *c);
void rr_client_net_failed(rr_client *c, const char *reason);

/* Accepts up to len received bytes; returns how many were consumed. */
size_t rr_client_feed(rr_client *c, const uint8_t *data, size_t len);
void rr_client_outbox_consume(rr_client *c, size_t n);

/* The tile the camera follows (player in walk mode, cursor in build mode). */
void rr_client_focus(const rr_client *c, int *x, int *y);

#endif
