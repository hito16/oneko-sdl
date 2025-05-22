#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // for sleep

// note: DanielKO noted XPM is a superset of XBM and should be supported by
// SDL_image directly.   TBD - see if we can remove this

// Include the neko_xbm.h header file
#ifdef BUILD_AS_MAIN
#include "neko_image_states.h"
#include "neko_xbm.h"
#endif

// Function to create an SDL surface from XBM data
SDL_Surface* create_surface_from_xbm(const unsigned char* xbm_data, int width,
                                     int height) {
    // Calculate bytes per row, accounting for padding
    int bytes_per_row = (width + 7) / 8;

    // Create an SDL surface with 8-bit depth (SDL_PIXELFORMAT_INDEX8)
    SDL_Surface* surface =
        SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    if (!surface) {
        fprintf(stderr, "SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
        return NULL;
    }

    // Set the color palette (black and white)
    SDL_Color colors[2];
    colors[0].r = 0;  // Black
    colors[0].g = 0;
    colors[0].b = 0;
    colors[1].r = 255;  // White
    colors[1].g = 255;
    colors[1].b = 255;

    // Create a color palette for the surface.  Important for 8-bit surfaces.
    if (SDL_SetPaletteColors(surface->format->palette, colors, 0, 2) < 0) {
        fprintf(stderr, "SDL_SetPaletteColors failed: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return NULL;
    }

    // Get a pointer to the surface's pixel data
    Uint8* pixels = (Uint8*)surface->pixels;
    if (!pixels) {
        fprintf(stderr, "Could not get surface pixels\n");
        SDL_FreeSurface(surface);
        return NULL;
    }

    // Initialize the surface to black (important for ensuring correct pixels)
    memset(pixels, 0, surface->pitch * height);

    // Copy the XBM data to the SDL surface, handling bit order correctly
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate the byte offset for the current pixel in the XBM data
            int xbm_byte_index = (y * bytes_per_row) + (x / 8);
            // Get the byte containing the pixel from XBM data
            Uint8 xbm_byte = xbm_data[xbm_byte_index];
            // Extract the bit for the current pixel, LSB-first
            Uint8 bit = (xbm_byte >> (x % 8)) & 0x01;

            // Calculate the index into the pixel array.
            size_t pixel_index = y * surface->pitch + x;
            pixels[pixel_index] = bit ? 1 : 0;  // 0 for black, 1 for white
        }
    }
    return surface;
}

