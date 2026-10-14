/*
 * Tests for the terminal protocol state machine (terminalemulator.c).
 *
 * The emulator is linked against a fake renderer that logs each call,
 * so these tests check which renderer calls each byte sequence makes.
 */

#include "testing.h"
#include "fakerenderer.h"
#include "terminalemulator.h"

#include <string.h>

/* Feed a sequence of bytes to the emulator */
#define FEED(...) feedBytes((const unsigned char[]){__VA_ARGS__}, \
        sizeof((const unsigned char[]){__VA_ARGS__}))

static void feedBytes(const unsigned char* bytes, size_t count) {
    size_t i;

    for(i = 0; i < count; i++)
        terminalHandleByte((char)bytes[i]);
}

static void feedStr(const char* str) {
    feedBytes((const unsigned char*)str, strlen(str));
}

static void setUp(void) {
    resetTerminal();
    fakeRendererReset();
}


/*========================================================
 * Single commands (idle state)
 *========================================================
 */

static void putcPrintsWithWrap(void) {
    FEED(PUTC, 'A');
    CHECK_STR_EQ("putc('A',1,1)", fakeRendererLog());
}

static void putcClipPrintsWithoutWrap(void) {
    FEED(PUTCCLIP, 'A');
    CHECK_STR_EQ("putc('A',1,0)", fakeRendererLog());
}

static void putcFixPrintsWithoutMoving(void) {
    FEED(PUTCFIX, 'A');
    CHECK_STR_EQ("putc('A',0,0)", fakeRendererLog());
}

static void singleCommandReturnsToIdle(void) {
    /* Without repeat mode, the byte after the argument is a new command */
    FEED(PUTC, 'A', PUTC, 'B');
    CHECK_STR_EQ("putc('A',1,1) putc('B',1,1)", fakeRendererLog());
}

static void movementCommands(void) {
    FEED(MVLEFT, 0, MVLEFTCLIP, 0,
         MVRIGHT, 0, MVRIGHTCLIP, 0,
         MVUP, 0, MVUPCLIP, 0,
         MVDOWN, 0, MVDOWNCLIP, 0);
    CHECK_STR_EQ("left(0) left(1) right(0) right(1) up(0) up(1) down(0) down(1)", fakeRendererLog());
}

static void movementIgnoresArgument(void) {
    FEED(MVRIGHT, 5);
    CHECK_STR_EQ("right(0)", fakeRendererLog());
}

static void setRowAndCol(void) {
    FEED(SETROW, 12, SETCOL, 40);
    CHECK_STR_EQ("row(12) col(40)", fakeRendererLog());
}

static void setForegroundColor(void) {
    FEED(SETFG, 4, SETFG, 15);
    CHECK_STR_EQ("fg(170,0,0) fg(255,255,255)", fakeRendererLog());
}

static void setBackgroundColor(void) {
    FEED(SETBG, 1, SETBG, 0);
    CHECK_STR_EQ("bg(0,0,170) bg(0,0,0)", fakeRendererLog());
}

static void colorOutOfRangeIsIgnored(void) {
    FEED(SETFG, 16, SETBG, 0x80, SETFG, 0xFF);
    CHECK_STR_EQ("", fakeRendererLog());

    /* The argument is still consumed */
    FEED(PUTC, 'A');
    CHECK_STR_EQ("putc('A',1,1)", fakeRendererLog());
}

static void colorTableIsEgaOrder(void) {
    static const char* expected[16] = {
        "fg(0,0,0)", "fg(0,0,170)", "fg(0,170,0)", "fg(0,170,170)",
        "fg(170,0,0)", "fg(170,0,170)", "fg(170,85,0)", "fg(170,170,170)",
        "fg(85,85,85)", "fg(85,85,255)", "fg(85,255,85)", "fg(85,255,255)",
        "fg(255,85,85)", "fg(255,85,255)", "fg(255,255,85)", "fg(255,255,255)"
    };
    int i;

    for(i = 0; i < 16; i++) {
        fakeRendererReset();
        FEED(SETFG, i);
        CHECK_STR_EQ(expected[i], fakeRendererLog());
    }
}

