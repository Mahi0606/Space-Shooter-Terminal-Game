/* ============================================================
 *  screen.c — ANSI Terminal Rendering with Double Buffer
 *  -----------------------------------------------------------
 *  Performance: all output batched into a single write() call.
 * ============================================================ */
#include "screen.h"
#include <stdio.h>
#include <unistd.h>

/* ---- internal cell ---- */
typedef struct { char ch; int fg; int bg; } Cell;

static Cell g_front[SCREEN_H][SCREEN_W];
static Cell g_back [SCREEN_H][SCREEN_W];

/* ---- output buffer (batches all ANSI to one write) ---- */
#define OB_CAP (1 << 17)   /* 128 KB */
static char g_ob[OB_CAP];
static int  g_ob_len;

static void ob_reset(void) { g_ob_len = 0; }
static void ob_put(char c) { if (g_ob_len < OB_CAP - 1) g_ob[g_ob_len++] = c; }
static void ob_str(const char *s) { while (*s) ob_put(*s++); }
static void ob_int(int n) {
    if (n == 0) { ob_put('0'); return; }
    char tmp[12]; int len = 0;
    while (n > 0) { tmp[len++] = (char)('0' + n % 10); n /= 10; }
    while (len > 0) ob_put(tmp[--len]);
}
static void ob_move(int r, int c) {
    ob_str("\033["); ob_int(r+1); ob_put(';'); ob_int(c+1); ob_put('H');
}
static void ob_do_flush(void) {
    if (g_ob_len > 0) { write(STDOUT_FILENO, g_ob, g_ob_len); ob_reset(); }
}

/* ---- colour helpers (write to output buffer) ---- */
static void ob_fg(int col) {
    switch (col) {
        case COL_RESET:          ob_str("\033[39m"); break;
        case COL_BLACK:          ob_str("\033[30m"); break;
        case COL_RED:            ob_str("\033[31m"); break;
        case COL_GREEN:          ob_str("\033[32m"); break;
        case COL_YELLOW:         ob_str("\033[33m"); break;
        case COL_BLUE:           ob_str("\033[34m"); break;
        case COL_MAGENTA:        ob_str("\033[35m"); break;
        case COL_CYAN:           ob_str("\033[36m"); break;
        case COL_WHITE:          ob_str("\033[37m"); break;
        case COL_BRIGHT_RED:     ob_str("\033[91m"); break;
        case COL_BRIGHT_GREEN:   ob_str("\033[92m"); break;
        case COL_BRIGHT_YELLOW:  ob_str("\033[93m"); break;
        case COL_BRIGHT_BLUE:    ob_str("\033[94m"); break;
        case COL_BRIGHT_MAGENTA: ob_str("\033[95m"); break;
        case COL_BRIGHT_CYAN:    ob_str("\033[96m"); break;
        case COL_BRIGHT_WHITE:   ob_str("\033[97m"); break;
        case COL_ORANGE:         ob_str("\033[38;5;208m"); break;
        case COL_PINK:           ob_str("\033[38;5;205m"); break;
        case COL_NEON_GREEN:     ob_str("\033[38;5;46m");  break;
        default:                 ob_str("\033[39m"); break;
    }
}
static void ob_bg(int col) {
    switch (col) {
        case COL_RESET: ob_str("\033[49m"); break;
        case COL_BLACK: ob_str("\033[40m"); break;
        case COL_RED:   ob_str("\033[41m"); break;
        case COL_GREEN: ob_str("\033[42m"); break;
        case COL_YELLOW:ob_str("\033[43m"); break;
        case COL_BLUE:  ob_str("\033[44m"); break;
        case COL_MAGENTA:ob_str("\033[45m");break;
        case COL_CYAN:  ob_str("\033[46m"); break;
        case COL_WHITE: ob_str("\033[47m"); break;
        default:        ob_str("\033[49m"); break;
    }
}

/* ---- public API ---- */

void screen_init(void)
{
    int r, c;
    for (r = 0; r < SCREEN_H; r++)
        for (c = 0; c < SCREEN_W; c++) {
            g_front[r][c].ch = '\0';   /* force full draw on first flush */
            g_front[r][c].fg = -1;
            g_front[r][c].bg = -1;
            g_back[r][c].ch  = ' ';
            g_back[r][c].fg  = COL_RESET;
            g_back[r][c].bg  = COL_RESET;
        }
    ob_reset();
    ob_str("\033[2J");        /* clear terminal */
    ob_str("\033[?25l");      /* hide cursor    */
    ob_do_flush();
}

void screen_shutdown(void)
{
    ob_reset();
    ob_str("\033[0m\033[?25h\033[2J\033[H");
    ob_do_flush();
}

void screen_clear_buf(void)
{
    int r, c;
    for (r = 0; r < SCREEN_H; r++)
        for (c = 0; c < SCREEN_W; c++) {
            g_back[r][c].ch = ' ';
            g_back[r][c].fg = COL_RESET;
            g_back[r][c].bg = COL_RESET;
        }
}

void screen_invalidate(void)
{
    int r, c;
    for (r = 0; r < SCREEN_H; r++)
        for (c = 0; c < SCREEN_W; c++)
            g_front[r][c].ch = '\0';   /* force mismatch on every cell */
}

void screen_flush(void)
{
    int r, c;
    int last_fg = -1, last_bg = -1;
    ob_reset();

    for (r = 0; r < SCREEN_H; r++) {
        for (c = 0; c < SCREEN_W; c++) {
            Cell *f = &g_front[r][c];
            Cell *b = &g_back[r][c];
            if (f->ch == b->ch && f->fg == b->fg && f->bg == b->bg)
                continue;
            ob_move(r, c);
            if (b->fg != last_fg) { ob_fg(b->fg); last_fg = b->fg; }
            if (b->bg != last_bg) { ob_bg(b->bg); last_bg = b->bg; }
            ob_put(b->ch);
            *f = *b;
        }
    }
    ob_move(SCREEN_H, 0);
    ob_do_flush();
}

void screen_put(int r, int c, char ch, int fg, int bg)
{
    if (r < 0 || r >= SCREEN_H || c < 0 || c >= SCREEN_W) return;
    g_back[r][c].ch = ch;
    g_back[r][c].fg = fg;
    g_back[r][c].bg = bg;
}

void screen_draw_string(int r, int c, const char *s, int fg, int bg)
{
    while (*s) { screen_put(r, c, *s, fg, bg); s++; c++; }
}

void screen_draw_box(int r, int c, int w, int h, int fg, int bg)
{
    int i;
    screen_put(r, c, '+', fg, bg);
    for (i = 1; i < w - 1; i++) screen_put(r, c + i, '-', fg, bg);
    screen_put(r, c + w - 1, '+', fg, bg);
    for (i = 1; i < h - 1; i++) {
        screen_put(r + i, c,         '|', fg, bg);
        screen_put(r + i, c + w - 1, '|', fg, bg);
    }
    screen_put(r + h - 1, c, '+', fg, bg);
    for (i = 1; i < w - 1; i++) screen_put(r + h - 1, c + i, '-', fg, bg);
    screen_put(r + h - 1, c + w - 1, '+', fg, bg);
}

void screen_hide_cursor(void) { ob_reset(); ob_str("\033[?25l"); ob_do_flush(); }
void screen_show_cursor(void) { ob_reset(); ob_str("\033[?25h"); ob_do_flush(); }
void screen_move_cursor(int r, int c) {
    ob_reset(); ob_move(r, c); ob_do_flush();
}
int screen_width(void)  { return SCREEN_W; }
int screen_height(void) { return SCREEN_H; }
