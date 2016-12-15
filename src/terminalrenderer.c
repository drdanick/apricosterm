#include "apricosterm.h"
#include "terminalrenderer.h"
#include "managedtextures.h"
#include "screen.h"

SDL_Texture* textBuffer = NULL;
SDL_Texture* screenBuffer = NULL;
SDL_Texture* fontTexture = NULL;
int fontWidth;
int fontHeight;
int bufferWidth;
int bufferHeight;
SDL_Color backgroundColor;
SDL_Color foregroundColor;
char cursorEnabled;

/* State variables */
int cursorRow;
int cursorCol;

/* Private function defs */
void charCodeToFontCoordinates(char c, int* outRow, int* outCol);
SDL_Palette* createTerminalPalette(SDL_Color backgroundColor, SDL_Color foregroundColor);
void drawCursor();

int termRendererInit(SDL_Color bgColor, SDL_Color fgColor) {
    cursorEnabled = 1;
    backgroundColor = bgColor;
    foregroundColor = fgColor;
    cursorRow = 0;
    cursorCol = 0;

    SDL_Palette* palette = createTerminalPalette(bgColor, fgColor);
    if(!palette) {
        screenSetError("termRendererInit", (char*)screenGetError(), 0);
        return 0;
    }

    textBuffer = createManagedTexture(getScreenWidth(), getScreenHeight(), getScreenRenderer(), SDL_TEXTUREACCESS_TARGET);
    if(!textBuffer) {
        SDL_FreePalette(palette);
        screenSetError("termRendererInit", (char*)screenGetError(), 0);
        return 0;
    }

    screenBuffer = createManagedTexture(getScreenWidth(), getScreenHeight(), getScreenRenderer(), SDL_TEXTUREACCESS_TARGET);
    if(!screenBuffer) {
        SDL_FreePalette(palette);
        destroyManagedTexture(textBuffer);
        screenSetError("termRendererInit", (char*)screenGetError(), 0);
        return 0;
    }

    fontTexture = createManagedTextureFromFile(FONT_FILE, palette, getScreenRenderer());
    SDL_FreePalette(palette);
    if(!fontTexture) {
        destroyManagedTexture(textBuffer);
        destroyManagedTexture(screenBuffer);
        screenSetError("termRendererInit", (char*)screenGetError(), 0);
        return 0;
    }

    bufferWidth = getScreenWidth();
    bufferHeight = getScreenHeight();
    SDL_QueryTexture(fontTexture, NULL, NULL, &fontWidth, &fontHeight);

    SDL_SetRenderDrawColor(getScreenRenderer(), backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    setRenderTarget(textBuffer);
    SDL_RenderClear(getScreenRenderer());

    resetRenderTarget();

    return 1;
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

    setRenderTarget(textBuffer);
    copyTextureSegmentToScreen(fontTexture, &fontSrcRect, &destRect);

    if(moveCursor)
        terminalAdvanceCursor(0);
}

void terminalNewLine(char carriageReturn, char scroll) {
    cursorRow++;

    if(carriageReturn)
        cursorCol = 0;

    if(cursorRow >= SCREEN_ROWS) {
        cursorRow--;
        if(scroll) {
            terminalScroll(1);
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
        terminalScroll(1);
        cursorRow--;
    }
}

void terminalScroll(int lines) {
    if(lines > SCREEN_ROWS)
        lines = SCREEN_ROWS;

    /* Rect covering everything that is preserved after the scroll */
    SDL_Rect scrollRect = {
        0,
        lines * CHAR_HEIGHT,
        SCREEN_WIDTH,
        SCREEN_HEIGHT - lines * CHAR_HEIGHT
    };

    SDL_Rect destRect = {
        0,
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT - lines * CHAR_HEIGHT
    };

    SDL_Rect blankRect = {
        0,
        SCREEN_HEIGHT - lines * CHAR_HEIGHT,
        SCREEN_WIDTH,
        lines * CHAR_HEIGHT
    };

    SDL_SetRenderTarget(getScreenRenderer(), textBuffer);
    SDL_RenderCopy(getScreenRenderer(), textBuffer, &scrollRect, &destRect);

    /* Erase scrolled portion of screen */
    SDL_SetRenderDrawColor(getScreenRenderer(), backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(getScreenRenderer(), &blankRect);
}

void terminalClear(char returnCursor) {
    setRenderTarget(textBuffer);
    SDL_SetRenderDrawColor(getScreenRenderer(), backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    clearRenderer();

    if(returnCursor) {
        cursorRow = 0;
        cursorCol = 0;
    }
}

void enableCursor() {
    cursorEnabled = 1;
}

void disableCursor() {
    cursorEnabled = 0;
}

void terminalRefresh() {
    SDL_Rect bufferRect = {
        0,
        0,
        bufferWidth,
        bufferHeight
    };

    setRenderTarget(screenBuffer);
    copyFullTextureToScreen(textBuffer, NULL);

    if(cursorEnabled)
        drawCursor();

    resetRenderTarget();
    copyFullTextureToScreen(screenBuffer, NULL);
    presentRenderer();
}


/* private */
void charCodeToFontCoordinates(char c, int* outRow, int* outCol) {
    *outRow = c / FONT_COLS;
    *outCol = c % FONT_COLS;
}

/* private */
SDL_Palette* createTerminalPalette(SDL_Color backgroundColor, SDL_Color foregroundColor) {
    SDL_Palette* palette = SDL_AllocPalette(256);

    if(!palette) {
        screenSetError("createTerminalPalette", "Could not allocate palette", 1);
        return NULL;
    }

    palette->colors[0] = backgroundColor;
    palette->colors[palette->ncolors - 1] = foregroundColor;

    return palette;
}

/* private */
void drawCursor() {
    SDL_Rect cursorRect = {
        cursorCol * CHAR_WIDTH,
        cursorRow * CHAR_HEIGHT,
        2,
        CHAR_HEIGHT
    };

    SDL_SetRenderDrawColor(getScreenRenderer(), foregroundColor.r, foregroundColor.g, foregroundColor.b, foregroundColor.a);
    SDL_RenderFillRect(getScreenRenderer(), &cursorRect);
}
