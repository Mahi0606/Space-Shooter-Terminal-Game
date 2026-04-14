/* ============================================================
 *  main.c — Entry Point for Space Shooter
 *  -----------------------------------------------------------
 *  Init order:  memory → keyboard → screen → game
 *  Teardown:    screen → keyboard → memory
 * ============================================================ */
#include "memory.h"
#include "keyboard.h"
#include "screen.h"
#include "game.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    /* ---- initialise subsystems ---- */
    mem_init();
    kb_init();
    screen_init();

    /* ---- run game ---- */
    game_run();

    /* ---- teardown ---- */
    screen_shutdown();
    kb_restore();

    /* Memory diagnostics before shutdown */
    if (mem_used() > 0) {
        fprintf(stderr, "[main] WARNING: memory leak detected (%d bytes)\n",
                mem_used());
    }

    mem_shutdown();

    printf("Thanks for playing Space Shooter!\n");
    return 0;
}
