#include "rr/rr_render.h"

#include <stdio.h>
#include <string.h>

#define W RR_SCREEN_W
#define H RR_SCREEN_H
#define T RR_TILE
#define CLEAR 255 /* transparent sprite pixel */
#define BODY 254  /* sprite pixel recoloured per player */

enum {
    C_BLACK, C_NAVY, C_WHITE, C_LGRAY, C_GRAY, C_DGRAY, C_GRASS, C_FOREST, C_LGREEN, C_DEEP,
    C_WATER, C_FOAM, C_SAND, C_DSAND, C_BROWN, C_DBROWN, C_RED, C_DRED, C_ORANGE, C_YELLOW,
    C_BLUE, C_PURPLE, C_SKIN, C_CYAN, C_ZONE_R, C_ZONE_C, C_ZONE_I, C_WINDOW, C_ROCK, C_DROCK,
    C_SLIME, C_UNUSED
};

static const uint32_t PALETTE[RR_PALETTE_SIZE] = {
    0x000000, 0x1a1c2c, 0xf4f4f4, 0x94b0c2, 0x566c86, 0x333c57, 0x38b764, 0x1e6b3a,
    0xa7f070, 0x1d2b6b, 0x3b5dc9, 0x41a6f6, 0xe8d48a, 0xc2a35a, 0x8f563b, 0x4e2f23,
    0xb13e53, 0x6b1f2c, 0xef7d57, 0xffcd75, 0x2f4fb0, 0x7a3b9e, 0xf2b8a0, 0x73eff7,
    0x8ee08a, 0x7fb0ff, 0xf5e15a, 0x29366f, 0xb0a8a0, 0x6e6660, 0x9be35b, 0xff00ff,
};

/* Player tunic colours, indexed by player id. */
static const uint8_t PLAYER_COLORS[RR_MAX_PLAYERS] = {
    C_RED, C_BLUE, C_PURPLE, C_ORANGE, C_CYAN, C_LGREEN, C_YELLOW, C_WHITE,
};

/* ---- bitmap font (3x5) -------------------------------------------------- */

static const struct {
    char ch;
    const char *bits;
} GLYPHS[] = {
    {'0', "111101101101111"}, {'1', "010110010010111"}, {'2', "111001111100111"},
    {'3', "111001111001111"}, {'4', "101101111001001"}, {'5', "111100111001111"},
    {'6', "111100111101111"}, {'7', "111001001010010"}, {'8', "111101111101111"},
    {'9', "111101111001111"}, {'A', "010101111101101"}, {'B', "110101110101110"},
    {'C', "011100100100011"}, {'D', "110101101101110"}, {'E', "111100110100111"},
    {'F', "111100110100100"}, {'G', "011100101101011"}, {'H', "101101111101101"},
    {'I', "111010010010111"}, {'J', "001001001101010"}, {'K', "101101110101101"},
    {'L', "100100100100111"}, {'M', "101111111101101"}, {'N', "110101101101101"},
    {'O', "010101101101010"}, {'P', "110101110100100"}, {'Q', "010101101110011"},
    {'R', "110101110101101"}, {'S', "011100010001110"}, {'T', "111010010010010"},
    {'U', "101101101101111"}, {'V', "101101101101010"}, {'W', "101101111111101"},
    {'X', "101101010101101"}, {'Y', "101101010010010"}, {'Z', "111001010100111"},
    {':', "000010000010000"}, {'.', "000000000000010"}, {'-', "000000111000000"},
    {'+', "000010111010000"}, {'/', "001001010100100"}, {'$', "011110010011110"},
    {'!', "010010010000010"}, {'>', "100010001010100"}, {'<', "001010100010001"},
    {'%', "101001010100101"}, {'(', "010100100100010"}, {')', "010001001001010"},
    {'?', "111001010000010"}, {'=', "000111000111000"}, {',', "000000000010100"},
    {'\'', "010010000000000"}, {'[', "110100100100110"}, {']', "011001001001011"},
};

static uint16_t g_font[128];

static void font_init(void)
{
    memset(g_font, 0, sizeof(g_font));
    for (size_t i = 0; i < sizeof(GLYPHS) / sizeof(GLYPHS[0]); i++) {
        uint16_t bits = 0;
        for (int k = 0; k < 15; k++) bits = (uint16_t)(bits << 1 | (GLYPHS[i].bits[k] == '1'));
        g_font[(int)GLYPHS[i].ch] = bits;
    }
}

