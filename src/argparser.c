#include "argparser.h"
#include "apricosterm.h"

#include <stdio.h>

const struct option long_options[] = {
    {"version", no_argument, 0, 'v'},
    {"help", no_argument, 0, 'h'},
    {"font", required_argument, 0, 't'},
    {"fifo_tty_file", required_argument, 0, 'f'},
    {0, 0, 0, 0}
};

const char* option_descriptions[] = {
    "Print version information and exit",
    "Print this message and exit",
    "Set custom bitmap font",
    "Set fifo tty file"
};

void printHelp() {
    int i = 0;
    struct option coption;
    printf("Aprsim usage:\n");
    printf("\tapricosterm [options] [-t file] [-f file]\n");
    printf("Options:\n");
    while(coption = long_options[i],
            !(coption.flag == 0
            && coption.has_arg == 0
            && coption.val == 0
            && coption.name == 0)) {
        printf("\t--%-20s -%-5c %s\n", coption.name, coption.val, option_descriptions[i]);
        i++;
    }
}

Settings getSettingsFromArgs(int argc, char** argv) {
    Settings s;
    int c;

    s.fontFile = NULL;
    s.fifoFile = NULL;

    while(c = getopt_long(argc, argv, "vht:f:", long_options, NULL), c != -1) {
        switch(c) {
            case 'v':
                printf("Apricosterm V%s\n", VERSION);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                printHelp();
                exit(EXIT_SUCCESS);
                break;
            case 't':
                s.fontFile = optarg;
                break;
            case 'f':
                s.fifoFile = optarg;
                break;
            case '?':
            default:
                exit(EXIT_FAILURE);
                break;
        }
    }

    return s;
}
