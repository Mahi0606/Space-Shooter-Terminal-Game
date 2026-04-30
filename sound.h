/* sound.h — Non-blocking Sound Effects via macOS afplay */
#ifndef SOUND_H
#define SOUND_H

typedef enum {
    SND_FIRE,          /* player shoots           */
    SND_EXPLODE,       /* enemy destroyed          */
    SND_HIT,           /* player takes damage      */
    SND_BOOST,         /* boost collected          */
    SND_SHIELD_BREAK,  /* shield depleted          */
    SND_GAMEOVER,      /* game over                */
    SND_BOSS,          /* boss appears             */
    SND_LEVELUP,       /* level up                 */
    SND_COUNT
} SoundEvent;

void sound_init(void);
void sound_play(SoundEvent event);
void sound_tick(void);   /* call each frame for cooldowns */

#endif