int rr_text_width(const char *s, int scale) { return (int)strlen(s) * 4 * scale; }

static void put(uint8_t *fb, int x, int y, uint8_t c)
{
    if ((unsigned)x < W && (unsigned)y < H) fb[y * W + x] = c;
}

void rr_draw_rect(uint8_t *fb, int x, int y, int w, int h, uint8_t color)
{
    int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
    int x1 = x + w > W ? W : x + w, y1 = y + h > H ? H : y + h;
    for (int yy = y0; yy < y1; yy++) memset(fb + yy * W + x0, color, (size_t)(x1 > x0 ? x1 - x0 : 0));
}

static void glyph_into(uint8_t *dst, int stride, int dw, int dh, int x, int y, char ch, uint8_t color, int scale)
{
    if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
    uint16_t bits = (ch > 0 && (unsigned char)ch < 128) ? g_font[(int)ch] : 0;
    for (int k = 0; k < 15; k++) {
        if (!(bits & (1u << (14 - k)))) continue;
        int gx = x + (k % 3) * scale, gy = y + (k / 3) * scale;
        for (int sy = 0; sy < scale; sy++)
            for (int sx = 0; sx < scale; sx++) {
                int px = gx + sx, py = gy + sy;
                if (px >= 0 && py >= 0 && px < dw && py < dh) dst[py * stride + px] = color;
            }
    }
}

int rr_draw_text(uint8_t *fb, int x, int y, const char *s, uint8_t color, int scale)
{
    int x0 = x;
    for (; *s; s++, x += 4 * scale) glyph_into(fb, W, W, H, x, y, *s, color, scale);
    return x - x0;
}

static void text_shadow(uint8_t *fb, int x, int y, const char *s, uint8_t color, int scale)
{
    rr_draw_text(fb, x + scale, y + scale, s, C_BLACK, scale);
    rr_draw_text(fb, x, y, s, color, scale);
}

static void text_center(uint8_t *fb, int y, const char *s, uint8_t color, int scale)
{
    text_shadow(fb, (W - rr_text_width(s, scale)) / 2, y, s, color, scale);
}

/* ---- art ---------------------------------------------------------------- */

static uint8_t g_tiles[RR_T_COUNT][T * T];

enum { SPR_PLAYER, SPR_SLIME, SPR_WOLF, SPR_TROLL, SPR_COUNT };
static uint8_t g_sprites[SPR_COUNT][T * T];

static uint8_t legend(char ch)
{
    switch (ch) {
    case '0': return C_BLACK;
    case 'n': return C_NAVY;
    case 'W': return C_WHITE;
    case 'g': return C_LGRAY;
    case 'G': return C_GRAY;
    case 'D': return C_DGRAY;
    case 'v': return C_GRASS;
    case 'f': return C_FOREST;
    case 'l': return C_LGREEN;
    case 'w': return C_WATER;
    case 'c': return C_FOAM;
    case 's': return C_SAND;
    case 'b': return C_BROWN;
    case 'B': return C_DBROWN;
    case 'r': return C_RED;
    case 'R': return C_DRED;
    case 'o': return C_ORANGE;
    case 'y': return C_YELLOW;
    case 'u': return C_BLUE;
    case 'p': return C_PURPLE;
    case 'k': return C_SKIN;
    case 'C': return C_CYAN;
    case 'i': return C_WINDOW;
    case 'm': return C_ROCK;
    case 'M': return C_DROCK;
    case 'e': return C_SLIME;
    case 'x': return BODY;
    default: return CLEAR;
    }
}

/* Paints 16 rows of 16 chars over dst; '.' leaves dst untouched. */
static int paint(uint8_t *dst, const char *const rows[T])
{
    for (int y = 0; y < T; y++) {
        if (strlen(rows[y]) != T) return 0;
        for (int x = 0; x < T; x++) {
            uint8_t c = legend(rows[y][x]);
            if (c != CLEAR) dst[y * T + x] = c;
        }
    }
    return 1;
}

