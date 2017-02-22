#include <stdio.h>
#include <stdlib.h>
#include "apricosterm.h"
#include "argparser.h"
#include "screen.h"
#include "terminalrenderer.h"
#include "terminalemulator.h"
#include "managedtextures.h"
#include "serialtermio.h"
#include "resources.h"

char enableFilePipe = 0;

int main(int argc, char** argv) {
    SDL_Event event;
    char* fontFilePath;

    Settings settings = getSettingsFromArgs(argc, argv);

    fontFilePath =
        settings.fontFile ?
        settings.fontFile :
        findResourcePath(FONT_FILE);

    if(!fontFilePath) {
        fprintf(stderr, "ERROR: Could not find font file\n");
        return EXIT_FAILURE;
    }

    initScreen("ApricosTerm", SCREEN_WIDTH, SCREEN_HEIGHT);
    char done = 0;
    int termbyte = -1;

    if(settings.fifoFile)
        enableFilePipe = 1;

    SDL_Color bgColor = BACKGROUND_COLOR;
    SDL_Color fgColor = FOREGROUND_COLOR;
    if(!termRendererInit(fontFilePath, bgColor, fgColor)) {
        printf("ERROR: %s\n", screenGetError());
    }

    if(enableFilePipe) {
        printf("Enabling fifo pipe @ %s\n", settings.fifoFile);
        termioInit(settings.fifoFile);
        terminalDisableCursor();
    }

    if(enableFilePipe)
        SDL_StartTextInput();

    while(!done) {
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN:
                    if(!enableFilePipe) {
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
                    }
                    break;
                case SDL_TEXTINPUT:
                    if(!enableFilePipe)
                        terminalPutStr(event.text.text);
                    break;
                case SDL_QUIT:
                    done = 1;
                    break;
                default:
                    break;
            }
        }
        SDL_Delay(1);
        if(enableFilePipe) {
            termbyte = termioRead();
            if(termbyte > -1)
                terminalHandleByte((char)termbyte);
        }
        terminalRefresh();
    }


    SDL_StopTextInput();
    termioClose();
    destroyTerminalRenderer();
    destroyAllTextures();
    destroyScreen();

    return EXIT_SUCCESS;
}
