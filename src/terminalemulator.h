#ifndef TERM_EMULATOR_H
#define TERM_EMULATOR_H

/*========================================================
 * Terminal control codes
 *========================================================
 */

#define PUTC          0x00 /* Place character at cursor position and advance cursor */
#define PUTCCLIP      0x01 /* Place character at cursor position and advance cursor without wraparound */
#define PUTCFIX       0x02 /* Place character at cursor position without advancing cursor */
#define ENDMSG        0x03 /* Force termination of the current message */
#define MVLEFT        0x04 /* Move cursor left n times with wraparound */
#define MVLEFTCLIP    0x05 /* Move cursor left n times without wraparound */
#define MVRIGHT       0x06 /* Move cursor right n times with wraparound */
#define MVRIGHTCLIP   0x07 /* Move cursor right n times without wraparound */
#define MVUP          0x08 /* Move cursor up n times with wraparound */
#define MVUPCLIP      0x09 /* Move cursor up n times without wraparound */
#define MVDOWN        0x0A /* Move cursor down n times with wraparound */
#define MVDOWNCLIP    0x0B /* Move cursor down n times without wraparound */
#define SETFG         0x0C /* Set foreground color */
#define SETBG         0x0D /* Set background color */
#define SETROW        0x0E /* Set cursor row */
#define SETCOL        0x0F /* Set cursor column */
#define CURSORVISIBLE 0x10 /* Set cursor to be visible or invisible */
#define SCROLL        0x11 /* Scroll screen by n lines */
#define CLS           0x12 /* Clear screen */
#define RPT           0x7F /* Enable repeat message mode (this message is handled in the previous comm state) */


#endif /* TERM_EMULATOR_H */
