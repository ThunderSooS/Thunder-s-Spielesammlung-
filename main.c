/*
 * BREAKOUT fuer Game Boy / Game Boy Pocket (4 Graustufen, DMG)
 * Erster Entwurf - GBDK-2020
 *
 * Physik:
 *  - Ball mit konstantem Geschwindigkeitsbetrag (elastische Stoesse, keine Energieverluste)
 *  - Position in 8.8-Festkomma, Bewegung in 2 Teilschritten pro Frame,
 *    X- und Y-Achse werden getrennt bewegt und geprueft -> exakte Reflexion
 *    (Einfallswinkel = Ausfallswinkel) an Waenden und Steinen, auch an Ecken.
 *  - Paddle ist leicht gewoelbt modelliert: die Flaechennormale neigt sich zum Rand
 *    hin (bis ca. 20 Grad). Reflexion an dieser Normalen: a_aus = a_ein + 2*alpha.
 *  - Reibung: die Paddle-Geschwindigkeit im Moment des Treffers uebertraegt
 *    einen Teil ihres Impulses auf die Horizontalkomponente des Balls.
 *  - Paddle hat Masse: es beschleunigt und bremst statt sofort zu springen.
 */
#include <gb/gb.h>
#include <stdint.h>
#include "tiles.h"

/* ---------- Spielfeld ---------- */
#define FIELD_L      8u      /* erste freie Pixelspalte  */
#define FIELD_R      151u    /* letzte freie Pixelspalte */
#define FIELD_T      16u     /* erste freie Pixelzeile   */
#define BRICK_ROWS   3u
#define BRICK_COLS   9u
#define BRICK_TROW   4u      /* Kachelzeile der obersten Steinreihe */
#define BRICK_Y0     (BRICK_TROW * 8u)
#define BRICK_Y1     (BRICK_Y0 + BRICK_ROWS * 8u)   /* exklusiv */

#define BALL_SIZE    6u      /* Ball 6x6 Pixel */
#define PADDLE_W_N   24u     /* normale Breite */
#define PADDLE_W_W   48u     /* doppelte Breite */
#define PADDLE_Y     132u    /* Oberkante Paddle (Bildschirm-Pixel) */

#define SPEED        420     /* Ballgeschwindigkeit: 8.8 Pixel/Frame (~1.64 px) */
#define MAX_ANGLE    46      /* max. Winkel zur Senkrechten (256 = 360 Grad) ~65 Grad */
#define MAX_TILT     7       /* max. Neigung der Paddle-Normalen (~10 Grad -> 20 Grad Ablenkung) */

#define PAD_ACCEL    80      /* 8.8 */
#define PAD_FRICT    60
#define PAD_MAXV     832     /* 3.25 px/Frame */

/* Sprites */
#define SPR_PADDLE   0       /* 0..5 */
#define SPR_BALL     6
#define SPR_PU       7
#define SPR_BEAM     8       /* 8, 9 */
/* Sprite-Tiles */
#define T_PAD_L 0
#define T_PAD_M 1
#define T_PAD_R 2
#define T_BALL  3
#define T_LAS_L 4
#define T_LAS_R 5
#define T_BEAM  6
#define T_PU0   7            /* 7 Laser, 8 Breit, 9 Leben */

/* Power-Ups */
#define PU_LASER 0
#define PU_WIDE  1
#define PU_LIFE  2
#define LASER_FRAMES 180     /* 3 s bei ~60 fps */
#define WIDE_FRAMES  600     /* 10 s */
#define PU_FALL      176     /* 8.8 px/Frame (~0.7 px) */
#define BEAM_STEP    3       /* px pro Teilschritt, 2 Teilschritte/Frame */

enum { ST_MENU, ST_SERVE, ST_PLAY, ST_OVER, ST_WIN,
       ST_PONG_SERVE, ST_PONG_PLAY, ST_PONG_WAIT, ST_PONG_CLEAR, ST_PONG_OVER };
#define IS_PONG(s) ((s) >= ST_PONG_SERVE)

/* sin(i * 90/64 Grad) * 256, i = 0..64 */
static const uint16_t sin_tab[65] = {
    0,  6, 13, 19, 25, 31, 38, 44, 50, 56, 62, 68, 74, 80, 86, 92,
   98,104,109,115,121,126,132,137,142,147,152,157,162,167,172,177,
  181,185,190,194,198,202,206,209,213,216,220,223,226,229,231,234,
  237,239,241,243,245,247,248,250,251,252,253,254,255,255,256,256,
  256 };

