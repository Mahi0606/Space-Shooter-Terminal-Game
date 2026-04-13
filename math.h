/* ============================================================
 *  math.h — Custom Math Utilities
 * ============================================================ */
#ifndef CUSTOM_MATH_H
#define CUSTOM_MATH_H

/* basic arithmetic */
int  custom_abs(int x);
int  custom_min(int a, int b);
int  custom_max(int a, int b);
int  custom_clamp(int val, int lo, int hi);
int  custom_mod(int a, int b);
int  custom_div(int a, int b);
int  custom_mul(int a, int b);

/* PRNG */
void custom_srand(unsigned int seed);
int  custom_rand(void);
int  custom_rand_range(int lo, int hi);   /* inclusive */

/* collision / bounds */
typedef struct { int x, y, w, h; } Rect;

int  check_collision(Rect a, Rect b);
int  check_bounds(int x, int y, int w, int h, int sw, int sh);

#endif /* CUSTOM_MATH_H */
