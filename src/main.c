#include <stdio.h>
#include <stdlib.h>
#include "apricosterm.h"
#include "screen.h"

int main(int argc, char** argv) {
    SDL_Event event;
    initScreen("Potato", SCREEN_WIDTH, SCREEN_HEIGHT);
    char done = 0;

    while(!done) {
        while(SDL_PollEvent(&event)) {
            switch(event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    done = 1;
                    break;
            }
        }
        SDL_Delay(100);
        updateWindow();
    }
    destroyScreen();
    
    return EXIT_SUCCESS;
}
