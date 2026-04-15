/* ============================================================
 *  game.c — Space Shooter (simple menu + ship movement + shots)
 * ============================================================ */
#include "screen.h"
#include "game.h"
#include "keyboard.h"
#include "memory.h"
#include "math.h"
#include "string.h"
#include <unistd.h>
#include <time.h>

static void ns(int v, char *b) { int_to_str(v, b, 16); }

static void state_menu(GameState *gs);
static void state_controls(GameState *gs);
static void state_playing(GameState *gs);
static void init_game(GameState *gs);
static void free_ent(GameState *gs);
static void process_input(GameState *gs);
static void update(GameState *gs);
static void render(GameState *gs);
static void fire_bullet(GameState *gs);

void game_run(void)
{
    GameState gs;
    gs.running = 1;
    gs.mode = STATE_MENU;
    gs.high_score = 0;
    gs.bullets = NULL;
    custom_srand((unsigned int)time(NULL));
    while (gs.running) {
        switch (gs.mode) {
        case STATE_MENU:     state_menu(&gs);     break;
        case STATE_CONTROLS: state_controls(&gs); break;
        case STATE_PLAYING:  state_playing(&gs);  break;
        }
    }
}

/* =============== MENU =============== */
static void state_menu(GameState *gs)
{
    screen_invalidate();
    screen_clear_buf();
    int ty = 2, tx = 15;
    screen_draw_string(ty,   tx, " ___  ___   _    ___ ___",     COL_BRIGHT_CYAN, COL_RESET);
    screen_draw_string(ty+1, tx, "/ __||  _ \\/ \\  / __| __|", COL_CYAN, COL_RESET);
    screen_draw_string(ty+2, tx, "\\__ \\| |_)/ _ \\| (__| _|", COL_BRIGHT_BLUE, COL_RESET);
    screen_draw_string(ty+3, tx, "|___/|___/_/ \\_\\___|___|",   COL_BRIGHT_MAGENTA, COL_RESET);
    int ty2 = ty + 5, tx2 = 10;
    screen_draw_string(ty2,   tx2, " ___ _  _  ___   ___ _____ ___ ___",   COL_BRIGHT_RED, COL_RESET);
    screen_draw_string(ty2+1, tx2, "/ __| || |/ _ \\ / _ \\_   _| __| _ \\", COL_ORANGE, COL_RESET);
    screen_draw_string(ty2+2, tx2, "\\__ \\ __ | (_) | (_) || | | _||   /", COL_YELLOW, COL_RESET);
    screen_draw_string(ty2+3, tx2, "|___/|_||_|\\___/ \\___/ |_| |___|_|_\\", COL_BRIGHT_YELLOW, COL_RESET);
    int my = ty2 + 5;
    screen_draw_string(my,   24, ">>>  [1]  START GAME  <<<", COL_BRIGHT_GREEN, COL_RESET);
    screen_draw_string(my+1, 24, "     [2]  CONTROLS      ", COL_BRIGHT_YELLOW, COL_RESET);
    screen_draw_string(my+2, 24, "     [3]  QUIT          ", COL_BRIGHT_RED, COL_RESET);
    {
        char h[48] = "HIGH SCORE: ";
        char n[16];
        ns(gs->high_score, n);
        str_concat(h, n);
        screen_draw_string(my + 4, 24, h, COL_BRIGHT_MAGENTA, COL_RESET);
    }
    screen_draw_box(0, 0, SCREEN_W, SCREEN_H, COL_BRIGHT_BLUE, COL_RESET);
    screen_flush();
    while (1) {
        if (kb_hit()) {
            int k = kb_read_key();
            if (k == '1') {
                gs->mode = STATE_PLAYING;
                return;
            }
            if (k == '2') {
                gs->mode = STATE_CONTROLS;
                return;
            }
            if (k == '3') {
                gs->running = 0;
                return;
            }
        }
        usleep(50000);
    }
}