static void cursorVisibility(void) {
    FEED(CURSORVISIBLE, 0, CURSORVISIBLE, 1, CURSORVISIBLE, 0x80);
    CHECK_STR_EQ("cursor(0) cursor(1) cursor(1)", fakeRendererLog());
}

static void scrollMovesCursor(void) {
    FEED(SCROLL, 3);
    CHECK_STR_EQ("scroll(3,1)", fakeRendererLog());
}

static void clearScreenHomesCursor(void) {
    FEED(CLS, CLS);
    CHECK_STR_EQ("clear(1)", fakeRendererLog());
}

static void endMessageWhileIdleIsNoOp(void) {
    FEED(ENDMSG, ENDMSG, PUTC, 'A');
    CHECK_STR_EQ("putc('A',1,1)", fakeRendererLog());
}

static void unknownCommandConsumesOneArgument(void) {
    FEED(0x13, 'x', 'A', 'y', PUTC, 'B');
    CHECK_STR_EQ("putc('B',1,1)", fakeRendererLog());
}

static void endMessageAsSingleArgumentIsPrinted(void) {
    /* Outside repeat mode, 03 is an ordinary argument */
    FEED(PUTC, ENDMSG);
    CHECK_STR_EQ("putc(0x03,1,1)", fakeRendererLog());
}


/*========================================================
 * Control characters in print commands
 *========================================================
 */

static void backspaceWithMove(void) {
    FEED(PUTC, 0x08, PUTCCLIP, 0x08);
    CHECK_STR_EQ("backspace(1) backspace(1)", fakeRendererLog());
}

static void backspaceWithoutMoveErasesInPlace(void) {
    FEED(PUTCFIX, 0x08);
    CHECK_STR_EQ("putc(' ',0,0)", fakeRendererLog());
}

static void newline(void) {
    FEED(PUTC, 0x0A, PUTCCLIP, 0x0A, PUTCFIX, 0x0A);
    CHECK_STR_EQ("newline(1,1) newline(1,1) newline(1,1)", fakeRendererLog());
}

static void carriageReturn(void) {
    FEED(PUTC, 0x0D);
    CHECK_STR_EQ("col(0)", fakeRendererLog());
}

static void otherControlCharactersAreDrawn(void) {
    FEED(PUTC, 0x00, PUTC, 0x07, PUTC, 0x1B);
    CHECK_STR_EQ("putc(0x00,1,1) putc(0x07,1,1) putc(0x1b,1,1)", fakeRendererLog());
}

static void highCharactersArePassedThrough(void) {
    FEED(PUTC, 0x80, PUTC, 0xDB, PUTC, 0xFF);
    CHECK_STR_EQ("putc(0x80,1,1) putc(0xdb,1,1) putc(0xff,1,1)", fakeRendererLog());
}


/*========================================================
 * Repeat mode
 *========================================================
 */

static void repeatPrintsUntilEndMessage(void) {
    FEED(RPT, PUTC);
    feedStr("Hi!");
    FEED(ENDMSG);
    CHECK_STR_EQ("putc('H',1,1) putc('i',1,1) putc('!',1,1)", fakeRendererLog());
}

static void repeatEndsOnlyOnEndMessage(void) {
    /* Bytes that look like commands are arguments in repeat mode */
    FEED(RPT, PUTC, CLS, SETROW, RPT, 0x0A);
    CHECK_STR_EQ("putc(0x12,1,1) putc(0x0e,1,1) putc(0x7f,1,1) newline(1,1)", fakeRendererLog());
}

static void repeatReturnsToIdleAfterEndMessage(void) {
    FEED(RPT, PUTC, 'a', ENDMSG, SETROW, 5, 'b');
    /* 'b' is a command byte (unknown), so nothing is printed */
    CHECK_STR_EQ("putc('a',1,1) row(5)", fakeRendererLog());
}

static void repeatWithMovementCommand(void) {
    FEED(RPT, MVRIGHT, 0, 0, 0, ENDMSG);
    CHECK_STR_EQ("right(0) right(0) right(0)", fakeRendererLog());
}

static void repeatWithSetColor(void) {
    FEED(RPT, SETFG, 1, 2, 99, ENDMSG);
    CHECK_STR_EQ("fg(0,0,170) fg(0,170,0)", fakeRendererLog());
}

