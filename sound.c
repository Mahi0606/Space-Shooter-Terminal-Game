/* sound.c — Non-blocking sound effects using macOS system sounds
 *
 * Uses fork()+exec("afplay") to play sounds without blocking the game loop.
 * Each event has a frame-based cooldown to prevent audio spam.
 * Zombie processes are auto-reaped via SIGCHLD = SIG_IGN.
 */
 #include "sound.h"
 #include <unistd.h>
 #include <signal.h>
 
 /* ---- sound file paths (macOS built-in) ---- */
 static const char *g_files[SND_COUNT] = {
     "/System/Library/Sounds/Bottle.aiff",     /* fire: short pew        */
     "/System/Library/Sounds/Pop.aiff",        /* explode: satisfying pop*/
     "/System/Library/Sounds/Basso.aiff",      /* hit: deep impact       */
     "/System/Library/Sounds/Hero.aiff",       /* boost: triumphant      */
     "/System/Library/Sounds/Funk.aiff",       /* shield break: alert    */
     "/System/Library/Sounds/Sosumi.aiff",     /* game over: iconic      */
     "/System/Library/Sounds/Submarine.aiff",  /* boss: dramatic rumble  */
     "/System/Library/Sounds/Glass.aiff",      /* level up: bright chime */
 };
 
 /* ---- cooldown values (in frames, ~30fps) ---- */
 static const int g_cd_max[SND_COUNT] = {
     5,   /* fire: ~6/sec max        */
     4,   /* explode: ~7/sec max     */
     15,  /* hit: once per 0.5s      */
     20,  /* boost: once per 0.6s    */
     30,  /* shield break: once/sec  */
     90,  /* game over: once/3s      */
     60,  /* boss: once/2s           */
     30,  /* level up: once/sec      */
 };
 
 static int g_cd[SND_COUNT];
 
 void sound_init(void)
 {
     /* auto-reap child processes so we don't accumulate zombies */
     signal(SIGCHLD, SIG_IGN);
     int i;
     for (i = 0; i < SND_COUNT; i++) g_cd[i] = 0;
 }
 
 void sound_play(SoundEvent event)
 {
     if (event < 0 || event >= SND_COUNT) return;
     if (g_cd[event] > 0) return;           /* still on cooldown */
 
     g_cd[event] = g_cd_max[event];
 
     pid_t pid = fork();
     if (pid == 0) {
         /* child: silence stdout/stderr, play sound, exit */
         close(STDOUT_FILENO);
         close(STDERR_FILENO);
         execlp("afplay", "afplay", g_files[event], (char *)0);
         _exit(1); /* exec failed */
     }
     /* parent returns immediately — non-blocking */
 }
 
 void sound_tick(void)
 {
     int i;
     for (i = 0; i < SND_COUNT; i++)
         if (g_cd[i] > 0) g_cd[i]--;
 }
 