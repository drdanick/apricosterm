#include "managedtextures.h"
#include "screen.h"

typedef struct ManagedTexture_ ManagedTexture_;
struct ManagedTexture_ {
    SDL_Texture* texture;
    ManagedTexture_* next;
    ManagedTexture_* prev;
};
ManagedTexture_* managedTextures = NULL;

SDL_Texture* createTexture(unsigned int width, unsigned int height, SDL_Renderer* renderer, SDL_TextureAccess access) {
    ManagedTexture_* newmtex;

    if(!width || !height) {
        screenSetError("createTexture", "Width and height must be both non-zero", 0);
        return NULL;
    }

    if(!renderer) {
        screenSetError("createTexture", "Renderer cannot be null", 0);
        return NULL;
    }

    newmtex = malloc(sizeof(ManagedTexture_));
    if(!newmtex) {
        screenSetError("createTexture", "Could not allocate texture metadata", 0);
        return NULL;
    }

    newmtex->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, access, width, height);
    if(!(newmtex->texture)) {
        free(newmtex);
        screenSetError("createTexture", "Could not create SDL texture", 1);
        return NULL;
    }

    /* Insert the new texture into the list */
    if(managedTextures) {
        newmtex->next = managedTextures;
        managedTextures->prev = newmtex;
    }
    managedTextures = newmtex;


    return newmtex->texture;
}

/* private */
ManagedTexture_* findManagedTexture(SDL_Texture* ptr) {
    ManagedTexture_* current = managedTextures;

    while(current) {
        if(current->texture == ptr)
            return current;
        current = current->next;
    }

    return NULL;
}

int destroyTexture(SDL_Texture* ptr) {
    ManagedTexture_* mtex = findManagedTexture(ptr);

    if(!mtex) {
        screenSetError("destroyTexture", "Given texture is not a managed texture", 0);
        return 0;
    }

    SDL_DestroyTexture(mtex->texture);
    if(mtex->prev) mtex->prev->next = mtex->next;
    if(mtex->next) mtex->next->prev = mtex->prev;
    if(managedTextures == mtex) managedTextures = mtex->next;
    free(mtex);

    return 1;
}

void destroyAllTextures() {
    while(managedTextures) destroyTexture(managedTextures->texture);
}