static const char *const ART_PLAYER[T] = {
    "......gggg......", ".....gWgggg.....", ".....gggggg.....", ".....DkkkkD.....",
    ".....k0kk0k.....", ".....kkkkkk.....", "......kkkk......", "....xxxxxxxx....",
    "...xxxyxxyxxx...", "...kxxxxxxxxk...", "...k.xxxxxx.k...", ".....BBBBBB.....",
    ".....GG..GG.....", ".....GG..GG.....", ".....BB..BB.....", "....BBB..BBB....",
};
static const char *const ART_SLIME[T] = {
    "................", "................", "................", "................",
    "................", "................", "......eeee......", "....eeeeeeee....",
    "...eeWeeeeWee...", "...ee0eeee0ee...", "..eeeeeeeeeeee..", "..eeeeeeeeeeee..",
    "..eeeeeeeeeeee..", "...eeeeeeeeee...", "....ffffffff....", "................",
};
static const char *const ART_WOLF[T] = {
    "................", "................", "................", "..G.G...........",
    "..GGG...........", ".GGrGG..........", "0GGGGGGGGGGG..G.", "..GGGGGGGGGGGGG.",
    "...GGgggggGGGG..", "...GGgggggGGG...", "...GG.....GG....", "...GG.....GG....",
    "...DD.....DD....", "................", "................", "................",
};
static const char *const ART_TROLL[T] = {
    "................", ".....ffffff.....", "....ffffffff....", "....fyffffyf....",
    "....ffffffff....", "....fWfWWfWf....", "..ffffffffffff..", ".fffffffffffffBB",
    ".ff.ffffffff.BBB", ".ff.ffffffff.BB.", ".kk.BBBBBBBB.B..", "....BBBBBBBB....",
    "....ff....ff....", "....ff....ff....", "...fff....fff...", "................",
};
static const char *const ART_FOREST[T] = {
    "................", ".......ff.......", "......ffff......", ".....fflfff.....",
    ".....flffff.....", "....fflfffff....", "....ffffffff....", "...fflfffffff...",
    "...ffffffffff...", "..ffffffffffff..", "..ffffffffffff..", "...ffffffffff...",
    ".......BB.......", ".......BB.......", "......BBBB......", "................",
};
static const char *const ART_MOUNTAIN[T] = {
    "................", ".......mM.......", "......WWMM......", ".....WWWMMM.....",
    ".....mmmMMM.....", "....mmmmMMMM....", "....mmmmMMMM....", "...mmmmmMMMMM...",
    "...mmmmmMMMMM...", "..mmmmmmMMMMMM..", "..mmmmmmMMMMMM..", ".mmmmmmmMMMMMMM.",
    ".mmmmmmmMMMMMMM.", "mmmmmmmmMMMMMMMM", "mmmmmmmmMMMMMMMM", "mmmmmmmmMMMMMMMM",
};
static const char *const ART_DUNGEON[T] = {
    "................", "................", "................", "................",
    "................", "................", "......0000......", ".....000000.....",
    "....00000000....", "....00000000....", "...M00000000M...", "...M00000000M...",
    "...M00r00r00M...", "...M00000000M...", "..MM00000000MM..", "..MM00000000MM..",
};
static const char *const ART_HOUSE[T] = {
    "................", ".......rr.......", "......rrrr......", ".....rrrrrr.....",
    "....rrrrrrrr....", "...rrrrrrrrrr...", "..RRRRRRRRRRRR..", "...gggggggggg...",
    "...gciggggcig...", "...giiggggiig...", "...gggggggggg...", "...ggggBBgggg...",
    "...ggggBBgggg...", "...ggggBygggg...", "...ggggBBgggg...", "..DDDDDDDDDDDD..",
};
static const char *const ART_SHOP[T] = {
    "................", "................", "..uuuuuuuuuuuu..", "..uuuuuuuuuuuu..",
    "..WrWrWrWrWrWr..", "..rWrWrWrWrWrW..", "...WWWWWWWWWW...", "...WCCCWWCCCW...",
    "...WCCCWWCCCW...", "...WCCCWWCCCW...", "...WWWWWWWWWW...", "...WWWWBBWWWW...",
    "...WWWWBBWWWW...", "...WWWWBBWWWW...", "...WWWWBBWWWW...", "..DDDDDDDDDDDD..",
};
static const char *const ART_FACTORY[T] = {
    "..........gg....", ".........ggg....", "..........DD....", "..........DD....",
    "..........DD....", ".M.M.M....DD....", "MMMMMMM...DD....", "GGGGGGGGGGGGGG..",
    "GiiGiiGiiGiiGG..", "GiiGiiGiiGiiGG..", "GGGGGGGGGGGGGG..", "GGGGGDDDDGGGGG..",
    "GGGGGDDDDGGGGG..", "GGGGGDDDDGGGGG..", "GGGGGDDDDGGGGG..", "DDDDDDDDDDDDDDD.",
};
static const char *const ART_TAVERN[T] = {
    "................", "....RRRRRRRR....", "...RRRRRRRRRR...", "..RRRRRRRRRRRR..",
    ".RRRRRRRRRRRRRR.", "..bbbbbbbbbbbb..", "..bBbbBbbBbbBb..", "..byybbbbbbyyb..",
    "..byybbbbbbyyb..", "..bBbbBbbBbbBb..", "..bbbbbBBbbbbb..", "..bbbbbBBbbbbb..",
    "..bbbbbBBbbbbb..", "..bbbbbByBbbbb..", "..bbbbbBBbbbbb..", ".DDDDDDDDDDDDDD.",
};
static const char *const ART_RUBBLE[T] = {
    "................", "................", "................", "................",
    "................", "................", "................", "................",
    "....G...........", "...GGg....b.....", "..GGGgG..bBb....", ".GGDGGGG.BBb.G..",
    ".DDDDGGGGBBBGGG.", "..DDDDDDDDDDDD..", "................", "................",
};

