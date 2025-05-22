#ifndef SDL_XBM_HELPER_H
#define SDL_XBM_HELPER_H

#include <SDL.h>

// Function to create an SDL surface from XBM data
SDL_Surface* create_surface_from_xbm(const unsigned char* xbm_data, int width,
                                     int height);

SDL_Texture* load_xbm_as_texture(SDL_Renderer* renderer, const unsigned char* image_bits, int width, int height, const unsigned char* mask_bits);

#endif // SDL_XBM_HELPER_H
