/* ============================================================
 *  string.h — Custom String Utilities
 * ============================================================ */
#ifndef CUSTOM_STRING_H
#define CUSTOM_STRING_H

int   str_len(const char *s);
void  str_copy(char *dst, const char *src);
int   str_compare(const char *a, const char *b);
void  str_concat(char *dst, const char *src);
void  int_to_str(int val, char *buf, int buf_size);
void  str_pad_center(const char *src, char *dst, int width);

#endif /* CUSTOM_STRING_H */
