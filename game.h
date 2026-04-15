/* game.h — minimal Space Shooter (menu + simple play) */
#ifndef GAME_H
#define GAME_H
#include "screen.h"

#define PLAY_X       1
#define PLAY_Y       2
#define PLAY_W       (SCREEN_W - 2)
#define PLAY_H       (SCREEN_H - 5)

#define PLAYER_W         4
#define PLAYER_H         3
#define PLAYER_SPEED     2
#define PLAYER_FIRE_CD   8
#define PLAYER_START_X   (PLAY_X + 2)
#define PLAYER_MIN_X     (PLAY_X + 1)
#define PLAYER_MAX_X     (PLAY_X + PLAY_W / 3)

#define BULLET_SPEED  2
#define MAX_BULLETS     24

#define FRAME_DELAY_US  33333
#define SCORE_TICKS     30   /* ~1s at FRAME_DELAY_US for score tick */

typedef enum { STATE_MENU, STATE_CONTROLS, STATE_PLAYING } GameMode;

typedef struct {
    int x, y, fire_cd;
} Player;

typedef struct { int x, y, active; } Bullet;

typedef struct {
    Player   player;
    Bullet  *bullets;
    int      score, high_score;
    int      tick;
    GameMode mode;
    int      running;
} GameState;

void game_run(void);
#endif
