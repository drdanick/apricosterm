#ifndef TERM_RENDERER_H
#define TERM_RENDERER_H

#include <SDL2/SDL.h>

void termRendererInit();
void terminalPutStr(char* str);
void terminalPutChar(char c, char moveCursor);
void terminalNewLine(char carriageReturn, char scroll);
void terminalBackspace(char lineWrap);
void terminalSetRow(int row);
void terminalSetCol(int col);
void terminalAdvanceCursor(char clamp);
void terminalRefresh();

#endif /* TERM_RENDERER_H */
