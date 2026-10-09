#include "rr/rr_client.h"

#include <stdio.h>
#include <string.h>
#include "rr/rr_input.h"
#include "rr/rr_proto.h"

#define FRAMES_PER_TICK (RR_FRAME_HZ / RR_TICK_HZ)

static void say(rr_client *c, const char *msg)
{
    snprintf(c->message, sizeof(c->message), "%s", msg);
    c->message_timer = RR_FRAME_HZ * 2;
}

void rr_client_init(rr_client *c, const char *name, uint32_t seed)
{
    memset(c, 0, sizeof(*c));
    snprintf(c->name, sizeof(c->name), "%s", name ? name : "HERO");
    c->seed = seed;
    c->my_id = -1;
    c->screen = RR_SCREEN_TITLE;
}

static void reset_session(rr_client *c)
{
    memset(&c->world, 0, sizeof(c->world));
    c->my_id = -1;
    c->have_map = 0;
    c->mode = RR_MODE_WALK;
    c->tool = RR_TOOL_ROAD;
    c->sent_intent = RR_DIR_NONE;
    c->inbox_len = 0;
    c->outbox_len = 0;
    c->message_timer = 0;
}

void rr_client_start_solo(rr_client *c)
{
    reset_session(c);
    c->online = 0;
    rr_world_init(&c->world, c->seed);
    c->my_id = rr_world_add_player(&c->world, c->name);
    c->have_map = 1;
    c->screen = RR_SCREEN_PLAY;
    say(c, "WELCOME TO THE REALM");
}

void rr_client_request_online(rr_client *c)
{
    reset_session(c);
    c->online = 1;
    c->screen = RR_SCREEN_CONNECTING;
    c->want_connect = 1;
}

void rr_client_net_connected(rr_client *c)
{
    rr_writer w;
    c->want_connect = 0;
    rr_writer_init(&w, c->outbox + c->outbox_len, RR_OUTBOX_CAP - c->outbox_len);
    if (rr_msg_hello(&w, c->name)) c->outbox_len += w.len;
}

void rr_client_net_failed(rr_client *c, const char *reason)
{
    c->want_connect = 0;
    c->want_disconnect = 1;
    c->screen = RR_SCREEN_ERROR;
    snprintf(c->message, sizeof(c->message), "%s", reason ? reason : "NETWORK ERROR");
}

void rr_client_outbox_consume(rr_client *c, size_t n)
{
    if (n > c->outbox_len) n = c->outbox_len;
    memmove(c->outbox, c->outbox + n, c->outbox_len - n);
    c->outbox_len -= n;
}

void rr_client_focus(const rr_client *c, int *x, int *y)
{
    if (c->mode == RR_MODE_BUILD) {
        *x = c->cursor_x;
        *y = c->cursor_y;
        return;
    }
    if (c->my_id >= 0 && c->world.players[c->my_id].active) {
        *x = c->world.players[c->my_id].x;
        *y = c->world.players[c->my_id].y;
        return;
    }
    *x = c->world.spawn_x;
    *y = c->world.spawn_y;
}

/* ---- network input ------------------------------------------------------ */

static void handle_frame(rr_client *c, uint8_t type, const uint8_t *payload, size_t plen)
{
    rr_reader r;
    rr_reader_init(&r, payload, plen);
    int ok = 1;
    switch (type) {
    case RR_MSG_WELCOME:
        c->my_id = rr_r_u8(&r);
        c->world.seed = rr_r_u32(&r);
        ok = !r.err && c->my_id < RR_MAX_PLAYERS;
        break;
    case RR_MSG_MAP:
        ok = rr_read_map(&r, &c->world);
        c->have_map = (uint8_t)ok;
        break;
    case RR_MSG_SNAPSHOT:
        ok = rr_read_snapshot(&r, &c->world);
        break;
    case RR_MSG_TILES:
        ok = rr_read_tiles(&r, &c->world);
        break;
    case RR_MSG_REJECT: {
        char reason[32];
        size_t n = plen < sizeof(reason) - 1 ? plen : sizeof(reason) - 1;
        memcpy(reason, payload, n);
        reason[n] = 0;
        rr_client_net_failed(c, reason);
        return;
    }
    default:
        break;
    }
    if (!ok) {
        rr_client_net_failed(c, "PROTOCOL ERROR");
        return;
    }
    if (c->screen == RR_SCREEN_CONNECTING && c->have_map && c->my_id >= 0) {
        c->screen = RR_SCREEN_PLAY;
        say(c, "CONNECTED TO REALM");
    }
}

size_t rr_client_feed(rr_client *c, const uint8_t *data, size_t len)
{
    if (!c->online || c->screen == RR_SCREEN_ERROR) return len; /* drop */
    size_t n = RR_INBOX_CAP - c->inbox_len;
    if (n > len) n = len;
    memcpy(c->inbox + c->inbox_len, data, n);
    c->inbox_len += n;

    size_t pos = 0;
    for (;;) {
        uint8_t type;
        const uint8_t *payload;
        size_t plen;
        long f = rr_frame_peek(c->inbox + pos, c->inbox_len - pos, &type, &payload, &plen);
        if (f < 0) {
            rr_client_net_failed(c, "PROTOCOL ERROR");
            c->inbox_len = 0;
            return len;
        }
        if (f == 0) break;
        handle_frame(c, type, payload, plen);
        pos += (size_t)f;
        if (c->screen == RR_SCREEN_ERROR) {
            c->inbox_len = 0;
            return len;
        }
    }
    memmove(c->inbox, c->inbox + pos, c->inbox_len - pos);
    c->inbox_len -= pos;
    return n;
}