static uint8_t  bricks[BRICK_ROWS][BRICK_COLS];
static uint8_t  bricks_left;
static uint16_t score;
static uint8_t  lives;
static uint8_t  state;

static uint16_t bx, by;        /* Ballposition 8.8 */
static int8_t   ang;           /* Winkel zur Senkrechten, + = nach rechts */
static int8_t   dir_y;         /* +1 runter, -1 hoch */
static int16_t  vx, vy;        /* Geschwindigkeit pro Teilschritt, 8.8 */

static uint16_t px;            /* Paddle linke Kante 8.8 */
static int16_t  pv;            /* Paddle Geschwindigkeit 8.8 */

static uint8_t  paddle_w;      /* aktuelle Breite in Pixeln */
static uint8_t  level;

/* Power-Up im Fall (max. eines gleichzeitig) */
static uint8_t  pu_active, pu_type, pu_x;
static uint16_t pu_y;          /* 8.8 */
static uint16_t laser_timer, wide_timer;

/* Laserstrahlen */
static uint8_t  beam_on[2], beam_x[2], beam_y[2];


static uint8_t  keys, prev_keys;
static uint8_t  start_level = 1;   /* im Menue waehlbar */
#define PRESSED(k) ((keys & (k)) && !(prev_keys & (k)))
static uint8_t  frame_cnt;

/* ---------- Zufall ---------- */
static uint16_t rnd16 = 0xACE1;
static uint8_t rnd(void) {
    /* 16-Bit-Xorshift; Startwert wird beim Spielstart mit dem DIV-Timer gemischt */
    rnd16 ^= rnd16 << 7;
    rnd16 ^= rnd16 >> 9;
    rnd16 ^= rnd16 << 8;
    return (uint8_t)(rnd16 >> 8);
}
/* Drop-Wahrscheinlichkeit pro zerstoertem Stein (x/256), sinkt mit jedem Level */
static const uint8_t drop_chance[8] = { 64, 44, 32, 24, 18, 14, 11, 9 };
static uint8_t get_drop_chance(void) {
    return (level <= 8) ? drop_chance[level - 1] : 7;
}

/* ---------- Text ---------- */
static uint8_t char_tile(char c) {
    const char *f = FONT_CHARS;
    uint8_t i = 0;
    while (f[i]) { if (f[i] == c) return i; i++; }
    return 0;
}
static void print_at(uint8_t x, uint8_t y, const char *s) {
    while (*s) { set_bkg_tile_xy(x++, y, char_tile(*s++)); }
}
static void print_num(uint8_t x, uint8_t y, uint16_t n, uint8_t digits) {
    x += digits;
    while (digits--) { set_bkg_tile_xy(--x, y, 1 + (n % 10)); n /= 10; }
}
static void clear_row(uint8_t y) {
    uint8_t x;
    for (x = 1; x < 19; x++) set_bkg_tile_xy(x, y, 0);
}

/* ---------- Sound ---------- */
static void sound_init(void) {
    NR52_REG = 0x80;   /* Sound an */
    NR50_REG = 0x77;   /* Lautstaerke links/rechts max */
    NR51_REG = 0xFF;   /* alle Kanaele auf beide Seiten */
}
/* Kanal 1: Stein getroffen - Tonhoehe je nach Reihe */
static void snd_brick(uint8_t row) {
    static const uint16_t f[3] = { 1923, 1899, 1849 };  /* ~1050Hz, ~880Hz, ~660Hz */
    uint16_t fr = f[row];
    NR10_REG = 0x00;
    NR11_REG = 0x80 | 50;       /* Duty 50%, kurze Laenge */
    NR12_REG = 0xF1;            /* Start-Lautst. 15, abklingend */
    NR13_REG = (uint8_t)fr;
    NR14_REG = 0xC0 | (uint8_t)(fr >> 8);
}
/* Kanal 2: Paddle getroffen - tiefer, "dumpfer" Ton */
static void snd_paddle(void) {
    uint16_t fr = 1546;         /* ~261 Hz */
    NR21_REG = 0x40 | 44;       /* Duty 25% */
    NR22_REG = 0xF2;
    NR23_REG = (uint8_t)fr;
    NR24_REG = 0xC0 | (uint8_t)(fr >> 8);
}
/* Kanal 2: leises Klicken an der Wand */
static void snd_wall(void) {
    uint16_t fr = 1985;
    NR21_REG = 0x00 | 60;
    NR22_REG = 0x61;
    NR23_REG = (uint8_t)fr;
    NR24_REG = 0xC0 | (uint8_t)(fr >> 8);
}
/* Kanal 4: Ball verloren - Rauschen */
static void snd_lose(void) {
    NR41_REG = 0x00;
    NR42_REG = 0xF5;
    NR43_REG = 0x64;
    NR44_REG = 0x80;
}
/* Kanal 4: Laser-Schuss - kurzes helles Zischen */
static void snd_laser(void) {
    NR41_REG = 0x30;
    NR42_REG = 0xA1;
    NR43_REG = 0x22;
    NR44_REG = 0xC0;
}
/* Kanal 1: Power-Up eingesammelt - schneller Sweep nach oben */
static void snd_powerup(void) {
    uint16_t fr = 1650;
    NR10_REG = 0x15;
    NR11_REG = 0x80 | 20;
    NR12_REG = 0xD2;
    NR13_REG = (uint8_t)fr;
    NR14_REG = 0xC0 | (uint8_t)(fr >> 8);
}
/* Kanal 1 mit Sweep nach oben: gewonnen */
static void snd_win(void) {
    uint16_t fr = 1600;
    NR10_REG = 0x26;
    NR11_REG = 0x80;
    NR12_REG = 0xF7;
    NR13_REG = (uint8_t)fr;
    NR14_REG = 0x80 | (uint8_t)(fr >> 8);
}

