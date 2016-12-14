#include <stdio.h>
#include <stdlib.h>
#include "apricosterm.h"
#include "screen.h"
#include "terminalrenderer.h"
#include "managedtextures.h"

int main(int argc, char** argv) {
    SDL_Event event;
    initScreen("Potato", SCREEN_WIDTH, SCREEN_HEIGHT);
    char done = 0;
    termRendererInit();

    SDL_StartTextInput();
    while(!done) {
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            done = 1;
                            break;
                        case SDLK_RETURN:
                            terminalNewLine(1, 1);
                            break;
                        case SDLK_BACKSPACE:
                            terminalBackspace(1);
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_TEXTINPUT:
                    terminalPutStr(event.text.text);
                    break;
                case SDL_QUIT:
                    done = 1;
                    break;
                default:
                    break;
            }
        }
        SDL_Delay(10);
        terminalRefresh();
    }

    
    SDL_StopTextInput();
    destroyAllTextures();
    destroyScreen();
    
    return EXIT_SUCCESS;
}
