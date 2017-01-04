#ifndef MANAGED_TEXTURES_H
#define MANAGED_TEXTURES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

SDL_Texture* createManagedTexture(unsigned int width, unsigned int height, SDL_Renderer* renderer, SDL_TextureAccess access);
int manageExistingTexture(SDL_Texture* texture, SDL_Surface* surface);
SDL_Texture* createManagedTextureFromFile(char* path, SDL_Palette* newPalette, SDL_Renderer* renderer);

/**
 * Apply a new palette to a texture if it is already palettized (currently only applicable 
 * for textures created from files).
 * If this function is successful, the original texture will be destroyed. Be careful!
 *
 * @returns non-null if the palette swap was successful, null otherwise.
 */
SDL_Texture* applyPaletteToTexture(SDL_Texture* texture, SDL_Palette* newPalette, SDL_Renderer* renderer);

int destroyManagedTexture(SDL_Texture* ptr);
void destroyAllTextures();

#endif /* MANAGED_TEXTURES_H */
