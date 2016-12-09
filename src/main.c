#include <stdio.h>
#include <stdlib.h>
#include "apricosterm.h"
#include "screen.h"

int main(int argc, char** argv) {
    Color c = {255,255,255};
    SDL_Event event;
    initScreen("Potato", 640, 480, c);
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
    
    return EXIT_SUCCESS;
}