/* ---------- Spielfeld zeichnen ---------- */
static void draw_hud(void) {
    print_num(6, 0, score, 4);
    set_bkg_tile_xy(18, 0, 1 + lives);
}
static uint8_t map_buf[20 * 18];
static void build_field(void) {
    uint8_t x, y;
    uint8_t *m = map_buf;
    for (y = 0; y < 18; y++)
        for (x = 0; x < 20; x++) {
            uint8_t t = 0;
            if (y >= 1 && (x == 0 || x == 19 || y == 1)) t = TILE_WALL;
            else if (y >= BRICK_TROW && y < BRICK_TROW + BRICK_ROWS && x >= 1 && x <= 18)
                t = TILE_BRICK0 + (y - BRICK_TROW) * 2 + ((x - 1) & 1);
            *m++ = t;
        }
    for (y = 0; y < BRICK_ROWS; y++)
        for (x = 0; x < BRICK_COLS; x++) bricks[y][x] = 1;
    set_bkg_tiles(0, 0, 20, 18, map_buf);
    print_at(0, 0, "SCORE");
    print_at(12, 0, "BALLS");
    bricks_left = BRICK_ROWS * BRICK_COLS;
    draw_hud();
}

/* ---------- Physik ---------- */
static void update_velocity(void) {
    uint8_t a = (ang < 0) ? (uint8_t)(-ang) : (uint8_t)ang;
    /* halbe Geschwindigkeit pro Teilschritt (2 Teilschritte/Frame) */
    int16_t sx = (int16_t)(((uint32_t)SPEED * sin_tab[a]) >> 9);
    int16_t sy = (int16_t)(((uint32_t)SPEED * sin_tab[64 - a]) >> 9);
    vx = (ang < 0) ? -sx : sx;
    vy = (dir_y < 0) ? -sy : sy;
}

/* Stein an Pixelposition? Gibt Reihe+1 zurueck oder 0. Entfernt den Stein. */
static uint8_t hit_brick_at(uint8_t x, uint8_t y) {
    uint8_t r, c;
    if (y < BRICK_Y0 || y >= BRICK_Y1 || x < FIELD_L || x > FIELD_R) return 0;
    r = (y - BRICK_Y0) >> 3;
    c = (x - FIELD_L) >> 4;
    if (!bricks[r][c]) return 0;
    bricks[r][c] = 0;
    bricks_left--;
    set_bkg_tile_xy(1 + c * 2, BRICK_TROW + r, 0);
    set_bkg_tile_xy(2 + c * 2, BRICK_TROW + r, 0);
    score += (uint16_t)(BRICK_ROWS - r) * 10u;
    /* Power-Up fallen lassen? */
    if (!pu_active && rnd() < get_drop_chance()) {
        uint8_t t = rnd();
        pu_type = (t < 102) ? PU_LASER : (t < 204) ? PU_WIDE : PU_LIFE;  /* 40/40/20 % */
        pu_x = FIELD_L + c * 16 + 4;
        pu_y = (uint16_t)(BRICK_Y0 + r * 8) << 8;
        pu_active = 1;
    }
    return r + 1;
}

