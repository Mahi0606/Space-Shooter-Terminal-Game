/* ============================================================
 *  keyboard.h — Non-blocking Terminal Input
 * ============================================================ */
#ifndef KEYBOARD_H
#define KEYBOARD_H

void  kb_init(void);           /* enter raw mode   */
void  kb_restore(void);        /* restore cooked    */

int   kb_hit(void);            /* non-blocking: 1 if key ready */
int   kb_read_key(void);       /* read single char (blocking)  */

/* Blocking line input — used for menus.
   Reads up to (max-1) chars until Enter. Returns length. */
int   kb_read_line(char *buf, int max);

#endif /* KEYBOARD_H */