/* =============== CONTROLS =============== */
static void state_controls(GameState *gs)
{
    screen_invalidate();
    screen_clear_buf();
    screen_draw_box(0, 0, SCREEN_W, SCREEN_H, COL_BRIGHT_BLUE, COL_RESET);
    screen_draw_string(1, 28, "=== CONTROLS ===", COL_BRIGHT_CYAN, COL_RESET);
    int y = 3;
    screen_draw_string(y,   8, "W / UP      Move up",       COL_BRIGHT_GREEN, COL_RESET);
    screen_draw_string(y+1, 8, "S / DOWN    Move down",     COL_BRIGHT_GREEN, COL_RESET);
    screen_draw_string(y+2, 8, "A / LEFT    Move backward", COL_BRIGHT_GREEN, COL_RESET);
    screen_draw_string(y+3, 8, "D / RIGHT   Move forward",  COL_BRIGHT_GREEN, COL_RESET);
    screen_draw_string(y+4, 8, "SPACE       Fire (single shot)", COL_BRIGHT_YELLOW, COL_RESET);
    screen_draw_string(y+5, 8, "Q           Return to menu", COL_BRIGHT_RED, COL_RESET);
    screen_draw_string(SCREEN_H - 2, 20, "Press any key to return...", COL_WHITE, COL_RESET);
    screen_flush();
    while (1) {
        if (kb_hit()) {
            kb_read_key();
            gs->mode = STATE_MENU;
            return;
        }
        usleep(50000);
    }
}

/* =============== INIT / FREE =============== */
static void init_game(GameState *gs)
{
    int i;
    gs->bullets = (Bullet *)mem_alloc((int)sizeof(Bullet) * MAX_BULLETS);
    for (i = 0; i < MAX_BULLETS; i++)
        gs->bullets[i].active = 0;

    gs->player.x = PLAYER_START_X;
    gs->player.y = PLAY_Y + PLAY_H / 2 - PLAYER_H / 2;
    gs->player.fire_cd = 0;

    gs->score = 0;
    gs->tick = 0;
    screen_invalidate();
}

static void free_ent(GameState *gs)
{
    if (gs->bullets) {
        mem_free(gs->bullets);
        gs->bullets = NULL;
    }
}

/* =============== GAME LOOP =============== */
static void state_playing(GameState *gs)
{
    init_game(gs);
    while (gs->mode == STATE_PLAYING && gs->running) {
        process_input(gs);
        update(gs);
        screen_clear_buf();
        render(gs);
        screen_flush();
        usleep(FRAME_DELAY_US);
    }
    if (gs->score > gs->high_score)
        gs->high_score = gs->score;
    free_ent(gs);
}

static void process_input(GameState *gs)
{
    int mu = 0, md = 0, ml = 0, mr = 0, fire = 0;
    while (kb_hit()) {
        int k = kb_read_key();
        if (k == 27) {
            if (kb_hit()) {
                int k2 = kb_read_key();
                if (k2 == '[' && kb_hit()) {
                    int k3 = kb_read_key();
                    if (k3 == 'A') k = 'w';
                    else if (k3 == 'B') k = 's';
                    else if (k3 == 'C') k = 'd';
                    else if (k3 == 'D') k = 'a';
                    else
                        continue;
                } else
                    continue;
            } else
                continue;
        }
        if (k == 'w' || k == 'W') mu = 1;
        if (k == 's' || k == 'S') md = 1;
        if (k == 'a' || k == 'A') ml = 1;
        if (k == 'd' || k == 'D') mr = 1;
        if (k == ' ') fire = 1;
        if (k == 'q' || k == 'Q') {
            gs->mode = STATE_MENU;
            return;
        }
    }

    {
        int s;
        if (mu)
            for (s = 0; s < PLAYER_SPEED; s++) {
                int ny = gs->player.y - 1;
                if (ny < PLAY_Y)
                    break;
                gs->player.y = ny;
            }
        if (md)
            for (s = 0; s < PLAYER_SPEED; s++) {
                int ny = gs->player.y + 1;
                if (ny > PLAY_Y + PLAY_H - PLAYER_H)
                    break;
                gs->player.y = ny;
            }
        if (ml)
            for (s = 0; s < PLAYER_SPEED; s++) {
                int nx = gs->player.x - 1;
                if (nx < PLAYER_MIN_X)
                    break;
                gs->player.x = nx;
            }
        if (mr)
            for (s = 0; s < PLAYER_SPEED; s++) {
                int nx = gs->player.x + 1;
                if (nx > PLAYER_MAX_X)
                    break;
                gs->player.x = nx;
            }
    }
    if (fire)
        fire_bullet(gs);
}

