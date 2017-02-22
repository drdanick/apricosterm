#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include "resources.h"
#include "apricosterm.h"


const char* searchPaths[] = {
    ".",
    RESOURCE_DIR,
    NULL
};

char pathBuffer[1024];

char* findResourcePath(char* resourceName) {
    int i = 0;
    const char* path;

    while(path = searchPaths[i++], path != NULL) {
        sprintf(pathBuffer, "%s/%s", path, resourceName);
        if(access(pathBuffer, F_OK) != -1 ) {
            char* path = (char*)malloc(sizeof(char) * strlen(pathBuffer) + 1);
            strcpy(path, pathBuffer);

            return path;
        }
    }

    return NULL;
}