static void brick_sound(uint8_t h1, uint8_t h2) {
    uint8_t h = h1 ? h1 : h2;
    if (h2 && h2 < h) h = h2;
    snd_brick(h - 1);
    draw_hud();
}

static void paddle_bounce(void) {
    int16_t ball_c = (int16_t)(bx >> 8) + BALL_SIZE / 2;
    int16_t half   = paddle_w / 2;
    int16_t pad_c  = (int16_t)(px >> 8) + half;
    int16_t off    = ball_c - pad_c;              /* -half-2 .. +half+2 */
    int16_t tilt   = (off * MAX_TILT) / (half + 2); /* Neigung der Normalen */
    int16_t a      = (int16_t)ang + 2 * tilt + (pv >> 7);  /* Reflexion + Reibung */
    if (a >  MAX_ANGLE) a =  MAX_ANGLE;
    if (a < -MAX_ANGLE) a = -MAX_ANGLE;
    ang = (int8_t)a;
    dir_y = -1;
    update_velocity();
    snd_paddle();
}

/* Ein Teilschritt. Rueckgabe 1 = Ball verloren */
static uint8_t ball_step(void) {
    uint16_t nx, ny;
    uint8_t x, y, h1, h2;

    /* ---- X-Achse ---- */
    nx = bx + (uint16_t)vx;
    x = nx >> 8; y = by >> 8;
    if (vx < 0 && x < FIELD_L) {
        nx = ((uint16_t)FIELD_L << 9) - nx;          /* exakte Spiegelung */
        ang = -ang; update_velocity(); snd_wall();
    } else if (vx > 0 && x + BALL_SIZE - 1 > FIELD_R) {
        nx = ((uint16_t)(FIELD_R - BALL_SIZE + 1) << 9) - nx;
        ang = -ang; update_velocity(); snd_wall();
    } else {
        uint8_t ex = (vx > 0) ? x + BALL_SIZE - 1 : x;
        h1 = hit_brick_at(ex, y);
        h2 = hit_brick_at(ex, y + BALL_SIZE - 1);
        if (h1 || h2) { nx = bx; ang = -ang; update_velocity(); brick_sound(h1, h2); }
    }
    bx = nx;

    /* ---- Y-Achse ---- */
    ny = by + (uint16_t)vy;
    x = bx >> 8; y = ny >> 8;
    if (vy < 0 && y < FIELD_T) {
        ny = ((uint16_t)FIELD_T << 9) - ny;
        dir_y = 1; update_velocity(); snd_wall();
    } else {
        uint8_t ey = (vy > 0) ? y + BALL_SIZE - 1 : y;
        h1 = hit_brick_at(x, ey);
        h2 = hit_brick_at(x + BALL_SIZE - 1, ey);
        if (h1 || h2) {
            ny = by; dir_y = -dir_y; update_velocity(); brick_sound(h1, h2);
        } else if (vy > 0) {
            uint8_t oldb = (by >> 8) + BALL_SIZE - 1;
            uint8_t newb = y + BALL_SIZE - 1;
            uint8_t pl = px >> 8;
            if (oldb < PADDLE_Y && newb >= PADDLE_Y &&
                x + BALL_SIZE > pl && x < pl + paddle_w) {
                /* Spiegelung an der Paddle-Oberkante */
                ny = ((uint16_t)(PADDLE_Y - BALL_SIZE) << 9) - ny;
                paddle_bounce();
            } else if (y >= 144) {
                return 1;
            }
        }
    }
    by = ny;
    return 0;
}

/* ---------- Paddle ---------- */
static void paddle_update(void) {
    if (keys & J_LEFT) {
        pv -= PAD_ACCEL; if (pv < -PAD_MAXV) pv = -PAD_MAXV;
    } else if (keys & J_RIGHT) {
        pv += PAD_ACCEL; if (pv > PAD_MAXV) pv = PAD_MAXV;
    } else {
        if (pv > PAD_FRICT) pv -= PAD_FRICT;
        else if (pv < -PAD_FRICT) pv += PAD_FRICT;
        else pv = 0;
    }
    {
        int16_t np = (int16_t)(px >> 4) + (pv >> 4);   /* in 12.4 rechnen (kein Ueberlauf) */
        int16_t lo = FIELD_L << 4;
        int16_t hi = (FIELD_R - paddle_w + 1) << 4;
        if (np < lo) { np = lo; pv = 0; }
        if (np > hi) { np = hi; pv = 0; }
        px = (uint16_t)np << 4;
    }
}