static void fire_bullet(GameState *gs)
{
    int i;
    if (gs->player.fire_cd > 0)
        return;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!gs->bullets[i].active) {
            gs->bullets[i].active = 1;
            gs->bullets[i].x = gs->player.x + PLAYER_W;
            gs->bullets[i].y = gs->player.y + 1;
            gs->player.fire_cd = PLAYER_FIRE_CD;
            return;
        }
    }
}

static void update(GameState *gs)
{
    int i;
    gs->tick++;
    if (gs->player.fire_cd > 0)
        gs->player.fire_cd--;

    if (gs->tick > 0 && custom_mod(gs->tick, SCORE_TICKS) == 0)
        gs->score++;

    for (i = 0; i < MAX_BULLETS; i++) {
        if (!gs->bullets[i].active)
            continue;
        gs->bullets[i].x += BULLET_SPEED;
        if (gs->bullets[i].x >= PLAY_X + PLAY_W)
            gs->bullets[i].active = 0;
    }
}

static void render(GameState *gs)
{
    int i;
    char line[96];
    char num[16];

    /* status line */
    str_copy(line, " SCORE: ");
    ns(gs->score, num);
    str_concat(line, num);
    str_concat(line, "   HIGH: ");
    ns(gs->high_score, num);
    str_concat(line, num);
    str_concat(line, "   [Q] MENU");
    screen_draw_string(0, 0, line, COL_BRIGHT_CYAN, COL_RESET);

    /* starfield */
    for (i = 0; i < 5; i++) {
        int sr = custom_rand_range(PLAY_Y, PLAY_Y + PLAY_H - 1);
        int sc = custom_rand_range(PLAY_X, PLAY_X + PLAY_W - 1);
        screen_put(sr, sc, '.', (custom_rand() & 1) ? COL_WHITE : COL_BLUE, COL_RESET);
    }

    screen_draw_box(PLAY_Y - 1, PLAY_X - 1, PLAY_W + 2, PLAY_H + 2, COL_BRIGHT_BLUE, COL_RESET);

    for (i = 0; i < MAX_BULLETS; i++) {
        if (!gs->bullets[i].active)
            continue;
        {
            int cl = (gs->tick & 1) ? COL_BRIGHT_YELLOW : COL_BRIGHT_CYAN;
            screen_put(gs->bullets[i].y, gs->bullets[i].x, '=', cl, COL_RESET);
        }
    }

    {
        int px = gs->player.x, py = gs->player.y;
        int sc = COL_CYAN, sg = COL_NEON_GREEN;
        screen_put(py,   px+1, '/', sc, COL_RESET);
        screen_put(py,   px+2, '=', sc, COL_RESET);
        screen_put(py,   px+3, '>', COL_BRIGHT_WHITE, COL_RESET);
        screen_put(py+1, px,   '>', sg, COL_RESET);
        screen_put(py+1, px+1, '=', sg, COL_RESET);
        screen_put(py+1, px+2, '=', sg, COL_RESET);
        screen_put(py+1, px+3, '|', COL_BRIGHT_WHITE, COL_RESET);
        screen_put(py+2, px+1, '\\', sc, COL_RESET);
        screen_put(py+2, px+2, '=', sc, COL_RESET);
        screen_put(py+2, px+3, '>', COL_BRIGHT_WHITE, COL_RESET);
    }

    /* help row */
    screen_draw_string(SCREEN_H - 2, PLAY_X, "WASD move   SPACE fire", COL_WHITE, COL_RESET);
}
