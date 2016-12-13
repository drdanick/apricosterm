#include "apricosterm.h"
#include "screen.h"

#define SDL_IMG_INIT_FLAGS IMG_INIT_PNG

/* Error string buffer */
char errbuff[512];

/* SDL Stuff */
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Color backgroundColor;

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

int initScreen(char* title, unsigned int width, unsigned int height, Color background) {
    if(window || renderer) return 0;

    screenWidth = width;
    screenHeight = height;
    backgroundColor = background;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        screenSetError("initScreen", "Error initializing SDL", 1);
        return 0;
    }

    if(!(IMG_Init(SDL_IMG_INIT_FLAGS) & SDL_IMG_INIT_FLAGS)) {
        screenSetError("initScreen", "Error initializing SDL image loader", 0);
        return 0;
    }

    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_SHOWN, &window, &renderer);

    if(!window || !renderer) {
        screenSetError("initScreen", "Could not create SDL window", 1);
        return 0;
    }

    SDL_SetWindowTitle(window, title);

    setDrawColor(255,255,255, 255);
    SDL_RenderClear(renderer);
    fillRect(0,0,width,height);
    SDL_RenderPresent(renderer);

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

/*========================================================
 * Drawing functions
 *========================================================
 */

void setDrawColor(int r, int g, int b, int a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void drawLine(int x1, int y1, int x2, int y2) {
    int xOffset = 0;
    int yOffset = 0;

    if (x2 - x1 > 0) xOffset = -1;
    if (y2 - y1 > 0) yOffset = -1;

    SDL_RenderDrawLine(renderer, x1, y1, x2 + xOffset, y2 + yOffset);
}

void fillRect(int x, int y, int w, int h) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_RenderFillRect(renderer, &rect);
}

void drawRect(int x, int y, int w, int h) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_RenderDrawRect(renderer, &rect);
}

void presentRenderer() {
    SDL_RenderPresent(renderer);
}

void clearRenderer() {
    SDL_RenderClear(renderer);
}

void updateWindow() {
    SDL_UpdateWindowSurface(window);
}

void destroyWindow() {
    if(window && renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
        SDL_DestroyWindow(window);
        window = NULL;

        IMG_Quit();
        SDL_Quit();
    }
}
