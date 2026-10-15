/*
 * Tests for the terminal renderer (terminalrenderer.c), driven through
 * the emulator.
 *
 * These run the real renderer on SDL's dummy video driver with the
 * software renderer, so no window is opened. Each test feeds protocol
 * bytes and then reads the text buffer back, checking every pixel of a
 * character cell against the glyph in the font bitmap.
 */

#include "testing.h"
#include "apricosterm.h"
#include "screen.h"
#include "terminalemulator.h"
#include "terminalrenderer.h"
#include "managedtextures.h"

#include <stdio.h>
#include <string.h>

/* Renderer state (terminalrenderer.c) */
extern int cursorRow;
extern int cursorCol;
extern SDL_Texture* textBuffer;
extern SDL_Texture* screenBuffer;

/* Colors as 0xRRGGBB */
#define BLACK 0x000000
#define DBLUE 0x0000AA
#define GRAY  0xAAAAAA
#define RED   0xFF5555

/* The font, converted to 32-bit pixels, for working out what a glyph should look like */
static SDL_Surface* font;

/* Feed a sequence of bytes to the emulator */
#define FEED(...) feedBytes((const unsigned char[]){__VA_ARGS__}, \
        sizeof((const unsigned char[]){__VA_ARGS__}))

static void feedBytes(const unsigned char* bytes, size_t count) {
    size_t i;

    for(i = 0; i < count; i++)
        terminalHandleByte((char)bytes[i]);
}

/* Print a string in repeat mode, leaving the terminal idle */
static void print(const char* str) {
    FEED(RPT, PUTC);
    feedBytes((const unsigned char*)str, strlen(str));
    FEED(ENDMSG);
}

static void moveTo(int row, int col) {
    FEED(SETROW, row, SETCOL, col);
}

/* 1 if pixel (x, y) of glyph c is set in the font */
static int glyphPixel(unsigned char c, int x, int y) {
    int fx = (c % FONT_COLS) * CHAR_WIDTH + x;
    int fy = (c / FONT_COLS) * CHAR_HEIGHT + y;
    Uint32 pixel = ((Uint32*)((Uint8*)font->pixels + fy * font->pitch))[fx];

    return (pixel & 0xFF) > 0x7F;
}

/*
 * Compare a character cell of a target texture with glyph c drawn in the
 * given colors. Returns the number of pixels that differ, and describes the
 * first one in mismatch.
 */
static int cellDiff(SDL_Texture* texture, int row, int col, unsigned char c,
        Uint32 fg, Uint32 bg, char* mismatch, size_t mismatchSize) {
    Uint32 pixels[CHAR_WIDTH * CHAR_HEIGHT];
    SDL_Rect cell = { col * CHAR_WIDTH, row * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT };
    int x, y;
    int diffs = 0;

    setRenderTarget(texture);
    SDL_RenderReadPixels(getScreenRenderer(), &cell, SDL_PIXELFORMAT_ARGB8888,
            pixels, CHAR_WIDTH * sizeof(Uint32));
    resetRenderTarget();

    for(y = 0; y < CHAR_HEIGHT; y++) {
        for(x = 0; x < CHAR_WIDTH; x++) {
            Uint32 expected = glyphPixel(c, x, y) ? fg : bg;
            Uint32 actual = pixels[y * CHAR_WIDTH + x] & 0xFFFFFF;

            if(expected != actual) {
                if(!diffs)
                    snprintf(mismatch, mismatchSize, "pixel (%d,%d) is %06X, expected %06X",
                            x, y, (unsigned)actual, (unsigned)expected);
                diffs++;
            }
        }
    }

    return diffs;
}

#define CHECK_CELL_IN(texture, row, col, c, fg, bg) do { \
    char mismatch_[64]; \
    int diffs_ = cellDiff((texture), (row), (col), (unsigned char)(c), (fg), (bg), mismatch_, sizeof(mismatch_)); \
    if(diffs_) { \
        testFail(__FILE__, __LINE__, "cell (%d,%d) does not show 0x%02X: %d pixels differ, %s", \
                (row), (col), (unsigned char)(c), diffs_, mismatch_); \
        return; \
    } \
} while(0)

