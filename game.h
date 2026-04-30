/* game.h — Game State (100-HP, boost system, shield hits) */
#ifndef GAME_H
#define GAME_H
#include "screen.h"
#include "boost.h"

#define PLAY_X       1
#define PLAY_Y       2
#define PLAY_W       (SCREEN_W - 2)
#define PLAY_H       (SCREEN_H - 5)

#define PLAYER_W         4
#define PLAYER_H         3
#define PLAYER_SPEED     2
#define PLAYER_MAX_HP    100
#define PLAYER_FIRE_CD   4
#define PLAYER_START_X   (PLAY_X + 2)
#define PLAYER_MIN_X     (PLAY_X + 1)
#define PLAYER_MAX_X     (PLAY_X + PLAY_W / 3)
#define PLAYER_INV_TIME  30

#define BULLET_SPEED        2
#define MAX_BULLETS         48
#define ENEMY_BULLET_SPEED  2
#define MAX_ENEMY_BULLETS   48

#define MAX_ENEMIES         24
#define ENEMY_W             3
#define ENEMY_H             2
#define ENEMY_BASE_SPD_Q    2
#define ENEMY_SPD_INC_Q     1
#define BASE_SPAWN_CD       40
#define MIN_SPAWN_CD        14
#define ENEMY_FIRE_CD_BASE  90

#define BOSS_W           7
#define BOSS_H           3
#define BOSS_HP          10
#define BOSS_INTERVAL    5

#define MAX_OBSTACLES       40
#define TERRAIN_SPAWN_CD    12
#define ASTEROID_SPAWN_CD   120
#define OBS_WALL_TOP  0
#define OBS_WALL_BOT  1
#define OBS_ASTEROID  2

#define SCORE_PER_LEVEL  200
#define MAX_LEVEL        99
#define MAX_MISSED       10
#define MAX_EXPLOSIONS   24
#define EXPLOSION_TICKS  5
#define FRAME_DELAY_US   33333
#define MAX_SCORE        9999999

typedef enum { STATE_MENU, STATE_CONTROLS, STATE_PLAYING, STATE_GAMEOVER } GameMode;

typedef struct {
    int x, y, hp, fire_cd, invincible;
    int rapid_timer, spread_timer;
    int shield_hits;   /* absorbs N hits, 0=off */
    int double_timer;  /* double shot duration */
} Player;

typedef struct { int x, y, dy, active; } Bullet;
typedef struct { int x, y, active; } EnemyBullet;
typedef struct {
    int x, y, hp, speed_quarter, move_acc;
    int active, is_boss, color, fire_cd;
} Enemy;
typedef struct { int x, y, w, h, active, type; } Obstacle;
typedef struct { int x, y, timer, active; } Explosion;

typedef struct {
    Player       player;
    Bullet      *bullets;
    EnemyBullet *enemy_bullets;
    Enemy       *enemies;
    Obstacle    *obstacles;
    Explosion   *explosions;
    Boost       *boosts;
    int          bullet_count, enemy_bullet_count;
    int          enemy_count, obstacle_count, explosion_count, boost_count;
    int          score, high_score, level;
    int          spawn_cd, spawn_timer;
    int          terrain_timer, asteroid_timer;
    int          ships_destroyed, ships_missed;
    int          tick;
    GameMode     mode;
    int          running;
} GameState;

void game_run(void);
#endif
