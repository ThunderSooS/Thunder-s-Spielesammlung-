/*
 * PONG - zweites Spiel der Sammlung
 * Wird von main.c per #include eingebunden (eine Uebersetzungseinheit,
 * nutzt dieselben Hilfsfunktionen fuer Text, Sound, Zufall und Physik).
 *
 * Links: Spieler, rechts: CPU.
 * 3 Tore fuer den Spieler -> naechstes Level, CPU wird staerker.
 * 3 Tore fuer die CPU     -> Game Over.
 *
 * Physik wie bei Breakout: konstanter Geschwindigkeitsbetrag, exakte
 * Spiegelung an Banden, leicht gewoelbte Schlaeger (Ablenkung je nach
 * Trefferpunkt) und Reibung (Schlaegergeschwindigkeit uebertraegt Impuls).
 * Pro Ballwechsel wird der Ball mit jedem Schlaegertreffer etwas schneller.
 */

#define P_TOP       16u      /* erste freie Zeile (unter der oberen Bande) */
#define P_BOT       135u     /* letzte freie Zeile (ueber der unteren Bande) */
#define P_PAD_H     24u
#define P_PAD_W     4u
#define P_PL_X      8u       /* Spieler-Schlaeger linke Kante */
#define P_CPU_X     148u     /* CPU-Schlaeger linke Kante */
#define P_MAX_ANG   42       /* max. Winkel zur Waagerechten (~59 Grad) */
#define P_TILT      7        /* Woelbung: max. Neigung der Schlaegernormalen */
#define P_WIN_GOALS 3

#define SPR_PONG_PL  0       /* 0..2 */
#define SPR_PONG_CPU 3       /* 3..5 */
#define T_PONG_T     10
#define T_PONG_M     11
#define T_PONG_B     12

static uint8_t  p_goals_pl, p_goals_cpu;
static uint16_t p_pl_y, p_cpu_y;     /* Schlaeger-Oberkante 8.8 */
static int16_t  p_pl_v, p_cpu_v;     /* Geschwindigkeit 8.8 */
static uint16_t p_speed;             /* aktueller Ballbetrag 8.8 px/Frame */
static int8_t   p_dir_x;             /* +1 nach rechts, -1 nach links */
static uint8_t  p_wait;              /* Frames bis zum automatischen Aufschlag */
static int8_t   p_aim;               /* Zielfehler der CPU in Pixeln */

/* ---- Levelabhaengige Staerke der CPU ---- */
static uint16_t cpu_maxv(void)   { uint16_t v = 160 + (uint16_t)level * 40;  return v > 704 ? 704 : v; }
static uint8_t  cpu_react_x(void){ return (level >= 10) ? 40 : 120 - level * 8; }
static uint8_t  cpu_err(void)    { return (level >= 12) ? 2 : 14 - level; }
static uint16_t ball_base(void)  { uint16_t v = 352 + (uint16_t)level * 16;  return v > 560 ? 560 : v; }

/* ---- Sounds ---- */
static void snd_pong_cpu(void) {           /* CPU-Schlaeger: etwas hoeher als der Spieler */
    uint16_t fr = 1714;                    /* ~392 Hz */
    NR10_REG = 0x00;
    NR11_REG = 0x80 | 44;
    NR12_REG = 0xF2;
    NR13_REG = (uint8_t)fr;
    NR14_REG = 0xC0 | (uint8_t)(fr >> 8);
}

/* ---- Spielfeld ---- */
static void pong_clear_row(uint8_t y) {
    uint8_t x;
    for (x = 0; x < 20; x++) set_bkg_tile_xy(x, y, 0);
    set_bkg_tile_xy(9, y, TILE_NET);
    set_bkg_tile_xy(10, y, TILE_NET + 1);
}

static void pong_hud(void) {
    set_bkg_tile_xy(4, 0, 1 + p_goals_pl);
    set_bkg_tile_xy(12, 0, 0);
    print_num(11, 0, level, (level > 9) ? 2 : 1);
    set_bkg_tile_xy(19, 0, 1 + p_goals_cpu);
}

