#include "apricosterm.h"
#include "screen.h"

#define SDL_IMG_INIT_FLAGS IMG_INIT_PNG

/* Error string buffer */
char errbuff[512];

/* SDL Stuff */
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

unsigned int screenWidth  = -1;
unsigned int screenHeight = -1;

/*========================================================
 * Library debug functions
 *========================================================
 */

const char* screenGetError() {
    return errbuff;
}

void screenSetError(char* functionName, char* error, char showSdlErr) {
    if(showSdlErr)
        sprintf(errbuff, "(%s):: %s: %s", functionName, error, SDL_GetError());
    else
        sprintf(errbuff, "(%s):: %s", functionName, error);
}

/*========================================================
 * GFX management functions
 *========================================================
 */

int initScreen(char* title, unsigned int width, unsigned int height) {
    if(window || renderer) return 0;

    screenWidth = width;
    screenHeight = height;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        screenSetError("initScreen", "Error initializing SDL", 1);
        return 0;
    }

    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );

    if(!(IMG_Init(SDL_IMG_INIT_FLAGS) & SDL_IMG_INIT_FLAGS)) {
        screenSetError("initScreen", "Error initializing SDL image loader", 0);
        return 0;
    }

    window = SDL_CreateWindow(title, 50, 50, width, height, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, APRICOSTERM_RENDERER_TYPE);

    if(!window || !renderer) {
        screenSetError("initScreen", "Could not create SDL window", 1);
        return 0;
    }

    return 1;
}

SDL_Renderer* getScreenRenderer() {
    return renderer;
}

unsigned int getScreenWidth() {
    return screenWidth;
}

unsigned int getScreenHeight() {
    return screenHeight;
}

void copyTextureSegmentToScreen(SDL_Texture* tex, SDL_Rect* srcRect, SDL_Rect* destRect) {
    SDL_RenderCopy(renderer, tex, srcRect, destRect);
}

void copyFullTextureToScreen(SDL_Texture* tex, SDL_Rect* destRect) {
    SDL_RenderCopy(renderer, tex, NULL, destRect);
}

void resetRenderTarget() {
    SDL_SetRenderTarget(renderer, NULL);
}

void setRenderTarget(SDL_Texture* target) {
    SDL_SetRenderTarget(renderer, target);
}

void presentRenderer() {
    SDL_RenderPresent(renderer);
}

void clearRenderer() {
    SDL_RenderClear(renderer);
}

void updateScreen() {
    SDL_UpdateWindowSurface(window);
}

void destroyScreen() {
    if(window && renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
        SDL_DestroyWindow(window);
        window = NULL;

        IMG_Quit();
        SDL_Quit();
    }
}
