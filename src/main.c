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

/* Bytes read from the pipe at a time */
#define TERMIO_BUFFER_SIZE 4096
/* How long to sleep when the pipe is idle (ms) */
#define TERMIO_IDLE_WAIT   10
/* Longest time to spend handling pipe data before redrawing (ms) */
#define TERMIO_MAX_BATCH   15

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

    char done = 0;
    char termbuff[TERMIO_BUFFER_SIZE];
    int termcount;
    int i;
    Uint32 batchStart;

    if(settings.fifoFile) {
        enableFilePipe = 1;

        printf("Enabling fifo pipe...\n");
        termioInit(settings.fifoFile);
    }

    initScreen("ApricosTerm", SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_Color bgColor = BACKGROUND_COLOR;
    SDL_Color fgColor = FOREGROUND_COLOR;
    if(!termRendererInit(fontFilePath, bgColor, fgColor)) {
        printf("ERROR: %s\n", screenGetError());
    }

    if(enableFilePipe) {
        terminalDisableCursor();
        SDL_StartTextInput();
    }

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
        if(enableFilePipe) {
            /* Handle everything waiting in the pipe before redrawing, so
             * output never queues up behind the frame rate. Give up after
             * TERMIO_MAX_BATCH ms so that events still get processed. */
            batchStart = SDL_GetTicks();
            termcount = termioRead(termbuff, TERMIO_BUFFER_SIZE);
            if(termcount == 0)
                SDL_Delay(TERMIO_IDLE_WAIT); /* Idle, or the writer has closed the pipe */

            while(termcount > 0) {
                for(i = 0; i < termcount; i++)
                    terminalHandleByte(termbuff[i]);

                if(SDL_GetTicks() - batchStart >= TERMIO_MAX_BATCH)
                    break;

                termcount = termioRead(termbuff, TERMIO_BUFFER_SIZE);
            }
        } else {
            SDL_Delay(1);
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