static uint32_t art_hash(uint32_t a, uint32_t b)
{
    uint32_t h = a * 0x9E3779B1u ^ b * 0x85EBCA77u;
    h ^= h >> 13;
    h *= 0xC2B2AE35u;
    return h ^ (h >> 16);
}

static void fill(uint8_t *t, uint8_t c) { memset(t, c, T * T); }

static void speckle(uint8_t *t, uint8_t c, uint32_t salt, int per256)
{
    for (int i = 0; i < T * T; i++)
        if ((int)(art_hash((uint32_t)i, salt) & 255) < per256) t[i] = c;
}

static void zone_tile(uint8_t *t, uint8_t color, char letter)
{
    fill(t, C_GRASS);
    speckle(t, C_LGREEN, 7, 14);
    for (int i = 0; i < T; i++) {
        if (i % 4 < 2) {
            t[i] = t[(T - 1) * T + i] = color;      /* dashed top/bottom border */
            t[i * T] = t[i * T + T - 1] = color;    /* dashed left/right border */
        }
    }
    char s[2] = {letter, 0};
    glyph_into(t, T, T, T, 5, 3, s[0], color, 2);
}

static int build_art(void)
{
    int ok = 1;
    for (int i = 0; i < RR_T_COUNT; i++) {
        fill(g_tiles[i], C_GRASS);
        speckle(g_tiles[i], C_LGREEN, 1, 18);
        speckle(g_tiles[i], C_FOREST, 2, 10);
    }
    uint8_t *t;

    t = g_tiles[RR_T_WATER];
    fill(t, C_WATER);
    for (int x = 0; x < T; x++) {
        if (x % 8 >= 1 && x % 8 <= 3) t[3 * T + x] = C_FOAM;
        if ((x + 4) % 8 >= 1 && (x + 4) % 8 <= 3) t[11 * T + x] = C_FOAM;
    }
    speckle(t, C_DEEP, 3, 12);

    t = g_tiles[RR_T_SAND];
    fill(t, C_SAND);
    speckle(t, C_DSAND, 4, 30);

    t = g_tiles[RR_T_ROAD];
    fill(t, C_GRAY);
    speckle(t, C_LGRAY, 5, 10);
    speckle(t, C_DGRAY, 6, 16);

    zone_tile(g_tiles[RR_T_ZONE_R], C_ZONE_R, 'R');
    zone_tile(g_tiles[RR_T_ZONE_C], C_ZONE_C, 'C');
    zone_tile(g_tiles[RR_T_ZONE_I], C_ZONE_I, 'I');

    ok &= paint(g_tiles[RR_T_FOREST], ART_FOREST);
    ok &= paint(g_tiles[RR_T_MOUNTAIN], ART_MOUNTAIN);
    ok &= paint(g_tiles[RR_T_DUNGEON], ART_MOUNTAIN);
    ok &= paint(g_tiles[RR_T_DUNGEON], ART_DUNGEON);
    ok &= paint(g_tiles[RR_T_HOUSE], ART_HOUSE);
    ok &= paint(g_tiles[RR_T_SHOP], ART_SHOP);
    ok &= paint(g_tiles[RR_T_FACTORY], ART_FACTORY);
    ok &= paint(g_tiles[RR_T_TAVERN], ART_TAVERN);
    ok &= paint(g_tiles[RR_T_RUBBLE], ART_RUBBLE);

    static const char *const *const SPRITES[SPR_COUNT] = {ART_PLAYER, ART_SLIME, ART_WOLF, ART_TROLL};
    for (int i = 0; i < SPR_COUNT; i++) {
        memset(g_sprites[i], CLEAR, T * T);
        ok &= paint(g_sprites[i], SPRITES[i]);
    }
    return ok;
}