static void pong_build(void) {
    uint8_t x, y, *m = map_buf;
    for (y = 0; y < 18; y++)
        for (x = 0; x < 20; x++) {
            uint8_t t = 0;
            if (y == 1 || y == 17) t = TILE_WALL;
            else if (y > 1 && x == 9)  t = TILE_NET;
            else if (y > 1 && x == 10) t = TILE_NET + 1;
            *m++ = t;
        }
    set_bkg_tiles(0, 0, 20, 18, map_buf);
    print_at(0, 0, "YOU");
    print_at(8, 0, "LV");
    print_at(15, 0, "CPU");
    pong_hud();
}

/* ---- Physik ---- */
static void pong_velocity(void) {
    uint8_t a = (ang < 0) ? (uint8_t)(-ang) : (uint8_t)ang;
    /* Winkel zur Waagerechten; halbe Geschwindigkeit pro Teilschritt */
    int16_t sx = (int16_t)(((uint32_t)p_speed * sin_tab[64 - a]) >> 9);
    int16_t sy = (int16_t)(((uint32_t)p_speed * sin_tab[a]) >> 9);
    vx = (p_dir_x < 0) ? -sx : sx;
    vy = (ang < 0) ? -sy : sy;
}

static void pong_center_ball(void) {
    bx = (uint16_t)(80 - BALL_SIZE / 2) << 8;
    by = (uint16_t)((P_TOP + P_BOT) / 2 - BALL_SIZE / 2) << 8;
}

static void pong_serve(int8_t dir) {
    uint8_t r = rnd();
    p_speed = ball_base();
    p_dir_x = dir;
    ang = 8 + (r & 15);                   /* 11 .. 32 Grad */
    if (r & 0x80) ang = -ang;
    pong_velocity();
    p_aim = (int8_t)((rnd() % (2 * cpu_err() + 1))) - (int8_t)cpu_err();
}

/* Reflexion am Schlaeger: gewoelbte Flaeche + Reibung */
static void pong_paddle_hit(uint16_t pad_y, int16_t pad_v) {
    int16_t ball_c = (int16_t)(by >> 8) + BALL_SIZE / 2;
    int16_t pad_c  = (int16_t)(pad_y >> 8) + P_PAD_H / 2;
    int16_t off    = ball_c - pad_c;                     /* ca. -15 .. +15 */
    int16_t a      = (int16_t)ang + 2 * ((off * P_TILT) / 15) + (pad_v >> 7);
    if (a >  P_MAX_ANG) a =  P_MAX_ANG;
    if (a < -P_MAX_ANG) a = -P_MAX_ANG;
    ang = (int8_t)a;
    p_dir_x = -p_dir_x;
    if (p_speed < ball_base() + 192) p_speed += 8;      /* Ballwechsel wird schneller */
    pong_velocity();
}

