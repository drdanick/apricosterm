#include "managedtextures.h"
#include "screen.h"

typedef struct ManagedTexture_ ManagedTexture_;
struct ManagedTexture_ {
    SDL_Texture* texture;
    SDL_Surface* surface;
    ManagedTexture_* next;
    ManagedTexture_* prev;
};
ManagedTexture_* managedTextures = NULL;

/* Private helper functions */
ManagedTexture_* findManagedTexture(SDL_Texture* ptr);
void insertManagedTextureIntoList(ManagedTexture_* newmtex);


SDL_Texture* createManagedTexture(unsigned int width, unsigned int height, SDL_Renderer* renderer, SDL_TextureAccess access) {
    SDL_Texture* texture;

    if(!width || !height) {
        screenSetError("createTexture", "Width and height must be both non-zero", 0);
        return NULL;
    }

    if(!renderer) {
        screenSetError("createTexture", "Renderer cannot be null", 0);
        return NULL;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, access, width, height);
    if(!texture) {
        screenSetError("createTexture", "Could not create SDL texture", 1);
        return NULL;
    }

    /* Manage the texture */
    if(!manageExistingTexture(texture, NULL)) {
        screenSetError("createTexture", (char*)screenGetError(), 0);
        return NULL;
    }

    return texture;
}

SDL_Texture* createManagedTextureFromFile(char* path, SDL_Palette* newPalette, SDL_Renderer* renderer) {
    SDL_Texture* tex = NULL;
    SDL_Surface* surface = NULL;

    surface = IMG_Load(path);

    if(!surface) {
        screenSetError("createManagedTextureFromFile", "Could not load image", 0);
        return NULL;
    }

    if(newPalette) {
        if(SDL_SetSurfacePalette(surface, newPalette) < 0) {
            screenSetError("createManagedTextureFromFile", "Could not apply palette to image", 1);
            return NULL;
        }
    }

    tex = SDL_CreateTextureFromSurface(renderer, surface);

    if(!tex) {
        screenSetError("createManagedTextureFromFile", "Could not create texture from image", 0);
        SDL_FreeSurface(surface);
        return NULL;
    }

    if(!manageExistingTexture(tex, surface)) {
        screenSetError("createManagedTextureFromFile", (char*)screenGetError(), 0);
        SDL_DestroyTexture(tex);
        SDL_FreeSurface(surface);
        return NULL;
    }

    return tex;
}

SDL_Texture* applyPaletteToTexture(SDL_Texture* texture, SDL_Palette* newPalette, SDL_Renderer* renderer) {
    SDL_Texture* newTex;
    ManagedTexture_* mtex = findManagedTexture(texture);


    if(mtex->surface == NULL) {
        screenSetError("applyPaletteToTexture", "Given texture is not palettized.", 0);
        return NULL;
    }

    if(SDL_SetSurfacePalette(mtex->surface, newPalette) < 0) {
        screenSetError("createManagedTextureFromFile", "Could not apply palette to image", 1);
        return NULL;
    }

    newTex = SDL_CreateTextureFromSurface(renderer, mtex->surface);

    if(!newTex) {
        screenSetError("createManagedTextureFromFile", "Could not create texture from image", 0);
        return NULL;
    }

    SDL_DestroyTexture(mtex->texture);
    mtex->texture = newTex;

    return newTex;
}

int manageExistingTexture(SDL_Texture* texture, SDL_Surface* surface) {
    ManagedTexture_* newmtex;

    if(findManagedTexture(texture)) {
        screenSetError("manageExistingTexture", "Texture is already managed", 0);
        return 0;
    }

    newmtex = malloc(sizeof(ManagedTexture_));
    if(!newmtex) {
        screenSetError("manageExistingTexture", "Could not allocate texture metadata", 0);
        return 0;
    }

    newmtex->texture = texture;
    newmtex->surface = surface;
    newmtex->next = NULL;
    newmtex->prev = NULL;

    /* Insert the new texture into the list */
    insertManagedTextureIntoList(newmtex);

    return 1;
}

int destroyManagedTexture(SDL_Texture* ptr) {
    ManagedTexture_* mtex = findManagedTexture(ptr);

    if(!mtex) {
        screenSetError("destroyManagedTexture", "Given texture is not a managed texture", 0);
        return 0;
    }

    SDL_DestroyTexture(mtex->texture);
    SDL_FreeSurface(mtex->surface);
    if(mtex->prev) mtex->prev->next = mtex->next;
    if(mtex->next) mtex->next->prev = mtex->prev;
    if(managedTextures == mtex) managedTextures = mtex->next;
    free(mtex);

    return 1;
}

void destroyAllTextures() {
    while(managedTextures) destroyManagedTexture(managedTextures->texture);
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

/* private */
void insertManagedTextureIntoList(ManagedTexture_* newmtex) {
    if(managedTextures) {
        newmtex->next = managedTextures;
        managedTextures->prev = newmtex;
    }

    managedTextures = newmtex;
}
