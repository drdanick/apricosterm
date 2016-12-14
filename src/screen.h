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

int initScreen(char* title, unsigned int width, unsigned int height);
SDL_Renderer* getScreenRenderer();
unsigned int getScreenWidth();
unsigned int getScreenHeight();


void setDrawColor(int r, int g, int b);
void drawLine(int x1, int y1, int x2, int y2);
void fillRect(int x, int y, int w, int h);
void drawRect(int x, int y, int w, int h);
void copyTextureSegmentToScreen(SDL_Texture* tex, SDL_Rect* srcRect, SDL_Rect* destRect);
void copyFullTextureToScreen(SDL_Texture*, SDL_Rect* destRect);
void resetRenderTarget();
void setRenderTarget(SDL_Texture* target);
void presentRenderer();
void clearRenderer();
void updateScreen();
void destroyScreen();

#endif /* SCREEN_H */
