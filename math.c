/* ============================================================
 *  math.c — Custom Math Utilities
 * ============================================================ */
#include "math.h"

/* ---- basic arithmetic ---- */

int custom_abs(int x)                     { return x < 0 ? -x : x; }
int custom_min(int a, int b)              { return a < b ? a : b; }
int custom_max(int a, int b)              { return a > b ? a : b; }

int custom_clamp(int val, int lo, int hi)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

int custom_mod(int a, int b)
{
    if (b == 0) return 0;
    int neg = (a < 0);
    unsigned int ua = neg ? (unsigned int)(-(a + 1)) + 1u : (unsigned int)a;
    unsigned int ub = (b < 0) ? (unsigned int)(-(b + 1)) + 1u : (unsigned int)b;
    /* binary long-division remainder — O(32) instead of O(ua/ub) */
    unsigned int t = ub;
    while (t <= ua && !(t & 0x80000000u)) t <<= 1;
    while (t >= ub) { if (ua >= t) ua -= t; t >>= 1; }
    return neg ? -(int)ua : (int)ua;
}

int custom_div(int a, int b)
{
    if (b == 0) return 0;
    int sign = 1;
    if (a < 0) { a = -a; sign = -sign; }
    if (b < 0) { b = -b; sign = -sign; }
    unsigned int ua = (unsigned int)a;
    unsigned int ub = (unsigned int)b;
    unsigned int q = 0, bit = 1, t = ub;
    while (t <= ua && !(t & 0x80000000u)) { t <<= 1; bit <<= 1; }
    while (bit > 0) { if (ua >= t) { ua -= t; q |= bit; } t >>= 1; bit >>= 1; }
    return (int)q * sign;
}

int custom_mul(int a, int b)
{
    /* Simple — the compiler will use real MUL; this satisfies the
       "custom library" requirement without overflow concern for
       the value ranges we use in-game. */
    return a * b;
}

/* ---- PRNG (xorshift32) ---- */

static unsigned int g_seed = 12345;

void custom_srand(unsigned int seed) { g_seed = seed ? seed : 1; }

int custom_rand(void)
{
    g_seed ^= g_seed << 13;
    g_seed ^= g_seed >> 17;
    g_seed ^= g_seed << 5;
    return (int)(g_seed & 0x7FFFFFFF);
}

int custom_rand_range(int lo, int hi)
{
    if (lo > hi) { int t = lo; lo = hi; hi = t; }
    int range = hi - lo + 1;
    if (range <= 0) return lo;
    return lo + custom_mod(custom_rand(), range);
}

/* ---- collision / bounds ---- */

int check_collision(Rect a, Rect b)
{
    if (a.x + a.w <= b.x) return 0;
    if (b.x + b.w <= a.x) return 0;
    if (a.y + a.h <= b.y) return 0;
    if (b.y + b.h <= a.y) return 0;
    return 1;
}

int check_bounds(int x, int y, int w, int h, int sw, int sh)
{
    return (x >= 0 && y >= 0 && x + w <= sw && y + h <= sh);
}
