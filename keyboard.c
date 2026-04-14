/* ============================================================
 *  keyboard.c — Non-blocking Terminal Input via termios
 * ============================================================ */
#include "keyboard.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

static struct termios g_orig;  /* saved terminal state */
static int            g_raw = 0;

void kb_init(void)
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &g_orig);
    raw = g_orig;
    raw.c_lflag &= (unsigned int)~(ICANON | ECHO);  /* disable line-mode & echo */
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    g_raw = 1;
}

void kb_restore(void)
{
    if (g_raw) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_orig);
        g_raw = 0;
    }
}

int kb_hit(void)
{
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

int kb_read_key(void)
{
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) == 1)
        return (int)c;
    return -1;
}

int kb_read_line(char *buf, int max)
{
    /* Temporarily switch to a semi-raw mode that still echoes */
    struct termios tmp;
    tcgetattr(STDIN_FILENO, &tmp);
    tmp.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tmp);

    int i = 0;
    while (i < max - 1) {
        unsigned char c;
        if (read(STDIN_FILENO, &c, 1) != 1) break;
        if (c == '\n' || c == '\r') break;
        buf[i++] = (char)c;
    }
    buf[i] = '\0';

    /* Restore raw mode */
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= (unsigned int)~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    return i;
}