int rr_render_init(void)
{
    font_init();
    return build_art();
}

/* ---- palette ------------------------------------------------------------ */

void rr_render_palette(const rr_client *c, uint32_t out[RR_PALETTE_SIZE])
{
    int dark = c->screen == RR_SCREEN_PLAY ? rr_world_darkness(&c->world) : 0;
    for (int i = 0; i < RR_PALETTE_SIZE; i++) {
        uint32_t p = PALETTE[i];
        int r = (int)(p >> 16 & 255), g = (int)(p >> 8 & 255), b = (int)(p & 255);
        if (dark && i != C_YELLOW) { /* lamp-lit windows keep glowing at night */
            int nr = r * 40 / 100, ng = g * 48 / 100, nb = b * 70 / 100 + 24;
            r += (nr - r) * dark / 256;
            g += (ng - g) * dark / 256;
            b += (nb - b) * dark / 256;
            if (b > 255) b = 255;
        }
        out[i] = (uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b;
    }
}

/* ---- world view --------------------------------------------------------- */

#define VIEW_Y RR_HUD_H
#define VIEW_H (H - RR_HUD_H)

/* Blits a 16x16 image with clipping to the world viewport. */
static void blit(uint8_t *fb, const uint8_t *src, int dx, int dy, int transparent, uint8_t body, int flash)
{
    for (int y = 0; y < T; y++) {
        int py = dy + y;
        if (py < VIEW_Y || py >= H) continue;
        for (int x = 0; x < T; x++) {
            int px = dx + x;
            if (px < 0 || px >= W) continue;
            uint8_t c = src[y * T + x];
            if (transparent && c == CLEAR) continue;
            if (c == BODY) c = body;
            fb[py * W + px] = flash ? C_WHITE : c;
        }
    }
}

static void hline(uint8_t *fb, int x, int y, int w, uint8_t c)
{
    if (y < VIEW_Y || y >= H) return;
    for (int i = 0; i < w; i++) put(fb, x + i, y, c);
}

static void vline(uint8_t *fb, int x, int y, int h, uint8_t c)
{
    for (int i = 0; i < h; i++)
        if (y + i >= VIEW_Y) put(fb, x, y + i, c);
}

static void draw_sword(uint8_t *fb, int sx, int sy, int facing)
{
    switch (facing) {
    case RR_DIR_LEFT: vline(fb, sx + 1, sy + 4, 7, C_WHITE); hline(fb, sx, sy + 10, 3, C_DBROWN); break;
    case RR_DIR_RIGHT: vline(fb, sx + 14, sy + 4, 7, C_WHITE); hline(fb, sx + 13, sy + 10, 3, C_DBROWN); break;
    case RR_DIR_UP: vline(fb, sx + 12, sy, 7, C_WHITE); hline(fb, sx + 11, sy + 6, 3, C_DBROWN); break;
    default: vline(fb, sx + 3, sy + 9, 7, C_WHITE); hline(fb, sx + 2, sy + 9, 3, C_DBROWN); break;
    }
}

static void draw_world(uint8_t *fb, const rr_client *c)
{
    const rr_world *w = &c->world;
    int fx, fy;
    rr_client_focus(c, &fx, &fy);
    int cam_x = fx * T + T / 2 - W / 2, cam_y = fy * T + T / 2 - VIEW_H / 2;
    if (cam_x < 0) cam_x = 0;
    if (cam_y < 0) cam_y = 0;
    if (cam_x > RR_MAP_W * T - W) cam_x = RR_MAP_W * T - W;
    if (cam_y > RR_MAP_H * T - VIEW_H) cam_y = RR_MAP_H * T - VIEW_H;

    int tx0 = cam_x / T, ty0 = cam_y / T;
    for (int ty = ty0; ty <= ty0 + VIEW_H / T + 1 && ty < RR_MAP_H; ty++) {
        for (int tx = tx0; tx <= tx0 + W / T + 1 && tx < RR_MAP_W; tx++) {
            uint8_t tile = w->tiles[ty][tx];
            if (tile >= RR_T_COUNT) tile = RR_T_GRASS;
            int sx = tx * T - cam_x, sy = ty * T - cam_y + VIEW_Y;
            blit(fb, g_tiles[tile], sx, sy, 0, 0, 0);
            if (tile == RR_T_ROAD) { /* curbs on sides that don't continue */
                if (rr_world_tile(w, tx, ty - 1) != RR_T_ROAD) hline(fb, sx, sy, T, C_DGRAY);
                if (rr_world_tile(w, tx, ty + 1) != RR_T_ROAD) hline(fb, sx, sy + T - 1, T, C_DGRAY);
                if (rr_world_tile(w, tx - 1, ty) != RR_T_ROAD) vline(fb, sx, sy, T, C_DGRAY);
                if (rr_world_tile(w, tx + 1, ty) != RR_T_ROAD) vline(fb, sx + T - 1, sy, T, C_DGRAY);
            }
        }
    }

    for (int i = 0; i < RR_MAX_MOBS; i++) {
        const rr_mob *m = &w->mobs[i];
        if (!m->kind) continue;
        int spr = m->kind == RR_MOB_SLIME ? SPR_SLIME : m->kind == RR_MOB_WOLF ? SPR_WOLF : SPR_TROLL;
        int bob = (c->frame / 16 + i) % 2;
        blit(fb, g_sprites[spr], m->x * T - cam_x, m->y * T - cam_y + VIEW_Y - bob, 1, 0, m->hurt & 1);
    }

    for (int i = 0; i < RR_MAX_PLAYERS; i++) {
        const rr_player *p = &w->players[i];
        if (!p->active) continue;
        int sx = p->x * T - cam_x, sy = p->y * T - cam_y + VIEW_Y;
        blit(fb, g_sprites[SPR_PLAYER], sx, sy, 1, PLAYER_COLORS[i], p->hurt & 1);
        draw_sword(fb, sx, sy, p->facing);
        if (i != c->my_id && sy - 7 >= VIEW_Y) {
            int tw = rr_text_width(p->name, 1);
            rr_draw_text(fb, sx + T / 2 - tw / 2, sy - 7, p->name, C_WHITE, 1);
        }
    }

    if (c->mode == RR_MODE_BUILD) {
        int sx = c->cursor_x * T - cam_x, sy = c->cursor_y * T - cam_y + VIEW_Y;
        int cost = rr_world_can_build(w, c->tool, c->cursor_x, c->cursor_y);
        uint8_t col = cost >= 0 ? ((c->frame / 8) % 2 ? C_WHITE : C_YELLOW) : C_RED;
        hline(fb, sx, sy, T, col);
        hline(fb, sx, sy + T - 1, T, col);
        vline(fb, sx, sy, T, col);
        vline(fb, sx + T - 1, sy, T, col);
    }
}

static void draw_hud(uint8_t *fb, const rr_client *c)
{
    const rr_world *w = &c->world;
    const rr_city *city = &w->city;
    char line[72];
    rr_draw_rect(fb, 0, 0, W, RR_HUD_H, C_NAVY);
    rr_draw_rect(fb, 0, RR_HUD_H - 1, W, 1, C_DGRAY);

    int mins = (int)city->time * 24 * 60 / RR_DAY_TICKS + 6 * 60;
    snprintf(line, sizeof(line), "$%ld POP %u JOBS %u", (long)city->funds, (unsigned)city->population,
             (unsigned)city->jobs);
    rr_draw_text(fb, 3, 2, line, C_YELLOW, 1);
    snprintf(line, sizeof(line), "DAY %u %02d:%02d", (unsigned)city->day, mins / 60 % 24, mins % 60);
    rr_draw_text(fb, W - 3 - rr_text_width(line, 1), 2, line, rr_world_is_night(w) ? C_ZONE_C : C_WHITE, 1);

    if (c->mode == RR_MODE_BUILD) {
        snprintf(line, sizeof(line), "%s $%d  R%+d C%+d I%+d", rr_tool_name(c->tool), rr_tool_cost(c->tool),
                 city->demand_r, city->demand_c, city->demand_i);
        rr_draw_text(fb, 3, 9, line, C_ZONE_R, 1);
        rr_draw_text(fb, W - 3 - rr_text_width("L/R TOOL", 1), 9, "L/R TOOL", C_LGRAY, 1);
    } else if (c->my_id >= 0 && w->players[c->my_id].active) {
        const rr_player *p = &w->players[c->my_id];
        snprintf(line, sizeof(line), "HP %u/%u  LV %u  XP %u/%d  GOLD %lu", (unsigned)p->hp, (unsigned)p->max_hp,
                 (unsigned)p->level, (unsigned)p->xp, rr_xp_to_level(p->level), (unsigned long)p->gold);
        rr_draw_text(fb, 3, 9, line, p->hp * 4 <= p->max_hp ? C_RED : C_WHITE, 1);
        rr_draw_text(fb, W - 3 - rr_text_width("X BUILD", 1), 9, "X BUILD", C_LGRAY, 1);
    }

    if (c->message_timer > 0) {
        int tw = rr_text_width(c->message, 1);
        rr_draw_rect(fb, (W - tw) / 2 - 4, H - 14, tw + 8, 11, C_NAVY);
        rr_draw_text(fb, (W - tw) / 2, H - 11, c->message, C_WHITE, 1);
    }
}

static void draw_title(uint8_t *fb, const rr_client *c, const char *status)
{
    static const uint8_t SKY[] = {C_NAVY, C_NAVY, C_WINDOW, C_BLUE, C_PURPLE, C_DRED, C_ORANGE};
    for (int y = 0; y < H; y++) {
        int band = y * (int)sizeof(SKY) / 150;
        rr_draw_rect(fb, 0, y, W, 1, SKY[band < (int)sizeof(SKY) ? band : (int)sizeof(SKY) - 1]);
    }
    for (int x = 0; x < W; x++) { /* mountain silhouette */
        int h = 40 + (int)(art_hash((uint32_t)(x / 24), 9) % 40);
        int peak = x % 24 < 12 ? x % 24 : 24 - x % 24;
        int top = H - 70 - h / 2 - peak * 2;
        rr_draw_rect(fb, x, top, 1, H - top, C_DROCK);
    }
    rr_draw_rect(fb, 0, H - 60, W, 60, C_FOREST);
    for (int i = 0; i < 6; i++) { /* a little town on the hill */
        int x = 70 + i * 20;
        blit(fb, g_tiles[i % 3 == 0 ? RR_T_TAVERN : i % 2 ? RR_T_HOUSE : RR_T_SHOP], x, H - 76, 1, 0, 0);
    }

    text_center(fb, 34, "RETRO REALM", C_YELLOW, 3);
    text_center(fb, 60, "BUILD THE CITY. DEFEND THE REALM.", C_WHITE, 1);
    if (status) {
        text_center(fb, 100, status, C_WHITE, 1);
    } else if ((c->frame / 30) % 2 == 0) {
        text_center(fb, 96, "A  SOLO", C_WHITE, 2);
    }
    if (!status) text_center(fb, 114, "B  ONLINE", C_LGRAY, 2);
    rr_draw_text(fb, 3, H - 8, "V" RR_VERSION_STRING, C_LGREEN, 1);
}

void rr_render_frame(uint8_t *fb, const rr_client *c)
{
    switch (c->screen) {
    case RR_SCREEN_TITLE:
        draw_title(fb, c, NULL);
        break;
    case RR_SCREEN_CONNECTING:
        draw_title(fb, c, (c->frame / 20) % 2 ? "CONNECTING..." : "CONNECTING");
        text_center(fb, 114, "B  CANCEL", C_LGRAY, 1);
        break;
    case RR_SCREEN_ERROR:
        draw_title(fb, c, c->message);
        text_center(fb, 114, "A  BACK", C_LGRAY, 1);
        break;
    case RR_SCREEN_PLAY:
        draw_world(fb, c);
        draw_hud(fb, c);
        break;
    }
}
