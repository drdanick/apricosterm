#ifndef TERM_RENDERER_H
#define TERM_RENDERER_H

#include <SDL2/SDL.h>

int termRendererInit(char* fontPath, SDL_Color bgColor, SDL_Color fgColor);
void terminalPutStr(char* str);
void terminalPutChar(char c, char moveCursor, char autoNewLine);
void terminalNewLine(char carriageReturn, char scroll);
void terminalBackspace(char lineWrap);
void terminalSetRow(int row);
void terminalSetCol(int col);
void terminalAdvanceCursorLeft(char clamp);
void terminalAdvanceCursorRight(char clamp);
void terminalAdvanceCursorUp(char clamp);
void terminalAdvanceCursorDown(char clamp);
void terminalScroll(int lines, char moveCursor);
void terminalClear(char returnCursor);
void terminalSetBackgroundColor(SDL_Color bgColor);
void terminalSetForegroundColor(SDL_Color fgColor);
void terminalEnableCursor();
void terminalDisableCursor();
void terminalRefresh();
void destroyTerminalRenderer();

#endif /* TERM_RENDERER_H */