/* ---- gameplay input ----------------------------------------------------- */

static void send_cmd(rr_client *c, uint8_t type, uint8_t arg, int x, int y)
{
    rr_command cmd = {type, arg, (uint8_t)x, (uint8_t)y};
    if (!c->online) {
        rr_world_apply(&c->world, c->my_id, &cmd);
        return;
    }
    rr_writer w;
    rr_writer_init(&w, c->outbox + c->outbox_len, RR_OUTBOX_CAP - c->outbox_len);
    if (rr_msg_cmd(&w, &cmd)) c->outbox_len += w.len;
}

static void try_build(rr_client *c)
{
    int cost = rr_world_can_build(&c->world, c->tool, c->cursor_x, c->cursor_y);
    if (cost == RR_BUILD_NO_FUNDS) {
        say(c, "NOT ENOUGH FUNDS");
        return;
    }
    if (cost == RR_BUILD_OCCUPIED) {
        say(c, "SOMEONE IS IN THE WAY");
        return;
    }
    if (cost < 0) return; /* silently skip invalid tiles so road painting feels smooth */
    send_cmd(c, RR_CMD_BUILD, (uint8_t)c->tool, c->cursor_x, c->cursor_y);
}

static int held_dir(uint16_t b)
{
    if (b & RR_BTN_UP) return RR_DIR_UP;
    if (b & RR_BTN_DOWN) return RR_DIR_DOWN;
    if (b & RR_BTN_LEFT) return RR_DIR_LEFT;
    if (b & RR_BTN_RIGHT) return RR_DIR_RIGHT;
    return RR_DIR_NONE;
}

static void play_input(rr_client *c, uint16_t held, uint16_t pressed)
{
    if (pressed & (RR_BTN_X | RR_BTN_SELECT)) {
        if (c->mode == RR_MODE_WALK) {
            int x, y;
            rr_client_focus(c, &x, &y);
            c->cursor_x = x;
            c->cursor_y = y;
            c->mode = RR_MODE_BUILD;
            send_cmd(c, RR_CMD_INTENT, RR_DIR_NONE, 0, 0);
            c->sent_intent = RR_DIR_NONE;
        } else {
            c->mode = RR_MODE_WALK;
        }
        return;
    }

    if (c->mode == RR_MODE_WALK) {
        uint8_t dir = (uint8_t)held_dir(held);
        if (dir != c->sent_intent) {
            send_cmd(c, RR_CMD_INTENT, dir, 0, 0);
            c->sent_intent = dir;
        }
        if (pressed & RR_BTN_A) send_cmd(c, RR_CMD_ATTACK, 0, 0, 0);
        return;
    }

    /* Build mode: d-pad moves the cursor with key repeat; holding A paints. */
    if (pressed & RR_BTN_B) {
        c->mode = RR_MODE_WALK;
        return;
    }
    if (pressed & RR_BTN_L) c->tool = (c->tool + RR_TOOL_COUNT - 1) % RR_TOOL_COUNT;
    if (pressed & RR_BTN_R) c->tool = (c->tool + 1) % RR_TOOL_COUNT;

    int dir = held_dir(held), moved = 0;
    if (dir && held_dir(pressed) == dir) {
        moved = 1;
        c->repeat = 12;
    } else if (dir && --c->repeat <= 0) {
        moved = 1;
        c->repeat = 3;
    }
    if (moved) {
        if (dir == RR_DIR_UP && c->cursor_y > 0) c->cursor_y--;
        if (dir == RR_DIR_DOWN && c->cursor_y < RR_MAP_H - 1) c->cursor_y++;
        if (dir == RR_DIR_LEFT && c->cursor_x > 0) c->cursor_x--;
        if (dir == RR_DIR_RIGHT && c->cursor_x < RR_MAP_W - 1) c->cursor_x++;
    }
    if ((pressed & RR_BTN_A) || (moved && (held & RR_BTN_A))) try_build(c);
}

void rr_client_frame(rr_client *c, uint16_t buttons)
{
    uint16_t pressed = (uint16_t)(buttons & ~c->prev_buttons);
    c->prev_buttons = buttons;
    c->buttons = buttons;
    c->frame++;
    if (c->message_timer) c->message_timer--;

    switch (c->screen) {
    case RR_SCREEN_TITLE:
        if (pressed & (RR_BTN_A | RR_BTN_START)) rr_client_start_solo(c);
        else if (pressed & RR_BTN_B) rr_client_request_online(c);
        break;
    case RR_SCREEN_CONNECTING:
        if (pressed & RR_BTN_B) {
            c->want_connect = 0;
            c->want_disconnect = 1;
            c->screen = RR_SCREEN_TITLE;
        }
        break;
    case RR_SCREEN_ERROR:
        if (pressed & (RR_BTN_A | RR_BTN_START)) c->screen = RR_SCREEN_TITLE;
        break;
    case RR_SCREEN_PLAY: {
        int level = c->my_id >= 0 ? c->world.players[c->my_id].level : 0;
        play_input(c, buttons, pressed);
        if (!c->online) {
            if (c->frame % FRAMES_PER_TICK == 0) rr_world_tick(&c->world);
            rr_world_clear_changes(&c->world);
        }
        if (c->my_id >= 0 && c->world.players[c->my_id].level > level && level > 0) say(c, "LEVEL UP!");
        break;
    }
    }
}