/* Check a cell of the text buffer */
#define CHECK_CELL_COLORS(row, col, c, fg, bg) CHECK_CELL_IN(textBuffer, row, col, c, fg, bg)
#define CHECK_CELL(row, col, c) CHECK_CELL_COLORS(row, col, c, GRAY, BLACK)
#define CHECK_BLANK(row, col) CHECK_CELL(row, col, ' ')

#define CHECK_CURSOR(row, col) do { \
    CHECK_INT_EQ((row), cursorRow); \
    CHECK_INT_EQ((col), cursorCol); \
} while(0)

static void setUp(void) {
    resetTerminal();
    FEED(SETFG, 7, SETBG, 0, CLS, 0, CURSORVISIBLE, 1);
}


/*========================================================
 * Printing
 *========================================================
 */

static void putcDrawsGlyphAndAdvances(void) {
    FEED(PUTC, 'A');
    CHECK_CELL(0, 0, 'A');
    CHECK_BLANK(0, 1);
    CHECK_CURSOR(0, 1);
}

static void printsAtCursorPosition(void) {
    moveTo(12, 40);
    print("Hello");
    CHECK_CELL(12, 40, 'H');
    CHECK_CELL(12, 44, 'o');
    CHECK_BLANK(12, 39);
    CHECK_BLANK(0, 0);
    CHECK_CURSOR(12, 45);
}

static void printOverwrites(void) {
    print("abc");
    moveTo(0, 1);
    print("X");
    CHECK_CELL(0, 0, 'a');
    CHECK_CELL(0, 1, 'X');
    CHECK_CELL(0, 2, 'c');
}

static void putcWrapsAtEndOfLine(void) {
    moveTo(3, SCREEN_COLS - 1);
    print("AB");
    CHECK_CELL(3, SCREEN_COLS - 1, 'A');
    CHECK_CELL(4, 0, 'B');
    CHECK_CURSOR(4, 1);
}

static void putcClipStaysOnLastColumn(void) {
    moveTo(3, SCREEN_COLS - 2);
    FEED(RPT, PUTCCLIP, 'A', 'B', 'C', ENDMSG);
    CHECK_CELL(3, SCREEN_COLS - 2, 'A');
    CHECK_CELL(3, SCREEN_COLS - 1, 'C');
    CHECK_BLANK(4, 0);
    CHECK_CURSOR(3, SCREEN_COLS - 1);
}

static void putcFixDoesNotMove(void) {
    moveTo(5, 5);
    FEED(PUTCFIX, 'A', PUTCFIX, 'B');
    CHECK_CELL(5, 5, 'B');
    CHECK_CURSOR(5, 5);
}

static void wrapOnLastRowScrolls(void) {
    print("top");
    moveTo(1, 0);
    print("second");
    moveTo(SCREEN_ROWS - 1, SCREEN_COLS - 1);
    print("XY");
    CHECK_CELL(0, 0, 's');
    CHECK_CELL(SCREEN_ROWS - 2, SCREEN_COLS - 1, 'X');
    CHECK_CELL(SCREEN_ROWS - 1, 0, 'Y');
    CHECK_BLANK(SCREEN_ROWS - 1, SCREEN_COLS - 1);
    CHECK_CURSOR(SCREEN_ROWS - 1, 1);
}

static void allGlyphsRender(void) {
    int c;

    for(c = 0x20; c <= 0xFF; c++) {
        FEED(CLS, 0, PUTC, c);
        CHECK_CELL(0, 0, c);
    }
}

static void highGlyphsRender(void) {
    /* 0x80-0xFF are the CP437 drawing and accented characters */
    FEED(PUTC, 0xDB, PUTC, 0xB0, PUTC, 0x82);
    CHECK_CELL(0, 0, 0xDB);
    CHECK_CELL(0, 1, 0xB0);
    CHECK_CELL(0, 2, 0x82);
}


/*========================================================
 * Control characters
 *========================================================
 */