static void clamp_paddle(void) {
    int16_t p = (int16_t)(px >> 8);
    if (p < (int16_t)FIELD_L) p = FIELD_L;
    if (p > (int16_t)(FIELD_R - paddle_w + 1)) p = FIELD_R - paddle_w + 1;
    px = (uint16_t)p << 8;
}

static void set_wide(uint8_t on) {
    if (on && paddle_w == PADDLE_W_N) { px -= (uint16_t)((PADDLE_W_W - PADDLE_W_N) / 2) << 8; paddle_w = PADDLE_W_W; }
    if (!on && paddle_w == PADDLE_W_W) { px += (uint16_t)((PADDLE_W_W - PADDLE_W_N) / 2) << 8; paddle_w = PADDLE_W_N; }
    clamp_paddle();
}

static void reset_powerups(void) {
    pu_active = 0;
    laser_timer = 0;
    wide_timer = 0;
    beam_on[0] = beam_on[1] = 0;
    set_wide(0);
}

static void apply_powerup(void) {
    snd_powerup();
    if (pu_type == PU_LASER) laser_timer = LASER_FRAMES;
    else if (pu_type == PU_WIDE) { wide_timer = WIDE_FRAMES; set_wide(1); }
    else { if (lives < 9) lives++; draw_hud(); }
}

static void powerup_update(void) {
    uint8_t y, pl;
    if (!pu_active) return;
    pu_y += PU_FALL;
    y = pu_y >> 8;
    pl = px >> 8;
    if (y + 6 >= PADDLE_Y && y <= PADDLE_Y + 4 && pu_x + 7 > pl && pu_x < pl + paddle_w) {
        pu_active = 0;
        apply_powerup();
    } else if (y >= 144) pu_active = 0;
}

static void timers_update(void) {
    if (laser_timer) laser_timer--;
    if (wide_timer) { if (--wide_timer == 0) set_wide(0); }
}

static void beams_update(void) {
    uint8_t i, s, h;
    /* Feuern: A (gedrueckt halten = Dauerfeuer, sobald beide Strahlen weg sind) */
    if (laser_timer && (keys & J_A) && !beam_on[0] && !beam_on[1]) {
        uint8_t pl = px >> 8;
        beam_on[0] = beam_on[1] = 1;
        beam_x[0] = pl + 2;
        beam_x[1] = pl + paddle_w - 4;
        beam_y[0] = beam_y[1] = PADDLE_Y - 6;
        snd_laser();
    }
    for (i = 0; i < 2; i++) {
        if (!beam_on[i]) continue;
        for (s = 0; s < 2; s++) {
            if (beam_y[i] < FIELD_T + BEAM_STEP) { beam_on[i] = 0; break; }
            beam_y[i] -= BEAM_STEP;
            h = hit_brick_at(beam_x[i], beam_y[i]);
            if (!h) h = hit_brick_at(beam_x[i] + 1, beam_y[i]);
            if (h) { beam_on[i] = 0; snd_brick(h - 1); draw_hud(); break; }
        }
    }
}

static void draw_sprites(void) {
    uint8_t i, n, p, t, pal, y;
    uint16_t tm;
    /* Paddle: blinkt in der letzten 1,5 s eines Power-Ups */
    n = paddle_w >> 3;
    p = (px >> 8) + 8;
    y = PADDLE_Y + 16 - 3;
    tm = laser_timer;
    if (wide_timer && (tm == 0 || wide_timer < tm)) tm = wide_timer;
    pal = (tm && tm < 90 && (frame_cnt & 8)) ? S_PALETTE : 0;
    if (state == ST_MENU) n = 0;
    for (i = 0; i < 6; i++) {
        if (i < n) {
            if (i == 0)          t = laser_timer ? T_LAS_L : T_PAD_L;
            else if (i == n - 1) t = laser_timer ? T_LAS_R : T_PAD_R;
            else                 t = T_PAD_M;
            set_sprite_tile(SPR_PADDLE + i, t);
            set_sprite_prop(SPR_PADDLE + i, pal);
            move_sprite(SPR_PADDLE + i, p + i * 8, y);
        } else move_sprite(SPR_PADDLE + i, 0, 0);
    }
    if (state == ST_MENU || state == ST_OVER || state == ST_WIN) move_sprite(SPR_BALL, 0, 0);
    else move_sprite(SPR_BALL, (bx >> 8) + 8, (by >> 8) + 16);
    if (pu_active) {
        set_sprite_tile(SPR_PU, T_PU0 + pu_type);
        move_sprite(SPR_PU, pu_x + 8, (pu_y >> 8) + 16);
    } else move_sprite(SPR_PU, 0, 0);
    for (i = 0; i < 2; i++) {
        if (beam_on[i]) move_sprite(SPR_BEAM + i, beam_x[i] - 3 + 8, beam_y[i] + 16);
        else move_sprite(SPR_BEAM + i, 0, 0);
    }
}