SDL_Texture* load_xbm_as_texture(SDL_Renderer* renderer,
                                 const unsigned char* image_bits, int width,
                                 int height, const unsigned char* mask_bits) {
    SDL_Surface* image_surface_8bit = NULL;
    SDL_Surface* mask_surface_8bit = NULL;
    SDL_Surface* rgba_surface = NULL;  // New 32-bit RGBA surface
    SDL_Texture* texture = NULL;

    // Create 8-bit surfaces from image and mask data using the existing helper
    image_surface_8bit = create_surface_from_xbm(image_bits, width, height);
    if (!image_surface_8bit) {
        fprintf(stderr,
                "load_xbm_as_texture: Failed to create 8-bit image surface.\n");
        return NULL;
    }

    mask_surface_8bit = create_surface_from_xbm(mask_bits, width, height);
    if (!mask_surface_8bit) {
        fprintf(stderr,
                "load_xbm_as_texture: Failed to create 8-bit mask surface.\n");
        SDL_FreeSurface(image_surface_8bit);  // Clean up image surface
        return NULL;
    }

    // --- NEW: Create a 32-bit RGBA surface ---
    rgba_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32,
                                                  SDL_PIXELFORMAT_RGBA32);
    if (!rgba_surface) {
        fprintf(
            stderr,
            "load_xbm_as_texture: Failed to create 32-bit RGBA surface: %s\n",
            SDL_GetError());
        SDL_FreeSurface(image_surface_8bit);
        SDL_FreeSurface(mask_surface_8bit);
        return NULL;
    }

    // --- NEW: Manually copy pixel data and set alpha based on the mask ---
    Uint8* src_pixels_img = (Uint8*)image_surface_8bit->pixels;
    Uint8* src_pixels_mask = (Uint8*)mask_surface_8bit->pixels;
    Uint32* dest_pixels_rgba = (Uint32*)rgba_surface->pixels;

    if (!src_pixels_img || !src_pixels_mask || !dest_pixels_rgba) {
        fprintf(
            stderr,
            "load_xbm_as_texture: Failed to get pixel access for surfaces.\n");
        SDL_FreeSurface(image_surface_8bit);
        SDL_FreeSurface(mask_surface_8bit);
        SDL_FreeSurface(rgba_surface);
        return NULL;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            size_t index_8bit = y * image_surface_8bit->pitch + x;
            size_t index_rgba = y * (rgba_surface->pitch / 4) + x;

            Uint8 color_index = src_pixels_img[index_8bit];  // 0 or 1
            Uint8 mask_value = src_pixels_mask[index_8bit];  // 0 or 1

            Uint8 r = (color_index == 1)
                          ? 255
                          : 0;  // White if pixel is 1, Black if 0
            Uint8 g = (color_index == 1) ? 255 : 0;
            Uint8 b = (color_index == 1) ? 255 : 0;

            Uint8 a = (mask_value == 1)
                          ? 255
                          : 0;  // Opaque if mask is 1, Transparent if 0

            // Combine RGBA into a 32-bit pixel value
            dest_pixels_rgba[index_rgba] =
                SDL_MapRGBA(rgba_surface->format, r, g, b, a);
        }
    }

    // --- OLD: SDL_SetColorKey(image_surface_8bit, SDL_TRUE,
    // SDL_MapRGB(image_surface_8bit->format, 0, 0, 0)); --- This line was
    // removed and replaced by the manual pixel manipulation above.

    // --- Create texture from the 32-bit RGBA surface ---
    texture = SDL_CreateTextureFromSurface(renderer, rgba_surface);
    if (!texture) {
        fprintf(stderr,
                "load_xbm_as_texture: SDL_CreateTextureFromSurface failed for "
                "RGBA surface: %s\n",
                SDL_GetError());
    } else {
        // --- NEW: Enable blending for the texture ---
        if (SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND) < 0) {
            fprintf(stderr,
                    "load_xbm_as_texture: SDL_SetTextureBlendMode failed: %s\n",
                    SDL_GetError());
        }
    }

    // Clean up temporary surfaces
    SDL_FreeSurface(image_surface_8bit);
    SDL_FreeSurface(mask_surface_8bit);
    SDL_FreeSurface(rgba_surface);  // Free the 32-bit surface

    return texture;
}

/*
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow(
        "XBM Image Cycle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        640,  // Fixed window size
        480, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Surface* image_surface = NULL;
    SDL_Texture* texture = NULL;
    int current_image_index = 0;  // Add this line

    int num_images = sizeof(xbm_images) / sizeof(xbm_images[0]);
    // Main loop to display the images
    int quit = 0;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
        }

        // Clear the renderer
        SDL_RenderClear(renderer);

        // Create and display the current image
        const unsigned char* xbm_data = xbm_images[current_image_index].bits;
        int width = xbm_images[current_image_index].width;
        int height = xbm_images[current_image_index].height;

        // Create a surface from the XBM data
        if (image_surface) {
            SDL_FreeSurface(image_surface);
        }
        image_surface = create_surface_from_xbm(xbm_data, width, height);
        if (!image_surface) {
            fprintf(stderr, "create_surface_from_xbm failed\n");
            // create_surface_from_xbm already prints the error.
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        // Create a texture from the surface
        if (texture) {
            SDL_DestroyTexture(texture);
        }
        texture = SDL_CreateTextureFromSurface(renderer, image_surface);
        if (!texture) {
            fprintf(stderr, "SDL_CreateTextureFromSurface failed: %s\n",
                    SDL_GetError());
            SDL_FreeSurface(image_surface);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
        // Define the destination rectangle to center the image.
        SDL_Rect dest_rect;
        dest_rect.x = (640 - width) / 2;   // Center horizontally
        dest_rect.y = (480 - height) / 2;  // Center vertically
        dest_rect.w = width;
        dest_rect.h = height;

        // Copy the texture to the renderer
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);

        // Update the screen
        SDL_RenderPresent(renderer);

        // Increment the image index and wrap around
        current_image_index = (current_image_index + 1) % num_images;

        // Wait for 2 seconds
        sleep(2);
    }

    // Clean up
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_FreeSurface(image_surface);
    SDL_Quit();

    return 0;
}
*/
