/* hud.h — Premium Retro Arcade HUD */
#ifndef HUD_H
#define HUD_H
#define NOTIF_DURATION 90
void hud_init(void);
void hud_notify(const char *msg, int color);
void hud_tick(void);
void hud_draw_top(int score,int level,int hp,int max_hp,
                  int gun_mode,int shield_hits,
                  int kills,int missed,int max_missed);
void hud_draw_bottom(int tick,int rapid_t,int spread_t,
                     int double_t,int shield_h,
                     int enemy_count,int hi_score);
void hud_draw_notification(int tick);
#endif
