/* boost.h — Collectible Power-Up Boost System */
#ifndef BOOST_H_INCLUDED
#define BOOST_H_INCLUDED

typedef enum { BOOST_HEAL, BOOST_RAPID, BOOST_SHIELD, BOOST_DOUBLE } BoostType;

typedef struct { int x, y, active; BoostType type; } Boost;

#define MAX_BOOSTS      12
#define BOOST_W         3
#define BOOST_HT        3
#define HEAL_AMOUNT     20
#define RAPID_DUR       300
#define SHIELD_MAX_HITS 5
#define DOUBLE_DUR      300
#define BOOST_DROP_PCT  30  /* % chance to drop on enemy kill */
#define DMG_ENEMY       20
#define DMG_BULLET      15

void boost_spawn_at(Boost *b, int max, int x, int y, int *count);
void boost_update(Boost *b, int max, int tick, int play_x, int *count);
void boost_render(Boost *b, int max, int tick);
int  boost_check_collect(Boost *b, int px, int py, int pw, int ph);

#endif /* BOOST_H_INCLUDED */