static void newlineMovesToStartOfNextLine(void) {
    print("ab\ncd");
    CHECK_CELL(0, 1, 'b');
    CHECK_CELL(1, 0, 'c');
    CHECK_CURSOR(1, 2);
}

static void newlineOnLastRowScrolls(void) {
    print("first\nsecond");
    moveTo(SCREEN_ROWS - 1, 0);
    print("last\n");
    CHECK_CELL(0, 0, 's');
    CHECK_CELL(SCREEN_ROWS - 2, 0, 'l');
    CHECK_BLANK(SCREEN_ROWS - 1, 0);
    CHECK_CURSOR(SCREEN_ROWS - 1, 0);
}

static void carriageReturnGoesToStartOfLine(void) {
    print("abc\rX");
    CHECK_CELL(0, 0, 'X');
    CHECK_CELL(0, 1, 'b');
    CHECK_CURSOR(0, 1);
}

static void backspaceErasesPreviousCharacter(void) {
    print("abc\b");
    CHECK_CELL(0, 1, 'b');
    CHECK_BLANK(0, 2);
    CHECK_CURSOR(0, 2);
}

static void backspaceWrapsToPreviousLine(void) {
    moveTo(4, SCREEN_COLS - 1);
    print("ab\b\b");
    CHECK_BLANK(4, SCREEN_COLS - 1);
    CHECK_BLANK(5, 0);
    CHECK_CURSOR(4, SCREEN_COLS - 1);
}

static void backspaceAtHomeErasesInPlace(void) {
    print("a");
    moveTo(0, 0);
    print("\b");
    CHECK_BLANK(0, 0);
    CHECK_CURSOR(0, 0);
}

static void putcFixBackspaceErasesWithoutMoving(void) {
    print("ab");
    moveTo(0, 1);
    FEED(PUTCFIX, 0x08);
    CHECK_CELL(0, 0, 'a');
    CHECK_BLANK(0, 1);
    CHECK_CURSOR(0, 1);
}


/*========================================================
 * Cursor movement
 *========================================================
 */

static void movementWraps(void) {
    moveTo(5, 0);
    FEED(MVLEFT, 0);
    CHECK_CURSOR(5, SCREEN_COLS - 1);

    FEED(MVRIGHT, 0);
    CHECK_CURSOR(6, 0);

    moveTo(0, 7);
    FEED(MVUP, 0);
    CHECK_CURSOR(SCREEN_ROWS - 1, 7);

    FEED(MVDOWN, 0);
    CHECK_CURSOR(0, 7);
}

static void movementClamps(void) {
    moveTo(0, 0);
    FEED(MVLEFTCLIP, 0, MVUPCLIP, 0);
    CHECK_CURSOR(0, 0);

    moveTo(SCREEN_ROWS - 1, SCREEN_COLS - 1);
    FEED(MVRIGHTCLIP, 0, MVDOWNCLIP, 0);
    CHECK_CURSOR(SCREEN_ROWS - 1, SCREEN_COLS - 1);
}

static void movementInRepeatMode(void) {
    moveTo(10, 10);
    FEED(RPT, MVRIGHT, 0, 0, 0, ENDMSG, RPT, MVDOWN, 0, 0, ENDMSG);
    CHECK_CURSOR(12, 13);
}

static void moveRightOffLastCellScrolls(void) {
    print("top\nsecond");
    moveTo(SCREEN_ROWS - 1, SCREEN_COLS - 1);
    FEED(MVRIGHT, 0);
    CHECK_CELL(0, 0, 's');
    CHECK_CURSOR(SCREEN_ROWS - 1, 0);
}


/*========================================================
 * Scrolling and clearing
 *========================================================
 */

static void scrollMovesContentAndCursorUp(void) {
    print("row0\nrow1\nrow2\nrow3");
    moveTo(SCREEN_ROWS - 1, 0);
    print("bottom");
    moveTo(5, 3);
    FEED(SCROLL, 2);
    CHECK_CELL(0, 3, '2');
    CHECK_CELL(1, 3, '3');
    CHECK_BLANK(2, 0);
    CHECK_CELL(SCREEN_ROWS - 3, 0, 'b');
    CHECK_BLANK(SCREEN_ROWS - 2, 0);
    CHECK_BLANK(SCREEN_ROWS - 1, 0);
    CHECK_CURSOR(3, 3);
}

