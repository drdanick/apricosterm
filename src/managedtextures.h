#ifndef MANAGED_TEXTURES_H
#define MANAGED_TEXTURES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

SDL_Texture* createManagedTexture(unsigned int width, unsigned int height, SDL_Renderer* renderer, SDL_TextureAccess access);
int manageExistingTexture(SDL_Texture* texture);
SDL_Texture* createManagedTextureFromFile(char* path, SDL_Renderer* renderer);

int destroyManagedTexture(SDL_Texture* ptr);
void destroyAllTextures();

#endif /* MANAGED_TEXTURES_H */