static void serve_reset(void) {
    state = ST_SERVE;
    reset_powerups();
    print_at(6, 10, "LEVEL");
    print_num(12, 10, level, 1 + (level > 9));
    print_at(6, 12, "PRESS A");
}

static void new_game(void) {
    score = 0; lives = 3; level = start_level;
    paddle_w = PADDLE_W_N;
    px = (uint16_t)((80 - PADDLE_W_N / 2)) << 8; pv = 0;
    build_field();
    serve_reset();
}

/* ================= Spiel 2: Pong ================= */
static void menu_enter(void);
#include "pong.inc"

/* ================= Hauptmenue & Musik ================= */

/* Frequenzwerte (11 Bit) fuer die Tonkanaele: x = 2048 - 131072 / f */
#define N_G2   711
#define N_A2   856
#define N_B2   986
#define N_C3  1046
#define N_D3  1155
#define N_E3  1253
#define N_FS4 1694
#define N_G4  1714
#define N_A4  1750
#define N_B4  1783
#define N_C5  1798
#define N_D5  1825
#define N_E5  1849

/* "God Save the King" (traditionell, gemeinfrei), G-Dur, 3/4-Takt.
 * Dauer in Achteln: 1 = Achtel, 2 = Viertel, 3 = punkt. Viertel, 6 = punkt. Halbe */
typedef struct { uint16_t f; uint8_t len; } note_t;
static const note_t melody[] = {
    /* God save our gracious King */
    {N_G4,2},{N_G4,2},{N_A4,2},
    {N_FS4,3},{N_G4,1},{N_A4,2},
    /* long live our noble King */
    {N_B4,2},{N_B4,2},{N_C5,2},
    {N_B4,3},{N_A4,1},{N_G4,2},
    /* God save the King */
    {N_A4,2},{N_G4,2},{N_FS4,2},
    {N_G4,6},
    /* Send him victorious */
    {N_D5,2},{N_D5,2},{N_D5,2},
    {N_D5,3},{N_C5,1},{N_B4,2},
    /* happy and glorious */
    {N_C5,2},{N_C5,2},{N_C5,2},
    {N_C5,3},{N_B4,1},{N_A4,2},
    /* long to reign over us */
    {N_B4,2},{N_C5,1},{N_B4,1},{N_A4,1},{N_G4,1},
    {N_B4,3},{N_C5,1},{N_D5,2},
    /* God save the King */
    {N_E5,1},{N_C5,1},{N_B4,2},{N_A4,2},
    {N_G4,6},
};
#define MELODY_LEN (sizeof(melody) / sizeof(melody[0]))
/* Bass: ein Ton pro Takt (punktierte Halbe) */
static const uint16_t bass[] = {
    N_G2, N_D3, N_G2, N_D3, N_D3, N_G2,
    N_B2, N_G2, N_C3, N_A2, N_G2, N_G2, N_C3, N_G2,
};
#define BASS_LEN (sizeof(bass) / sizeof(bass[0]))
#define EIGHTH_FRAMES 14     /* Tempo: 1 Achtel = 14 Frames (~128 bpm) */
#define BAR_FRAMES   (6 * EIGHTH_FRAMES)

static uint8_t  mus_on = 1, mus_playing;
static uint8_t  mus_idx, mus_bass;
static uint8_t  mus_wait;     /* Frames bis zur naechsten Melodienote */
static uint8_t  mus_bar;      /* Frames bis zum naechsten Bass-Ton */
static uint8_t  mus_pause;    /* kurze Pause vor der Wiederholung */