static void scrollCursorStopsAtTop(void) {
    moveTo(1, 4);
    FEED(SCROLL, 3);
    CHECK_CURSOR(0, 4);
}

static void scrollMoreThanScreenClearsIt(void) {
    print("abc");
    moveTo(SCREEN_ROWS - 1, 0);
    print("xyz");
    FEED(SCROLL, SCREEN_ROWS + 10);
    CHECK_BLANK(0, 0);
    CHECK_BLANK(SCREEN_ROWS - 1, 0);
    CHECK_CURSOR(0, 3);
}

static void clearScreenErasesAndHomes(void) {
    print("abc");
    moveTo(SCREEN_ROWS - 1, SCREEN_COLS - 1);
    FEED(PUTCFIX, 'z');
    FEED(CLS, CLS);
    CHECK_BLANK(0, 0);
    CHECK_BLANK(SCREEN_ROWS - 1, SCREEN_COLS - 1);
    CHECK_CURSOR(0, 0);
}


/*========================================================
 * Colors
 *========================================================
 */

static void foregroundColorAppliesToNewText(void) {
    print("a");
    FEED(SETFG, 12);
    print("b");
    CHECK_CELL_COLORS(0, 0, 'a', GRAY, BLACK);
    CHECK_CELL_COLORS(0, 1, 'b', RED, BLACK);
}

static void backgroundColorAppliesToNewText(void) {
    print("a");
    FEED(SETBG, 1);
    print("b");
    CHECK_CELL_COLORS(0, 0, 'a', GRAY, BLACK);
    CHECK_CELL_COLORS(0, 1, 'b', GRAY, DBLUE);
    /* Untouched cells keep the old background */
    CHECK_CELL_COLORS(0, 2, ' ', GRAY, BLACK);
}

static void clearScreenUsesBackgroundColor(void) {
    FEED(SETBG, 1, CLS, 0);
    CHECK_CELL_COLORS(0, 0, ' ', GRAY, DBLUE);
    CHECK_CELL_COLORS(SCREEN_ROWS - 1, SCREEN_COLS - 1, ' ', GRAY, DBLUE);
}

static void scrollFillsWithBackgroundColor(void) {
    FEED(SETBG, 1, SCROLL, 1);
    CHECK_CELL_COLORS(SCREEN_ROWS - 1, 0, ' ', GRAY, DBLUE);
    CHECK_CELL_COLORS(SCREEN_ROWS - 2, 0, ' ', GRAY, BLACK);
}


/*========================================================
 * Cursor display
 *========================================================
 */

/* The cursor is a bar two pixels wide on the left of its cell */
static int cursorShownAt(int row, int col) {
    Uint32 pixels[CHAR_WIDTH * CHAR_HEIGHT];
    SDL_Rect cell = { col * CHAR_WIDTH, row * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT };
    int y;

    terminalRefresh();
    setRenderTarget(screenBuffer);
    SDL_RenderReadPixels(getScreenRenderer(), &cell, SDL_PIXELFORMAT_ARGB8888,
            pixels, CHAR_WIDTH * sizeof(Uint32));
    resetRenderTarget();

    for(y = 0; y < CHAR_HEIGHT; y++) {
        if((pixels[y * CHAR_WIDTH] & 0xFFFFFF) != GRAY
                || (pixels[y * CHAR_WIDTH + 1] & 0xFFFFFF) != GRAY
                || (pixels[y * CHAR_WIDTH + 2] & 0xFFFFFF) != BLACK)
            return 0;
    }

    return 1;
}

static void cursorDrawnOnRefresh(void) {
    moveTo(7, 9);
    CHECK(cursorShownAt(7, 9));
    CHECK(!cursorShownAt(0, 0));
}

