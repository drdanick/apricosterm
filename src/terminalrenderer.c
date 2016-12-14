#include "apricosterm.h"
#include "terminalrenderer.h"
#include "managedtextures.h"
#include "screen.h"

SDL_Texture* screenBuffer = NULL;
SDL_Texture* fontTexture = NULL;
int fontWidth;
int fontHeight;
int bufferWidth;
int bufferHeight;

/* State variables */
int cursorRow;
int cursorCol;

/* Private function defs */
void charCodeToFontCoordinates(char c, int* outRow, int* outCol);

void termRendererInit() {
    screenBuffer = createManagedTexture(getScreenWidth(), getScreenHeight(), getScreenRenderer(), SDL_TEXTUREACCESS_TARGET);
    fontTexture = createManagedTextureFromFile(FONT_FILE, getScreenRenderer());
    bufferWidth = getScreenWidth();
    bufferHeight = getScreenHeight();
    SDL_QueryTexture(fontTexture, NULL, NULL, &fontWidth, &fontHeight);

    cursorRow = 0;
    cursorCol = 0;

    setRenderTarget(screenBuffer);
    setDrawColor(0, 0, 0);
    SDL_RenderClear(getScreenRenderer());

    resetRenderTarget();
}

void terminalPutStr(char* str) {
    while(*str) {
        terminalPutChar(*str, 1);
        str++;
    }
}

void terminalPutChar(char c, char moveCursor) {
    SDL_Rect fontSrcRect = {
        0,
        0,
        CHAR_WIDTH,
        CHAR_HEIGHT
    };

    SDL_Rect destRect = {
        cursorCol * CHAR_WIDTH,
        cursorRow * CHAR_HEIGHT,
        CHAR_WIDTH,
        CHAR_HEIGHT
    };

    charCodeToFontCoordinates(c, &fontSrcRect.y, &fontSrcRect.x);
    fontSrcRect.x *= CHAR_WIDTH;
    fontSrcRect.y *= CHAR_HEIGHT;

    setRenderTarget(screenBuffer);
    copyTextureSegmentToScreen(fontTexture, &fontSrcRect, &destRect);

    if(moveCursor)
        terminalAdvanceCursor(0);
}

void terminalNewLine(char carriageReturn, char scroll) {
    cursorRow++;

    if(carriageReturn)
        cursorCol = 0;

    if(cursorRow >= SCREEN_COLS) {
        cursorRow--;
        if(scroll) {
            // TODO: Scroll screen
        }
    }
}

void terminalBackspace(char lineWrap) {
    cursorCol--;
    if(cursorCol < 0) {
        cursorCol = 0;

        if(lineWrap) {
            cursorRow--;
            if(cursorRow < 0)
                cursorRow = 0;
            else
                cursorCol = SCREEN_COLS - 1;
        }
    }

    terminalPutChar(' ', 0);
}

void terminalSetRow(int row) {
    cursorRow = row;
}

void terminalSetCol(int col) {
    cursorCol = col;
}

void terminalAdvanceCursor(char clamp) {
    cursorCol++;
    if(cursorCol >= SCREEN_COLS) {
        if(clamp) {
            cursorCol--;
        } else {
            cursorCol = 0;
            cursorRow++;
        }
    }

    if(cursorRow >= SCREEN_ROWS) {
        /* TODO: Scroll screen */
    }
}

void terminalRefresh() {
    SDL_Rect bufferRect = {
        0,
        0,
        bufferWidth,
        bufferHeight
    };
    resetRenderTarget();
    clearRenderer();
    copyFullTextureToScreen(screenBuffer, NULL);
    presentRenderer();
}


/* private */
void charCodeToFontCoordinates(char c, int* outRow, int* outCol) {
    *outRow = c / FONT_COLS;
    *outCol = c % FONT_COLS;
}