static void mus_note(uint16_t f, uint8_t len) {
    NR10_REG = 0x00;
    NR11_REG = 0x80;                 /* Duty 50 % */
    NR12_REG = (len >= 6) ? 0xA5 : (len >= 3) ? 0xA3 : 0xA2;  /* lange Noten klingen laenger aus */
    NR13_REG = (uint8_t)f;
    NR14_REG = 0x80 | (uint8_t)(f >> 8);
}
static void mus_bass_note(uint16_t f) {
    NR21_REG = 0x00;                 /* Duty 12,5 % - weicher Bass */
    NR22_REG = 0x75;
    NR23_REG = (uint8_t)f;
    NR24_REG = 0x80 | (uint8_t)(f >> 8);
}
static void music_start(void) {
    mus_idx = 0; mus_bass = 0; mus_wait = 0; mus_bar = 0; mus_pause = 0;
    mus_playing = mus_on;
}
static void music_stop(void) {
    mus_playing = 0;
    NR12_REG = 0x00; NR14_REG = 0x80;   /* Kanal 1 stumm */
    NR22_REG = 0x00; NR24_REG = 0x80;   /* Kanal 2 stumm */
}
static void music_update(void) {
    if (!mus_playing) return;
    if (mus_pause) { if (--mus_pause == 0) { mus_idx = 0; mus_bass = 0; mus_wait = 0; mus_bar = 0; } return; }
    if (mus_bar == 0) {
        mus_bass_note(bass[mus_bass]);
        mus_bass++;
        mus_bar = BAR_FRAMES;
    }
    mus_bar--;
    if (mus_wait == 0) {
        if (mus_idx >= MELODY_LEN) { mus_pause = 60; return; }   /* 1 s Pause, dann von vorn */
        mus_note(melody[mus_idx].f, melody[mus_idx].len);
        mus_wait = melody[mus_idx].len * EIGHTH_FRAMES;
        mus_idx++;
    }
    mus_wait--;
}

/* Menue-Klick (Rauschkanal, stoert die Musik nicht) */
static void snd_menu(void) {
    NR41_REG = 0x3A;
    NR42_REG = 0x61;
    NR43_REG = 0x11;
    NR44_REG = 0xC0;
}

#define MENU_ITEMS 4
#define MENU_Y0    8          /* erste Menuezeile, Abstand 2 */
static uint8_t menu_sel;

static void menu_draw_values(void) {
    uint8_t i;
    for (i = 0; i < MENU_ITEMS; i++)
        set_bkg_tile_xy(4, MENU_Y0 + i * 2, (i == menu_sel) ? char_tile('>') : 0);
    print_num(13, MENU_Y0 + 4, start_level, 1);
    print_at(13, MENU_Y0 + 6, mus_on ? "ON " : "OFF");
}

static void menu_enter(void) {
    uint8_t x, y, *m = map_buf;
    state = ST_MENU;
    reset_powerups();
    for (y = 0; y < 18; y++)
        for (x = 0; x < 20; x++) {
            uint8_t t = 0;
            /* Steinreihen als Rahmen oben und unten */
            if (y == 0 || y == 17)      t = TILE_BRICK0 + 0 + (x & 1);
            else if (y == 1 || y == 16) t = TILE_BRICK0 + 2 + (x & 1);
            *m++ = t;
        }
    /* grosses Logo "THUNDER'S": 9 Zeichen x 2 Tiles, Zeilen 3-4 */
    for (x = 0; x < BIG_LEN; x++) {
        uint8_t o = 1 + x * 2;
        map_buf[3 * 20 + o]     = TILE_BIG0 + x * 4;
        map_buf[3 * 20 + o + 1] = TILE_BIG0 + x * 4 + 1;
        map_buf[4 * 20 + o]     = TILE_BIG0 + x * 4 + 2;
        map_buf[4 * 20 + o + 1] = TILE_BIG0 + x * 4 + 3;
    }
    set_bkg_tiles(0, 0, 20, 18, map_buf);
    print_at(3, 6, "SPIELESAMMLUNG");
    print_at(6, MENU_Y0,     "BREAKOUT");
    print_at(6, MENU_Y0 + 2, "PONG");
    print_at(6, MENU_Y0 + 4, "LEVEL");
    print_at(6, MENU_Y0 + 6, "MUSIC");
    menu_draw_values();
    music_start();
}

