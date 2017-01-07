#include <stdio.h>
#include <stdlib.h>
#include "apricosterm.h"
#include "screen.h"
#include "terminalrenderer.h"
#include "terminalemulator.h"
#include "managedtextures.h"

int main(int argc, char** argv) {
    SDL_Event event;
    initScreen("ApricosTerm", SCREEN_WIDTH, SCREEN_HEIGHT);
    char done = 0;
    SDL_Color bgColor = BACKGROUND_COLOR;
    SDL_Color fgColor = FOREGROUND_COLOR;
    if(!termRendererInit(bgColor, fgColor)) {
        printf("ERROR: %s\n", screenGetError());
    }

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
    destroyTerminalRenderer();
    destroyAllTextures();
    destroyScreen();

    return EXIT_SUCCESS;
}
