/*
 * The shared, deterministic game simulation.
 *
 * The server owns the authoritative rr_world. Clients keep a mirror that is
 * filled from network snapshots, or run the simulation locally in solo mode.
 * No OS, allocation or I/O in here, so it runs unchanged on an ESP32.
 */
#ifndef RR_WORLD_H
#define RR_WORLD_H

#include <stdint.h>
#include "rr_config.h"

typedef enum {
    RR_T_GRASS,
    RR_T_WATER,
    RR_T_SAND,
    RR_T_FOREST,
    RR_T_MOUNTAIN,
    RR_T_ROAD,
    RR_T_ZONE_R,
    RR_T_ZONE_C,
    RR_T_ZONE_I,
    RR_T_HOUSE,
    RR_T_SHOP,
    RR_T_FACTORY,
    RR_T_TAVERN,
    RR_T_DUNGEON,
    RR_T_RUBBLE,
    RR_T_COUNT
} rr_tile;

typedef enum { RR_DIR_NONE, RR_DIR_UP, RR_DIR_DOWN, RR_DIR_LEFT, RR_DIR_RIGHT } rr_dir;

typedef enum {
    RR_TOOL_ROAD,
    RR_TOOL_ZONE_R,
    RR_TOOL_ZONE_C,
    RR_TOOL_ZONE_I,
    RR_TOOL_TAVERN,
    RR_TOOL_BULLDOZE,
    RR_TOOL_COUNT
} rr_tool;

typedef enum { RR_CMD_NONE, RR_CMD_INTENT, RR_CMD_ATTACK, RR_CMD_BUILD } rr_cmd_type;

/* Everything a player can ask the simulation to do. Sent over the wire as-is. */
typedef struct {
    uint8_t type; /* rr_cmd_type */
    uint8_t arg;  /* direction for INTENT, tool for BUILD */
    uint8_t x, y; /* target tile for BUILD */
} rr_command;

/* rr_world_can_build() failure codes (success returns the cost, >= 0). */
#define RR_BUILD_INVALID (-1)
#define RR_BUILD_NO_FUNDS (-2)
#define RR_BUILD_OCCUPIED (-3)

typedef enum { RR_MOB_NONE, RR_MOB_SLIME, RR_MOB_WOLF, RR_MOB_TROLL } rr_mob_kind;

typedef struct {
    uint8_t active;
    uint8_t x, y;
    uint8_t facing; /* rr_dir */
    uint8_t intent; /* held direction, rr_dir */
    uint8_t move_cd, attack_cd, hurt;
    uint8_t hp, max_hp, level;
    uint16_t xp;
    uint32_t gold;
    char name[RR_NAME_LEN + 1];
} rr_player;

typedef struct {
    uint8_t kind; /* rr_mob_kind, NONE = slot free */
    uint8_t x, y, hp, cd, facing, hurt;
} rr_mob;

typedef struct {
    int32_t funds;
    uint16_t population, jobs;
    int16_t demand_r, demand_c, demand_i;
    uint8_t tax_rate;
    uint16_t day;  /* 1-based */
    uint16_t time; /* ticks into the current day */
} rr_city;

#define RR_MAX_CHANGES 512

typedef struct {
    uint32_t seed;
    uint32_t rng;
    uint32_t tick;
    uint8_t tiles[RR_MAP_H][RR_MAP_W];
    rr_player players[RR_MAX_PLAYERS];
    rr_mob mobs[RR_MAX_MOBS];
    rr_city city;
    uint8_t spawn_x, spawn_y;
    uint8_t dungeon_count;
    uint8_t dungeons[RR_MAX_DUNGEONS][2];
    /* Tiles changed since the last rr_world_clear_changes(), for replication. */
    uint16_t change_count;
    uint8_t changes_overflow;
    uint8_t changes[RR_MAX_CHANGES][2];
} rr_world;

uint32_t rr_rand(uint32_t *state);

void rr_world_init(rr_world *w, uint32_t seed);
void rr_world_tick(rr_world *w);

/* Returns the new player id, or -1 if the world is full. */
int rr_world_add_player(rr_world *w, const char *name);
void rr_world_remove_player(rr_world *w, int id);

/* Applies a player command. Returns 1 if it did something. */
int rr_world_apply(rr_world *w, int player_id, const rr_command *cmd);

/* Cost (>= 0) of using `tool` at x,y right now, or an RR_BUILD_* error. */
int rr_world_can_build(const rr_world *w, int tool, int x, int y);

uint8_t rr_world_tile(const rr_world *w, int x, int y);
void rr_world_set_tile(rr_world *w, int x, int y, uint8_t tile);
void rr_world_clear_changes(rr_world *w);
void rr_world_rescan(rr_world *w); /* rebuild derived data after loading tiles */

int rr_world_player_at(const rr_world *w, int x, int y);
int rr_world_mob_at(const rr_world *w, int x, int y);
int rr_world_is_night(const rr_world *w);
/* 0 = full day, 256 = full night, ramps through dusk and dawn. */
int rr_world_darkness(const rr_world *w);

int rr_tile_walkable(uint8_t tile);
int rr_tool_cost(int tool);
const char *rr_tool_name(int tool);
int rr_xp_to_level(int level);

/* Save/load the persistent parts of a world (map + city), for the server. */
#define RR_SAVE_SIZE (64 + RR_MAP_W * RR_MAP_H)
int rr_world_save(const rr_world *w, uint8_t *buf, int cap);
int rr_world_load(rr_world *w, const uint8_t *buf, int len);

#endif
