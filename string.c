/* ============================================================
 *  string.c — Custom String Utilities
 * ============================================================ */
#include "string.h"

int str_len(const char *s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

void str_copy(char *dst, const char *src)
{
    while (*src) *dst++ = *src++;
    *dst = '\0';
}

int str_compare(const char *a, const char *b)   
{
    while (*a && *b && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

void str_concat(char *dst, const char *src)// display score+number;
{
    while (*dst) dst++;
    while (*src) *dst++ = *src++;
    *dst = '\0';
}

void int_to_str(int val, char *buf, int buf_size)
{
    if (buf_size <= 0) return;

    int neg = 0;
    unsigned int uv;

    if (val < 0) {
        neg = 1;
        uv  = (unsigned int)(-(val + 1)) + 1u;
    } else {
        uv = (unsigned int)val;
    }

    /* build reversed */
    char tmp[20];
    int len = 0;
    if (uv == 0) {
        tmp[len++] = '0';
    } else {
        while (uv > 0) {
            tmp[len++] = '0' + (char)(uv % 10);
            uv /= 10;
        }
    }
    if (neg) tmp[len++] = '-';

    /* clamp to buffer */
    if (len >= buf_size) len = buf_size - 1;

    /* reverse into dst */
    int i;
    for (i = 0; i < len; i++) buf[i] = tmp[len - 1 - i];
    buf[len] = '\0';
}

void str_pad_center(const char *src, char *dst, int width)
{
    int slen = str_len(src);
    if (slen >= width) {
        str_copy(dst, src);
        return;
    }
    int pad = (width - slen) / 2;
    int i;
    for (i = 0; i < pad; i++) dst[i] = ' ';
    for (i = 0; i < slen; i++) dst[pad + i] = src[i];
    for (i = pad + slen; i < width; i++) dst[i] = ' ';
    dst[width] = '\0';
}
