/*
 * A stand-in for terminalrenderer.c that records each call in a log
 * instead of drawing. This lets the emulator's protocol handling be
 * tested on its own.
 */

#include "fakerenderer.h"
#include "terminalrenderer.h"

#include <stdarg.h>
#include <stdio.h>

static char callLog[8192];
static size_t callLogLength;

static void logCall(const char* format, ...) {
    va_list args;
    int written;

    va_start(args, format);
    written = vsnprintf(callLog + callLogLength, sizeof(callLog) - callLogLength, format, args);
    va_end(args);

    if(written > 0)
        callLogLength += written;
    if(callLogLength >= sizeof(callLog))
        callLogLength = sizeof(callLog) - 1;
}

/* Characters are logged as 'c' when printable, and as hex otherwise */
static void logChar(const char* name, char c, char moveCursor, char autoNewLine) {
    unsigned char u = (unsigned char)c;

    if(u >= 0x20 && u < 0x7F)
        logCall("%s('%c',%d,%d) ", name, u, moveCursor, autoNewLine);
    else
        logCall("%s(0x%02x,%d,%d) ", name, u, moveCursor, autoNewLine);
}

const char* fakeRendererLog() {
    /* Drop the trailing space */
    if(callLogLength > 0 && callLog[callLogLength - 1] == ' ')
        callLog[--callLogLength] = '\0';
    return callLog;
}

void fakeRendererReset() {
    callLogLength = 0;
    callLog[0] = '\0';
}


/*========================================================
 * terminalrenderer.h
 *========================================================
 */

int termRendererInit(char* fontPath, SDL_Color bgColor, SDL_Color fgColor) {
    (void)fontPath;
    (void)bgColor;
    (void)fgColor;
    return 1;
}

void terminalPutStr(char* str) {
    logCall("putstr(\"%s\") ", str);
}

void terminalPutChar(char c, char moveCursor, char autoNewLine) {
    logChar("putc", c, moveCursor, autoNewLine);
}

void terminalNewLine(char carriageReturn, char scroll) {
    logCall("newline(%d,%d) ", carriageReturn, scroll);
}

void terminalBackspace(char lineWrap) {
    logCall("backspace(%d) ", lineWrap);
}

void terminalSetRow(int row) {
    logCall("row(%d) ", row);
}

void terminalSetCol(int col) {
    logCall("col(%d) ", col);
}

void terminalAdvanceCursorLeft(char clamp) {
    logCall("left(%d) ", clamp);
}

void terminalAdvanceCursorRight(char clamp) {
    logCall("right(%d) ", clamp);
}

void terminalAdvanceCursorUp(char clamp) {
    logCall("up(%d) ", clamp);
}

void terminalAdvanceCursorDown(char clamp) {
    logCall("down(%d) ", clamp);
}

void terminalScroll(int lines, char moveCursor) {
    logCall("scroll(%d,%d) ", lines, moveCursor);
}

void terminalClear(char returnCursor) {
    logCall("clear(%d) ", returnCursor);
}

void terminalSetBackgroundColor(SDL_Color bgColor) {
    logCall("bg(%d,%d,%d) ", bgColor.r, bgColor.g, bgColor.b);
}

void terminalSetForegroundColor(SDL_Color fgColor) {
    logCall("fg(%d,%d,%d) ", fgColor.r, fgColor.g, fgColor.b);
}

void terminalEnableCursor() {
    logCall("cursor(1) ");
}

void terminalDisableCursor() {
    logCall("cursor(0) ");
}

void terminalRefresh() {
    logCall("refresh() ");
}

void destroyTerminalRenderer() {
}
