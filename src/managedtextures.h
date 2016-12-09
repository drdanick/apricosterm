#ifndef MANAGED_TEXTURES_H
#define MANAGED_TEXTURES_H

#include <SDL2/SDL.h>

SDL_Texture* createTexture(unsigned int width, unsigned int height, SDL_Renderer* renderer, SDL_TextureAccess access);

int destroyTexture(SDL_Texture* ptr);

void destroyAllTextures();

#endif /* MANAGED_TEXTURES_H */
