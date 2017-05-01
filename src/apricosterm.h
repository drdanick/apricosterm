#ifndef APRTERM_H
#define APRTERM_H

#include <SDL2/SDL.h>

#ifndef VERSION
#define VERSION "UNKNOWN"
#endif /* VERSION */

#ifndef RESOURCE_DIR
#define RESOURCE_DIR "."
#endif /* RESOURCE_DIR */

/* Font info */
#define FONT_FILE   "vga8x16.png"
#define CHAR_WIDTH  8
#define CHAR_HEIGHT 16
#define FONT_COLS   32
#define FONT_LINES  8

/* Screen info */
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_ROWS   (SCREEN_HEIGHT / CHAR_HEIGHT)
#define SCREEN_COLS   (SCREEN_WIDTH / CHAR_WIDTH)

/* Terminal settings */
#define BACKGROUND_COLOR EGA_BLACK
#define FOREGROUND_COLOR EGA_GRAY

#endif /* APRTERM_H */