static void repeatWithEmptyBody(void) {
    FEED(RPT, PUTC, ENDMSG, PUTC, 'A');
    CHECK_STR_EQ("putc('A',1,1)", fakeRendererLog());
}

static void repeatEndMessageCommandIsIgnored(void) {
    /* 7F 03: repeat mode on an ENDMSG command. The emulator stays idle
     * with repeat on, so the next byte is taken as the command. */
    FEED(RPT, ENDMSG, PUTC, 'A', 'B', ENDMSG);
    CHECK_STR_EQ("putc('A',1,1) putc('B',1,1)", fakeRendererLog());
}

static void repeatTwiceIsSameAsOnce(void) {
    FEED(RPT, RPT, PUTC, 'A', ENDMSG, PUTC, 'B');
    CHECK_STR_EQ("putc('A',1,1) putc('B',1,1)", fakeRendererLog());
}

static void switchingCommandsInRepeatMode(void) {
    /* The documented way to send a command while printing */
    FEED(RPT, PUTC, 'a', ENDMSG,
         SETCOL, 10,
         RPT, PUTC, 'b', ENDMSG);
    CHECK_STR_EQ("putc('a',1,1) col(10) putc('b',1,1)", fakeRendererLog());
}


/*========================================================
 * Reset
 *========================================================
 */

static void resetLeavesRepeatMode(void) {
    FEED(RPT, PUTC, 'a');
    resetTerminal();
    FEED(SETROW, 3);
    CHECK_STR_EQ("putc('a',1,1) row(3)", fakeRendererLog());
}

static void resetMidCommand(void) {
    FEED(SETROW);
    resetTerminal();
    FEED(PUTC, 'A');
    CHECK_STR_EQ("putc('A',1,1)", fakeRendererLog());
}

static void osResetSequence(void) {
    /* LIBDISP_TTYRESET, sent from the middle of a print */
    FEED(RPT, PUTC, 'x');
    FEED(ENDMSG, CURSORVISIBLE, 0, CLS, CLS);
    FEED(RPT, PUTC);
    feedStr("ok");
    CHECK_STR_EQ("putc('x',1,1) cursor(0) clear(1) putc('o',1,1) putc('k',1,1)", fakeRendererLog());
}


static const TestCase tests[] = {
    TEST(putcPrintsWithWrap),
    TEST(putcClipPrintsWithoutWrap),
    TEST(putcFixPrintsWithoutMoving),
    TEST(singleCommandReturnsToIdle),
    TEST(movementCommands),
    TEST(movementIgnoresArgument),
    TEST(setRowAndCol),
    TEST(setForegroundColor),
    TEST(setBackgroundColor),
    TEST(colorOutOfRangeIsIgnored),
    TEST(colorTableIsEgaOrder),
    TEST(cursorVisibility),
    TEST(scrollMovesCursor),
    TEST(clearScreenHomesCursor),
    TEST(endMessageWhileIdleIsNoOp),
    TEST(unknownCommandConsumesOneArgument),
    TEST(endMessageAsSingleArgumentIsPrinted),
    TEST(backspaceWithMove),
    TEST(backspaceWithoutMoveErasesInPlace),
    TEST(newline),
    TEST(carriageReturn),
    TEST(otherControlCharactersAreDrawn),
    TEST(highCharactersArePassedThrough),
    TEST(repeatPrintsUntilEndMessage),
    TEST(repeatEndsOnlyOnEndMessage),
    TEST(repeatReturnsToIdleAfterEndMessage),
    TEST(repeatWithMovementCommand),
    TEST(repeatWithSetColor),
    TEST(repeatWithEmptyBody),
    TEST(repeatEndMessageCommandIsIgnored),
    TEST(repeatTwiceIsSameAsOnce),
    TEST(switchingCommandsInRepeatMode),
    TEST(resetLeavesRepeatMode),
    TEST(resetMidCommand),
    TEST(osResetSequence),
};

int main(int argc, char** argv) {
    return runTests(tests, sizeof(tests) / sizeof(tests[0]), setUp, argc > 1 ? argv[1] : NULL);
}
