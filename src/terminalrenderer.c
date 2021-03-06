#include "apricosterm.h"
#include "terminalrenderer.h"
#include "managedtextures.h"
#include "screen.h"

SDL_Texture* textBuffer = NULL;
SDL_Texture* screenBuffer = NULL;
SDL_Texture* fontTexture = NULL;
SDL_Palette* palette = NULL;
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
void adjustPaletteColors(SDL_Palette* palette, SDL_Color backgroundColor, SDL_Color foregroundColor);
void drawCursor();

int termRendererInit(char* fontPath, SDL_Color bgColor, SDL_Color fgColor) {
    cursorEnabled = 1;
    backgroundColor = bgColor;
    foregroundColor = fgColor;
    cursorRow = 0;
    cursorCol = 0;

    textBuffer = createManagedTexture(getScreenWidth(), getScreenHeight(), getScreenRenderer(), SDL_TEXTUREACCESS_TARGET);
    if(!textBuffer) {
        screenSetError("termRendererInit", (char*)screenGetError(), 0);
        return 0;
    }

    screenBuffer = createManagedTexture(getScreenWidth(), getScreenHeight(), getScreenRenderer(), SDL_TEXTUREACCESS_TARGET);
    if(!screenBuffer) {
        destroyManagedTexture(textBuffer);
        screenSetError("termRendererInit", (char*)screenGetError(), 0);
        return 0;
    }

    palette = createTerminalPalette(bgColor, fgColor);
    if(!palette) {
        destroyManagedTexture(textBuffer);
        screenSetError("termRendererInit", (char*)screenGetError(), 0);
        return 0;
    }

    fontTexture = createManagedTextureFromFile(fontPath, palette, getScreenRenderer());
    if(!fontTexture) {
        SDL_FreePalette(palette);
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
        terminalPutChar(*str, 1, 1);
        str++;
    }
}

void terminalPutChar(char c, char moveCursor, char autoNewLine) {
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
        terminalAdvanceCursorRight(!autoNewLine);
}

void terminalNewLine(char carriageReturn, char scroll) {
    cursorRow++;

    if(carriageReturn)
        cursorCol = 0;

    if(cursorRow >= SCREEN_ROWS) {
        cursorRow--;
        if(scroll) {
            terminalScroll(1, 0);
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

    terminalPutChar(' ', 0, 1);
}

void terminalSetRow(int row) {
    cursorRow = row;
}

void terminalSetCol(int col) {
    cursorCol = col;
}

void terminalAdvanceCursorLeft(char clamp) {
    cursorCol--;
    if(cursorCol < 0) {
        if(clamp)
            cursorCol = 0;
        else
            cursorCol = SCREEN_COLS - 1;
    }
}

void terminalAdvanceCursorRight(char clamp) {
    cursorCol++;
    if(cursorCol >= SCREEN_COLS) {
        if(clamp) {
            cursorCol = SCREEN_COLS - 1;
        } else {
            cursorCol = 0;
            cursorRow++;
        }
    }

    if(cursorRow >= SCREEN_ROWS) {
        terminalScroll(1, 0);
        cursorRow--;
    }
}

void terminalAdvanceCursorUp(char clamp) {
    cursorRow--;

    if(cursorRow < 0) {
        if(clamp)
            cursorRow = 0;
        else
            cursorRow = SCREEN_ROWS - 1;
    }
}

void terminalAdvanceCursorDown(char clamp) {
    cursorRow++;

    if(cursorRow >= SCREEN_ROWS) {
        if(clamp)
            cursorRow = SCREEN_ROWS - 1;
        else
            cursorRow = 0;
    }
}

void terminalScroll(int lines, char moveCursor) {
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

    while(moveCursor && lines > 0) {
        terminalAdvanceCursorUp(1);
        lines--;
    }
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

void terminalSetBackgroundColor(SDL_Color bgColor) {
    backgroundColor = bgColor;
    adjustPaletteColors(palette, backgroundColor, foregroundColor);
    fontTexture = applyPaletteToTexture(fontTexture, palette, getScreenRenderer());
}

void terminalSetForegroundColor(SDL_Color fgColor) {
    foregroundColor = fgColor;
    adjustPaletteColors(palette, backgroundColor, foregroundColor);
    fontTexture = applyPaletteToTexture(fontTexture, palette, getScreenRenderer());
}

void terminalEnableCursor() {
    cursorEnabled = 1;
}

void terminalDisableCursor() {
    cursorEnabled = 0;
}

void terminalRefresh() {
    setRenderTarget(screenBuffer);
    copyFullTextureToScreen(textBuffer, NULL);

    if(cursorEnabled)
        drawCursor();

    resetRenderTarget();
    copyFullTextureToScreen(screenBuffer, NULL);
    presentRenderer();
}

void destroyTerminalRenderer() {
    SDL_FreePalette(palette);
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
void adjustPaletteColors(SDL_Palette* palette, SDL_Color backgroundColor, SDL_Color foregroundColor) {
    palette->colors[0] = backgroundColor;
    palette->colors[palette->ncolors - 1] = foregroundColor;
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
