#include "rr/rr_world.h"

#include <string.h>

/* ---- randomness & noise ------------------------------------------------- */

uint32_t rr_rand(uint32_t *state)
{
    uint32_t x = *state ? *state : 0x9E3779B9u;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static uint32_t hash2(uint32_t seed, int x, int y)
{
    uint32_t h = seed * 0x9E3779B1u ^ (uint32_t)x * 0x85EBCA77u ^ (uint32_t)y * 0xC2B2AE3Du;
    h ^= h >> 15;
    h *= 0x2C1B3C6Du;
    h ^= h >> 12;
    h *= 0x297A2D39u;
    h ^= h >> 15;
    return h;
}

/* Smoothed value noise in 0..255. x, y >= 0. */
static int noise(uint32_t seed, int x, int y, int scale)
{
    int gx = x / scale, gy = y / scale;
    int fx = (x % scale) * 256 / scale, fy = (y % scale) * 256 / scale;
    int sx = fx * fx * (3 * 256 - 2 * fx) / (256 * 256);
    int sy = fy * fy * (3 * 256 - 2 * fy) / (256 * 256);
    int a = (int)(hash2(seed, gx, gy) & 255), b = (int)(hash2(seed, gx + 1, gy) & 255);
    int c = (int)(hash2(seed, gx, gy + 1) & 255), d = (int)(hash2(seed, gx + 1, gy + 1) & 255);
    int top = a + (b - a) * sx / 256;
    int bot = c + (d - c) * sx / 256;
    return top + (bot - top) * sy / 256;
}

static int iabs(int v) { return v < 0 ? -v : v; }
static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

static void dir_delta(int dir, int *dx, int *dy)
{
    *dx = dir == RR_DIR_LEFT ? -1 : dir == RR_DIR_RIGHT ? 1 : 0;
    *dy = dir == RR_DIR_UP ? -1 : dir == RR_DIR_DOWN ? 1 : 0;
}

/* ---- tiles -------------------------------------------------------------- */

uint8_t rr_world_tile(const rr_world *w, int x, int y)
{
    if (x < 0 || y < 0 || x >= RR_MAP_W || y >= RR_MAP_H) return RR_T_WATER;
    return w->tiles[y][x];
}

void rr_world_set_tile(rr_world *w, int x, int y, uint8_t tile)
{
    if (x < 0 || y < 0 || x >= RR_MAP_W || y >= RR_MAP_H) return;
    if (w->tiles[y][x] == tile) return;
    w->tiles[y][x] = tile;
    if (w->change_count < RR_MAX_CHANGES) {
        w->changes[w->change_count][0] = (uint8_t)x;
        w->changes[w->change_count][1] = (uint8_t)y;
        w->change_count++;
    } else {
        w->changes_overflow = 1;
    }
}

void rr_world_clear_changes(rr_world *w)
{
    w->change_count = 0;
    w->changes_overflow = 0;
}

int rr_tile_walkable(uint8_t t)
{
    switch (t) {
    case RR_T_GRASS:
    case RR_T_SAND:
    case RR_T_FOREST:
    case RR_T_ROAD:
    case RR_T_ZONE_R:
    case RR_T_ZONE_C:
    case RR_T_ZONE_I:
    case RR_T_RUBBLE:
        return 1;
    default:
        return 0;
    }
}

static int is_building(uint8_t t)
{
    return t == RR_T_HOUSE || t == RR_T_SHOP || t == RR_T_FACTORY || t == RR_T_TAVERN;
}

static int is_zone(uint8_t t) { return t == RR_T_ZONE_R || t == RR_T_ZONE_C || t == RR_T_ZONE_I; }

static int has_neighbor(const rr_world *w, int x, int y, uint8_t t)
{
    return rr_world_tile(w, x + 1, y) == t || rr_world_tile(w, x - 1, y) == t ||
           rr_world_tile(w, x, y + 1) == t || rr_world_tile(w, x, y - 1) == t;
}

int rr_world_player_at(const rr_world *w, int x, int y)
{
    for (int i = 0; i < RR_MAX_PLAYERS; i++)
        if (w->players[i].active && w->players[i].x == x && w->players[i].y == y) return i;
    return -1;
}

int rr_world_mob_at(const rr_world *w, int x, int y)
{
    for (int i = 0; i < RR_MAX_MOBS; i++)
        if (w->mobs[i].kind && w->mobs[i].x == x && w->mobs[i].y == y) return i;
    return -1;
}

static int occupied(const rr_world *w, int x, int y)
{
    return rr_world_player_at(w, x, y) >= 0 || rr_world_mob_at(w, x, y) >= 0;
}

static int free_walkable(const rr_world *w, int x, int y)
{
    return rr_tile_walkable(rr_world_tile(w, x, y)) && !occupied(w, x, y);
}

/* Nearest walkable, unoccupied tile to (x, y), searching outward in rings. */
static int find_free_near(const rr_world *w, int x, int y, uint8_t *ox, uint8_t *oy)
{
    for (int r = 0; r < 16; r++) {
        for (int dy = -r; dy <= r; dy++) {
            for (int dx = -r; dx <= r; dx++) {
                if (iabs(dx) != r && iabs(dy) != r) continue;
                if (free_walkable(w, x + dx, y + dy)) {
                    *ox = (uint8_t)(x + dx);
                    *oy = (uint8_t)(y + dy);
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* ---- time --------------------------------------------------------------- */

int rr_world_is_night(const rr_world *w) { return w->city.time >= RR_DAY_TICKS * 2 / 3; }

int rr_world_darkness(const rr_world *w)
{
    int t = w->city.time, dusk = RR_DAY_TICKS * 2 / 3, ramp = RR_HOUR_TICKS;
    if (t >= dusk - ramp && t < dusk) return (t - (dusk - ramp)) * 256 / ramp;
    if (t >= RR_DAY_TICKS - ramp) return (RR_DAY_TICKS - t) * 256 / ramp;
    return t >= dusk ? 256 : 0;
}

/* ---- world generation --------------------------------------------------- */

void rr_world_rescan(rr_world *w)
{
    w->dungeon_count = 0;
    for (int y = 0; y < RR_MAP_H; y++)
        for (int x = 0; x < RR_MAP_W; x++)
            if (w->tiles[y][x] == RR_T_DUNGEON && w->dungeon_count < RR_MAX_DUNGEONS) {
                w->dungeons[w->dungeon_count][0] = (uint8_t)x;
                w->dungeons[w->dungeon_count][1] = (uint8_t)y;
                w->dungeon_count++;
            }
}

void rr_world_init(rr_world *w, uint32_t seed)
{
    memset(w, 0, sizeof(*w));
    w->seed = seed;
    w->rng = seed ^ 0xA5A5F00Du;

    const int half = RR_MAP_W / 2;
    for (int y = 0; y < RR_MAP_H; y++) {
        for (int x = 0; x < RR_MAP_W; x++) {
            int h = (noise(seed, x, y, 32) * 5 + noise(seed + 11, x, y, 16) * 3 +
                     noise(seed + 23, x, y, 8) * 2) / 10;
            int dx = x - half, dy = y - half;
            int d = (dx * dx + dy * dy) * 256 / (half * half); /* 0 centre, 256 at edge */
            h = h + 60 - d * 140 / 256;
            int m = noise(seed + 37, x, y, 12);
            uint8_t t;
            if (h < 70) t = RR_T_WATER;
            else if (h < 84) t = RR_T_SAND;
            else if (h > 205) t = RR_T_MOUNTAIN;
            else if (m > 150) t = RR_T_FOREST;
            else t = RR_T_GRASS;
            w->tiles[y][x] = t;
        }
    }

    /* Starting village: a clearing with a crossroads, tavern and a few homes. */
    int cx = half, cy = RR_MAP_H / 2;
    for (int y = cy - 6; y <= cy + 6; y++)
        for (int x = cx - 6; x <= cx + 6; x++) w->tiles[y][x] = RR_T_GRASS;
    for (int i = -6; i <= 6; i++) {
        w->tiles[cy][cx + i] = RR_T_ROAD;
        w->tiles[cy + i][cx] = RR_T_ROAD;
    }
    w->tiles[cy - 1][cx + 1] = RR_T_TAVERN;
    w->tiles[cy - 1][cx - 1] = RR_T_HOUSE;
    w->tiles[cy - 1][cx - 2] = RR_T_HOUSE;
    w->tiles[cy + 1][cx - 1] = RR_T_HOUSE;
    w->tiles[cy - 1][cx - 3] = RR_T_ZONE_R;
    w->tiles[cy + 1][cx - 2] = RR_T_ZONE_R;
    w->tiles[cy + 1][cx + 1] = RR_T_ZONE_C;
    w->tiles[cy + 1][cx + 2] = RR_T_ZONE_C;
    w->tiles[cy + 1][cx + 4] = RR_T_ZONE_I;
    w->spawn_x = (uint8_t)cx;
    w->spawn_y = (uint8_t)cy;

    /* Dungeons: mountain faces that border open land, away from town. */
    uint32_t r = seed ^ 0x51ED270Bu;
    for (int tries = 0; tries < 4000 && w->dungeon_count < 6; tries++) {
        int x = 1 + (int)(rr_rand(&r) % (RR_MAP_W - 2));
        int y = 1 + (int)(rr_rand(&r) % (RR_MAP_H - 2));
        if (w->tiles[y][x] != RR_T_MOUNTAIN) continue;
        if (iabs(x - cx) + iabs(y - cy) < 20) continue;
        if (!has_neighbor(w, x, y, RR_T_GRASS) && !has_neighbor(w, x, y, RR_T_FOREST)) continue;
        w->tiles[y][x] = RR_T_DUNGEON;
        w->dungeons[w->dungeon_count][0] = (uint8_t)x;
        w->dungeons[w->dungeon_count][1] = (uint8_t)y;
        w->dungeon_count++;
    }

    w->city.funds = 2000;
    w->city.tax_rate = 7;
    w->city.day = 1;
}

/* ---- players ------------------------------------------------------------ */

int rr_xp_to_level(int level) { return level * 10; }

int rr_world_add_player(rr_world *w, const char *name)
{
    for (int i = 0; i < RR_MAX_PLAYERS; i++) {
        rr_player *p = &w->players[i];
        if (p->active) continue;
        memset(p, 0, sizeof(*p));
        p->active = 1;
        p->level = 1;
        p->max_hp = p->hp = 12;
        p->gold = 20;
        p->facing = RR_DIR_DOWN;
        /* Names are restricted to what the bitmap font can draw. */
        int n = 0;
        for (const char *s = name ? name : ""; *s && n < RR_NAME_LEN; s++) {
            char ch = *s;
            if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
            if ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')) p->name[n++] = ch;
        }
        if (n == 0) {
            p->name[0] = 'P';
            p->name[1] = (char)('1' + i);
        }
        if (!find_free_near(w, w->spawn_x, w->spawn_y, &p->x, &p->y)) {
            p->x = w->spawn_x;
            p->y = w->spawn_y;
        }
        return i;
    }
    return -1;
}

void rr_world_remove_player(rr_world *w, int id)
{
    if (id >= 0 && id < RR_MAX_PLAYERS) w->players[id].active = 0;
}

static void kill_player(rr_world *w, rr_player *p)
{
    p->gold /= 2;
    p->hp = p->max_hp;
    p->intent = RR_DIR_NONE;
    p->active = 0; /* so it doesn't block its own respawn search */
    if (!find_free_near(w, w->spawn_x, w->spawn_y, &p->x, &p->y)) {
        p->x = w->spawn_x;
        p->y = w->spawn_y;
    }
    p->active = 1;
}

static void damage_player(rr_world *w, rr_player *p, int dmg)
{
    p->hurt = 4;
    if (p->hp <= dmg) kill_player(w, p);
    else p->hp = (uint8_t)(p->hp - dmg);
}

static const struct {
    uint8_t hp, speed, dmg, gold, xp;
} MOB_STATS[] = {
    {0, 0, 0, 0, 0},     /* none */
    {3, 7, 1, 2, 1},     /* slime */
    {5, 3, 2, 5, 3},     /* wolf */
    {14, 5, 4, 25, 10},  /* troll */
};

static void player_attack(rr_world *w, rr_player *p)
{
    if (p->attack_cd) return;
    p->attack_cd = 3;
    int dx, dy;
    dir_delta(p->facing, &dx, &dy);
    int m = rr_world_mob_at(w, p->x + dx, p->y + dy);
    if (m < 0) return;
    rr_mob *mob = &w->mobs[m];
    int dmg = 2 + p->level;
    mob->hurt = 3;
    if (mob->hp > dmg) {
        mob->hp = (uint8_t)(mob->hp - dmg);
        return;
    }
    /* Loot is shared: the hero keeps it and the realm's treasury gets a cut. */
    p->gold += MOB_STATS[mob->kind].gold;
    w->city.funds += MOB_STATS[mob->kind].gold;
    p->xp = (uint16_t)(p->xp + MOB_STATS[mob->kind].xp);
    while (p->xp >= rr_xp_to_level(p->level) && p->level < 99) {
        p->xp = (uint16_t)(p->xp - rr_xp_to_level(p->level));
        p->level++;
        p->max_hp = (uint8_t)(p->max_hp + 2);
        p->hp = p->max_hp;
    }
    mob->kind = RR_MOB_NONE;
}

static void tick_player(rr_world *w, rr_player *p)
{
    if (p->hurt) p->hurt--;
    if (p->attack_cd) p->attack_cd--;
    int regen = has_neighbor(w, p->x, p->y, RR_T_TAVERN) ? 5 : 30;
    if (w->tick % (uint32_t)regen == 0 && p->hp < p->max_hp) p->hp++;

    if (p->move_cd) {
        p->move_cd--;
        return;
    }
    if (p->intent == RR_DIR_NONE) return;
    p->facing = p->intent;
    int dx, dy;
    dir_delta(p->intent, &dx, &dy);
    int nx = p->x + dx, ny = p->y + dy;
    if (!free_walkable(w, nx, ny)) return;
    p->x = (uint8_t)nx;
    p->y = (uint8_t)ny;
    p->move_cd = rr_world_tile(w, nx, ny) == RR_T_FOREST ? 2 : 1;
}

/* ---- monsters ----------------------------------------------------------- */

static void mob_step(rr_world *w, rr_mob *m, int dx, int dy)
{
    if (dx || dy) m->facing = dx < 0 ? RR_DIR_LEFT : dx > 0 ? RR_DIR_RIGHT : dy < 0 ? RR_DIR_UP : RR_DIR_DOWN;
    if (free_walkable(w, m->x + dx, m->y + dy)) {
        m->x = (uint8_t)(m->x + dx);
        m->y = (uint8_t)(m->y + dy);
    }
}

static void mob_toward(rr_world *w, rr_mob *m, int tx, int ty)
{
    int dx = tx - m->x, dy = ty - m->y;
    int sx = dx < 0 ? -1 : dx > 0, sy = dy < 0 ? -1 : dy > 0;
    if (iabs(dx) >= iabs(dy)) {
        if (sx && free_walkable(w, m->x + sx, m->y)) mob_step(w, m, sx, 0);
        else if (sy) mob_step(w, m, 0, sy);
    } else {
        if (sy && free_walkable(w, m->x, m->y + sy)) mob_step(w, m, 0, sy);
        else if (sx) mob_step(w, m, sx, 0);
    }
}

static void tick_mob(rr_world *w, rr_mob *m)
{
    if (m->hurt) m->hurt--;
    if (m->cd) {
        m->cd--;
        return;
    }
    m->cd = MOB_STATS[m->kind].speed;

    int best = -1, best_d = 8;
    for (int i = 0; i < RR_MAX_PLAYERS; i++) {
        const rr_player *p = &w->players[i];
        if (!p->active) continue;
        int d = iabs(p->x - m->x) + iabs(p->y - m->y);
        if (d < best_d) {
            best_d = d;
            best = i;
        }
    }
    int night = rr_world_is_night(w);

    if (best >= 0) {
        rr_player *p = &w->players[best];
        if (best_d == 1) {
            damage_player(w, p, MOB_STATS[m->kind].dmg);
            return;
        }
        mob_toward(w, m, p->x, p->y);
        return;
    }

    /* Raids: at night monsters wreck buildings they reach. */
    if (night) {
        static const int8_t D[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (int k = 0; k < 4; k++) {
            int bx = m->x + D[k][0], by = m->y + D[k][1];
            if (is_building(rr_world_tile(w, bx, by))) {
                if (rr_rand(&w->rng) % 6 == 0) rr_world_set_tile(w, bx, by, RR_T_RUBBLE);
                return;
            }
        }
        if (rr_rand(&w->rng) % 2) {
            mob_toward(w, m, w->spawn_x, w->spawn_y);
            return;
        }
    }
    uint32_t r = rr_rand(&w->rng) % 5;
    if (r < 4) {
        int dx, dy;
        dir_delta((int)r + 1, &dx, &dy);
        mob_step(w, m, dx, dy);
    }

    /* Daylight sends far-away monsters home. */
    if (!night && rr_rand(&w->rng) % 40 == 0) m->kind = RR_MOB_NONE;
}

static void spawn_mobs(rr_world *w)
{
    int night = rr_world_is_night(w);
    int count = 0, slot = -1;
    for (int i = 0; i < RR_MAX_MOBS; i++) {
        if (w->mobs[i].kind) count++;
        else if (slot < 0) slot = i;
    }
    if (slot < 0 || count >= (night ? 24 : 6)) return;
    if (rr_rand(&w->rng) % (night ? 6u : 25u)) return;

    uint8_t kind, x, y;
    if (w->dungeon_count && rr_rand(&w->rng) % 3 == 0) {
        int d = (int)(rr_rand(&w->rng) % w->dungeon_count);
        if (!find_free_near(w, w->dungeons[d][0], w->dungeons[d][1], &x, &y)) return;
        kind = night ? RR_MOB_TROLL : RR_MOB_SLIME;
    } else {
        x = (uint8_t)(rr_rand(&w->rng) % RR_MAP_W);
        y = (uint8_t)(rr_rand(&w->rng) % RR_MAP_H);
        uint8_t t = rr_world_tile(w, x, y);
        if (t != RR_T_FOREST && t != RR_T_GRASS) return;
        if (!free_walkable(w, x, y)) return;
        kind = (t == RR_T_FOREST && night) ? RR_MOB_WOLF : RR_MOB_SLIME;
    }
    if (iabs(x - w->spawn_x) + iabs(y - w->spawn_y) < 14) return;

    rr_mob *m = &w->mobs[slot];
    memset(m, 0, sizeof(*m));
    m->kind = kind;
    m->x = x;
    m->y = y;
    m->hp = MOB_STATS[kind].hp;
    m->cd = MOB_STATS[kind].speed;
    m->facing = RR_DIR_DOWN;
}

/* ---- city simulation ---------------------------------------------------- */

static void simulate_city(rr_world *w)
{
    int houses = 0, shops = 0, factories = 0, taverns = 0;
    for (int y = 0; y < RR_MAP_H; y++)
        for (int x = 0; x < RR_MAP_W; x++) switch (w->tiles[y][x]) {
            case RR_T_HOUSE: houses++; break;
            case RR_T_SHOP: shops++; break;
            case RR_T_FACTORY: factories++; break;
            case RR_T_TAVERN: taverns++; break;
            default: break;
            }

    int pop = houses * 8 + taverns * 2;
    int jobs = shops * 4 + factories * 8 + taverns * 4;
    int dr = jobs + 24 - pop;
    int dc = pop / 3 - shops * 4 + 4;
    int di = pop / 2 - factories * 8 + 8;

    for (int y = 0; y < RR_MAP_H; y++) {
        for (int x = 0; x < RR_MAP_W; x++) {
            uint8_t t = w->tiles[y][x];
            int roaded = has_neighbor(w, x, y, RR_T_ROAD);
            if (is_zone(t)) {
                int *demand = t == RR_T_ZONE_R ? &dr : t == RR_T_ZONE_C ? &dc : &di;
                if (!roaded || *demand <= 0 || occupied(w, x, y)) continue;
                if ((int)(rr_rand(&w->rng) % 100) >= clampi(*demand / 8 + 1, 1, 12)) continue;
                if (t == RR_T_ZONE_R) {
                    rr_world_set_tile(w, x, y, RR_T_HOUSE);
                    dr -= 8, dc += 3, di += 4;
                } else if (t == RR_T_ZONE_C) {
                    rr_world_set_tile(w, x, y, RR_T_SHOP);
                    dc -= 4, dr += 4;
                } else {
                    rr_world_set_tile(w, x, y, RR_T_FACTORY);
                    di -= 8, dr += 8;
                }
            } else if (t == RR_T_HOUSE || t == RR_T_SHOP || t == RR_T_FACTORY) {
                /* Cut off from the road network, businesses slowly fold. */
                if (!roaded && rr_rand(&w->rng) % 100 == 0)
                    rr_world_set_tile(w, x, y, t == RR_T_HOUSE ? RR_T_ZONE_R : t == RR_T_SHOP ? RR_T_ZONE_C : RR_T_ZONE_I);
            }
        }
    }

    w->city.population = (uint16_t)pop;
    w->city.jobs = (uint16_t)jobs;
    w->city.demand_r = (int16_t)clampi(dr, -99, 99);
    w->city.demand_c = (int16_t)clampi(dc, -99, 99);
    w->city.demand_i = (int16_t)clampi(di, -99, 99);
}

static void collect_taxes(rr_world *w)
{
    int roads = 0;
    for (int y = 0; y < RR_MAP_H; y++)
        for (int x = 0; x < RR_MAP_W; x++) roads += w->tiles[y][x] == RR_T_ROAD;
    w->city.funds += (w->city.population + w->city.jobs) * w->city.tax_rate / 20 - roads / 25;
}

void rr_world_tick(rr_world *w)
{
    w->tick++;
    w->city.time = (uint16_t)(w->tick % RR_DAY_TICKS);
    w->city.day = (uint16_t)(w->tick / RR_DAY_TICKS + 1);

    for (int i = 0; i < RR_MAX_PLAYERS; i++)
        if (w->players[i].active) tick_player(w, &w->players[i]);
    for (int i = 0; i < RR_MAX_MOBS; i++)
        if (w->mobs[i].kind) tick_mob(w, &w->mobs[i]);
    spawn_mobs(w);

    if (w->tick % RR_TICK_HZ == 0) simulate_city(w);
    if (w->tick % RR_HOUR_TICKS == 0) collect_taxes(w);
}

/* ---- building ----------------------------------------------------------- */

static const struct {
    const char *name;
    int cost;
} TOOLS[RR_TOOL_COUNT] = {
    {"ROAD", 10}, {"RES ZONE", 20}, {"COM ZONE", 20}, {"IND ZONE", 20}, {"TAVERN", 300}, {"BULLDOZE", 5},
};

int rr_tool_cost(int tool) { return tool >= 0 && tool < RR_TOOL_COUNT ? TOOLS[tool].cost : 0; }
const char *rr_tool_name(int tool) { return tool >= 0 && tool < RR_TOOL_COUNT ? TOOLS[tool].name : "?"; }

static int clearable(uint8_t t)
{
    return t == RR_T_GRASS || t == RR_T_SAND || t == RR_T_FOREST || t == RR_T_RUBBLE;
}

int rr_world_can_build(const rr_world *w, int tool, int x, int y)
{
    if (x < 0 || y < 0 || x >= RR_MAP_W || y >= RR_MAP_H) return RR_BUILD_INVALID;
    uint8_t t = w->tiles[y][x];
    switch (tool) {
    case RR_TOOL_ROAD:
    case RR_TOOL_ZONE_R:
    case RR_TOOL_ZONE_C:
    case RR_TOOL_ZONE_I:
        if (!clearable(t)) return RR_BUILD_INVALID;
        break;
    case RR_TOOL_TAVERN:
        if (!clearable(t)) return RR_BUILD_INVALID;
        if (occupied(w, x, y)) return RR_BUILD_OCCUPIED;
        break;
    case RR_TOOL_BULLDOZE:
        if (t == RR_T_GRASS || t == RR_T_WATER || t == RR_T_MOUNTAIN || t == RR_T_DUNGEON || t == RR_T_SAND)
            return RR_BUILD_INVALID;
        break;
    default:
        return RR_BUILD_INVALID;
    }
    if (w->city.funds < TOOLS[tool].cost) return RR_BUILD_NO_FUNDS;
    return TOOLS[tool].cost;
}

static int build(rr_world *w, int tool, int x, int y)
{
    int cost = rr_world_can_build(w, tool, x, y);
    if (cost < 0) return 0;
    static const uint8_t RESULT[RR_TOOL_COUNT] = {
        RR_T_ROAD, RR_T_ZONE_R, RR_T_ZONE_C, RR_T_ZONE_I, RR_T_TAVERN, RR_T_GRASS,
    };
    rr_world_set_tile(w, x, y, RESULT[tool]);
    w->city.funds -= cost;
    return 1;
}

int rr_world_apply(rr_world *w, int pid, const rr_command *c)
{
    if (pid < 0 || pid >= RR_MAX_PLAYERS || !w->players[pid].active) return 0;
    rr_player *p = &w->players[pid];
    switch (c->type) {
    case RR_CMD_INTENT:
        if (c->arg > RR_DIR_RIGHT) return 0;
        p->intent = c->arg;
        if (c->arg != RR_DIR_NONE) p->facing = c->arg;
        return 1;
    case RR_CMD_ATTACK:
        player_attack(w, p);
        return 1;
    case RR_CMD_BUILD:
        return build(w, c->arg, c->x, c->y);
    default:
        return 0;
    }
}

/* ---- persistence -------------------------------------------------------- */

static void put32(uint8_t *b, uint32_t v)
{
    b[0] = (uint8_t)v, b[1] = (uint8_t)(v >> 8), b[2] = (uint8_t)(v >> 16), b[3] = (uint8_t)(v >> 24);
}
static uint32_t get32(const uint8_t *b)
{
    return (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
}

int rr_world_save(const rr_world *w, uint8_t *b, int cap)
{
    if (cap < RR_SAVE_SIZE) return -1;
    memset(b, 0, 64);
    memcpy(b, "RRW1", 4);
    put32(b + 4, w->seed);
    put32(b + 8, w->tick);
    put32(b + 12, (uint32_t)w->city.funds);
    b[16] = w->city.tax_rate;
    b[17] = w->spawn_x;
    b[18] = w->spawn_y;
    memcpy(b + 64, w->tiles, sizeof(w->tiles));
    return RR_SAVE_SIZE;
}

int rr_world_load(rr_world *w, const uint8_t *b, int len)
{
    if (len < RR_SAVE_SIZE || memcmp(b, "RRW1", 4) != 0) return 0;
    memset(w, 0, sizeof(*w));
    w->seed = get32(b + 4);
    w->tick = get32(b + 8);
    w->rng = w->seed ^ w->tick ^ 0xA5A5F00Du;
    w->city.funds = (int32_t)get32(b + 12);
    w->city.tax_rate = b[16];
    w->spawn_x = b[17];
    w->spawn_y = b[18];
    w->city.time = (uint16_t)(w->tick % RR_DAY_TICKS);
    w->city.day = (uint16_t)(w->tick / RR_DAY_TICKS + 1);
    memcpy(w->tiles, b + 64, sizeof(w->tiles));
    rr_world_rescan(w);
    simulate_city(w); /* recompute population/jobs/demand */
    rr_world_clear_changes(w);
    return 1;
}
