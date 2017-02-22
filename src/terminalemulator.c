#include "terminalemulator.h"
#include "terminalrenderer.h"

/* State functions */
void termStateIdle(char b);
void termStateReadArgs(char b);

/* Helper functions */
void handleCharacterInput(char c, char moveCursor, char autoNewLine);

/* Terminal fuction currently being processed (Initially set to an invalid value) */
char currentFunction = 0xFF;

/* 1 if Repeat mode is enabled, 0 otherwise */
char repeatEnabled = 0;

/* Pointer to current state function */
void (*currentStateHandler)(char) = &termStateIdle;

const SDL_Color EGA_COLOR_TABLE[] = {
    EGA_BLACK,
    EGA_DBLUE,
    EGA_DGREEN,
    EGA_DCYAN,
    EGA_DRED,
    EGA_DMAGENTA,
    EGA_BROWN,
    EGA_GRAY,
    EGA_DGRAY,
    EGA_BLUE,
    EGA_GREEN,
    EGA_CYAN,
    EGA_RED,
    EGA_MAGENTA,
    EGA_YELLOW,
    EGA_WHITE
};


void terminalHandleByte(char b) {
    currentStateHandler(b);
}

void resetTerminal() {
    currentStateHandler = &termStateIdle;
    currentFunction = 0xFF;
    repeatEnabled = 0;
}


/*========================================================
 * Internal functions
 *========================================================
 */

void termStateIdle(char b) {
    currentFunction = b;

    if(b != ENDMSG && b != RPT)
        currentStateHandler = &termStateReadArgs;
    if(b == RPT)
        repeatEnabled = 1;
}

void termStateReadArgs(char b) {
    if(b == ENDMSG && repeatEnabled)
        repeatEnabled = 0;
    else
        switch(currentFunction) {
            case PUTC:
                handleCharacterInput(b, 1, 1);
                break;
            case PUTCCLIP:
                handleCharacterInput(b, 1, 0);
                break;
            case PUTCFIX:
                handleCharacterInput(b, 0, 0);
                break;
            case MVLEFT:
                terminalAdvanceCursorLeft(0);
                break;
            case MVLEFTCLIP:
                terminalAdvanceCursorLeft(1);
                break;
            case MVRIGHT:
                terminalAdvanceCursorRight(0);
                break;
            case MVRIGHTCLIP:
                terminalAdvanceCursorRight(1);
                break;
            case MVUP:
                terminalAdvanceCursorUp(0);
                break;
            case MVUPCLIP:
                terminalAdvanceCursorUp(1);
                break;
            case MVDOWN:
                terminalAdvanceCursorDown(0);
                break;
            case MVDOWNCLIP:
                terminalAdvanceCursorDown(1);
                break;
            case SETFG:
                if(b >= 0 && b < 16)
                    terminalSetForegroundColor(EGA_COLOR_TABLE[(int)b]);
                break;
            case SETBG:
                if(b >= 0 && b < 16)
                    terminalSetBackgroundColor(EGA_COLOR_TABLE[(int)b]);
                break;
            case SETROW:
                terminalSetRow(b);
                break;
            case SETCOL:
                terminalSetCol(b);
                break;
            case CURSORVISIBLE:
                if(b)
                    terminalEnableCursor();
                else
                    terminalDisableCursor();
                break;
            case SCROLL:
                terminalScroll(b, 1);
                break;
            case CLS:
                terminalClear(1);
                break;
        default:
            break;
    }

    if(!repeatEnabled)
        currentStateHandler = &termStateIdle;
}

void handleCharacterInput(char c, char moveCursor, char autoNewLine) {
    switch(c) {
        case 0x08: /* backspace */
            if(moveCursor)
                terminalBackspace(1);
            else
                terminalPutChar(' ', 0, 0);
            break;
        case 0x0A: /* Newline (unixstyle) */
            terminalNewLine(1, 1);
            break;
        case 0x0D: /* Carriage return */
            terminalSetCol(0);
            break;
        default:
            terminalPutChar(c, moveCursor, autoNewLine);
            break;
    }
}