/* Ein Teilschritt. Rueckgabe: 0 = weiter, 1 = Tor fuer CPU, 2 = Tor fuer Spieler */
static uint8_t pong_step(void) {
    uint16_t nx, ny;
    uint8_t x, y, ox;

    /* ---- Y: Banden ---- */
    ny = by + (uint16_t)vy;
    y = ny >> 8;
    if (vy < 0 && (y < P_TOP || y > 200)) {
        ny = ((uint16_t)P_TOP << 9) - ny;
        ang = -ang; pong_velocity(); snd_wall();
    } else if (vy > 0 && y + BALL_SIZE - 1 > P_BOT) {
        ny = ((uint16_t)(P_BOT - BALL_SIZE + 1) << 9) - ny;
        ang = -ang; pong_velocity(); snd_wall();
    }
    by = ny;
    y = by >> 8;

    /* ---- X: Schlaeger und Tore ---- */
    ox = bx >> 8;
    nx = bx + (uint16_t)vx;
    x = nx >> 8;
    if (vx < 0) {
        uint8_t face = P_PL_X + P_PAD_W;                 /* rechte Kante Spielerschlaeger */
        uint8_t py = p_pl_y >> 8;
        if (ox >= face && (x < face || x > 200) &&
            y + BALL_SIZE > py && y < py + P_PAD_H) {
            nx = ((uint16_t)face << 9) - nx;             /* Spiegelung an der Schlaegerflaeche */
            bx = nx;
            pong_paddle_hit(p_pl_y, p_pl_v);
            snd_paddle();
            /* neuer Zielfehler fuer die CPU */
            p_aim = (int8_t)((rnd() % (2 * cpu_err() + 1))) - (int8_t)cpu_err();
            return 0;
        }
        if (x > 200 || x == 0) return 1;                  /* links raus */
    } else {
        uint8_t face = P_CPU_X;                           /* linke Kante CPU-Schlaeger */
        uint8_t cy = p_cpu_y >> 8;
        if (ox + BALL_SIZE <= face && x + BALL_SIZE > face &&
            y + BALL_SIZE > cy && y < cy + P_PAD_H) {
            nx = ((uint16_t)(face - BALL_SIZE) << 9) - nx;
            bx = nx;
            pong_paddle_hit(p_cpu_y, p_cpu_v);
            snd_pong_cpu();
            return 0;
        }
        if (x >= 160) return 2;                           /* rechts raus */
    }
    bx = nx;
    return 0;
}

/* ---- Schlaeger mit Traegheit ---- */
static void pong_move_pad(uint16_t *py, int16_t *pv, int8_t dir, int16_t maxv) {
    int16_t np;
    if (dir < 0)      { *pv -= PAD_ACCEL; if (*pv < -maxv) *pv = -maxv; }
    else if (dir > 0) { *pv += PAD_ACCEL; if (*pv >  maxv) *pv =  maxv; }
    else {
        if (*pv > PAD_FRICT) *pv -= PAD_FRICT;
        else if (*pv < -PAD_FRICT) *pv += PAD_FRICT;
        else *pv = 0;
    }
    np = (int16_t)(*py >> 4) + (*pv >> 4);               /* 12.4 */
    if (np < (int16_t)(P_TOP << 4)) { np = P_TOP << 4; *pv = 0; }
    if (np > (int16_t)((P_BOT - P_PAD_H + 1) << 4)) { np = (P_BOT - P_PAD_H + 1) << 4; *pv = 0; }
    *py = (uint16_t)np << 4;
}

static void pong_cpu_ai(void) {
    int16_t target, c = (int16_t)(p_cpu_y >> 8) + P_PAD_H / 2;
    int8_t dir = 0;
    if (vx > 0 && (bx >> 8) > cpu_react_x())
        target = (int16_t)(by >> 8) + BALL_SIZE / 2 + p_aim;   /* Ball verfolgen (mit Zielfehler) */
    else
        target = (P_TOP + P_BOT) / 2;                          /* sonst zur Mitte zurueck */
    if (target > c + 3) dir = 1;
    else if (target < c - 3) dir = -1;
    pong_move_pad(&p_cpu_y, &p_cpu_v, dir, (int16_t)cpu_maxv());
}

static void pong_draw(void) {
    uint8_t i, py = (p_pl_y >> 8) + 16, cy = (p_cpu_y >> 8) + 16;
    for (i = 0; i < 3; i++) {
        uint8_t t = (i == 0) ? T_PONG_T : (i == 1) ? T_PONG_M : T_PONG_B;
        set_sprite_tile(SPR_PONG_PL + i, t);
        set_sprite_tile(SPR_PONG_CPU + i, t);
        set_sprite_prop(SPR_PONG_PL + i, 0);
        set_sprite_prop(SPR_PONG_CPU + i, 0);
        move_sprite(SPR_PONG_PL + i,  P_PL_X + 8,  py + i * 8);
        move_sprite(SPR_PONG_CPU + i, P_CPU_X + 8, cy + i * 8);
    }
    if (state == ST_PONG_OVER) move_sprite(SPR_BALL, 0, 0);
    else move_sprite(SPR_BALL, (bx >> 8) + 8, (by >> 8) + 16);
    move_sprite(SPR_PU, 0, 0);
    move_sprite(SPR_BEAM, 0, 0);
    move_sprite(SPR_BEAM + 1, 0, 0);
}