static void menu_update(void) {
    uint8_t changed = 0;
    music_update();
    if (PRESSED(J_UP))   { menu_sel = (menu_sel == 0) ? MENU_ITEMS - 1 : menu_sel - 1; changed = 1; }
    if (PRESSED(J_DOWN)) { menu_sel = (menu_sel + 1) % MENU_ITEMS; changed = 1; }
    if (menu_sel == 2) {
        if (PRESSED(J_LEFT)  && start_level > 1) { start_level--; changed = 1; }
        if (PRESSED(J_RIGHT) && start_level < 9) { start_level++; changed = 1; }
    }
    if (menu_sel == 3 && (PRESSED(J_LEFT) || PRESSED(J_RIGHT) || PRESSED(J_A))) {
        mus_on = !mus_on;
        if (mus_on) music_start(); else music_stop();
        changed = 1;
    }
    if (changed) { snd_menu(); menu_draw_values(); }
    if (menu_sel <= 1 && (PRESSED(J_A) || PRESSED(J_START))) {
        music_stop();
        rnd16 ^= ((uint16_t)DIV_REG << 8) | frame_cnt;   /* Zufall vom Zeitpunkt des Starts */
        if (!rnd16) rnd16 = 0xACE1;
        if (menu_sel == 0) new_game(); else pong_new_game();
    }
}

void main(void) {
    DISPLAY_OFF;
    BGP_REG  = 0xE4;   /* 4 Graustufen: weiss, hellgrau, dunkelgrau, schwarz */
    OBP0_REG = 0xE4;
    set_bkg_data(0, BG_TILE_COUNT, bg_tiles);
    OBP1_REG = 0x90;   /* hellere Palette fuer Blinken */
    set_sprite_data(0, SPR_TILE_COUNT, spr_tiles);
    set_sprite_tile(SPR_BALL, T_BALL);
    set_sprite_tile(SPR_BEAM, T_BEAM);
    set_sprite_tile(SPR_BEAM + 1, T_BEAM);
    sound_init();

    lives = 3;
    paddle_w = PADDLE_W_N; level = 1;
    px = (uint16_t)((80 - PADDLE_W_N / 2)) << 8;
    menu_enter();

    SHOW_BKG; SHOW_SPRITES; DISPLAY_ON;

    while (1) {
        prev_keys = keys;
        keys = joypad();
        frame_cnt++;

        if (IS_PONG(state)) {
            pong_update();
            if (IS_PONG(state)) pong_draw(); else draw_sprites();
            wait_vbl_done();
            continue;
        }

        switch (state) {
        case ST_MENU:
            menu_update();
            break;
        case ST_SERVE:
            paddle_update();
            bx = px + ((uint16_t)(paddle_w / 2 - BALL_SIZE / 2) << 8);
            by = (uint16_t)(PADDLE_Y - BALL_SIZE) << 8;
            if (PRESSED(J_A)) {
                clear_row(10); clear_row(12);
                /* Startwinkel: leicht schraeg, Paddle-Bewegung beeinflusst Richtung */
                ang = (frame_cnt & 1) ? 12 : -12;
                ang += (int8_t)(pv >> 7);
                dir_y = -1;
                update_velocity();
                snd_paddle();
                state = ST_PLAY;
            }
            break;
        case ST_PLAY:
            if (PRESSED(J_START)) {           /* Pause */
                print_at(7, 11, "PAUSE");
                print_at(3, 13, "SELECT: MENU");
                do { prev_keys = keys; wait_vbl_done(); keys = joypad(); }
                while (!PRESSED(J_START) && !PRESSED(J_SELECT));
                clear_row(11); clear_row(13);
                if (keys & J_SELECT) { menu_enter(); break; }
            }
            paddle_update();
            timers_update();
            powerup_update();
            beams_update();
            if (bricks_left == 0) {
                /* Level geschafft (auch per Laser moeglich) */
            } else if (ball_step() || ball_step()) {
                snd_lose();
                lives--;
                draw_hud();
                if (lives == 0) {
                    reset_powerups();
                    state = ST_OVER;
                    print_at(5, 10, "GAME OVER");
                    print_at(4, 12, "PRESS START");
                } else serve_reset();
            }
            if (bricks_left == 0) {
                reset_powerups();
                snd_win();
                state = ST_WIN;
                print_at(4, 10, "LEVEL CLEAR!");
                print_at(4, 12, "PRESS START");
            }
            break;
        case ST_OVER:
        case ST_WIN:
            if (PRESSED(J_START)) {
                uint8_t w = (state == ST_WIN);
                uint16_t s = score; uint8_t l = lives, lv = level;
                clear_row(10); clear_row(12);
                if (w) {   /* naechstes Level: Punkte und Leben bleiben */
                    score = s; lives = l; level = lv + 1;
                    build_field();
                    serve_reset();
                } else menu_enter();   /* nach Game Over zurueck ins Hauptmenue */
            }
            break;
        }
        draw_sprites();
        wait_vbl_done();
    }
}