static void cursorHidden(void) {
    moveTo(7, 9);
    FEED(CURSORVISIBLE, 0);
    CHECK(!cursorShownAt(7, 9));
    FEED(CURSORVISIBLE, 1);
    CHECK(cursorShownAt(7, 9));
}

static void cursorNotInTextBuffer(void) {
    moveTo(7, 9);
    terminalRefresh();
    CHECK_BLANK(7, 9);
}


/*========================================================
 * End to end
 *========================================================
 */

static void osStyleSession(void) {
    /* LIBDISP_TTYRESET, then a prompt, a typed command, and its output */
    FEED(ENDMSG, CURSORVISIBLE, 0, CLS, CLS);
    print("A:> ");
    print("lx");
    print("\b");
    print("s\n");
    print("BOOT.BIN\nSHELL.BIN\n");
    print("A:> ");

    CHECK_CELL(0, 0, 'A');
    CHECK_CELL(0, 4, 'l');
    CHECK_CELL(0, 5, 's');
    CHECK_BLANK(0, 6);
    CHECK_CELL(1, 0, 'B');
    CHECK_CELL(2, 8, 'N');
    CHECK_CELL(3, 1, ':');
    CHECK_CURSOR(3, 4);
}


static const TestCase tests[] = {
    TEST(putcDrawsGlyphAndAdvances),
    TEST(printsAtCursorPosition),
    TEST(printOverwrites),
    TEST(putcWrapsAtEndOfLine),
    TEST(putcClipStaysOnLastColumn),
    TEST(putcFixDoesNotMove),
    TEST(wrapOnLastRowScrolls),
    TEST(allGlyphsRender),
    TEST(highGlyphsRender),
    TEST(newlineMovesToStartOfNextLine),
    TEST(newlineOnLastRowScrolls),
    TEST(carriageReturnGoesToStartOfLine),
    TEST(backspaceErasesPreviousCharacter),
    TEST(backspaceWrapsToPreviousLine),
    TEST(backspaceAtHomeErasesInPlace),
    TEST(putcFixBackspaceErasesWithoutMoving),
    TEST(movementWraps),
    TEST(movementClamps),
    TEST(movementInRepeatMode),
    TEST(moveRightOffLastCellScrolls),
    TEST(scrollMovesContentAndCursorUp),
    TEST(scrollCursorStopsAtTop),
    TEST(scrollMoreThanScreenClearsIt),
    TEST(clearScreenErasesAndHomes),
    TEST(foregroundColorAppliesToNewText),
    TEST(backgroundColorAppliesToNewText),
    TEST(clearScreenUsesBackgroundColor),
    TEST(scrollFillsWithBackgroundColor),
    TEST(cursorDrawnOnRefresh),
    TEST(cursorHidden),
    TEST(cursorNotInTextBuffer),
    TEST(osStyleSession),
};

int main(int argc, char** argv) {
    SDL_Surface* fontImage;
    SDL_Color bgColor = BACKGROUND_COLOR;
    SDL_Color fgColor = FOREGROUND_COLOR;
    int status;

    /* Render off screen */
    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);

    if(!initScreen("ApricosTerm tests", SCREEN_WIDTH, SCREEN_HEIGHT)) {
        fprintf(stderr, "ERROR: %s\n", screenGetError());
        return EXIT_FAILURE;
    }

    if(!termRendererInit(FONT_PATH, bgColor, fgColor)) {
        fprintf(stderr, "ERROR: %s\n", screenGetError());
        return EXIT_FAILURE;
    }

    fontImage = IMG_Load(FONT_PATH);
    font = fontImage ? SDL_ConvertSurfaceFormat(fontImage, SDL_PIXELFORMAT_ARGB8888, 0) : NULL;
    if(!font) {
        fprintf(stderr, "ERROR: could not load %s: %s\n", FONT_PATH, SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_FreeSurface(fontImage);

    status = runTests(tests, sizeof(tests) / sizeof(tests[0]), setUp, argc > 1 ? argv[1] : NULL);

    SDL_FreeSurface(font);
    destroyTerminalRenderer();
    destroyAllTextures();
    destroyScreen();

    return status;
}
