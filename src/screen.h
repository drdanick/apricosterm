#ifndef SCREEN_H
#define SCREEN_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

const char* screenGetError();
void screenSetError(char* functionName, char* error, char showSdlErr);

int initScreen(char* title, unsigned int width, unsigned int height);
SDL_Renderer* getScreenRenderer();
unsigned int getScreenWidth();
unsigned int getScreenHeight();

void copyTextureSegmentToScreen(SDL_Texture* tex, SDL_Rect* srcRect, SDL_Rect* destRect);
void copyFullTextureToScreen(SDL_Texture*, SDL_Rect* destRect);
void resetRenderTarget();
void setRenderTarget(SDL_Texture* target);
void presentRenderer();
void clearRenderer();
void updateScreen();
void destroyScreen();

#endif /* SCREEN_H */