static void pong_start_level(void) {
    p_goals_pl = p_goals_cpu = 0;
    p_pl_y = p_cpu_y = (uint16_t)((P_TOP + P_BOT + 1) / 2 - P_PAD_H / 2) << 8;
    p_pl_v = p_cpu_v = 0;
    pong_build();
    pong_center_ball();
    print_at(1, 5, "LEVEL");
    print_num(7, 5, level, (level > 9) ? 2 : 1);
    print_at(1, 13, "PRESS A");
    state = ST_PONG_SERVE;
}

static void pong_new_game(void) {
    level = start_level;
    pong_start_level();
}

static void pong_update(void) {
    int8_t dir = 0;
    uint8_t r;

    if (state == ST_PONG_OVER) {
        if (PRESSED(J_START)) menu_enter();
        return;
    }
    if (state == ST_PONG_CLEAR) {
        if (PRESSED(J_A) || PRESSED(J_START)) { level++; pong_start_level(); }
        return;
    }

    /* Pause */
    if (state == ST_PONG_PLAY && PRESSED(J_START)) {
        print_at(12, 7, "PAUSE");
        print_at(11, 9, "SELECT:");
        print_at(11, 10, "MENU");
        do { prev_keys = keys; wait_vbl_done(); keys = joypad(); }
        while (!PRESSED(J_START) && !PRESSED(J_SELECT));
        pong_clear_row(7); pong_clear_row(9); pong_clear_row(10);
        if (keys & J_SELECT) { menu_enter(); return; }
    }

    if (keys & J_UP) dir = -1; else if (keys & J_DOWN) dir = 1;
    pong_move_pad(&p_pl_y, &p_pl_v, dir, PAD_MAXV);
    pong_cpu_ai();

    if (state == ST_PONG_SERVE) {
        if (PRESSED(J_A)) {
            pong_clear_row(5); pong_clear_row(13);
            pong_serve(1);
            snd_paddle();
            state = ST_PONG_PLAY;
        }
        return;
    }
    if (state == ST_PONG_WAIT) {
        if (--p_wait == 0) state = ST_PONG_PLAY;
        return;
    }

    /* ST_PONG_PLAY: 2 Teilschritte */
    r = pong_step();
    if (!r) r = pong_step();
    if (r == 1) {                       /* Tor fuer die CPU */
        p_goals_cpu++;
        snd_lose();
        pong_hud();
        if (p_goals_cpu >= P_WIN_GOALS) {
            state = ST_PONG_OVER;
            print_at(2, 5, "GAME");
            print_at(13, 5, "OVER");
            print_at(1, 13, "PRESS");
            print_at(12, 13, "START");
            return;
        }
        pong_center_ball(); pong_serve(-1);       /* Aufschlag zum Spieler */
        p_wait = 60; state = ST_PONG_WAIT;
    } else if (r == 2) {                /* Tor fuer den Spieler */
        p_goals_pl++;
        pong_hud();
        if (p_goals_pl >= P_WIN_GOALS) {
            snd_win();
            state = ST_PONG_CLEAR;
            print_at(1, 5, "LEVEL");
            print_at(12, 5, "CLEAR");
            print_at(1, 13, "PRESS");
            print_at(12, 13, "A");
            return;
        }
        snd_powerup();
        pong_center_ball(); pong_serve(1);        /* Aufschlag zur CPU */
        p_wait = 60; state = ST_PONG_WAIT;
    }
}
