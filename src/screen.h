#ifndef SCREEN_H
#define SCREEN_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct {
    char red;
    char green;
    char blue;
} Color;

const char* screenGetError();
void screenSetError(char* functionName, char* error, char showSdlErr);

int initScreen(char* title, unsigned int width, unsigned int height, Color background);
SDL_Renderer* getScreenRenderer();
unsigned int getScreenWidth();
unsigned int getScreenHeight();


void setDrawColor(int r, int g, int b, int a);
void drawLine(int x1, int y1, int x2, int y2);
void fillRect(int x, int y, int w, int h);
void drawRect(int x, int y, int w, int h);
void presentRenderer();
void clearRenderer();
void updateWindow();
void destroyWindow();

#endif /* SCREEN_H */
