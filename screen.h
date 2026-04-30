/* ============================================================
 *  screen.h — ANSI Terminal Rendering with Double Buffer
 * ============================================================ */
 #ifndef SCREEN_H_INCLUDED
 #define SCREEN_H_INCLUDED
 
 /* ---- colours (ANSI 256) ---- */
 #define COL_RESET       0
 #define COL_BLACK       1
 #define COL_RED         2
 #define COL_GREEN       3
 #define COL_YELLOW      4
 #define COL_BLUE        5
 #define COL_MAGENTA     6
 #define COL_CYAN        7
 #define COL_WHITE       8
 #define COL_BRIGHT_RED      9
 #define COL_BRIGHT_GREEN   10
 #define COL_BRIGHT_YELLOW  11
 #define COL_BRIGHT_BLUE    12
 #define COL_BRIGHT_MAGENTA 13
 #define COL_BRIGHT_CYAN    14
 #define COL_BRIGHT_WHITE   15
 #define COL_ORANGE         16
 #define COL_PINK           17
 #define COL_NEON_GREEN     18
 
 /* ---- buffer dimensions (play area) ---- */
 #define SCREEN_W  80
 #define SCREEN_H  25
 
 /* ---- public API ---- */
 void screen_init(void);
 void screen_shutdown(void);
 
 void screen_clear_buf(void);       /* fill back-buffer with spaces */
 void screen_flush(void);           /* diff-write to terminal       */
 
 void screen_put(int r, int c, char ch, int fg, int bg);
 void screen_draw_string(int r, int c, const char *s, int fg, int bg);
 void screen_draw_box(int r, int c, int w, int h, int fg, int bg);
 
 void screen_hide_cursor(void);
 void screen_show_cursor(void);
 void screen_move_cursor(int r, int c);
 
 void screen_invalidate(void);   /* force full redraw on next flush */
 
 /* Call from idle/input loops that do not redraw every frame (menu, paused, …)
  * so a terminal resize triggers a full repaint from the existing back buffer. */
 void screen_ping_resize(void);
 
 /* helpers */
 int  screen_width(void);
 int  screen_height(void);
 
 #endif /* SCREEN_H_INCLUDED */
 